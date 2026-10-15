/*
 * Copyright © Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cec_output.h"

#include <mir/log.h>

#include <utility>

namespace { constexpr char const* log_component = "frame-cec"; }

CecOutput::CecOutput(mir::graphics::DisplayConfigurationOutputId output_id, std::unique_ptr<CecAdapter> adapter) :
    id{output_id},
    adapter{std::move(adapter)},
    worker{[this] { run(); }}
{
    mir::log(mir::logging::Severity::debug, log_component, "CEC worker created for output %d", id.as_value());
}

CecOutput::~CecOutput() { shutdown(); }

void CecOutput::request_power(bool on)
{
    {
        auto locked = state.lock();
        if (locked->stopping)
            return;
        locked->desired_power = on;
        locked->work_pending = true;
    }
    mir::log(
        mir::logging::Severity::debug,
        log_component,
        "Queued CEC power state %s for output %d",
        on ? "on" : "off",
        id.as_value());
    wake_worker.notify_one();
}

void CecOutput::shutdown()
{
    std::lock_guard shutdown_lock{shutdown_mutex};
    if (adapter_closed)
        return;

    mir::log(mir::logging::Severity::informational, log_component, "Shutting down CEC output %d", id.as_value());

    {
        auto locked = state.lock();
        locked->stopping = true;
    }
    wake_worker.notify_one();
    if (worker.joinable())
        worker.join();
    adapter->close();
    adapter_closed = true;
}

void CecOutput::run()
{
    // Process queued power changes on one worker, then reconcile with the latest request.
    for (;;)
    {
        bool desired{};
        {
            auto locked = state.lock();
            locked.wait(wake_worker, [&] { return locked->stopping || locked->work_pending; });

            if (locked->stopping)
            {
                locked.drop();
                adapter->send_standby_tv();
                return;
            }

            desired = locked->desired_power.value_or(false);
            locked->work_pending = false;
            locked.drop();
        }

        apply_power_state(desired);

        auto locked = state.lock();
        if (locked->desired_power != desired)
            locked->work_pending = true;
    }
}

void CecOutput::apply_power_state(bool on)
{
    if (on)
    {
        adapter->send_power_on_tv();
        adapter->send_make_active_source();
    }
    else
    {
        adapter->send_standby_tv();
    }
}

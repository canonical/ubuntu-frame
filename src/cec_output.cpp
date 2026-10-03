#include "cec_output.h"

#include <mir/log.h>

#include <utility>

namespace
{
constexpr char const* log_component = "frame-cec";
}

CecOutput::CecOutput(
    mir::graphics::DisplayConfigurationOutputId output_id,
    std::unique_ptr<CecAdapter> adapter)
    : id{output_id},
      adapter{std::move(adapter)},
      worker{[this] { run(); }}
{
    mir::log(mir::logging::Severity::debug, log_component,
             "CEC worker created for output %d", id.as_value());
}

CecOutput::~CecOutput()
{
    shutdown();
}

auto CecOutput::output_id() const -> mir::graphics::DisplayConfigurationOutputId
{
    return id;
}

auto CecOutput::adapter_port() const -> std::string_view
{
    return adapter->port();
}

void CecOutput::request_power(bool on)
{
    {
        auto locked = state.lock();
        if (locked->stopping)
            return;
        locked->desired_power = on;
        locked->work_pending = true;
    }
    mir::log(mir::logging::Severity::debug, log_component,
             "Queued CEC power state %s for output %d",
             on ? "on" : "off", id.as_value());
    wake_worker.notify_one();
}

void CecOutput::shutdown()
{
    std::lock_guard shutdown_lock{shutdown_mutex};
    if (adapter_closed)
        return;

    mir::log(mir::logging::Severity::informational, log_component,
             "Shutting down CEC output %d", id.as_value());

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
    for (;;)
    {
        bool desired{};
        {
            auto locked = state.lock();
            locked.wait(wake_worker, [&]
            {
                return locked->stopping || locked->work_pending;
            });

            if (locked->stopping)
            {
                locked.drop();
                adapter->standby_tv();
                return;
            }

            desired = locked->desired_power.value_or(false);
            locked->work_pending = false;
            if (locked->last_commanded_power == desired)
            {
                mir::log(mir::logging::Severity::debug, log_component,
                         "CEC output %d already has requested state %s",
                         id.as_value(), desired ? "on" : "off");
                continue;
            }

            locked.drop();
        }

        apply_power_state(desired);

        auto locked = state.lock();
        locked->last_commanded_power = desired;
        if (locked->desired_power != desired)
            locked->work_pending = true;
    }
}

void CecOutput::apply_power_state(bool on)
{
    if (on)
    {
        adapter->power_on_tv();
        adapter->make_active_source();
    }
    else
    {
        adapter->standby_tv();
    }
}

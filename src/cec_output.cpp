#include "cec_output.h"

#include <utility>

CecOutput::CecOutput(
    mir::graphics::DisplayConfigurationOutputId output_id,
    std::unique_ptr<CecAdapter> adapter)
    : id{output_id},
      adapter{std::move(adapter)},
      worker{[this] { run(); }}
{
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
    wake_worker.notify_one();
}

void CecOutput::shutdown()
{
    std::lock_guard shutdown_lock{shutdown_mutex};
    if (adapter_closed)
        return;

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
                continue;

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

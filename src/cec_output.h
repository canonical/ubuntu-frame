#ifndef FRAME_CEC_OUTPUT_H
#define FRAME_CEC_OUTPUT_H

#include "cec_adapter.h"

#include <mir/graphics/display_configuration.h>
#include <mir/synchronised.h>

#include <condition_variable>
#include <memory>
#include <optional>
#include <thread>

class CecOutput
{
public:
    CecOutput(
        mir::graphics::DisplayConfigurationOutputId output_id,
        std::unique_ptr<CecAdapter> adapter);
    ~CecOutput();

    CecOutput(CecOutput const&) = delete;
    auto operator=(CecOutput const&) -> CecOutput& = delete;

    auto output_id() const -> mir::graphics::DisplayConfigurationOutputId;
    auto adapter_port() const -> std::string_view;

    void request_power(bool on);
    void shutdown();

private:
    struct State
    {
        std::optional<bool> desired_power;
        bool work_pending{false};
        bool stopping{false};
    };

    void run();
    void apply_power_state(bool on);

    mir::graphics::DisplayConfigurationOutputId const id;
    std::unique_ptr<CecAdapter> const adapter;
    mir::Synchronised<State> state{State{}};
    std::condition_variable wake_worker;
    std::mutex shutdown_mutex;
    bool adapter_closed{false};
    std::thread worker;
};

#endif // FRAME_CEC_OUTPUT_H

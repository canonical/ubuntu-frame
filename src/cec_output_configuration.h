#ifndef FRAME_CEC_OUTPUT_CONFIGURATION_H
#define FRAME_CEC_OUTPUT_CONFIGURATION_H

#include "cec_manager.h"

#include <miral/output_configuration.h>

#include <memory>
#include <optional>
#include <span>
#include <vector>

class CecOutputConfiguration final : public miral::OutputConfiguration::Strategy
{
public:
    CecOutputConfiguration() = default;
    explicit CecOutputConfiguration(std::shared_ptr<CecManager> manager);

    void set_manager(std::shared_ptr<CecManager> manager);

    void apply_configuration(std::span<mir::graphics::UserDisplayConfigurationOutput> outputs) override;
    void confirm_configuration(std::span<mir::graphics::DisplayConfigurationOutput const> outputs) override;

private:
    struct State
    {
        std::shared_ptr<CecManager> manager;
        std::optional<std::vector<mir::graphics::DisplayConfigurationOutput>> confirmed_outputs;
    };

    mir::Synchronised<State> state{State{}};
};

#endif // FRAME_CEC_OUTPUT_CONFIGURATION_H

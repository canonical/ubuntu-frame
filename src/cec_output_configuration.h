#ifndef FRAME_CEC_OUTPUT_CONFIGURATION_H
#define FRAME_CEC_OUTPUT_CONFIGURATION_H

#include "cec_manager.h"

#include <miral/output_configuration.h>

#include <memory>
#include <span>

class CecOutputConfiguration final : public miral::OutputConfiguration::Strategy
{
public:
    explicit CecOutputConfiguration(std::shared_ptr<CecManager> manager);

    void apply_configuration(
        std::span<mir::graphics::UserDisplayConfigurationOutput> outputs) override;
    void confirm_configuration(
        std::span<mir::graphics::DisplayConfigurationOutput const> outputs) override;

private:
    std::shared_ptr<CecManager> const manager;
};

#endif // FRAME_CEC_OUTPUT_CONFIGURATION_H

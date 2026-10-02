#include "cec_output_configuration.h"

#include <utility>

CecOutputConfiguration::CecOutputConfiguration(std::shared_ptr<CecManager> manager)
    : manager{std::move(manager)}
{
}

void CecOutputConfiguration::apply_configuration(
    std::span<mir::graphics::UserDisplayConfigurationOutput> outputs)
{
    (void)outputs;
}

void CecOutputConfiguration::confirm_configuration(
    std::span<mir::graphics::DisplayConfigurationOutput const> outputs)
{
    manager->configuration_confirmed(outputs);
}

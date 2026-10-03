#include "cec_output_configuration.h"

#include <mir/log.h>

#include <utility>

namespace
{
constexpr char const* log_component = "frame-cec";
}

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
    mir::log(mir::logging::Severity::debug, log_component,
             "Forwarding %zu confirmed output(s) to CEC manager", outputs.size());
    manager->configuration_confirmed(outputs);
}

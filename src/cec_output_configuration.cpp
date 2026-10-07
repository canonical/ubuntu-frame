#include "cec_output_configuration.h"

#include <mir/log.h>

#include <utility>

namespace { constexpr char const* log_component = "frame-cec"; }

CecOutputConfiguration::CecOutputConfiguration(std::shared_ptr<CecManager> manager) { set_manager(std::move(manager)); }

void CecOutputConfiguration::set_manager(std::shared_ptr<CecManager> manager)
{
    auto locked = state.lock();
    locked->manager = std::move(manager);
    auto const& confirmed_outputs = locked->confirmed_outputs;
    if (locked->manager && confirmed_outputs)
        locked->manager->configuration_confirmed(confirmed_outputs.value());
}

void CecOutputConfiguration::apply_configuration(std::span<mir::graphics::UserDisplayConfigurationOutput> outputs)
{
    (void)outputs;
}

void CecOutputConfiguration::confirm_configuration(std::span<mir::graphics::DisplayConfigurationOutput const> outputs)
{
    auto locked = state.lock();
    locked->confirmed_outputs.emplace(outputs.begin(), outputs.end());
    if (!locked->manager)
        return;

    mir::log(
        mir::logging::Severity::debug,
        log_component,
        "Forwarding %zu confirmed output(s) to CEC manager",
        outputs.size());
    locked->manager->configuration_confirmed(outputs);
}

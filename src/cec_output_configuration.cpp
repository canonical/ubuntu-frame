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

#include "cec_output_configuration.h"

#include <mir/log.h>

#include <utility>

namespace { constexpr char const* log_component = "frame-cec"; }

CecOutputConfiguration::CecOutputConfiguration(std::shared_ptr<CecManager> manager)
    : manager{std::move(manager)}
{
}

void CecOutputConfiguration::apply_configuration(std::span<mir::graphics::UserDisplayConfigurationOutput> outputs)
{
    // CEC commands are sent only after confirmation; applying user settings stays untouched.
    (void)outputs;
}

void CecOutputConfiguration::confirm_configuration(std::span<mir::graphics::DisplayConfigurationOutput const> outputs)
{
    mir::log(mir::logging::Severity::debug, log_component,
             "Forwarding %zu confirmed output(s) to CEC manager", outputs.size());
    manager->configuration_confirmed(outputs);
}

/*
 * Copyright © Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 or 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "display_configuration_builder.h"
#include "frame_window_manager.h"
#include "layout_metadata.h"

#include <miral/version.h>

auto build_display_configuration(miral::MirRunner const& runner)
    -> miral::DisplayConfiguration
{
    miral::DisplayConfiguration display_config{runner};
    display_config.add_output_attribute(FrameWindowManagerPolicy::surface_title);
    display_config.add_output_attribute(FrameWindowManagerPolicy::snap_name);

#if MIRAL_VERSION >= MIR_VERSION_NUMBER(5, 3, 0)
    display_config.layout_userdata_builder("applications", [](miral::DisplayConfiguration::Node const& node) -> std::any
    {
        return std::make_shared<LayoutMetadata>(node);
    });
#endif
    return display_config;
}
/*
 * Copyright © 2016-2021 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
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
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#include "kiosk_window_manager.h"

#include <miral/command_line_option.h>
#include <miral/display_configuration.h>
#include <miral/external_client.h>
#include <miral/keymap.h>
#include <miral/runner.h>
#include <miral/set_window_management_policy.h>
#include <miral/wayland_extensions.h>

using namespace std::string_literals;

namespace
{
auto default_swaybg_launch_path()
{
    if (auto const snap = getenv("SNAP"))
        return snap + "/bin/swaybg.launcher"s;
    else
        return "scripts/bin/swaybg.launcher"s;
    };
}

int main(int argc, char const* argv[])
{
    using namespace miral;

    MirRunner runner{argc, argv};

    DisplayConfiguration display_config{runner};
    WaylandExtensions wayland_extensions;

    for (auto const& extension : {
        WaylandExtensions::zwlr_layer_shell_v1, // For swaybg
        "zwp_pointer_constraints_v1",           // Useful for games
        "zwp_relative_pointer_manager_v1"})     // Useful for games
    {
        wayland_extensions.enable(extension);
    }

    ExternalClientLauncher launcher;

    auto background = "#1f1f1f"s;

    auto startup_background = [&](std::string const& swaybg_launch_path)
        {
            launcher.launch({swaybg_launch_path, "-c", background});
        };

    return runner.run_with(
        {
            wayland_extensions,
            display_config,
            display_config.layout_option(),
            set_window_management_policy<KioskWindowManagerPolicy>(),
            launcher,
            CommandLineOption{[&](auto& option) { background = option; }, "bgcolor", "RGB color of background", background},
            CommandLineOption{startup_background, "bglaunch", "Path to background launch script", default_swaybg_launch_path()},
            Keymap{}
        });
}

/*
 * Copyright © 2016-2020 Canonical Ltd.
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

#include "frame_window_manager.h"
#include "egwallpaper.h"

#include <miral/command_line_option.h>
#include <miral/display_configuration.h>
#include <miral/internal_client.h>
#include <miral/keymap.h>
#include <miral/runner.h>
#include <miral/set_window_management_policy.h>
#include <miral/wayland_extensions.h>

int main(int argc, char const* argv[])
{
    using namespace miral;
    MirRunner runner{argc, argv};

    DisplayConfiguration display_config{runner};

    std::mutex privileged_pids_mutex;
    std::set<pid_t> privileged_pids;

    std::set<std::string> const privileged_protocols{
        WaylandExtensions::zwlr_layer_shell_v1,
        WaylandExtensions::zwp_virtual_keyboard_v1,
        WaylandExtensions::zwp_input_method_v2};
    WaylandExtensions wayland_extensions;
    for (auto const& protocol : privileged_protocols)
    {
        wayland_extensions.enable(protocol);
    }
    wayland_extensions.set_filter([&](Application const& app, char const* protocol) -> bool
        {
            std::lock_guard<std::mutex> lock{privileged_pids_mutex};
            return (
                privileged_protocols.find(protocol) == privileged_protocols.end() ||
                privileged_pids.find(pid_of(app)) != privileged_pids.end());
        });

    egmde::Wallpaper wallpaper;
    runner.add_stop_callback([&] { wallpaper.stop(); });

    return runner.run_with(
        {
            wayland_extensions,
            display_config,
            display_config.layout_option(),
            CommandLineOption{[&](auto& option) { wallpaper.top(option);},
                              "wallpaper-top",    "Colour of wallpaper RGB", "0x7f7f7f"},
            CommandLineOption{[&](auto& option) { wallpaper.bottom(option);},
                              "wallpaper-bottom", "Colour of wallpaper RGB", "0x1f1f1f"},
            StartupInternalClient{std::ref(wallpaper)},
            set_window_management_policy<FrameWindowManagerPolicy>(),
            Keymap{}
        });
}

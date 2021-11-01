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

#include "snap_name_of.h"
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
    WaylandExtensions wayland_extensions;

    std::set<std::string> const osk_protocols{
        WaylandExtensions::zwlr_layer_shell_v1,
        WaylandExtensions::zwp_virtual_keyboard_v1,
        WaylandExtensions::zwp_input_method_v2};

    for (auto const& protocol : osk_protocols)
    {
        wayland_extensions.enable(protocol);
    }

    std::set<std::string> const osk_snaps{
        "ubuntu-frame-osk"};

    wayland_extensions.set_filter([&](Application const& app, char const* protocol) -> bool
        {
            if (osk_protocols.find(protocol) != osk_protocols.end())
            {
                auto const snap_name = snap_name_of(app);
                return osk_snaps.find(snap_name) != osk_snaps.end();
            }
            else
            {
                return true;
            }
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

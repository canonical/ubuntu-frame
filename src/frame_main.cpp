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

#include "frame_authorization.h"
#include "frame_window_manager.h"
#include "egwallpaper.h"

#include <miral/command_line_option.h>
#include <miral/display_configuration.h>
#include <miral/internal_client.h>
#include <miral/keymap.h>
#include <miral/runner.h>
#include <miral/set_window_management_policy.h>
#include <miral/wayland_extensions.h>

#include <filesystem>

auto display_link_path(miral::MirRunner& runner) -> std::string
{
    return "/tmp/frame-display-" + runner.wayland_display().value();
}

auto display_path(miral::MirRunner& runner) -> std::string
{
    return std::string{getenv("XDG_RUNTIME_DIR")} + "/" + runner.wayland_display().value();
}

int main(int argc, char const* argv[])
{
    using namespace miral;
    MirRunner runner{argc, argv};

    DisplayConfiguration display_config{runner};
    WaylandExtensions wayland_extensions;
    init_authorization(wayland_extensions);

    // Symlinking frame's display from our private /tmp allows clients from this snap to choose the
    // correct display if, for example, we're running nested under another Wayland compositor.
    runner.add_start_callback([&]
        {
            auto const target = display_path(runner);
            auto const link = display_link_path(runner);
            std::filesystem::create_symlink(target, link);
        });
    runner.add_stop_callback([&]
        {
            remove(display_link_path(runner).c_str());
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

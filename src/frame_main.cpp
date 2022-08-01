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
#include "startup_client.h"

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
    init_authorization(wayland_extensions, auth_model);

    StartupClient startup_client;

    // Add file discriptor to main loop to watch inotify
    // "You can do register_fd_handler"
    // You can get access to the_main_loop 

    // Add something here to get run() and that will get the main loop, with which to run inotify on
    // "You want to end up with a top-level function that takes a server reference and sets up the file watching on that"
    // "...and then returns"

    runner.add_stop_callback([&] { startup_client.stop(); });
    
    return runner.run_with(
        {
            wayland_extensions,
            display_config,
            display_config.layout_option(),
            CommandLineOption{[&](auto& option) { startup_client.set_wallpaper_top_colour(option);},
                              "wallpaper-top",    "Colour of wallpaper RGB", "0x7f7f7f"},
            CommandLineOption{[&](auto& option) { startup_client.set_wallpaper_bottom_colour(option);},
                              "wallpaper-bottom", "Colour of wallpaper RGB", "0x1f1f1f"},
            CommandLineOption{[&](auto& option) { startup_client.set_crash_background_colour(option);},
                              "crash_background", "Colour of crash screen background RGB", "0x380c24"},
            CommandLineOption{[&](auto& option) {startup_client.set_crash_text_colour(option);},
                              "crash_text",       "Colour of crash screen text RGB", "0xffffff"},
            CommandLineOption{[&](auto& option) { startup_client.set_log_location(option);},
                              "log_location",      "Location of the log file", "/home/graysonguarino/Documents/log/log.txt"},
            StartupInternalClient{std::ref(startup_client)},
            set_window_management_policy<FrameWindowManagerPolicy>(),
            Keymap{}
        });
}

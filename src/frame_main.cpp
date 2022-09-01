/*
 * Copyright © 2016-2022 Canonical Ltd.
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

#include <miral/configuration_option.h>
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

    egmde::Wallpaper wallpaper;
    runner.add_stop_callback([&] { wallpaper.stop(); });

    return runner.run_with(
        {
            wayland_extensions,
            display_config,
            display_config.layout_option(),
            ConfigurationOption{[&](auto& option) { wallpaper.top(option); },
                "wallpaper-top", "Colour of wallpaper RGB", "0x7f7f7f"},
            ConfigurationOption{[&](auto& option) { wallpaper.bottom(option); },
                "wallpaper-bottom", "Colour of wallpaper RGB", "0x1f1f1f"},
            ConfigurationOption{[&](bool option) { init_authorise_without_apparmor(option); },
                "authorise-without-apparmor", "Use /proc/<pid>/cmdline if AppArmor is unavailable", false },
            StartupInternalClient{std::ref(wallpaper)},
            set_window_management_policy<FrameWindowManagerPolicy>(),
            Keymap{}
        });
}

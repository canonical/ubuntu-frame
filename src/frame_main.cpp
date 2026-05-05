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

#include "background_client.h"
#include "frame_authorization.h"
#include "frame_window_manager.h"
#include "display_configuration_builder.h"

#include <miral/configuration_option.h>
#include <miral/decorations.h>
#include <miral/internal_client.h>
#include <miral/keymap.h>
#include <miral/runner.h>
#include <miral/set_window_management_policy.h>
#include <miral/wayland_extensions.h>

#include <miral/version.h>

#if MIRAL_VERSION >= MIR_VERSION_NUMBER(5, 8, 0)
#include <miral/config_aggregator.h>
#include <miral/config_file_store_adapter.h>
#include <miral/live_config_ini_file.h>
#include <miral/cursor_scale.h>
#include <miral/output_filter.h>
#include <miral/magnifier.h>
#include <miral/config_file.h>

#include <cstdlib>
#include <filesystem>
#endif

int main(int argc, char const* argv[])
{
    using namespace miral;
    MirRunner runner{argc, argv};
    WindowManagerObserver window_manager_observer{};


    WaylandExtensions wayland_extensions;
    init_authorization(wayland_extensions, auth_model);

    BackgroundClient background_client(&runner, &window_manager_observer);

    runner.add_stop_callback([&] { background_client.stop(); });
    auto display_config = build_display_configuration(runner);

#if MIRAL_VERSION >= MIR_VERSION_NUMBER(5, 8, 0)
    miral::live_config::ConfigAggregator config_aggregator{};
    miral::ConfigFileStoreAdapter adapter{
        config_aggregator,
        [](std::unique_ptr<std::istream> stream, std::filesystem::path const& path)
        {
            auto parser = std::make_shared<miral::live_config::IniFile>();
            return miral::live_config::ConfigAggregator::Source{
                parser,
                [parser, stream = std::move(stream), path]()
                {
                    parser->load_file(*stream, path);
                },
                path,
            };
        }};

    miral::CursorScale cursor_scale{config_aggregator};
    miral::OutputFilter output_filter{config_aggregator};
    miral::Magnifier magnifier{config_aggregator};

    auto const accessibility_config_path = []() -> std::filesystem::path
    {
        if (auto const* env_path = std::getenv("UBUNTU_FRAME_ACCESSIBILITY_CONFIG_PATH"); env_path && *env_path)
            return env_path;
        if (auto const* xdg = std::getenv("XDG_CONFIG_HOME"); xdg && *xdg)
            return std::filesystem::path{xdg} / "mir" / "accessibility.ini";
        if (auto const* home = std::getenv("HOME"); home && *home)
            return std::filesystem::path{home} / ".config" / "mir" / "accessibility.ini";
        return ".config/mir/accessibility.ini";
    }();

    miral::ConfigFile config_file{
        runner,
        accessibility_config_path,
        miral::ConfigFile::Mode::reload_on_change,
        [&adapter](auto args) { adapter(args); },
    };
#endif
    return runner.run_with(
        {
            wayland_extensions,
            display_config,
            display_config.layout_option(),
            ConfigurationOption{[&](bool option) { background_client.set_wallpaper_enabled(option); },
                               "wallpaper", "Specifies whether or not the wallpaper is enabled", true},
            ConfigurationOption{[&](auto& option) { background_client.set_wallpaper_top_colour(option);},
                               "wallpaper-top",    "Colour of wallpaper RGB", "0x7f7f7f"},
            ConfigurationOption{[&](auto& option) { background_client.set_wallpaper_bottom_colour(option);},
                               "wallpaper-bottom", "Colour of wallpaper RGB", "0x1f1f1f"},
            ConfigurationOption{[&](auto& option) { background_client.set_crash_background_colour(option);},
                               "diagnostic-background", "Colour of diagnostic screen background RGB", "0x380c24"},
            ConfigurationOption{[&](auto& option) { background_client.set_crash_text_colour(option);},
                               "diagnostic-text",       "Colour of diagnostic screen text RGB", "0xffffff"},
            ConfigurationOption{[&] (auto& option) { background_client.set_diagnostic_path(option);},
                               "diagnostic-path",  "Path (including filename) of diagnostic file", ""},
            ConfigurationOption{[&] (int option) { background_client.set_diagnostic_delay(option);},
                                "diagnostic-delay", "Delay time (in seconds) before displaying diagnostic screen", 0},
            StartupInternalClient{std::ref(background_client)},
            ConfigurationOption{[&](bool option) { init_authorise_without_apparmor(option);},
                               "authorise-without-apparmor", "Use /proc/<pid>/cmdline if AppArmor is unavailable", false },
            set_window_management_policy<FrameWindowManagerPolicy>(
                window_manager_observer,
                display_config),
            Keymap{},
            miral::Decorations::always_csd(),
#if MIRAL_VERSION >= MIR_VERSION_NUMBER(5, 8, 0)
            cursor_scale,
            output_filter,
            magnifier,
#endif
        });
}

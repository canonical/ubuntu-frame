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

#include "frame_window_manager.h"
#include "layout_metadata.h"
#include "display_configuration_builder.h"

#include <mir_test_framework/window_management_test_harness.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <miral/display_configuration.h>
#include <miral/runner.h>
#include <fstream>

using namespace testing;
namespace mtf = mir_test_framework;
namespace geom = mir::geometry;

namespace
{
auto constexpr DISPLAY_RECT = geom::Rectangle({0, 0}, {800, 600});
}

class FrameWindowManagerTest : public mtf::WindowManagementTestHarness
{
public:
    FrameWindowManagerTest() : runner(argc, argv)
    {
    }

    auto get_builder() -> mir_test_framework::WindowManagementPolicyBuilder override
    {
        return [&](miral::WindowManagerTools const& tools)
        {
            return std::make_unique<FrameWindowManagerPolicy>(
                tools,
                observer,
                get_display_config());
        };
    }

    auto get_initial_output_configs() -> std::vector<mir::graphics::DisplayConfigurationOutput> override
    {
        return output_configs_from_output_rectangles({DISPLAY_RECT});
    }

protected:
    virtual miral::DisplayConfiguration get_display_config()
    {
        return miral::DisplayConfiguration(runner);
    }

    WindowManagerObserver observer;
    int const argc = 1;
    const char* argv[2] = {"test", nullptr};
    miral::MirRunner runner;
};

TEST_F(FrameWindowManagerTest, NewWindowsAreFullscreenByDefault)
{
    auto const app = open_application("test");
    miral::WindowSpecification const spec;
    auto const window = create_window(app, spec);
    auto const& info = tools().info_for(window);
    EXPECT_THAT(info.state(), Eq(mir_window_state_fullscreen));
}

TEST_F(FrameWindowManagerTest, NewWindowsTakeUpFullSizeOfDisplay)
{
    auto const app = open_application("test");
    miral::WindowSpecification const spec;
    auto const window = create_window(app, spec);
    EXPECT_THAT(window.top_left(), Eq(DISPLAY_RECT.top_left));
    EXPECT_THAT(window.size(), Eq(DISPLAY_RECT.size));
}

namespace
{
miral::DisplayConfiguration write_and_build_display_config(
    const char* path,
    std::string const& yaml,
    miral::MirRunner const& runner)
{
    // Set environment variables such that we'll always read the display
    // configuration from the temporary directory.
    setenv("XDG_CONFIG_HOME", "/tmp", 1);
    unsetenv("XDG_CONFIG_DIRS");
    unsetenv("HOME");

    std::ofstream file(path);
    file << yaml;
    return build_display_configuration(runner);
}
}

#if MIRAL_VERSION >= MIR_VERSION_NUMBER(5, 3, 0)
class FrameWindowManagerWithSurfaceTitleInDisplayConfig : public FrameWindowManagerTest
{
protected:
    FrameWindowManagerWithSurfaceTitleInDisplayConfig()
        : display_config(write_and_build_display_config(
            "/tmp/test.display",
            R"(
layouts:
  default:
    cards:
    - card-id: 0
      VGA-1:
        state: enabled
        mode: 800x600@60.0
        position: [0, 0]	# Defaults to [0, 0]
        orientation: normal	# {normal, left, right, inverted}, defaults to normal
        scale: 1
        group: 0	# Outputs with the same non-zero value are treated as a single display
    applications:
    - surface-title: test
      position: [ 100, 100 ]
      size: [ 50, 50 ]
)", runner))
    {
        display_config.operator()(server);
    }

    miral::DisplayConfiguration get_display_config() override
    {
        return display_config;
    }

    miral::DisplayConfiguration display_config;
};

TEST_F(FrameWindowManagerWithSurfaceTitleInDisplayConfig, WindowsCanBePlacedExactlyByTitle)
{
    auto const app = open_application("test");
    miral::WindowSpecification spec;
    spec.name() = "test";
    auto const window = create_window(app, spec);
    EXPECT_THAT(window.top_left(), Eq(geom::Point{100, 100}));
    EXPECT_THAT(window.size(), Eq(geom::Size{50, 50}));
}
#endif
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

#include <mir_test_framework/window_management_test_harness.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;
namespace mtf = mir_test_framework;
namespace geom = mir::geometry;

namespace
{
auto constexpr DISPLAY_RECT = geom::Rectangle({0, 0}, {800, 600});

class MockLayoutDataAccessor : public LayoutDataAccessor
{
public:
    MOCK_METHOD(std::shared_ptr<LayoutMetadata>, layout_metadata, (), (override));
};
}

class FrameWindowManagerTest : public mtf::WindowManagementTestHarness
{
public:
    FrameWindowManagerTest()
    {
        ON_CALL(*mock_layout_data_accessor, layout_metadata()).WillByDefault(testing::Return(nullptr));
    }
    auto get_builder() -> mir_test_framework::WindowManagementPolicyBuilder override
    {
        return [&](miral::WindowManagerTools const& tools)
        {
            return std::make_unique<FrameWindowManagerPolicy>(
                tools,
                observer,
                mock_layout_data_accessor);
        };
    }

    auto get_initial_output_configs() -> std::vector<mir::graphics::DisplayConfigurationOutput> override
    {
        auto r = output_configs_from_output_rectangles({DISPLAY_RECT});
        return r;
    }

    WindowManagerObserver observer;
    std::shared_ptr<MockLayoutDataAccessor> mock_layout_data_accessor
        = std::make_shared<NiceMock<MockLayoutDataAccessor>>();
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

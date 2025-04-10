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

#include "frame_authorization.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;

class AuthModelTest : public Test
{
};

TEST_F(AuthModelTest, True)
{
    std::vector<std::pair<std::string, std::vector<std::string>>> const protocols_for_snaps{
        std::pair("my-snap", std::vector<std::string>{ "zwlr_screencopy_manager_v1", "zwlr_layer_shell_v1" })
    };
    AuthModel model(protocols_for_snaps);
    EXPECT_THAT(model.snaps_for_protocols.size(), Eq(2));
    EXPECT_THAT(model.snaps_for_protocols.at("zwlr_screencopy_manager_v1").size(), Eq(1));
    EXPECT_THAT(*model.snaps_for_protocols.at("zwlr_screencopy_manager_v1").begin(), Eq("my-snap"));
    EXPECT_THAT(model.snaps_for_protocols.at("zwlr_layer_shell_v1").size(), Eq(1));
    EXPECT_THAT(*model.snaps_for_protocols.at("zwlr_layer_shell_v1").begin(), Eq("my-snap"));
}

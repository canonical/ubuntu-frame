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

#include "new_frame_authorization.h"

#include <mir/abnormal_exit.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;

class AuthModelTest : public AuthModel, public Test {};

TEST_F(AuthModelTest, add_new_snap) {
  add_privileged_client("snap:my-snap:some-protocol");
  add_privileged_client("snap:different-snap:some-protocol");

  std::map<std::string, std::set<std::string>> expected{
      {"my-snap", {"some-protocol"}}, {"different-snap", {"some-protocol"}}};
  EXPECT_THAT(privileged_clients, Eq(expected));
}
TEST_F(AuthModelTest, merge_with_existing_snap) {
  add_privileged_client("snap:my-snap:some-protocol");
  add_privileged_client("snap:my-snap:another-protocol");

  std::map<std::string, std::set<std::string>> expected{
      {"my-snap", {"some-protocol", "another-protocol"}}};
  EXPECT_THAT(privileged_clients, Eq(expected));
}
TEST_F(AuthModelTest, empty_protocol_clears_previous_policies) {
  add_privileged_client("snap:my-snap:some-protocol");
  add_privileged_client("snap:my-snap:");

  std::map<std::string, std::set<std::string>> expected{{"my-snap", {}}};
  EXPECT_THAT(privileged_clients, Eq(expected));
}
TEST_F(AuthModelTest, reject_non_snap_client) {
  EXPECT_THROW(
      { add_privileged_client("not-snap:thing:protocol"); }, mir::AbnormalExit);
}
TEST_F(AuthModelTest, reject_empty_snap_name) {
  EXPECT_THROW({ add_privileged_client("snap::protocol"); }, mir::AbnormalExit);
}
TEST_F(AuthModelTest, reject_missing_parts) {
  EXPECT_THROW({ add_privileged_client("snap:my-snap"); }, mir::AbnormalExit);
}
TEST_F(AuthModelTest, reject_too_many_parts) {
  EXPECT_THROW(
      { add_privileged_client("snap:my-snap:protocol:bonus-things"); },
      mir::AbnormalExit);
}

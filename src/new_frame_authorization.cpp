/*
 * Copyright © 2021 Canonical Ltd.
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
 * Authored by: William Wild <william.wold@canonical.com>
 */

#include "new_frame_authorization.h"

#include "snap_name_of.h"

#include <mir/abnormal_exit.h>

using namespace miral;


auto default_auth_model() -> AuthModel const&
{
    // A function-local static to ensure the WaylandExtension::* globals are known to be initialised
    // before we read them here.
    static AuthModel const default_auth_model{{
        {"ubuntu-frame-osk",
         {
             WaylandExtensions::zwlr_layer_shell_v1,
             WaylandExtensions::zwp_virtual_keyboard_manager_v1,
             WaylandExtensions::zwp_input_method_manager_v2,
         }},
        {"ubuntu-frame-vnc",
         {
             WaylandExtensions::zwlr_screencopy_manager_v1,
             WaylandExtensions::zwlr_virtual_pointer_manager_v1,
             WaylandExtensions::zwp_virtual_keyboard_manager_v1,
         }},
        {"ubuntu-frame",
         {WaylandExtensions::zwlr_screencopy_manager_v1,
          WaylandExtensions::zwlr_layer_shell_v1,
          WaylandExtensions::zwlr_foreign_toplevel_manager_v1}},
    }};

    return default_auth_model;
}

AuthModel::AuthModel(
    std::map<std::string, std::set<std::string>> const& privileged_clients)
    : privileged_clients{privileged_clients},
      authorise_without_apparmor{false} {}

void AuthModel::add_privileged_client(std::string const &policy) {
  auto const first_colon = policy.find(':');
  auto const second_colon = first_colon == std::string::npos ? std::string::npos : policy.find(':', first_colon + 1);
  auto const extra_colon = second_colon == std::string::npos ? std::string::npos : policy.find(':', second_colon + 1);
  if (first_colon == std::string::npos || second_colon == std::string::npos ||
      extra_colon != std::string::npos) {
    throw mir::AbnormalExit("Invalid privileged client, must be in form "
                            "`snap:<snap-name>:<protocol>` (`protocol` can be omitted to clear protocol list): " +
                            policy);
  }

  auto const policy_kind = policy.substr(0, first_colon);
  auto const policy_value =
      policy.substr(first_colon + 1, second_colon - (first_colon + 1));
  auto const protocol = policy.substr(second_colon + 1);

  if (policy_kind != "snap") {
    throw mir::AbnormalExit(
        "Invalid privileged client, only `snap` clients supported: " + policy);
  }

  if (policy_value == "") {
    throw mir::AbnormalExit(
        "Invalid privileged client, empty snap name is invalid: " + policy);
  }

  auto &snap_protocols =
      privileged_clients.insert({policy_value, {}}).first->second;
  if (protocol == "") {
    snap_protocols.clear();
  } else {
    snap_protocols.insert(protocol);
  }
}

void AuthModel::set_authorise_without_apparmor(bool value) {
  authorise_without_apparmor = value;
}

void AuthModel::apply_to(miral::WaylandExtensions &extensions) const {
  std::map<std::string, std::set<std::string>> protocols{};
  for (auto const &[snap, snap_protocols] : privileged_clients) {
    for (auto const &protocol : snap_protocols) {
      auto const snaps = protocols.insert({protocol, {}}).first;
      snaps->second.insert(snap);
    }
  }

  for (auto const &[protocol, snaps] : protocols) {
    extensions.conditionally_enable(
        protocol, [snaps, authorise_without_apparmor =
                              authorise_without_apparmor](auto const &info) {
          if (info.user_preference()) {
            return info.user_preference().value();
          }

          auto const snap_name =
              snap_name_of(info.app(), authorise_without_apparmor);
          return snaps.find(snap_name) != snaps.end();
        });
  }
}

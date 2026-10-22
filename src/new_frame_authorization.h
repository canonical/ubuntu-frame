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
 * Authored by: William Wold <william.wold@canonical.com>
 */

#ifndef NEW_FRAME_AUTHORIZATION_H
#define NEW_FRAME_AUTHORIZATION_H

#include <map>
#include <miral/wayland_extensions.h>
#include <set>

class AuthModel {
public:
  explicit AuthModel(std::map<std::string, std::set<std::string>> const
                         &privileged_clients = {});

  AuthModel(const AuthModel &) = default;

  /// Add a privileged client, in the form `kind:value:protocol`.
  ///
  /// Currently, only `snap` is supported, where `value` is a snap name. If
  /// `protocol` is empty, it will clear any previous policies for that snap.
  void add_privileged_client(std::string const &policy);
  void set_authorise_without_apparmor(bool value);

  void apply_to(miral::WaylandExtensions &extensions) const;

protected:
  std::map<std::string, std::set<std::string>> privileged_clients;
  bool authorise_without_apparmor;
};

auto default_auth_model() -> AuthModel const&;
#endif // NEW_FRAME_AUTHORIZATION_H

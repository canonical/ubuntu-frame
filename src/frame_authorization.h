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

#ifndef FRAME_AUTHORIZATION_H
#define FRAME_AUTHORIZATION_H

#include <miral/wayland_extensions.h>
#include <set>
#include <map>

class AuthModel
{
public:
    AuthModel(
        std::vector<std::pair<std::string, std::vector<std::string>>> const& protocols_for_snaps);

    std::map<std::string, std::set<std::string>> const snaps_for_protocols;
};

extern AuthModel const auth_model;

void init_authorization(miral::WaylandExtensions& extensions, AuthModel const& model);
void init_authorise_without_apparmor(bool enable_fallback);

#endif // FRAME_AUTHORIZATION_H

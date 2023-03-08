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

#include "frame_authorization.h"

#include "snap_name_of.h"

using namespace miral;

AuthModel const auth_model{{
    {"ubuntu-frame-osk", {
        WaylandExtensions::zwlr_layer_shell_v1,
        WaylandExtensions::zwp_virtual_keyboard_manager_v1,
        WaylandExtensions::zwp_input_method_manager_v2,
    }},
    {"ubuntu-frame-vnc", {
        WaylandExtensions::zwlr_screencopy_manager_v1,
        WaylandExtensions::zwlr_virtual_pointer_manager_v1,
        WaylandExtensions::zwp_virtual_keyboard_manager_v1,
    }},
    {"ubuntu-frame", {
        WaylandExtensions::zwlr_screencopy_manager_v1,
    }},
}};

namespace
{
bool authorise_without_apparmor = false;
}

AuthModel::AuthModel(
    std::vector<std::pair<std::string, std::vector<std::string>>> const& protocols_for_snaps)
    : snaps_for_protocols{[&]()
        {
            // The mapping of snap names -> allowed protocols is convenient and less error-prone to specify,
            // but to use it we need to reverse the mapping into one of protocols -> snap names.
            std::map<std::string, std::set<std::string>> snaps_for_protocols;
            for (auto const& [snap, protocols] : protocols_for_snaps)
            {
                for (auto const& protocol : protocols)
                {
                    auto const iter = snaps_for_protocols.insert({protocol, {}}).first;
                    iter->second.insert(snap);
                }
            }
            return snaps_for_protocols;
        }()}
{
}

void init_authorization(miral::WaylandExtensions& extensions, AuthModel const& model)
{
    for (auto const& [protocol, snaps] : model.snaps_for_protocols)
    {
        extensions.conditionally_enable(protocol, [snaps=snaps](auto const& info)
            {
                if (info.user_preference())
                {
                    return info.user_preference().value();
                }
                auto const snap_name = snap_name_of(info.app(), authorise_without_apparmor);
                return snaps.find(snap_name) != snaps.end();
            });
    }
}

void init_authorise_without_apparmor(bool enable_fallback)
{
    authorise_without_apparmor = enable_fallback;
}

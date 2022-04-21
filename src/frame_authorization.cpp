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

#include <miral/version.h>
#include <mir/log.h>
#include <unistd.h>
#include <cstring>
#include <sys/apparmor.h>
#include <set>
#include <map>

using namespace miral;

namespace
{
std::vector<std::pair<std::string, std::vector<std::string>>> const protocols_for_snaps{
    {"ubuntu-frame-osk", {
        WaylandExtensions::zwlr_layer_shell_v1,
        WaylandExtensions::zwp_virtual_keyboard_manager_v1,
        WaylandExtensions::zwp_input_method_manager_v2,
    }},
};

auto snap_name_of(miral::Application const& app) -> std::string
{
    int const app_fd = miral::socket_fd_of(app);
    char* label_cstr;
    char* mode_cstr;
    errno = 0;
    if (app_fd < 0)
    {
        return "";
    }
    else if (aa_getpeercon(app_fd, &label_cstr, &mode_cstr) < 0)
    {
        mir::log_debug("aa_getpeercon() failed for process %d: %s", miral::pid_of(app), strerror(errno));
        return "";
    }
    else
    {
        std::string const label{label_cstr};
        free(label_cstr);
        // mode_cstr should NOT be freed, as it's from the same buffer as label_cstr

        std::string const snap_prefix{"snap."};
        if (label.starts_with(snap_prefix))
        {
            auto right = label.find('.', snap_prefix.size());
            if (right == std::string::npos)
            {
                right = label.size();
            }
            return label.substr(snap_prefix.size(), right - snap_prefix.size());
        }
        else
        {
            return "";
        }
    }
}
}

void init_authorization(miral::WaylandExtensions& extensions)
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
    for (auto const& [protocol, snaps] : snaps_for_protocols)
    {
        extensions.conditionally_enable(protocol, [snaps=snaps](auto const& info)
            {
                if (info.user_preference())
                {
                    return info.user_preference().value();
                }
                auto const snap_name = snap_name_of(info.app());
                return snaps.find(snap_name) != snaps.end();
            });
    }
}

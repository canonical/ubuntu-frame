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
#include <sys/apparmor.h>
#include <cstring>
#include <fstream>

using namespace miral;

AuthModel const auth_model{{
    {"ubuntu-frame-osk", {
        WaylandExtensions::zwlr_layer_shell_v1,
        WaylandExtensions::zwp_virtual_keyboard_manager_v1,
        WaylandExtensions::zwp_input_method_manager_v2,
    }},
    {"ubuntu-frame-vnc", {
        WaylandExtensions::zwlr_screencopy_manager_v1,
#if MIRAL_VERSION >= MIR_VERSION_NUMBER(3, 6, 0)
        WaylandExtensions::zwlr_virtual_pointer_manager_v1,
#endif
        WaylandExtensions::zwp_virtual_keyboard_manager_v1,
    }},
    {"ubuntu-frame", {
        WaylandExtensions::zwlr_screencopy_manager_v1,
    }},
}};

namespace
{
bool authorise_without_apparmor = false;

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
        mir::log_info("aa_getpeercon() failed for process %d: %s", miral::pid_of(app), strerror(errno));

        if ((errno == EINVAL) && authorise_without_apparmor) // EINVAL is what is returned when AppArmor isn't setup
        {
            mir::log_info("Fall back (without AppArmor): Identify client via /proc/%%d/cmdline");

            // using the id, find the name of this process
            if (std::ifstream cmdline{"/proc/" + std::to_string(miral::pid_of(app)) + "/cmdline"})
            {
                std::string const path{std::istreambuf_iterator{cmdline}, std::istreambuf_iterator<char>{}};
                std::string const snap_prefix{"/snap/"};

                if (path.starts_with(snap_prefix))
                {
                    // Strip the prefix (after_snap_prefix) and app name (before_app_suffix) from the path
                    auto const after_snap_prefix = begin(path) + snap_prefix.size();
                    auto const before_app_suffix = std::find(after_snap_prefix, end(path), '/');

                    // We also need to discard any parallel-install suffix (which starts with an underscore)
                    auto const install_suffix = std::find(after_snap_prefix, before_app_suffix, '_');

                    return std::string{after_snap_prefix, install_suffix};
                }
            }
        }
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
            // Strip the prefix (after_snap_prefix) and app name (before_app_suffix) from the label
            auto const after_snap_prefix = begin(label) + snap_prefix.size();
            auto const before_app_suffix = std::find(after_snap_prefix, end(label), '.');

            // We also need to discard any parallel-install suffix (which starts with an underscore)
            auto const install_suffix = std::find(after_snap_prefix, before_app_suffix, '_');

            return std::string{after_snap_prefix, install_suffix};
        }
        else
        {
            return "";
        }
    }
}
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
                auto const snap_name = snap_name_of(info.app());
                return snaps.find(snap_name) != snaps.end();
            });
    }
}

void init_authorise_without_apparmor(bool enable_fallback)
{
    authorise_without_apparmor = enable_fallback;
}

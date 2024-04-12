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

#include "snap_name_of.h"

#include <mir/log.h>
#include <sys/apparmor.h>

#include <algorithm>
#include <cstring>
#include <fstream>

namespace
{

}
auto snap_name_of(miral::Application const& app, bool fallback_without_apparmor, bool strip_parellel) -> std::string
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

        // EINVAL is what is returned when AppArmor isn't setup
        // ENOPROTOOPT is what is returned when AppArmor doesn't have some Ubuntu patches (yet)
        if (((errno == EINVAL) || errno == ENOPROTOOPT) && fallback_without_apparmor)
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
                    if (strip_parellel)
                    {
                        auto const install_suffix = std::find(after_snap_prefix, before_app_suffix, '_');
                        return std::string{after_snap_prefix, install_suffix};
                    }
                    else
                    {
                        return std::string{after_snap_prefix, before_app_suffix};
                    }
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

            // We also need to discard any parallel-install suffix (which starts with an underscore)
            if (strip_parellel)
            {
                return std::string{after_snap_prefix, install_suffix};
            }
            else
            {
                return std::string{after_snap_prefix, before_app_suffix};
            }
        }
        else
        {
            return "";
        }
    }
}

auto snap_name_of(miral::Application const& app, bool fallback_without_apparmor) -> std::string
{
    return snap_name_of(app, fallback_without_apparmor, true);
}

auto snap_instance_name_of(miral::Application const& app) -> std::string
{
    return snap_name_of(app, true, false);
}

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

#include "snap_name_of.h"

#include <unistd.h>
#include <string.h>
#include <iostream>
#include <sys/apparmor.h>

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
        std::cerr
            << "aa_getpeercon() failed for process " << miral::pid_of(app)
            << ": " << strerror(errno)
            << std::endl;
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
            auto right = label.find(".", snap_prefix.size());
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

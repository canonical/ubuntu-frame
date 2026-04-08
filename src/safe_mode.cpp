/*
 * Copyright © Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 or 3 as
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

#include "safe_mode.h"

#include <miral/runner.h>
#include <mir/log.h>
#include <mir/abnormal_exit.h>
#include <boost/program_options/errors.hpp>

#include <cerrno>
#include <cstring>
#include <fstream>
#include <unistd.h>
#include <filesystem>

namespace 
{
    auto const safe_mode_env = "FRAME_SAFE_MODE";

    void reexec_in_safe_mode(std::string const& error_message, std::string const& diagnostic_path, char const* argv[])
    {
        bool diagnostic_written = false;
        if(std::ofstream diagnostic{diagnostic_path})
        {
            diagnostic << "Ubuntu Frame is running in safe mode after error:\n\n"
                    << error_message << "\n\n"
                    << "Please contact support";
            diagnostic_written = true;
        }
        else
        {
            mir::log_warning("Failed to write diagnostic file: %s", diagnostic_path.c_str());
        }

        std::map<std::string, std::string> safe_env_map;

        for(char** env = environ; *env; env++)
        {
            std::string entry{*env};
            auto const pos = entry.find('=');
            if (pos != std::string::npos)
            {
                safe_env_map[entry.substr(0, pos)] = entry.substr(pos + 1);
            }
        }

        for (auto it = safe_env_map.begin(); it != safe_env_map.end();)
        {
            if (it->first.find("MIR_SERVER_") == 0)
            {
                it = safe_env_map.erase(it);
            }
            else
            {
                it++;
            }
        }

        safe_env_map[safe_mode_env] = "1";
        safe_env_map["XDG_CONFIG_HOME"] = "/dev/null";
        safe_env_map["WAYLAND_DISPLAY"] = "wayland-66";

        std::vector<std::string> safe_env_strings;
        for(auto const& [key, value] : safe_env_map)
        {
            safe_env_strings.push_back(key + "=" + value);
        }
        
        std::vector<char const*> safe_env;
        for(auto const& e: safe_env_strings)
        {
            safe_env.push_back(e.c_str());
        }

        safe_env.push_back(nullptr);

        std::string const diagnostic_opt = "--diagnostic-path=" + diagnostic_path;
        std::vector<char const*> safe_argv;
        safe_argv.push_back(argv[0]);
        if (diagnostic_written)
        {
            safe_argv.push_back(diagnostic_opt.c_str());
        }
        safe_argv.push_back(nullptr);

        execvpe(argv[0], const_cast<char**>(safe_argv.data()), const_cast<char**>(safe_env.data()));

        mir::log_critical("Failed to re-exec in safe mode: %s", strerror(errno));
    }
}

void enable_safe_mode(char const* argv[], miral::MirRunner& runner)
{
    if  (getenv(safe_mode_env))
    {
        return;
    }

    auto const diagnostics_filename = std::filesystem::path{runner.config_file()}.filename().string() + ".diagnostic";

    auto const diagnostics_path = (std::filesystem::temp_directory_path() / diagnostics_filename).string();

    runner.set_exception_handler([argv, diagnostics_path]()
    {
        try
        {
            std::rethrow_exception(std::current_exception());
        }
        catch (mir::AbnormalExit const& e)
        {
            reexec_in_safe_mode(e.what(), diagnostics_path, argv);
        }
        catch (boost::program_options::error const& e)
        {
            reexec_in_safe_mode(e.what(), diagnostics_path, argv);
        }
    });
}
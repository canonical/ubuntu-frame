#include "safe_mode.h"

#include <miral/runner.h>
#include <mir/log.h>

#include <cerrno>
#include <cstring>
#include <fstream>
#include <unistd.h>

namespace 
{
    auto const safe_mode_env = "FRAME_SAFE_MODE";
}

void enable_safe_mode(int argc, char const* argv[], miral::MirRunner& runner)
{
    if (getenv(safe_mode_env))
    {
        return;
    }

    auto const diagnostic_path = runner.config_file() + ".diagnostic";

    runner.set_exception_handler([argc, argv, diagnostic_path]()
    {
        try 
        {
            std::rethrow_exception(std::current_exception());
        }
        catch (std::exception const& e)
        {
            if (std::ofstream diagnostic{diagnostic_path})
            {
                diagnostic << e.what() << "\n";
            }
            else
            {
                mir::log_warning("Failed to write diagnostic file: %s", diagnostic_path.c_str());
            }

            setenv(safe_mode_env, "1", 1);

            std::string const diagnostic_opt = "--diagnostic-path=" + diagnostic_path;
            char const* safe_argv[] = 
            {
                argv[0],
                diagnostic_opt.c_str(),
                nullptr
            };

            execvp(argv[0], const_cast<char**>(safe_argv));

            mir::log_critical("Failed to re-exec in safe mode: %s", strerror(errno));
        }

    });
}
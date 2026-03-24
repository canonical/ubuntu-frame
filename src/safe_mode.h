#ifndef UBUNTU_FRAME_SAFE_MODE_H
#define UBUNTU_FRAME_SAFE_MODE_H

namespace miral
{
    class MirRunner;
}

void enable_safe_mode(int argc, char const* argv[], miral::MirRunner& runner);

#endif
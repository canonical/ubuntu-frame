/*
 * Copyright © 2016-2018 Octopull Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#include "egwallpaper.h"
#include "egfullscreenclient.h"

#include <algorithm>
#include <cstring>
#include <sstream>

namespace
{
void render_gradient(int32_t width, int32_t height, unsigned char* row_, uint8_t* bottom_colour, uint8_t* top_colour)
{
    auto row = row_;
    for (int j = 0; j < height; j++)
    {
        auto* pixel = (uint32_t*)row;
        uint8_t pattern_[4];
        for (auto i = 0; i != 3; ++i)
            pattern_[i] = (j*bottom_colour[i] + (height - j) * top_colour[i]) / height;
        pattern_[3] = 0xff;

        for (int i = 0; i < width; i++)
            memcpy(pixel + i, pattern_, sizeof pixel[i]);

        row += 4*width;
    }
}
}

struct egmde::Wallpaper::Self : egmde::FullscreenClient
{
    Self(wl_display* display, uint8_t* bottom_colour, uint8_t* top_colour);

    void draw_screen(SurfaceInfo& info) const override;

    uint8_t* const bottom_colour;
    uint8_t* const top_colour;
};

void egmde::Wallpaper::Self::draw_screen(SurfaceInfo& info) const
{
    bool const rotated = info.output->transform & WL_OUTPUT_TRANSFORM_90;
    auto const width = rotated ? info.output->height : info.output->width;
    auto const height = rotated ? info.output->width : info.output->height;

    if (width <= 0 || height <= 0)
        return;

    auto const stride = 4*width;

    if (!info.surface)
    {
        info.surface = wl_compositor_create_surface(compositor);
    }

    if (!info.shell_surface)
    {
        info.shell_surface = wl_shell_get_shell_surface(shell, info.surface);
        wl_shell_surface_set_fullscreen(
            info.shell_surface,
            WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT,
            0,
            info.output->output);
    }

    if (info.buffer)
    {
        wl_buffer_destroy(info.buffer);
    }

    {
        auto const shm_pool = make_shm_pool(stride * height, &info.content_area);

        info.buffer = wl_shm_pool_create_buffer(
            shm_pool.get(),
            0,
            width, height, stride,
            WL_SHM_FORMAT_ARGB8888);
    }

    render_gradient(width, height, static_cast<unsigned char*>(info.content_area), bottom_colour, top_colour);

    wl_surface_attach(info.surface, info.buffer, 0, 0);
    wl_surface_set_buffer_scale(info.surface, info.output->scale_factor);
    wl_surface_commit(info.surface);
}

egmde::Wallpaper::Self::Self(wl_display* display, uint8_t* bottom_colour, uint8_t* top_colour) :
    FullscreenClient(display),
    bottom_colour{bottom_colour},
    top_colour{top_colour}
{
    wl_display_roundtrip(display);
    wl_display_roundtrip(display);
}

void egmde::Wallpaper::stop()
{
    if (auto ss = self.lock())
    {
        std::lock_guard<decltype(mutex)> lock{mutex};
        ss->stop();
        ss.reset();
    }
}


void egmde::Wallpaper::bottom(std::string const& option)
{
    uint32_t value;
    std::stringstream interpreter{option};

    if (interpreter >> std::hex >> value)
    {
        bottom_colour[0] = value & 0xff;
        bottom_colour[1] = (value >> 8) & 0xff;
        bottom_colour[2] = (value >> 16) & 0xff;
    }
}

void egmde::Wallpaper::top(std::string const& option)
{
    uint32_t value;
    std::stringstream interpreter{option};

    if (interpreter >> std::hex >> value)
    {
        top_colour[0] = value & 0xff;
        top_colour[1] = (value >> 8) & 0xff;
        top_colour[2] = (value >> 16) & 0xff;
    }
}

void egmde::Wallpaper::operator()(wl_display* display)
{
    auto client = std::make_shared<Self>(display, bottom_colour, top_colour);
    {
        std::lock_guard<decltype(mutex)> lock{mutex};
        self = client;
    }
    client->run(display);

    // Possibly need to wait for stop() to release the client.
    // (This would be less ugly with a ref-counted wrapper for wl_display* in the miral API)
    std::lock_guard<decltype(mutex)> lock{mutex};
    client.reset();
}

void egmde::Wallpaper::operator()(std::weak_ptr<mir::scene::Session> const& session)
{
    std::lock_guard<decltype(mutex)> lock{mutex};
    weak_session = session;
}

auto egmde::Wallpaper::session() const -> std::shared_ptr<mir::scene::Session>
{
    std::lock_guard<decltype(mutex)> lock{mutex};
    return weak_session.lock();
}

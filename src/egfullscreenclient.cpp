/*
 * Copyright © 2018-2019 Octopull Ltd.
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

#include "egfullscreenclient.h"

#include <wayland-client.h>

#include <boost/throw_exception.hpp>

#include <fcntl.h>
#include <sys/eventfd.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <stdlib.h>

#include <cstring>
#include <system_error>

void egmde::FullscreenClient::Output::geometry(
    void* data,
    struct wl_output* /*wl_output*/,
    int32_t x,
    int32_t y,
    int32_t /*physical_width*/,
    int32_t /*physical_height*/,
    int32_t /*subpixel*/,
    const char */*make*/,
    const char */*model*/,
    int32_t transform)
{
    auto output = static_cast<Output*>(data);

    output->x = x;
    output->y = y;
    output->transform = transform;
}


void egmde::FullscreenClient::Output::mode(
    void *data,
    struct wl_output* /*wl_output*/,
    uint32_t flags,
    int32_t width,
    int32_t height,
    int32_t /*refresh*/)
{
    if (!(WL_OUTPUT_MODE_CURRENT & flags))
        return;

    auto output = static_cast<Output*>(data);

    output->width = width,
    output->height = height;
}

void egmde::FullscreenClient::Output::scale(void* data, wl_output* /*wl_output*/, int32_t factor)
{
    auto output = static_cast<Output*>(data);

    output->scale_factor = factor;
}

egmde::FullscreenClient::Output::Output(
    wl_output* output,
    std::function<void(Output const&)> on_constructed,
    std::function<void(Output const&)> on_change)
    : output{output},
      on_done{[this, on_constructed = std::move(on_constructed), on_change=std::move(on_change)]
      (Output const& o) mutable { on_constructed(o), on_done = std::move(on_change); }}
{
    wl_output_add_listener(output, &output_listener, this);
}

egmde::FullscreenClient::Output::~Output()
{
    if (output)
        wl_output_destroy(output);
}

wl_output_listener const egmde::FullscreenClient::Output::output_listener = {
    &geometry,
    &mode,
    &done,
    &scale,
};

egmde::FullscreenClient::SurfaceInfo::SurfaceInfo(Output const* output) :
    output{output}
{

}

egmde::FullscreenClient::SurfaceInfo::~SurfaceInfo()
{
    clear_window();
}

void egmde::FullscreenClient::SurfaceInfo::clear_window()
{
    if (buffer)
        wl_buffer_destroy(buffer);

    if (shell_surface)
        wl_shell_surface_destroy(shell_surface);

    if (surface)
        wl_surface_destroy(surface);


    buffer = nullptr;
    shell_surface = nullptr;
    surface = nullptr;
}

void egmde::FullscreenClient::Output::done(void* data, struct wl_output* /*wl_output*/)
{
    auto output = static_cast<Output*>(data);
    output->on_done(*output);
}

egmde::FullscreenClient::FullscreenClient(wl_display* display) :
    flush_signal{::eventfd(0, EFD_SEMAPHORE)},
    shutdown_signal{::eventfd(0, EFD_CLOEXEC)},
    registry{nullptr, [](auto){}}
{
    if (shutdown_signal == mir::Fd::invalid)
    {
        BOOST_THROW_EXCEPTION((std::system_error{errno, std::system_category(), "Failed to create shutdown notifier"}));
    }

    this->display = display;

    registry = {wl_display_get_registry(display), &wl_registry_destroy};

    static wl_registry_listener const registry_listener = {
        [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->new_global(args...); },
        [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->remove_global(args...); },
    };

    wl_registry_add_listener(registry.get(), &registry_listener, this);
}

void egmde::FullscreenClient::on_output_changed(Output const* output)
{
    {
        std::lock_guard<decltype(outputs_mutex)> lock{outputs_mutex};
        auto const p = outputs.find(output);
        if (p != end(outputs))
        {
            if (auto& buffer = p->second.buffer)
            {
                wl_buffer_destroy(buffer);
                buffer = nullptr;
            }

            draw_screen(p->second);
        }

        auto i = begin(hidden_outputs);
        while (i != end(hidden_outputs))
        {
            mir::geometry::Rectangle const screen_rect{{(*i)->x, (*i)->y}, {(*i)->width, (*i)->height}};

            if (!display_area.bounding_rectangle().overlaps(screen_rect))
            {
                display_area.add(screen_rect);
                draw_screen(outputs.insert({*i, SurfaceInfo{*i}}).first->second);
                break;
            }

            ++i;
        }

        if (i != end(hidden_outputs))
        {
            hidden_outputs.erase(i);
        }
    }
    wl_display_flush(display);
}

void egmde::FullscreenClient::on_output_gone(Output const* output)
{
    {
        std::lock_guard<decltype(outputs_mutex)> lock{outputs_mutex};

        outputs.erase(output);

        auto i = begin(hidden_outputs);
        while (i != end(hidden_outputs))
        {
            if (output == *i)
            {
                break;
            }

            ++i;
        }

        if (i != end(hidden_outputs))
        {
            hidden_outputs.erase(i);
        }
        else
        {
            display_area.remove({{output->x, output->y}, {output->width, output->height}});
        }

        i = begin(hidden_outputs);
        while (i != end(hidden_outputs))
        {
            mir::geometry::Rectangle const screen_rect{{(*i)->x, (*i)->y}, {(*i)->width, (*i)->height}};

            if (!display_area.bounding_rectangle().overlaps(screen_rect))
            {
                display_area.add(screen_rect);
                draw_screen(outputs.insert({*i, SurfaceInfo{*i}}).first->second);
                break;
            }

            ++i;
        }

        if (i != end(hidden_outputs))
        {
            hidden_outputs.erase(i);
        }
    }
    wl_display_flush(display);
}

void egmde::FullscreenClient::on_new_output(Output const* output)
{
    {
        std::lock_guard<decltype(outputs_mutex)> lock{outputs_mutex};

        mir::geometry::Rectangle const screen_rect{
                {output->x,    output->y},
                {output->width,output->height}};

        if (!display_area.bounding_rectangle().overlaps(screen_rect))
        {
            display_area.add(screen_rect);
            draw_screen(outputs.insert({output, SurfaceInfo{output}}).first->second);
        }
        else
        {
            hidden_outputs.emplace_back(output);
        }
    }
    wl_display_flush(display);
}

auto egmde::FullscreenClient::make_shm_pool(size_t size, void** data) const
-> std::unique_ptr<wl_shm_pool, std::function<void(wl_shm_pool*)>>
{
    static auto (*open_shm_file)() -> mir::Fd = []
    {
        static char const* shm_dir;
        open_shm_file = []{ return mir::Fd{open(shm_dir, O_TMPFILE | O_RDWR | O_EXCL, S_IRWXU)}; };

        // Wayland based toolkits typically use $XDG_RUNTIME_DIR to open shm pools
        // so we try that before "/dev/shm". But confined snaps can't access "/dev/shm"
        // so we try "/tmp" if both of the above fail.
        for (auto dir : {const_cast<const char*>(getenv("XDG_RUNTIME_DIR")), "/dev/shm", "/tmp" })
        {
            if (dir)
            {
                shm_dir = dir;
                auto fd = open_shm_file();
                if (fd >= 0)
                    return fd;
            }
        }
        return mir::Fd{};
    };

    auto fd = open_shm_file();

    if (fd < 0) {
        BOOST_THROW_EXCEPTION((std::system_error{errno, std::system_category(), "Failed to open shm buffer"}));
    }

    if (auto error = posix_fallocate(fd, 0, size))
    {
        BOOST_THROW_EXCEPTION((std::system_error{error, std::system_category(), "Failed to allocate shm buffer"}));
    }

    if ((*data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
    {
        BOOST_THROW_EXCEPTION((std::system_error{errno, std::system_category(), "Failed to mmap buffer"}));
    }

    return {
        wl_shm_create_pool(shm, fd, size),
        [size](auto* shm)
        {
            wl_shm_pool_destroy(shm);
            munmap(shm, size);
        }};
}

egmde::FullscreenClient::~FullscreenClient()
{
    {
        std::lock_guard<decltype(outputs_mutex)> lock{outputs_mutex};
        outputs.clear();
    }
    bound_outputs.clear();
    registry.reset();
    wl_display_roundtrip(display);
}

void egmde::FullscreenClient::new_global(struct wl_registry* registry, uint32_t id, char const* interface, uint32_t version)
{
    (void)version;

    if (strcmp(interface, "wl_compositor") == 0)
    {
        compositor =
            static_cast<decltype(compositor)>(wl_registry_bind(registry, id, &wl_compositor_interface, 3));
    }
    else if (strcmp(interface, "wl_shm") == 0)
    {
        shm = static_cast<decltype(shm)>(wl_registry_bind(registry, id, &wl_shm_interface, 1));
        // Normally we'd add a listener to pick up the supported formats here
        // As luck would have it, I know that argb8888 is the only format we support :)
    }
    else if (strcmp(interface, "wl_seat") == 0)
    {
        seat = static_cast<decltype(seat)>(wl_registry_bind(registry, id, &wl_seat_interface, 4));
        static struct wl_seat_listener seatListener =
            {
                [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->seat_capabilities(args...); },
                [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->seat_name(args...); },
            };

        wl_seat_add_listener(seat, &seatListener, this);
    }
    else if (strcmp(interface, "wl_output") == 0)
    {
        // NOTE: We'd normally need to do std::min(version, 2), lest the compositor only support version 1
        // of the interface. However, we're an internal client of a compositor that supports version 2, so…
        auto output = static_cast<wl_output*>(wl_registry_bind(registry, id, &wl_output_interface, 2));
        bound_outputs.insert(
            std::make_pair(
                id,
                std::make_unique<Output>(
                    output,
                    [this](Output const& output) { on_new_output(&output); },
                    [this](Output const& output) { on_output_changed(&output); })));
    }
    else if (strcmp(interface, "wl_shell") == 0)
    {
        shell = static_cast<decltype(shell)>(wl_registry_bind(registry, id, &wl_shell_interface, 1));
    }
}

void egmde::FullscreenClient::remove_global(
    struct wl_registry* /*registry*/,
    uint32_t id)
{
    auto const output = bound_outputs.find(id);
    if (output != bound_outputs.end())
    {
        on_output_gone(output->second.get());
        bound_outputs.erase(output);
    }
    // TODO: We should probably also delete any other globals we've bound to that disappear.
}

void egmde::FullscreenClient::run(wl_display* display)
{
    enum FdIndices {
        display_fd = 0,
        flush,
        shutdown,
        indices
    };

    pollfd fds[indices] =
        {
            {wl_display_get_fd(display), POLLIN, 0},
            {flush_signal,               POLLIN, 0},
            {shutdown_signal,            POLLIN, 0},
        };

    while (!(fds[shutdown].revents & (POLLIN | POLLERR)))
    {
        while (wl_display_prepare_read(display) != 0)
        {
            if (wl_display_dispatch_pending(display) == -1)
            {
                BOOST_THROW_EXCEPTION((std::system_error{errno, std::system_category(), "Failed to dispatch Wayland events"}));
            }
        }

        if (poll(fds, indices, -1) == -1)
        {
            wl_display_cancel_read(display);
            BOOST_THROW_EXCEPTION((std::system_error{errno, std::system_category(), "Failed to wait for event"}));
        }

        if (fds[display_fd].revents & (POLLIN | POLLERR))
        {
            if (wl_display_read_events(display))
            {
                BOOST_THROW_EXCEPTION((std::system_error{errno, std::system_category(), "Failed to read Wayland events"}));
            }
        }
        else
        {
            wl_display_cancel_read(display);
        }

        if (fds[flush].revents & (POLLIN | POLLERR))
        {
            eventfd_t foo;
            eventfd_read(flush_signal, &foo);
            wl_display_flush(display);
        }
    }
}

void egmde::FullscreenClient::stop()
{
    if (eventfd_write(shutdown_signal, 1) == -1)
    {
        BOOST_THROW_EXCEPTION((std::system_error{errno, std::system_category(), "Failed to shutdown internal client"}));
    }
}

void egmde::FullscreenClient::for_each_surface(std::function<void(SurfaceInfo&)> const& f) const
{
    {
        std::lock_guard<decltype(outputs_mutex)> lock{outputs_mutex};
        for (auto& os : outputs)
        {
            f(const_cast<SurfaceInfo&>(os.second));
        }
    }

    flush_wl();
}

void egmde::FullscreenClient::flush_wl() const
{
    eventfd_write(flush_signal, 1);
}

void egmde::FullscreenClient::keyboard_keymap(wl_keyboard* /*keyboard*/, uint32_t /*format*/, int32_t /*fd*/, uint32_t /*size*/)
{
}

void egmde::FullscreenClient::keyboard_enter(
    wl_keyboard* /*keyboard*/,
    uint32_t /*serial*/,
    wl_surface*/*surface*/,
    wl_array* /*keys*/)
{
}

void egmde::FullscreenClient::keyboard_leave(wl_keyboard* /*keyboard*/, uint32_t /*serial*/, wl_surface* /*surface*/)
{
}

void egmde::FullscreenClient::keyboard_key(
    wl_keyboard* /*keyboard*/,
    uint32_t /*serial*/,
    uint32_t /*time*/,
    uint32_t /*key*/,
    uint32_t /*state*/)
{
}

void egmde::FullscreenClient::keyboard_modifiers(
    wl_keyboard */*keyboard*/,
    uint32_t /*serial*/,
    uint32_t /*mods_depressed*/,
    uint32_t /*mods_latched*/,
    uint32_t /*mods_locked*/,
    uint32_t /*group*/)
{
}

void egmde::FullscreenClient::keyboard_repeat_info(wl_keyboard* /*wl_keyboard*/, int32_t /*rate*/, int32_t /*delay*/)
{
}

void egmde::FullscreenClient::pointer_enter(
    wl_pointer* /*pointer*/,
    uint32_t /*serial*/,
    wl_surface* /*surface*/,
    wl_fixed_t /*x*/,
    wl_fixed_t /*y*/)
{
}

void egmde::FullscreenClient::pointer_leave(wl_pointer* /*pointer*/, uint32_t /*serial*/, wl_surface* /*surface*/)
{
}

void egmde::FullscreenClient::pointer_motion(wl_pointer* /*pointer*/, uint32_t /*time*/, wl_fixed_t /*x*/, wl_fixed_t /*y*/)
{
}

void egmde::FullscreenClient::pointer_button(
    wl_pointer* /*pointer*/,
    uint32_t /*serial*/,
    uint32_t /*time*/,
    uint32_t /*button*/,
    uint32_t /*state*/)
{
}

void egmde::FullscreenClient::pointer_axis(
    wl_pointer* /*pointer*/,
    uint32_t /*time*/,
    uint32_t /*axis*/,
    wl_fixed_t /*value*/)
{
}

void egmde::FullscreenClient::pointer_frame(wl_pointer* /*pointer*/)
{
}

void egmde::FullscreenClient::pointer_axis_source(wl_pointer* /*pointer*/, uint32_t /*axis_source*/)
{
}

void egmde::FullscreenClient::pointer_axis_stop(wl_pointer* /*pointer*/, uint32_t /*time*/, uint32_t /*axis*/)
{
}

void egmde::FullscreenClient::pointer_axis_discrete(wl_pointer* /*pointer*/, uint32_t /*axis*/, int32_t /*discrete*/)
{
}

void egmde::FullscreenClient::touch_down(
    wl_touch* /*touch*/,
    uint32_t /*serial*/,
    uint32_t /*time*/,
    wl_surface* /*surface*/,
    int32_t /*id*/,
    wl_fixed_t /*x*/,
    wl_fixed_t /*y*/)
{
}

void egmde::FullscreenClient::touch_up(
    wl_touch* /*touch*/,
    uint32_t /*serial*/,
    uint32_t /*time*/,
    int32_t /*id*/)
{
}

void egmde::FullscreenClient::touch_motion(
    wl_touch* /*touch*/,
    uint32_t /*time*/,
    int32_t /*id*/,
    wl_fixed_t /*x*/,
    wl_fixed_t /*y*/)
{
}

void egmde::FullscreenClient::touch_frame(wl_touch* /*touch*/)
{
}
    
void egmde::FullscreenClient::touch_cancel(wl_touch* /*touch*/)
{
}

void egmde::FullscreenClient::touch_shape(
    wl_touch* /*touch*/,
    int32_t /*id*/,
    wl_fixed_t /*major*/,
    wl_fixed_t /*minor*/)
{
}

void egmde::FullscreenClient::touch_orientation(
    wl_touch* /*touch*/,
    int32_t /*id*/,
    wl_fixed_t /*orientation*/)
{
}

void egmde::FullscreenClient::seat_capabilities(wl_seat* seat, uint32_t capabilities)
{
    if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
        static wl_pointer_listener pointer_listener =
            {
                [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->pointer_enter(args...); },
                [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->pointer_leave(args...); },
                [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->pointer_motion(args...); },
                [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->pointer_button(args...); },
                [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->pointer_axis(args...); },
                [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->pointer_frame(args...); },
                [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->pointer_axis_source(args...); },
                [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->pointer_axis_stop(args...); },
                [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->pointer_axis_discrete(args...); },
            };

        struct wl_pointer *pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener (pointer, &pointer_listener, this);
    }

    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD)
    {
        static struct wl_keyboard_listener keyboard_listener =
            {
                [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->keyboard_keymap(args...); },
                [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->keyboard_enter(args...); },
                [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->keyboard_leave(args...); },
                [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->keyboard_key(args...); },
                [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->keyboard_modifiers(args...); },
                [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->keyboard_repeat_info(args...); },
            };

        wl_keyboard_add_listener(wl_seat_get_keyboard(seat), &keyboard_listener, this);
    }

    if (capabilities & WL_SEAT_CAPABILITY_TOUCH)
    {
        static struct wl_touch_listener touch_listener =
        {
            [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->touch_down(args...); },
            [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->touch_up(args...); },
            [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->touch_motion(args...); },
            [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->touch_frame(args...); },
            [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->touch_cancel(args...); },
#ifdef WL_TOUCH_SHAPE_SINCE_VERSION
            [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->touch_shape(args...); },
#endif
#ifdef WL_TOUCH_ORIENTATION_SINCE_VERSION
            [](void* self, auto... args) { static_cast<FullscreenClient*>(self)->touch_orientation(args...); },
#endif
        };

        wl_touch_add_listener(wl_seat_get_touch(seat), &touch_listener, this);
    }
}

void egmde::FullscreenClient::seat_name(wl_seat* /*seat*/, const char */*name*/)
{
}

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

#ifndef EGMDE_EGFULLSCREENCLIENT_H
#define EGMDE_EGFULLSCREENCLIENT_H

#include <mir/fd.h>
#include <mir/geometry/rectangles.h>

#include <wayland-client.h>

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <unordered_map>

struct xkb_context;
struct xkb_keymap;
struct xkb_state;

namespace egmde
{
class FullscreenClient
{
public:
    explicit FullscreenClient(wl_display* display);

    virtual ~FullscreenClient();

    void run(wl_display* display);

    void stop();

    auto make_shm_pool(size_t size, void** data) const
    -> std::unique_ptr<wl_shm_pool, std::function<void(wl_shm_pool*)>>;

    wl_display* display = nullptr;
    wl_compositor* compositor = nullptr;
    wl_shell* shell = nullptr;

    class Output
    {
    public:
        Output(
            wl_output* output,
            std::function<void(Output const&)> on_constructed,
            std::function<void(Output const&)> on_change);

        ~Output();

        Output(Output const&) = delete;

        Output(Output&&) = delete;

        Output& operator=(Output const&) = delete;

        Output& operator=(Output&&) = delete;

        int32_t x;
        int32_t y;
        int32_t width;
        int32_t height;
        int32_t transform;
        wl_output* output;
        int32_t scale_factor = 1;
    private:
        static void done(void* data, wl_output* output);

        static void geometry(
            void* data,
            wl_output* wl_output,
            int32_t x,
            int32_t y,
            int32_t physical_width,
            int32_t physical_height,
            int32_t subpixel,
            const char* make,
            const char* model,
            int32_t transform);

        static void mode(
            void* data,
            wl_output* wl_output,
            uint32_t flags,
            int32_t width,
            int32_t height,
            int32_t refresh);

        static void scale(void* data, wl_output* wl_output, int32_t factor);

        static wl_output_listener const output_listener;

        std::function<void(Output const&)> on_done;
    };

    struct SurfaceInfo
    {
        SurfaceInfo(Output const* output);
        ~SurfaceInfo();

        void clear_window();

        // Screen description
        Output const* output;

        // Content
        void* content_area = nullptr;
        wl_surface* surface = nullptr;
        wl_shell_surface* shell_surface = nullptr;
        wl_buffer* buffer = nullptr;
    };

    virtual void draw_screen(SurfaceInfo& info) const = 0;

    void for_each_surface(std::function<void(SurfaceInfo&)> const& f) const;

protected:

    virtual void keyboard_keymap(wl_keyboard* keyboard, uint32_t format, int32_t fd, uint32_t size);
    virtual void keyboard_enter(wl_keyboard* keyboard, uint32_t serial, wl_surface* surface, wl_array* keys);
    virtual void keyboard_leave(wl_keyboard* keyboard, uint32_t serial, wl_surface* surface);
    virtual void keyboard_key(wl_keyboard* keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    virtual void keyboard_modifiers(
        wl_keyboard* keyboard,
        uint32_t serial,
        uint32_t mods_depressed,
        uint32_t mods_latched,
        uint32_t mods_locked,
        uint32_t group);
    virtual void keyboard_repeat_info(wl_keyboard* wl_keyboard, int32_t rate, int32_t delay);
    virtual void pointer_enter(wl_pointer* pointer, uint32_t serial, wl_surface* surface, wl_fixed_t x, wl_fixed_t y);
    virtual void pointer_leave(wl_pointer* pointer, uint32_t serial, wl_surface* surface);
    virtual void pointer_motion(wl_pointer* pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y);
    virtual void pointer_button(wl_pointer* pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
    virtual void pointer_axis(wl_pointer* pointer, uint32_t time, uint32_t axis, wl_fixed_t value);
    virtual void pointer_frame(wl_pointer* pointer);
    virtual void pointer_axis_source(wl_pointer* pointer, uint32_t axis_source);
    virtual void pointer_axis_stop(wl_pointer* pointer, uint32_t time, uint32_t axis);
    virtual void pointer_axis_discrete(wl_pointer* pointer, uint32_t axis, int32_t discrete);

    virtual void touch_down(
        wl_touch* touch,
        uint32_t serial,
        uint32_t time,
        wl_surface* surface,
        int32_t id,
        wl_fixed_t x,
        wl_fixed_t y);

    virtual void touch_up(
        wl_touch* touch,
        uint32_t serial,
        uint32_t time,
        int32_t id);

    virtual void touch_motion(
        wl_touch* touch,
        uint32_t time,
        int32_t id,
        wl_fixed_t x,
        wl_fixed_t y);

    virtual void touch_frame(wl_touch* touch);

    virtual void touch_cancel(wl_touch* touch);

    virtual void touch_shape(
        wl_touch* touch,
        int32_t id,
        wl_fixed_t major,
        wl_fixed_t minor);

    virtual void touch_orientation(
        wl_touch* touch,
        int32_t id,
        wl_fixed_t orientation);

private:
    void on_new_output(Output const*);

    void on_output_changed(Output const*);

    void on_output_gone(Output const*);

    // Flush pending requests (on a safe thread)
    void flush_wl() const;

    mir::Fd const flush_signal;
    mir::Fd const shutdown_signal;

    std::mutex mutable outputs_mutex;
    std::map<Output const*, SurfaceInfo> outputs;
    mir::geometry::Rectangles display_area;
    std::vector<Output const*> hidden_outputs;

    wl_seat* seat = nullptr;
    wl_shm* shm = nullptr;

    void new_global(
        struct wl_registry* registry,
        uint32_t id,
        char const* interface,
        uint32_t version);

    void remove_global(
        struct wl_registry* registry,
        uint32_t name);

    void seat_capabilities(wl_seat* seat, uint32_t capabilities);
    void seat_name(wl_seat* seat, const char* name);

    std::unique_ptr<wl_registry, decltype(&wl_registry_destroy)> registry;

    std::unordered_map<uint32_t, std::unique_ptr<Output>> bound_outputs;
};
}

#endif //EGMDE_EGFULLSCREENCLIENT_H

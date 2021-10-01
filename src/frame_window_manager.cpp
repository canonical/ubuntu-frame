/*
 * Copyright © 2016-2021 Canonical Ltd.
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
 *
 * Authored By: Alan Griffiths <alan@octopull.co.uk>
 */

#include "frame_window_manager.h"

#include <miral/application_info.h>
#include <miral/toolkit_event.h>
#include <miral/window_info.h>
#include <miral/window_manager_tools.h>

#include <linux/input.h>
#include <unistd.h>
#include <limits>
#include <iostream>

namespace ms = mir::scene;
using namespace miral;
using namespace miral::toolkit;

namespace
{
struct WindowData
{
    WindowData(bool is_ours)
        : is_ours{is_ours}
    {
    }

    bool is_ours;
};

template<typename T>
auto value_or(mir::optional_value<T> const& opt, T const& fallback) -> T const&
{
    return opt.is_set() ? opt.value() : fallback;
}

auto has_parent(WindowSpecification const& spec, bool fallback) -> bool
{
    if (spec.parent())
    {
        return spec.parent().value().lock().operator bool();
    }
    else
    {
        return fallback;
    }
}

auto should_be_fullscreen(
    MirWindowType type,
    MirWindowState state,
    bool has_parent,
    std::shared_ptr<void> userdata) -> bool
{
    // Only override behavior of windows of type normal and freestyle
    switch (type)
    {
    case mir_window_type_normal:
    case mir_window_type_freestyle:
        break;

    default:
        return false;
    }

    // Only override behavior of windows without a parent
    if (has_parent)
    {
        return false;
    }

    // Only override behavior if the new state is something other than minimized, hidden or attached
    switch (state)
    {
    case mir_window_state_minimized:
    case mir_window_state_hidden:
    case mir_window_state_attached:
        return false;

    default:;
    }

    if (userdata && static_cast<WindowData*>(userdata.get())->is_ours)
    {
        return false;
    }

    return true;
}

void override_spec(WindowSpecification& spec, Rectangle const& extents)
{
    spec.state() = mir_window_state_fullscreen;
    spec.size() = extents.size;
    spec.top_left() = extents.top_left;
}

/// Returns a positive overlap between the two rectangles or a negative distance if they do not overlap
auto overlap_or_distance(Rectangle const& a, Rectangle const& b) -> int
{
    // TODO
    return 0;
}
}

bool FrameWindowManagerPolicy::handle_keyboard_event(MirKeyboardEvent const* event)
{
    return false;
}

bool FrameWindowManagerPolicy::handle_touch_event(MirTouchEvent const* event)
{
    auto const count = mir_touch_event_point_count(event);

    long total_x = 0;
    long total_y = 0;

    for (auto i = 0U; i != count; ++i)
    {
        total_x += mir_touch_event_axis_value(event, i, mir_touch_axis_x);
        total_y += mir_touch_event_axis_value(event, i, mir_touch_axis_y);
    }

    Point const cursor{total_x/count, total_y/count};

    tools.select_active_window(tools.window_at(cursor));

    return false;
}

bool FrameWindowManagerPolicy::handle_pointer_event(MirPointerEvent const* event)
{
    auto const action = mir_pointer_event_action(event);

    Point const cursor{
        mir_pointer_event_axis_value(event, mir_pointer_axis_x),
        mir_pointer_event_axis_value(event, mir_pointer_axis_y)};

    if (action == mir_pointer_action_button_down)
    {
        tools.select_active_window(tools.window_at(cursor));
    }

    return false;
}

auto FrameWindowManagerPolicy::place_new_window(ApplicationInfo const& app_info, WindowSpecification const& request)
-> WindowSpecification
{
    WindowSpecification spec = CanonicalWindowManagerPolicy::place_new_window(app_info, request);

    bool const is_ours = pid_of(app_info.application()) == getpid();
    spec.userdata() = std::make_shared<WindowData>(is_ours);

    if (should_be_fullscreen(
            value_or(spec.type(), mir_window_type_normal),
            value_or(spec.state(), mir_window_state_restored),
            has_parent(spec, false),
            spec.userdata().value()))
    {
        Rectangle const requested_rect{
            spec.top_left().is_set() ? spec.top_left().value() : Point{},
            spec.size().is_set() ? spec.size().value() : Size{}};
        Rectangle const extents = container_for_rect(requested_rect).zone.extents();
        override_spec(spec, extents);
    }

    if (is_ours)
    {
        spec.depth_layer() = mir_depth_layer_background;
    }

    return spec;
}

void FrameWindowManagerPolicy::handle_modify_window(WindowInfo& window_info, WindowSpecification const& modifications)
{
    WindowSpecification spec = modifications;

    bool relevant_change =
        spec.state().is_set() ||
        spec.top_left().is_set() ||
        spec.size().is_set();

    if (relevant_change &&
        should_be_fullscreen(
            value_or(spec.type(), window_info.type()),
            value_or(spec.state(), window_info.state()),
            has_parent(spec, window_info.parent()),
            window_info.userdata()))
    {
        auto const extents = ensure_window_in_container(window_info.window());
        override_spec(spec, extents);
        tools.place_and_size_for_state(spec, window_info);
    }

    CanonicalWindowManagerPolicy::handle_modify_window(window_info, spec);
}

void FrameWindowManagerPolicy::handle_request_drag_and_drop(WindowInfo& /*window_info*/)
{
}

void FrameWindowManagerPolicy::handle_request_move(WindowInfo& /*window_info*/, MirInputEvent const* /*input_event*/)
{
}

void FrameWindowManagerPolicy::handle_request_resize(WindowInfo& /*window_info*/, MirInputEvent const* /*input_event*/, MirResizeEdge /*edge*/)
{
}

auto FrameWindowManagerPolicy::confirm_placement_on_display(
    WindowInfo const& window_info,
    MirWindowState new_state,
    Rectangle const& new_placement) -> Rectangle
{
    if (auto const extents = container_extents_of(window_info.window()))
    {
        return extents.value();
    }
    else
    {
        return new_placement;
    }
}

void FrameWindowManagerPolicy::advise_new_window(WindowInfo const& window_info)
{
    if (should_be_fullscreen(window_info.type(), window_info.state(), window_info.parent(), window_info.userdata()))
    {
        ensure_window_in_container(window_info.window());
    }
}

void FrameWindowManagerPolicy::advise_delete_window(WindowInfo const& window_info)
{
    ensure_window_not_in_container(window_info.window());
}

void FrameWindowManagerPolicy::advise_application_zone_create(Zone const& application_zone)
{
    containers.push_back({application_zone, {}});
}

void FrameWindowManagerPolicy::advise_application_zone_update(Zone const& updated, Zone const& original)
{
    for (auto& container : containers)
    {
        if (container.zone.is_same_zone(original))
        {
            container.zone = updated;
            WindowSpecification spec;
            spec.top_left() = updated.extents().top_left;
            spec.size() = updated.extents().size;
            for (auto const& window : container.attached)
            {
                tools.modify_window(window, spec);
            }
        }
    }
}

void FrameWindowManagerPolicy::advise_application_zone_delete(Zone const& application_zone)
{
    std::set<Window> attached_windows;
    for (auto& container : containers)
    {
        if (application_zone.is_same_zone(container.zone))
        {
            attached_windows = std::move(container.attached);
        }
    }
    containers.erase(
        std::remove_if(
            containers.begin(),
            containers.end(),
            [&](auto container)
            {
                return application_zone.is_same_zone(container.zone);
            }),
        containers.end());
    for (auto const& window : attached_windows)
    {
        WindowSpecification spec;
        auto const extents = ensure_window_in_container(window);
        spec.top_left() = extents.top_left;
        spec.size() = extents.size;
        tools.modify_window(window, spec);
    }
}

auto FrameWindowManagerPolicy::container_for_rect(Rectangle const& rect) -> Container&
{
    if (containers.empty())
    {
        std::cerr << "fatal: container_for_rect() called without any containers" << std::endl;
        abort();
    }
    int max_overlap = std::numeric_limits<int>::min();
    Container& result{containers[0]};
    for (auto& container : containers)
    {
        int overlap = overlap_or_distance(rect, container.zone.extents());
        if (overlap > max_overlap)
        {
            result = container;
            max_overlap = overlap;
        }
    }
    return result;
}

auto FrameWindowManagerPolicy::container_extents_of(Window const& window) -> std::optional<Rectangle>
{
    for (auto const& container : containers)
    {
        if (container.attached.find(window) != container.attached.end())
        {
            return container.zone.extents();
        }
    }

    return std::nullopt;
}

auto FrameWindowManagerPolicy::ensure_window_in_container(Window const& window) -> Rectangle
{
    if (containers.empty())
    {
        return tools.active_application_zone().extents();
    }

    for (auto const& container : containers)
    {
        if (container.attached.find(window) != container.attached.end())
        {
            return container.zone.extents();
        }
    }

    auto& container = container_for_rect({window.top_left(), window.size()});
    container.attached.insert(window);
    return container.zone.extents();
}

void FrameWindowManagerPolicy::ensure_window_not_in_container(Window const& window)
{
    for (auto& container : containers)
    {
        container.attached.erase(window);
    }
}

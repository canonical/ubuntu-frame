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

#include "frame_window_manager.h"
#include "layout_metadata.h"
#include "snap_name_of.h"

#include <mir/log.h>
#include <miral/application_info.h>
#include <miral/output.h>
#include <miral/toolkit_event.h>
#include <miral/version.h>
#include <miral/window_info.h>
#include <miral/window_manager_tools.h>

#include <linux/input.h>
#include <unistd.h>

#include <algorithm>

namespace ms = mir::scene;
using namespace miral;
using namespace miral::toolkit;

namespace
{
bool can_position_be_overridden(WindowSpecification& spec, WindowInfo const& window_info)
{
    // Only override behavior of windows of type normal and freestyle
    switch (spec.type().is_set() ? spec.type().value() : window_info.type())
    {
        case mir_window_type_normal:
        case mir_window_type_freestyle:
            break;

        default:
            return false;
    }

    // Only override behavior of windows without a parent
    if (spec.parent().is_set() ? spec.parent().value().lock() : window_info.parent())
    {
        return false;
    }

    // Only override behavior if the (new) state is something other than minimized, hidden or attached
    switch (spec.state().is_set() ? spec.state().value() : window_info.state())
    {
        case mir_window_state_minimized:
        case mir_window_state_hidden:
        case mir_window_state_attached:
            return false;
        default:;
    }

    return true;
}

void apply_fullscreen(WindowSpecification& spec)
{
    spec.state() = mir_window_state_fullscreen;
    spec.size() = mir::optional_value<Size>{};      // Ignore requested size (if any) when we fullscreen
    spec.top_left() = mir::optional_value<Point>{}; // Ignore requested position (if any) when we fullscreen
}

auto is_application(WindowInfo const& window_info)
{
    switch (window_info.depth_layer())
    {
    case mir_depth_layer_application:
    case mir_depth_layer_always_on_top:
        return window_info.state() != mir_window_state_attached;

    default:
        return false;
    }
}
}

auto WindowCount::increment_opened() -> unsigned short
{
    return total_opened++;
}

auto WindowCount::increment_closed() -> unsigned short
{
    return total_closed++;
}

auto WindowCount::currently_open() const -> unsigned short
{
    return total_opened - total_closed;
}

WindowManagerObserver::WindowManagerObserver()
{
}

void WindowManagerObserver::add_window_opened_callback(std::function<void()> const& callback)
{
    window_opened_callbacks.push_back(callback);
}

void WindowManagerObserver::add_window_closed_callback(std::function<void()> const& callback)
{
    window_closed_callbacks.push_back(callback);
}

void WindowManagerObserver::set_weak_window_count(std::shared_ptr<WindowCount> window_count)
{
    weak_window_count = window_count;
}

auto WindowManagerObserver::get_currently_open_windows() const -> unsigned int
{
    if (auto const window_count = weak_window_count.lock())
    {
        return window_count->currently_open();
    }

    return 0;
}

void WindowManagerObserver::process_window_opened_callbacks() const
{
    for (auto const& callback : window_opened_callbacks)
    {
        callback();
    }
}

void WindowManagerObserver::process_window_closed_callbacks() const
{
    for (auto const& callback : window_closed_callbacks)
    {
        callback();
    }
}

std::string const FrameWindowManagerPolicy::surface_title = "surface-title";
std::string const FrameWindowManagerPolicy::snap_name = "snap-name";

FrameWindowManagerPolicy::FrameWindowManagerPolicy(
    WindowManagerTools const& tools,
    WindowManagerObserver& window_manager_observer,
    miral::DisplayConfiguration const& display_config)
    : MinimalWindowManager{tools},
      window_manager_observer{window_manager_observer},
      display_config{display_config}
{
    window_manager_observer.set_weak_window_count(window_count);
}

bool FrameWindowManagerPolicy::handle_keyboard_event(MirKeyboardEvent const* event)
{
    return false;
}

void FrameWindowManagerPolicy::handle_layout(
    WindowSpecification& specification,
    Application const& application,
    WindowInfo const& window_info)
{
    // If a window's position cannot be overridden, we return the requested spec.
    if (!can_position_be_overridden(specification, window_info))
        return;

    auto const snap_instance_name = snap_instance_name_of(application);
    auto const surface_title = specification.name() ? specification.name() : window_info.name();
    auto const layout_userdata = display_config.layout_userdata("applications");
    std::shared_ptr<LayoutMetadata> layout_metadata;
    if (layout_userdata.has_value())
        layout_metadata = std::any_cast<std::shared_ptr<LayoutMetadata>>(layout_userdata.value());

    // If the snap name or surface title is mapped to a particular position and size, then the surface is placed there.
    if (layout_metadata && layout_metadata->try_layout(specification, surface_title, snap_instance_name))
    {
        // Let's warn if the user is placing their surface beyond the extents of all outputs
        Rectangle const extents(specification.top_left().value(), specification.size().value());
        bool found = false;
        for (auto const& output : active_outputs)
        {
            if (output.extents().overlaps(extents))
                found = true;;
        }

        if (!found)
            mir::log_warning(R"(Surface for snap="%s" with title="%s" was placed such that it overlaps no outputs)",
                              snap_instance_name.c_str(),
                              surface_title ? surface_title.value().c_str() : "");

        // Let's also warn if the user has also mapped this surface to a specific output
        WindowSpecification throwaway_spec;
        if (assign_to_output(throwaway_spec, surface_title, snap_instance_name))
            mir::log_warning(R"(Surface for snap="%s" with title="%s" is mapped to both a specific position)"
                              " and a specific card. The card mapping will be ignored.",
                              snap_instance_name.c_str(),
                              surface_title ? surface_title.value().c_str() : "");

        return;
    }

    // If the snap name or surface title is mapped to a particular output, then the surface is fullscreen on that output.
    if (assign_to_output(specification, surface_title, snap_instance_name))
    {
        if (specification.name())
        {
            if (!snap_instance_name.empty())
            {
                mir::log_info(R"(Surface for snap="%s" with title="%s")",
                              snap_instance_name.c_str(),
                              specification.name().value().c_str());
            }
            else
            {
                mir::log_info("Surface with title=\"%s\"",
                              specification.name().value().c_str());
            }
        }

        apply_fullscreen(specification);
        apply_bespoke_fullscreen_placement(specification, window_info);
    }
    else
    {
        // Otherwise, the snap appears fullscreen on whatever output is currently active.
        apply_fullscreen(specification);
        apply_bespoke_fullscreen_placement(specification, window_info);
    }
}

auto FrameWindowManagerPolicy::place_new_window(ApplicationInfo const& app_info, WindowSpecification const& request)
-> WindowSpecification
{
    WindowSpecification specification = MinimalWindowManager::place_new_window(app_info, request);
    WindowInfo const window_info{};
    handle_layout(specification, app_info.application(), window_info);

    // TODO This is a hack to ensure the wallpaper remains in the background
    // Ideally the wallpaper would use layer-shell, but there's no convenient -dev package
    // for that extension
    if (pid_of(app_info.application()) == getpid())
    {
        specification.depth_layer() = mir_depth_layer_background;
    }

    return specification;
}

bool FrameWindowManagerPolicy::assign_to_output(
    WindowSpecification& specification,
    mir::optional_value<std::string> const& title,
    std::string_view snap_name)
{
    return placement_mapping.set_output_for_surface(specification, title)
        || placement_mapping.set_output_for_snap(specification, snap_name);
}

void FrameWindowManagerPolicy::advise_delete_window(WindowInfo const& window_info)
{
    MinimalWindowManager::advise_delete_window(window_info);
    if (is_application(window_info))
    {
        window_count->increment_closed();
        window_manager_observer.process_window_closed_callbacks();
    }
}

void FrameWindowManagerPolicy::handle_modify_window(WindowInfo& window_info, WindowSpecification const& modifications)
{
    WindowSpecification specification = modifications;
    handle_layout(specification, window_info.window().application(), window_info);
    MinimalWindowManager::handle_modify_window(window_info, specification);
}

void FrameWindowManagerPolicy::apply_bespoke_fullscreen_placement(
    WindowSpecification& specification, WindowInfo const& window_info) const
{
    specification.state() = mir_window_state_maximized;
    tools.place_and_size_for_state(specification, window_info);
    specification.state() = mir_window_state_fullscreen;
}

auto FrameWindowManagerPolicy::confirm_placement_on_display(
    WindowInfo const& window_info,
    MirWindowState new_state,
    Rectangle const& new_placement) -> Rectangle
{
    if (new_state == mir_window_state_fullscreen)
    {
        WindowSpecification specification;
        specification.state() = mir_window_state_maximized;
        tools.place_and_size_for_state(specification, window_info);
        return {specification.top_left().value(), specification.size().value()};
    }
    return new_placement;
}

void FrameWindowManagerPolicy::advise_begin()
{
    WindowManagementPolicy::advise_begin();
}

void FrameWindowManagerPolicy::advise_end()
{
    WindowManagementPolicy::advise_end();

    if (display_layout_has_changed)
    {
        tools.for_each_application([this](auto& app)
            {
               for (auto& window : app.windows())
               {
                   if (window)
                   {
                       auto& info = tools.info_for(window);
                       WindowSpecification specification;
                       handle_layout(specification, app.application(), info);
                       tools.modify_window(info, specification);
                   }
               }
            });
        display_layout_has_changed = false;
    }

    if (application_zones_have_changed)
    {
        tools.for_each_application([this](auto& app)
            {
               for (auto& window : app.windows())
               {
                   if (window)
                   {
                       auto& info = tools.info_for(window);

                       if (info.state() == mir_window_state_fullscreen)
                       {
                           WindowSpecification specification;
                           specification.state() = mir_window_state_maximized;
                           tools.place_and_size_for_state(specification, info);
                           specification.state() = mir_window_state_fullscreen;
                           tools.modify_window(info, specification);
                       }
                   }
               }
            });

        application_zones_have_changed = false;
    }
}

void FrameWindowManagerPolicy::advise_application_zone_create(Zone const& application_zone)
{
    WindowManagementPolicy::advise_application_zone_create(application_zone);
    application_zones_have_changed = true;
}

void FrameWindowManagerPolicy::advise_application_zone_update(Zone const& updated, Zone const& original)
{
    WindowManagementPolicy::advise_application_zone_update(updated, original);
    application_zones_have_changed = true;
}

void FrameWindowManagerPolicy::advise_application_zone_delete(Zone const& application_zone)
{
    WindowManagementPolicy::advise_application_zone_delete(application_zone);
    application_zones_have_changed = true;
}

void FrameWindowManagerPolicy::advise_output_create(miral::Output const &output)
{
    WindowManagementPolicy::advise_output_create(output);

    placement_mapping.update(output);
    active_outputs.push_back(output);
    display_layout_has_changed = true;
}

void FrameWindowManagerPolicy::advise_output_delete(miral::Output const& output)
{
    WindowManagementPolicy::advise_output_delete(output);

    placement_mapping.clear(output);
    active_outputs.erase(std::remove_if(active_outputs.begin(), active_outputs.end(), [&output](miral::Output const& other)
    {
        return other.id() == output.id();
    }), active_outputs.end());
    display_layout_has_changed = true;
}

void FrameWindowManagerPolicy::advise_new_window(WindowInfo const& window_info)
{
    MinimalWindowManager::advise_new_window(window_info);
    if (is_application(window_info))
    {
        window_manager_observer.process_window_opened_callbacks();
        window_count->increment_opened();
    }
}

void FrameWindowManagerPolicy::advise_output_update(Output const& updated, Output const& /*original*/)
{
    placement_mapping.update(updated);
    display_layout_has_changed = true;
}

void FrameWindowManagerPolicy::PlacementMapping::update(Output const& output)
{
    auto const output_id = output.id();

    surface_title_to_output_id.erase(
        std::remove_if(
            begin(surface_title_to_output_id),
            end(surface_title_to_output_id),
            [output_id](auto const& e) { return e.second == output_id; }),
        end(surface_title_to_output_id));

    if (auto const attr = output.attribute(surface_title))
        surface_title_to_output_id.emplace_back(attr.value(), output_id);

    snap_name_to_output_id.erase(
        std::remove_if(
            begin(snap_name_to_output_id),
            end(snap_name_to_output_id),
            [output_id](auto const& e) { return e.second == output_id; }),
        end(snap_name_to_output_id));

    if (auto const attr = output.attribute(snap_name))
        snap_name_to_output_id.emplace_back(attr.value(), output_id);
}

void FrameWindowManagerPolicy::PlacementMapping::clear(Output const& output)
{
    auto const output_id = output.id();

    surface_title_to_output_id.erase(
        std::remove_if(
            begin(surface_title_to_output_id),
            end(surface_title_to_output_id),
            [output_id](auto const& e) { return e.second == output_id; }),
        end(surface_title_to_output_id));

    snap_name_to_output_id.erase(
        std::remove_if(
            begin(snap_name_to_output_id),
            end(snap_name_to_output_id),
            [output_id](auto const& e) { return e.second == output_id; }),
        end(snap_name_to_output_id));
}

bool FrameWindowManagerPolicy::PlacementMapping::set_output_for_surface(
    WindowSpecification& specification,
    mir::optional_value<std::string> const& title) const
{
    for (auto const& t2o : surface_title_to_output_id)
    {
        if (t2o.first == title)
        {
            specification.output_id() = t2o.second;
            return true;
        }
    }

    return false;
}

bool FrameWindowManagerPolicy::PlacementMapping::set_output_for_snap(
    WindowSpecification& specification,
    std::string_view name) const
{
    for (auto const& s2o : snap_name_to_output_id)
    {
        if (s2o.first == name)
        {
            specification.output_id() = s2o.second;
            return true;
        }
    }

    return false;
}

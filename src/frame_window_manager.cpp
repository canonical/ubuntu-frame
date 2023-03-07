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
bool needs_bespoke_fullscreen_placement(WindowSpecification& spec, WindowInfo const& window_info)
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

    // Only override behavior if the new state is something other than minimized, hidden or attached
    if (spec.state().is_set())
    {
        switch (spec.state().value())
        {
        case mir_window_state_minimized:
        case mir_window_state_hidden:
        case mir_window_state_attached:return false;

        default:;
        }
    }

    spec.state() = mir_window_state_fullscreen;
    spec.size() = mir::optional_value<Size>{};      // Ignore requested size (if any) when we fullscreen
    spec.top_left() = mir::optional_value<Point>{}; // Ignore requested position (if any) when we fullscreen

    return true;
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

FrameWindowManagerPolicy::FrameWindowManagerPolicy(WindowManagerTools const& tools, WindowManagerObserver& window_manager_observer)
    : MinimalWindowManager{tools},
      window_manager_observer{window_manager_observer}
{
    window_manager_observer.set_weak_window_count(window_count);
}

bool FrameWindowManagerPolicy::handle_keyboard_event(MirKeyboardEvent const* event)
{
    return false;
}

auto FrameWindowManagerPolicy::place_new_window(ApplicationInfo const& app_info, WindowSpecification const& request)
-> WindowSpecification
{
    WindowSpecification specification = MinimalWindowManager::place_new_window(app_info, request);

    {
        WindowInfo window_info{};
        if (needs_bespoke_fullscreen_placement(specification, window_info))
        {
            auto const snap_instance_name = snap_instance_name_of(app_info.application());

            if (specification.name())
            {
                if (!snap_instance_name.empty())
                {
                    mir::log_info("New surface for snap=\"%s\" with title=\"%s\"",
                                  snap_instance_name.c_str(),
                                  specification.name().value().c_str());
                }
                else
                {
                    mir::log_info("New surface with title=\"%s\"",
                                  specification.name().value().c_str());
                }
            }

            assign_to_output(specification, specification.name(), snap_instance_name);
            apply_bespoke_fullscreen_placement(specification, window_info);
        }
    }

    // TODO This is a hack to ensure the wallpaper remains in the background
    // Ideally the wallpaper would use layer-shell, but there's no convenient -dev package
    // for that extension
    if (pid_of(app_info.application()) == getpid())
    {
        specification.depth_layer() = mir_depth_layer_background;
    }

    return specification;
}

void FrameWindowManagerPolicy::assign_to_output(
    WindowSpecification& specification,
    mir::optional_value<std::string> const& title,
    std::string_view snap_name)
{
    placement_mapping.set_output_for_surface(specification, title);
    placement_mapping.set_output_for_snap(specification, snap_name);
}

void FrameWindowManagerPolicy::advise_delete_window(WindowInfo const& window_info)
{
    if (is_application(window_info))
    {
        window_count->increment_closed();
        window_manager_observer.process_window_closed_callbacks();
    }
}

void FrameWindowManagerPolicy::handle_modify_window(WindowInfo& window_info, WindowSpecification const& modifications)
{
    WindowSpecification specification = modifications;

    if (needs_bespoke_fullscreen_placement(specification, window_info))
    {
        auto const snap_instance_name = snap_instance_name_of(window_info.window().application());

        if (specification.name())
        {
            if (!snap_instance_name.empty())
            {
                mir::log_info(
                    "Surface for snap=\"%s\" retitled to \"%s\" (was \"%s\")",
                    snap_instance_name.c_str(),
                    specification.name().value().c_str(),
                    window_info.name().c_str());
            }
            else
            {
                mir::log_info(
                    "Surface retitled to \"%s\" (was \"%s\")",
                    specification.name().value().c_str(),
                    window_info.name().c_str());
            }
        }
        assign_to_output(specification, window_info.name(), snap_instance_name);

        apply_bespoke_fullscreen_placement(specification, window_info);
    }

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

                       if (needs_bespoke_fullscreen_placement(specification, info))
                       {
                           assign_to_output(specification, info.name(), snap_instance_name_of(info.window().application()));
                           apply_bespoke_fullscreen_placement(specification, info);
                           tools.modify_window(info, specification);
                       }
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
    display_layout_has_changed = true;
}

void FrameWindowManagerPolicy::advise_output_delete(miral::Output const& output)
{
    WindowManagementPolicy::advise_output_delete(output);

    placement_mapping.clear(output);
    display_layout_has_changed = true;
}

void FrameWindowManagerPolicy::advise_new_window(WindowInfo const& window_info)
{
    WindowManagementPolicy::advise_new_window(window_info);
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
#if MIRAL_VERSION >= MIR_VERSION_NUMBER(3, 8, 0)
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
#else
    (void)output;
#endif
}

void FrameWindowManagerPolicy::PlacementMapping::clear(Output const& output)
{
#if MIRAL_VERSION >= MIR_VERSION_NUMBER(3, 8, 0)
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
#endif
}

void FrameWindowManagerPolicy::PlacementMapping::set_output_for_surface(
    WindowSpecification& specification,
    mir::optional_value<std::string> const& title) const
{
    for (auto const& t2o : surface_title_to_output_id)
    {
        if (t2o.first == title)
        {
            specification.output_id() = t2o.second;
            break;
        }
    }
}

void FrameWindowManagerPolicy::PlacementMapping::set_output_for_snap(
    WindowSpecification& specification,
    std::string_view name) const
{
    for (auto const& s2o : snap_name_to_output_id)
    {
        if (s2o.first == name)
        {
            specification.output_id() = s2o.second;
            break;
        }
    }
}

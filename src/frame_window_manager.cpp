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

namespace ms = mir::scene;
using namespace miral;
using namespace miral::toolkit;

namespace
{
bool override_state(WindowSpecification& spec, WindowInfo const& window_info)
{
    // Only override state change if the state is being changed
    if (!spec.state().is_set())
    {
        return false;
    }

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
    switch (spec.state().value())
    {
    case mir_window_state_minimized:
    case mir_window_state_hidden:
    case mir_window_state_attached:
        return false;

    default:;
    }

    spec.state() = mir_window_state_fullscreen;
    spec.size() = mir::optional_value<Size>{};      // Ignore requested size (if any) when we fullscreen
    spec.top_left() = mir::optional_value<Point>{}; // Ignore requested position (if any) when we fullscreen

    return true;
}
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
        if (override_state(specification, window_info))
        {
            specification.state() = mir_window_state_maximized;
            tools.place_and_size_for_state(specification, window_info);
            specification.state() = mir_window_state_fullscreen;
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

void FrameWindowManagerPolicy::handle_modify_window(WindowInfo& window_info, WindowSpecification const& modifications)
{
    WindowSpecification specification = modifications;

    if (override_state(specification, window_info))
    {
        specification.state() = mir_window_state_maximized;
        tools.place_and_size_for_state(specification, window_info);
        specification.state() = mir_window_state_fullscreen;
    }

    MinimalWindowManager::handle_modify_window(window_info, specification);
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

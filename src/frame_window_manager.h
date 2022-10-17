/*
 * Copyright © 2016-2022 Canonical Ltd.
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

#ifndef FRAME_WINDOW_MANAGER_H
#define FRAME_WINDOW_MANAGER_H

#include <miral/minimal_window_manager.h>

#include <mir_toolkit/events/enums.h>

using namespace mir::geometry;

class FrameWindowManagerPolicy : public miral::MinimalWindowManager
{
public:
    using miral::MinimalWindowManager::MinimalWindowManager;

    auto place_new_window(miral::ApplicationInfo const& app_info, miral::WindowSpecification const& request)
    -> miral::WindowSpecification override;

    bool handle_keyboard_event(MirKeyboardEvent const* event) override;
    void handle_modify_window(miral::WindowInfo& window_info, miral::WindowSpecification const& modifications) override;

    auto confirm_placement_on_display(const miral::WindowInfo& window_info, MirWindowState new_state,
        Rectangle const& new_placement) -> Rectangle override;

    void advise_begin() override;
    void advise_end() override;
    void advise_application_zone_create(miral::Zone const& application_zone) override;
    void advise_application_zone_update(miral::Zone const& updated, miral::Zone const& original) override;
    void advise_application_zone_delete(miral::Zone const& application_zone) override;
    void advise_output_create(miral::Output const &output) override;
    void advise_output_delete(miral::Output const &output) override;

private:
    bool application_zones_have_changed = false;
    int output_count = 0;
    // This only used for modulo, so wrap-around is desired
    unsigned short window_count = 0;
};

#endif /* FRAME_WINDOW_MANAGER_H */

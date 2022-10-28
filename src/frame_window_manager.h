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

#include <memory>
#include <vector>

using namespace mir::geometry;

class WindowCount
{
public:
    // Intrements and returns amount of total windows opened
    auto increment_opened() -> unsigned short;

    // Increments and returns amount of total windows closed
    auto increment_closed() -> unsigned short;

    // Returns number of currently open windows
    auto currently_open() const -> unsigned short;

private:
    unsigned short total_opened = 0;
    unsigned short total_closed = 0;
};

class WindowManagerObserver
{
public:
    WindowManagerObserver();

    void add_window_opened_callback(std::function<void()> const& callback);
    
    void add_window_closed_callback(std::function<void()> const& callback);

    void set_weak_window_count(std::shared_ptr<WindowCount> window_count);

    auto get_currently_open_windows() const -> unsigned int;

private:
    friend class FrameWindowManagerPolicy;

    void process_window_opened_callbacks() const;

    void process_window_closed_callbacks() const;

    std::vector<std::function<void()>> window_opened_callbacks;
    std::vector<std::function<void()>> window_closed_callbacks;
    std::weak_ptr<WindowCount> weak_window_count;
};

class FrameWindowManagerPolicy : public miral::MinimalWindowManager
{
public:
    using miral::MinimalWindowManager::MinimalWindowManager;

    FrameWindowManagerPolicy(miral::WindowManagerTools const& tools, WindowManagerObserver& window_manager_observer);

    auto place_new_window(miral::ApplicationInfo const& app_info, miral::WindowSpecification const& request)
    -> miral::WindowSpecification override;

    bool handle_keyboard_event(MirKeyboardEvent const* event) override;
    void handle_modify_window(miral::WindowInfo& window_info, miral::WindowSpecification const& modifications) override;

    auto confirm_placement_on_display(const miral::WindowInfo& window_info, MirWindowState new_state,
        Rectangle const& new_placement) -> Rectangle override;

    void advise_delete_window(miral::WindowInfo const& /*window_info*/) override;
    void advise_begin() override;
    void advise_end() override;
    void advise_application_zone_create(miral::Zone const& application_zone) override;
    void advise_application_zone_update(miral::Zone const& updated, miral::Zone const& original) override;
    void advise_application_zone_delete(miral::Zone const& application_zone) override;
    void advise_output_create(miral::Output const &output) override;
    void advise_output_delete(miral::Output const &output) override;

private:
    WindowManagerObserver const& window_manager_observer;
    std::shared_ptr<WindowCount> window_count = std::make_shared<WindowCount>();

    bool application_zones_have_changed = false;
    std::vector<int> outputs = {};
};

#endif /* FRAME_WINDOW_MANAGER_H */

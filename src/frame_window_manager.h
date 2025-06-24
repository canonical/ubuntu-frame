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

#ifndef FRAME_WINDOW_MANAGER_H
#define FRAME_WINDOW_MANAGER_H

#include <miral/minimal_window_manager.h>
#include <miral/display_configuration.h>
#include <miral/output.h>
#include <mir_toolkit/events/enums.h>

#include <memory>
#include <vector>
#include <optional>

using namespace mir::geometry;

class LayoutMetadata;

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

    static std::string const surface_title;
    static std::string const snap_name;

    FrameWindowManagerPolicy(
        miral::WindowManagerTools const& tools,
        WindowManagerObserver& window_manager_observer,
        miral::DisplayConfiguration const& display_config);

    auto place_new_window(miral::ApplicationInfo const& app_info, miral::WindowSpecification const& request)
    -> miral::WindowSpecification override;

    void handle_window_ready(miral::WindowInfo& window_info) override;

    bool handle_keyboard_event(MirKeyboardEvent const* event) override;
    void handle_modify_window(miral::WindowInfo& window_info, miral::WindowSpecification const& modifications) override;

    auto confirm_placement_on_display(const miral::WindowInfo& window_info, MirWindowState new_state,
        Rectangle const& new_placement) -> Rectangle override;

    void advise_delete_window(miral::WindowInfo const& /*window_info*/) override;

    void advise_new_window(miral::WindowInfo const& window_info) override;

    void advise_begin() override;
    void advise_end() override;
    void advise_application_zone_create(miral::Zone const& application_zone) override;
    void advise_application_zone_update(miral::Zone const& updated, miral::Zone const& original) override;
    void advise_application_zone_delete(miral::Zone const& application_zone) override;
    void advise_output_create(miral::Output const &output) override;
    void advise_output_delete(miral::Output const &output) override;

    void advise_output_update(miral::Output const& updated, miral::Output const& original) override;

private:
    WindowManagerObserver const& window_manager_observer;
    std::shared_ptr<WindowCount> window_count = std::make_shared<WindowCount>();
    miral::DisplayConfiguration display_config;

    bool application_zones_have_changed = false;
    bool display_layout_has_changed = false;

    class PlacementMapping
    {
    public:
        void update(miral::Output const& output);
        void clear(miral::Output const& output);

        bool set_output_for_surface(miral::WindowSpecification& specification, mir::optional_value<std::string> const& title) const;
        bool set_output_for_snap(miral::WindowSpecification& specification, std::string_view name) const;

    private:
        std::vector<std::pair<std::string, int>> surface_title_to_output_id;
        std::vector<std::pair<std::string, int>> snap_name_to_output_id;
    } placement_mapping;

    std::vector<miral::Output> active_outputs;

    void handle_layout(
        miral::WindowSpecification& spec,
        miral::Application const& application_info,
        miral::WindowInfo& info);

    /// Try to assign the window to an output given its title and snap name.
    /// \returns true if successfully assigned, otherwise false
    bool assign_to_output(
        miral::WindowSpecification& specification,
        mir::optional_value<std::string> const& title,
        std::string_view snap_name);

    void apply_bespoke_fullscreen_placement(
        miral::WindowSpecification& specification,
        miral::WindowInfo const& window_info) const;

    bool try_position_exactly(
        miral::WindowSpecification& spec,
        miral::WindowInfo const& window_info,
        miral::Application const& application) const;
};

#endif /* FRAME_WINDOW_MANAGER_H */

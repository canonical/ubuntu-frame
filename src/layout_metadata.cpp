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

#include "layout_metadata.h"
#include <mir/log.h>
#include <miral/display_configuration.h>

namespace
{
bool try_parse_vec2(miral::DisplayConfigurationNode const& node, const char* field_name, int& x, int& y)
{
    if (!node.at(field_name))
    {
        mir::log_error("Invalid application strategy: '%s' is required", field_name);
        return false;
    }

    auto field = node.at(field_name);
    std::vector<int> integers;
    field.value()->for_each([&integers](std::unique_ptr<miral::DisplayConfigurationNode> const& node)
    {
        if (auto int_field = node->as_int())
            integers.push_back(int_field.value());
    });

    if (integers.size() != 2)
    {
        mir::log_error("Invalid application strategy: '%s' must have a length of 2, but has length %lu",
            field_name, integers.size());
        return false;
    }

    x = integers[0];
    y = integers[1];
    return true;
}
}

LayoutMetadata::LayoutMetadata(std::unique_ptr<miral::DisplayConfigurationNode> node)
{
    if (auto const applications_node = node->at("applications"))
    {
        applications_node.value()->for_each([&](std::unique_ptr<miral::DisplayConfigurationNode> node)
        {
            if (auto const app = LayoutApplicationPlacementStrategy::from_yaml(*node))
                applications.push_back(app.value());
        });
    }
}

bool LayoutMetadata::try_layout(miral::WindowSpecification& specification,
    mir::optional_value<std::string> const& title,
    std::string_view snap_name) const
{
    for (auto const& app : applications)
    {
        if (app.snap_name == snap_name || app.surface_title == title)
        {
            specification.state() = mir_window_state_restored;
            specification.top_left() = app.position;
            specification.size() = app.size;
            return true;
        }
    }

    return false;
}

LayoutMetadata::LayoutApplicationPlacementStrategy::LayoutApplicationPlacementStrategy(
    std::optional<std::string> const& snap_name,
    std::optional<std::string> const& surface_title,
    mir::geometry::Point const& position,
    mir::geometry::Size const& size)
    : snap_name(snap_name),
      surface_title(surface_title),
      position(position),
      size(size)
{}

std::optional<LayoutMetadata::LayoutApplicationPlacementStrategy> LayoutMetadata::LayoutApplicationPlacementStrategy::from_yaml(
    miral::DisplayConfigurationNode const& node)
{
    if (!node.at("snap-name") && !node.at("surface-title"))
    {
        mir::log_error("Invalid application strategy: missing snap-name and surface-title. One of them"
                       " must be provided.");
        return std::nullopt;
    }

    if (node.at("snap-name") && node.at("surface-title"))
    {
        mir::log_error("Invalid application strategy: provided both snap-name and surface-title, but"
                       " only one of them can be provided.");
        return std::nullopt;
    }

    std::optional<std::string> snap_name;
    std::optional<std::string> surface_title;

    if (node.at("snap-name"))
        snap_name = node.at("snap-name").value()->as_string();
    else
        surface_title = node.at("surface-title").value()->as_string();

    int x, y;
    int w, h;

    if (!try_parse_vec2(node, "position", x, y))
        return std::nullopt;

    if (!try_parse_vec2(node, "size", w, h))
        return std::nullopt;

    return LayoutApplicationPlacementStrategy(
        snap_name,
        surface_title,
        {x, y},
        {w, h});
}
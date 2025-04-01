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
#include <yaml-cpp/yaml.h>
#include <mir/log.h>

namespace
{
bool try_parse_vec2(YAML::Node const& node, const char* field_name, int& x, int& y)
{
    if (!node[field_name])
    {
        mir::log_error("Invalid application strategy: '%s' is required", field_name);
        return false;
    }

    if (!node[field_name].IsSequence())
    {
        mir::log_error("Invalid application strategy: '%s' must be a sequence", field_name);
        return false;
    }

    if (node[field_name].size() != 2)
    {
        mir::log_error("Invalid application strategy: '%s' must have a length of 2, but has length %lu",
            field_name, node[field_name].size());
        return false;
    }

    try
    {
        x = node[field_name][0].as<int>();
        y = node[field_name][1].as<int>();
        return true;
    }
    catch (YAML::Exception const& e)
    {
        mir::log_error("Invalid application strategy: failed to parse %s to values %s", field_name, e.what());
        return false;
    }
}
}

LayoutMetadata::LayoutMetadata(std::vector<LayoutApplicationPlacementStrategy> const& applications)
    : applications(applications)
{
}

std::shared_ptr<LayoutMetadata> LayoutMetadata::from_yaml(YAML::Node const& layout_node)
{
    std::vector<LayoutApplicationPlacementStrategy> applications;

    if (layout_node["applications"] && layout_node["applications"].IsSequence())
    {
        for (auto const& app_node : layout_node["applications"])
        {
            if (auto const app = LayoutApplicationPlacementStrategy::from_yaml(app_node))
                applications.push_back(app.value());
        }
    }

    return std::make_shared<LayoutMetadata>(applications);
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

LayoutApplicationPlacementStrategy::LayoutApplicationPlacementStrategy(
    std::optional<std::string> const& snap_name,
    std::optional<std::string> const& surface_title,
    mir::geometry::Point const& position,
    mir::geometry::Size const& size)
    : snap_name(snap_name),
      surface_title(surface_title),
      position(position),
      size(size)
{}

std::optional<LayoutApplicationPlacementStrategy> LayoutApplicationPlacementStrategy::from_yaml(YAML::Node const& node)
{
    if (!node["snap-name"] && !node["surface-title"])
    {
        mir::log_error("Invalid application strategy: missing snap-name and surface-title. One of them"
                       " must be provided.");
        return std::nullopt;
    }

    if (node["snap-name"] && node["surface-title"])
    {
        mir::log_error("Invalid application strategy: provided both snap-name and surface-title, but"
                       " only one of them can be provided.");
        return std::nullopt;
    }

    std::optional<std::string> snap_name;
    std::optional<std::string> surface_title;

    if (node["snap-name"])
    {
        if (!node["snap-name"].IsScalar())
        {
            mir::log_error("Invalid application strategy: snap-name should be a string");
            return std::nullopt;
        }

        snap_name = node["snap-name"].Scalar();
    }
    else
    {
        if (!node["surface-title"].IsScalar())
        {
            mir::log_error("Invalid application strategy: surface-title should be a string");
            return std::nullopt;
        }

        surface_title = node["surface-title"].Scalar();
    }

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
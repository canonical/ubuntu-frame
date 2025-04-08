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

#ifndef LAYOUT_METADATA_H
#define LAYOUT_METADATA_H

#include <optional>
#include <string>
#include <vector>
#include <mir/geometry/point.h>
#include <mir/geometry/size.h>
#include <miral/window_specification.h>
#include <miral/display_configuration.h>

class LayoutMetadata
{
public:
    explicit LayoutMetadata(miral::DisplayConfiguration::Node const& layout_node);

    /// Try to assign the window to a positition and size based on its title and snap name.
    /// \returns true if successfully assigned, otherwise false
    bool try_layout(miral::WindowSpecification& specification,
        mir::optional_value<std::string> const& title,
        std::string_view snap_name) const;

private:
    class LayoutApplicationPlacementStrategy
    {
    public:
        LayoutApplicationPlacementStrategy(
            std::optional<std::string> const& snap_name,
            std::optional<std::string> const& surface_title,
            mir::geometry::Point const& position,
            mir::geometry::Size const& size);
        static std::optional<LayoutApplicationPlacementStrategy> from_yaml(miral::DisplayConfiguration::Node const& node);

        std::optional<std::string> const snap_name;
        std::optional<std::string> const surface_title;
        mir::geometry::Point const position;
        mir::geometry::Size const size;
    };

    std::vector<LayoutApplicationPlacementStrategy> applications;

};


#endif //LAYOUT_METADATA_H

/*
 * Copyright © 2016-2018 Octopull Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#ifndef EGMDE_EGWALLPAPER_H
#define EGMDE_EGWALLPAPER_H

#include <miral/application.h>
#include <miral/window.h>

#include <memory>
#include <mutex>
#include <string>

struct wl_display;
namespace egmde
{
class Wallpaper
{
public:
    void operator()(wl_display* display);
    void operator()(std::weak_ptr<mir::scene::Session> const& session);

    auto session() const -> std::shared_ptr<mir::scene::Session>;

    void stop();

    // Used in initialization to set colour
    void bottom(std::string const& option);
    void top(std::string const& option);

private:
    std::mutex mutable mutex;
    std::weak_ptr<mir::scene::Session> weak_session;

    uint8_t bottom_colour[4] = { 0x0a, 0x24, 0x77, 0xFF };
    uint8_t top_colour[4] = { 0x00, 0x00, 0x00, 0xFF };

    struct Self;
    std::weak_ptr<Self> self;
};
}

#endif //EGMDE_EGWALLPAPER_H

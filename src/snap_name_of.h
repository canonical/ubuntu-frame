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

#ifndef FRAME_SNAP_NAME_OF_H
#define FRAME_SNAP_NAME_OF_H

#include <miral/application.h>

#include <string>

auto snap_name_of(miral::Application const& app, bool fallback_without_apparmor) -> std::string;
auto snap_instance_name_of(miral::Application const& app) -> std::string;

#endif // FRAME_SNAP_NAME_OF_H

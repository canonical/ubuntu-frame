/*
 * Copyright © Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 or 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import 'package:ubuntu_frame_launcher/models/desktop_file.dart';
import 'package:ubuntu_frame_launcher/services/window_handle.dart';

import '../services/window_service.dart';

class WindowController {
  WindowService service;

  WindowController(this.service);

  WindowHandle? getWindowById(String appId) {
    return service.getWindowById(appId);
  }

  List<WindowHandle> getWindowList() {
    return service.getWindowList();
  }

  bool isOpen(DesktopFile file) {
    return service.getWindowById(file.id) != null;
  }
}

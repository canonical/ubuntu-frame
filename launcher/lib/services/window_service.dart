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

import '../models/window_event.dart';
import 'window_handle.dart';
import 'window_watcher_service.dart';

class WindowService {
  final WindowWatcherService watcher;
  final List<WindowHandle> _windowList = [];

  WindowService(this.watcher) {
    watcher.listen((event) {
      switch (event.eventType) {
        case WindowEventType.focused:
          _onWindowFocused(event.handle!);
          break;
        case WindowEventType.removed:
          _onWindowRemoved(event.handle!);
          break;
        case WindowEventType.created:
          _onWindowCreated(event.handle!);
          break;
        default:
          break;
      }
    });
  }

  void _onWindowCreated(WindowHandle handle) {
    _windowList.add(handle);
  }

  void _onWindowRemoved(WindowHandle handle) {
    _windowList.remove(handle);
  }

  void _onWindowFocused(WindowHandle handle) {
    // Algoritm: Swap this handle to the position of the first instance
    // of this DesktopFile in the list, such that it is first
    // in the focus order AND the window creation order is maintained.
    int firtInstanceIndex = -1;
    int handleIndex = -1;
    for (int i = 0; i < _windowList.length; i++) {
      if (firtInstanceIndex < 0 &&
          _windowList[i].getAppId() == handle.getAppId()) {
        firtInstanceIndex = i;
      }
      if (handleIndex < 0 && _windowList[i] == handle) {
        handleIndex = i;
      }

      if (handleIndex > -1 && firtInstanceIndex > -1) {
        break;
      }
    }

    if (handleIndex < 0 || firtInstanceIndex < 0) {
      return;
    }

    final temp = _windowList[handleIndex];
    _windowList[handleIndex] = _windowList[firtInstanceIndex];
    _windowList[firtInstanceIndex] = temp;
  }

  List<WindowHandle> getWindowList() {
    // This function returns a copy of the list to combat the
    // possibility that the caller will use async functions
    // while the _windowList gets modified in the background.
    return [..._windowList];
  }

  WindowHandle? getWindowById(String appId) {
    final windowList = getWindowList();
    for (int i = 0; i < windowList.length; i++) {
      final handle = windowList[i];
      if (handle.getAppId() == appId) {
        return handle;
      }
    }
    return null;
  }
}

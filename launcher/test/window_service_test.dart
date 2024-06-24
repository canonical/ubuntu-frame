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

import 'package:ubuntu_frame_launcher/models/window_event.dart';
import 'package:ubuntu_frame_launcher/services/window_handle.dart';
import 'package:ubuntu_frame_launcher/services/window_service.dart';
import 'package:ubuntu_frame_launcher/services/window_watcher_service.dart';
import 'package:mockito/mockito.dart';
import 'package:test/test.dart';

class MockWindowWatcherService extends WindowWatcherService {}

class MockWindowHandle extends Mock implements WindowHandle {
  String appId;

  MockWindowHandle(this.appId);

  @override
  String getAppId() {
    return appId;
  }
}

void main() {
  late WindowWatcherService watcher;
  late WindowService windowService;

  setUp(() {
    watcher = MockWindowWatcherService();
    windowService = WindowService(watcher);
  });

  test(
      "When a window is focused, then it is swapped with the first occurence of the window with the same app id in the list",
      () async {
    const firstAppId = "first";
    const secondAppId = "second";
    MockWindowHandle firstHandle = MockWindowHandle(firstAppId);
    MockWindowHandle secondHandle = MockWindowHandle(secondAppId);
    MockWindowHandle thirdHandle = MockWindowHandle(firstAppId);

    watcher.controller.add(WindowEvent(WindowEventType.created, firstHandle));
    watcher.controller.add(WindowEvent(WindowEventType.created, secondHandle));
    watcher.controller.add(WindowEvent(WindowEventType.created, thirdHandle));

    watcher.controller.add(WindowEvent(WindowEventType.focused, thirdHandle));

    // We need to give the asynchronous "listen" time to run its events
    await Future.delayed(const Duration(milliseconds: 0));

    expect(windowService.getWindowList()[0], thirdHandle);
    expect(windowService.getWindowList()[1], secondHandle);
    expect(windowService.getWindowList()[2], firstHandle);
  });
}

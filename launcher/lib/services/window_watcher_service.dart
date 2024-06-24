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

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:ubuntu_frame_launcher/models/window_event.dart';

class WindowWatcherService extends Stream<WindowEvent> {
  final controller = StreamController<WindowEvent>.broadcast();

  @override
  StreamSubscription<WindowEvent> listen(Function(WindowEvent event)? onData,
      {Function? onError, VoidCallback? onDone, bool? cancelOnError}) {
    return controller.stream.listen(onData,
        onDone: onDone, onError: onError, cancelOnError: cancelOnError);
  }
}

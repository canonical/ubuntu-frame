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

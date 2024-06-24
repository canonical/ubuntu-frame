import 'package:ubuntu_frame_launcher/services/window_handle.dart';

enum WindowEventType { created, removed, focused, appId }

class WindowEvent {
  WindowEventType eventType;
  WindowHandle? handle;
  String? appId;

  WindowEvent(this.eventType, [this.handle, this.appId]);
}

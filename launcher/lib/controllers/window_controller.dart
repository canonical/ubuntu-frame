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

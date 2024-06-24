import 'dart:async';

import 'package:logging/logging.dart';
import 'package:ubuntu_frame_launcher/models/desktop_file.dart';
import 'package:ubuntu_frame_launcher/models/window_event.dart';
import 'package:ubuntu_frame_launcher/services/window_service.dart';

import '../services/desktop_file_manager.dart';

class ApplicationController {
  final WindowService windowService;
  final DesktopFileManager desktopFileManager;

  final _controller = StreamController<List<DesktopFile>>.broadcast();
  late Future<List<DesktopFile>> _installedAppFuture;
  final _logger = Logger("ApplicationController");

  ApplicationController(this.windowService, this.desktopFileManager) {
    _installedAppFuture = _loadInstalledApplications();

    windowService.watcher.listen((event) async {
      if (event.eventType == WindowEventType.created ||
          event.eventType == WindowEventType.removed) {
        _controller.add(await getOpenApplications());
      }
    });
  }

  Future<List<DesktopFile>> _loadInstalledApplications() async {
    final applications = await desktopFileManager.getAllDesktopFiles();
    applications.sort((a, b) {
      final aName = a.name?.toLowerCase() ?? '';
      final bName = b.name?.toLowerCase() ?? '';
      return aName.compareTo(bName);
    });
    return applications;
  }

  /// Returns a list of applications installed on the system. This list
  /// is guaranteed to be sorted alphabetically by name.
  Future<List<DesktopFile>> getInstalledApplications() async {
    // TODO: Watch directories and dynamically reload if they change
    return await _installedAppFuture;
  }

  void focus(DesktopFile file) {
    final handle = windowService.getWindowById(file.id);
    if (handle != null) {
      handle.activate();
      return;
    }

    _logger.shout("Unable to focus desktop file with id = ${file.id}}");
  }

  Stream<List<DesktopFile>> getOpenedAppsStream() {
    return _controller.stream;
  }

  Future<List<DesktopFile>> getOpenApplications() async {
    final List<DesktopFile> result = [];
    for (final handle in windowService.getWindowList()) {
      result.add(await desktopFileManager.resolveId(handle.getAppId()));
    }

    return result;
  }

  static DesktopFile? findDesktopFile(
      List<DesktopFile> list, DesktopFile file) {
    return list
        .cast<DesktopFile?>()
        .firstWhere((other) => other!.id == file.id, orElse: () => null);
  }
}

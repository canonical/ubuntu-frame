import 'dart:io';

import 'package:logging/logging.dart';
import 'package:ubuntu_frame_launcher/models/application_desktop_file.dart';
import 'package:ubuntu_frame_launcher/models/desktop_file.dart';
import 'package:ubuntu_frame_launcher/models/null_desktop_file.dart';
import 'package:path/path.dart' as p;
import 'package:xdg_directories/xdg_directories.dart';

class DesktopFileManager {
  late Future<List<DesktopFile>> _filePromise;
  final Map<String, DesktopFile> _idToApp = {};
  final List<String> _desktopEnvironmentList = [];
  final _logger = Logger("DesktopFileManager");

  DesktopFileManager() {
    final environment = Platform.environment["XDG_CURRENT_DESKTOP"];
    if (environment != null) {
      for (final env in environment.split(':')) {
        _desktopEnvironmentList.add(env);
      }
    }
    _filePromise = _loadInternal();
  }

  Future<List<DesktopFile>> getAllDesktopFiles() async {
    return await _filePromise;
  }

  Future<List<DesktopFile>> resolveIdList(List<String> idToList) async {
    List<DesktopFile> result = [];
    for (final id in idToList) {
      result.add(await resolveId(id));
    }
    return result;
  }

  Future<DesktopFile> resolveId(String id) async {
    if (_idToApp.containsKey(id)) {
      return _idToApp[id]!;
    }

    for (final dataDir in dataDirs) {
      final f = File(p.join(dataDir.path, 'applications', id));
      final app = await _loadFromPath(f, false);
      if (app != null) {
        _idToApp[id] = app;
        return app;
      }
    }

    final nullFile = NullDesktopFile(id);
    _idToApp[id] = nullFile;
    return nullFile;
  }

  Future<List<DesktopFile>> _loadInternal() async {
    final desktopFiles = <DesktopFile>[];
    for (final dataDir in dataDirs) {
      final dir = Directory(p.join(dataDir.path, 'applications'));
      if (!await dir.exists()) {
        continue;
      }

      await for (final f in dir.list()) {
        final app = await _loadFromPath(f, true);
        if (app != null) {
          desktopFiles.add(app);
          _idToApp[app.id] = app;
        }
      }
    }

    return desktopFiles;
  }

  Future<DesktopFile?> _loadFromPath(
      FileSystemEntity f, bool ensureEnvironmentValidity) async {
    // TODO: Handle file overrides, i.e. the same desktop file in different locations
    if (!await f.exists()) {
      return null;
    }

    if (!f.path.endsWith('.desktop')) {
      return null;
    }

    final desktopFile = ApplicationFileDesktopFile(
        f.path, Uri.parse(f.path).path.split("/").last);
    if (!await desktopFile.load()) {
      _logger.shout("Unable to load desktop file: ${f.path}");
      return null;
    }

    if (ensureEnvironmentValidity) {
      if (desktopFile.noDisplay) {
        return null;
      }

      final desktopEnvironmentList = desktopFile.onlyShowIn;
      if (desktopEnvironmentList != null) {
        for (final env in desktopEnvironmentList) {
          if (_desktopEnvironmentList.contains(env)) {
            return desktopFile;
          }
        }

        return null;
      }
    }

    return desktopFile;
  }
}

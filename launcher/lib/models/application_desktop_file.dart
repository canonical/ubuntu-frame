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

import 'package:flutter/services.dart';
import 'package:get_it/get_it.dart';
import 'package:ini/ini.dart';
import 'package:logging/logging.dart';
import 'dart:io';

import 'desktop_file.dart';

/// Represents an application that can be loaded from the fileystem
class ApplicationFileDesktopFile extends DesktopFile {
  final String applicationPath;
  final String identifier;
  late Logger _logger;
  Config config = Config();
  final launcherChannel = const MethodChannel('launcher');
  String? _displayName;

  ApplicationFileDesktopFile(this.applicationPath, this.identifier) {
    _logger = Logger("DesktopFile $id");
  }

  Future<bool> load() async {
    try {
      final file = File(applicationPath);
      config = Config.fromStrings(await file.readAsLines());
      return true;
    } catch (e) {
      _logger
          .shout("Failed to load application from path $applicationPath: $e");
      return false;
    }
  }

  @override
  String get id {
    return identifier;
  }

  @override
  String get path {
    return applicationPath;
  }

  @override
  String? get type {
    return _getDesktopEntry('Type');
  }

  String? get domain {
    return _getDesktopEntry('X-Ubuntu-Gettext-Domain');
  }

  @override
  String? get name {
    return _getDesktopEntry('Name');
  }

  @override
  String get icon {
    return _getDesktopEntry('Icon') ?? "application-x-executable";
  }

  @override
  bool get noDisplay {
    return _parseBool(_getDesktopEntry('NoDisplay'));
  }

  @override
  String? get exec {
    return _getDesktopEntry('Exec');
  }

  String? _getDesktopEntry(String key) {
    return config.get('Desktop Entry', key);
  }

  @override
  bool get singleMainWindow {
    return _parseBool(_getDesktopEntry("SingleMainWindow")) ||
        _parseBool(_getDesktopEntry("X-GNOME-SingleWindow"));
  }

  List<String>? get onlyShowIn {
    return _parseStringList(_getDesktopEntry("OnlyShowIn"));
  }

  static bool _parseBool(String? value) {
    return (value ?? "false") == "true";
  }

  static List<String>? _parseStringList(String? value) {
    if (value == null) {
      return null;
    }

    return value.split(";");
  }

  @override
  void launch() {
    launcherChannel.invokeMethod('launch', path);
  }

  @override
  String toString() => '$runtimeType($path)';
}

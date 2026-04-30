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
import 'dart:io';

import 'package:ini/ini.dart';
import 'package:logging/logging.dart';
import 'package:ubuntu_frame_launcher/models/accessibility_option.dart';

class AccessibilityController {
  final _logger = Logger('AccessibilityController');

  bool _isExpanded = false;
  bool get isExpanded => _isExpanded;

  final List<AccessibilityOption> options;

  final _controller = StreamController<bool>.broadcast();

  AccessibilityController({required this.options});

  /// Pending write chain — each write is appended here so they are
  /// executed strictly in order and never overlap.
  Future<void> _writeFuture = Future.value();

  Stream<bool> getStream() => _controller.stream;

  final _accessibilityConfigKey =
      "UBUNTU_FRAME_LAUNCHER_ACCESSIBILITY_CONFIG_PATH";

  /// The path at which the accessibility INI file is written. Reads the
  /// [UBUNTU_FRAME_LAUNCHER_ACCESSIBILITY_CONFIG_PATH] environment variable;
  /// returns an empty string (disabling file I/O) if the variable is not set.
  String get _configPath => Platform.environment[_accessibilityConfigKey] ?? '';

  /// Reads the accessibility config file on disk and sets each option's
  /// current value. Options absent from the file, or with an unrecognised
  /// value, are left at their default (index 0).
  Future<void> initialize() async {
    if (_configPath.isEmpty) {
      _logger.warning(
          '$_accessibilityConfigKey is not set; skipping startup read');
      return;
    }

    final file = File(_configPath);
    if (!await file.exists()) {
      _logger.info('No accessibility config file found at $_configPath; '
          'all options will use their defaults');
      return;
    }

    final Map<String, String> storedValues = {};
    try {
      final contents = await file.readAsString();
      final config = Config.fromString(contents);
      const section = 'default';
      for (final key in config.options(section)!) {
        storedValues[key] = config.get(section, key)!;
      }
    } catch (e, stackTrace) {
      _logger.shout('Failed to read accessibility config from $_configPath', e,
          stackTrace);
      return;
    }

    for (final option in options) {
      final stored = storedValues[option.id];
      if (stored == null) {
        _logger
            .info('Option "${option.id}" not found in config; keeping default');
        continue;
      }
      option.setValueFromString(stored);
    }
  }

  void expand() {
    _isExpanded = true;
    _controller.add(true);
  }

  void collapse() {
    _isExpanded = false;
    _controller.add(false);
  }

  void cycleForward(AccessibilityOption option) {
    option.cycleForward();
    _enqueueWrite();
  }

  void cycleBackward(AccessibilityOption option) {
    option.cycleBackward();
    _enqueueWrite();
  }

  /// Snapshot the current option values and append an atomic write to the
  /// serial queue. Any number of rapid calls will be ordered correctly
  /// because each write is chained onto the previous one.
  void _enqueueWrite() {
    // Capture the values *now*, before any await, so the write reflects
    // the state at the moment this cycle happened.
    final snapshot = {for (final opt in options) opt.id: opt.currentValue};
    _writeFuture = _writeFuture.then((_) => _writeSnapshot(snapshot));
  }

  Future<void> _writeSnapshot(Map<String, String> snapshot) async {
    if (_configPath.isEmpty) return;

    final buffer = StringBuffer();
    for (final entry in snapshot.entries) {
      buffer.writeln('${entry.key} = ${entry.value}');
    }

    final target = File(_configPath);
    final tmp = File('$_configPath.tmp');
    try {
      await target.parent.create(recursive: true);
      await tmp.writeAsString(buffer.toString());
      await tmp.rename(_configPath);
    } catch (e, stackTrace) {
      _logger.shout('Failed to write accessibility config to $_configPath', e,
          stackTrace);
      // Best-effort cleanup of the temp file.
      try {
        if (await tmp.exists()) await tmp.delete();
      } catch (_) {}
    }
  }
}

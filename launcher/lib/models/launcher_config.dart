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

import 'dart:convert';
import 'dart:io';

import 'package:flutter/foundation.dart';
import 'package:logging/logging.dart';

class LauncherConfig {
  static const _headEnvKey = 'UBUNTU_FRAME_LAUNCHER_HEAD_ITEMS_JSON';
  static const _bodyEnvKey = 'UBUNTU_FRAME_LAUNCHER_BODY_ITEMS_JSON';
  static const _tailEnvKey = 'UBUNTU_FRAME_LAUNCHER_TAIL_ITEMS_JSON';
  static const _accessibilityIds = {
    'magnifier',
    'cursor-scale',
    'output-filter',
  };

  static final _logger = Logger('LauncherConfig');

  final List<String> headItems;
  final List<String> bodyItems;
  final List<String> tailItems;

  LauncherConfig._({
    required this.headItems,
    required this.bodyItems,
    required this.tailItems,
  });

  factory LauncherConfig.fromEnvironment() =>
      LauncherConfig.fromMap(Platform.environment);

  @visibleForTesting
  factory LauncherConfig.fromMap(Map<String, String> environment) {
    final head = _withoutRunning(_parseEnv(environment, _headEnvKey), 'head');
    final body = _parseEnv(environment, _bodyEnvKey);
    final tail = _withoutRunning(_parseEnv(environment, _tailEnvKey), 'tail');
    _logger.info('head=$head body=$body tail=$tail');
    return LauncherConfig._(headItems: head, bodyItems: body, tailItems: tail);
  }

  static List<String> _withoutRunning(List<String> items, String group) {
    if (items.contains('running')) {
      _logger.warning(
        "'running' is only supported in body items; ignoring it in $group",
      );
    }
    return items.where((item) => item != 'running').toList();
  }

  static List<String> _parseEnv(Map<String, String> environment, String key) {
    final value = environment[key];
    if (value == null || value.isEmpty) return [];
    try {
      final decoded = jsonDecode(value);
      if (decoded is! List || !decoded.every((item) => item is String)) {
        throw const FormatException('must be an array of strings');
      }
      return decoded.cast<String>();
    } on FormatException catch (error) {
      _logger.warning('$key: $error');
      return [];
    }
  }

  List<String> get accessibilityOptionIds => [
    ...headItems,
    ...bodyItems,
    ...tailItems,
  ].where((id) => _accessibilityIds.contains(id)).toList();
}

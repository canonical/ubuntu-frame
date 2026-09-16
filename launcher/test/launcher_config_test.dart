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

import 'package:flutter_test/flutter_test.dart';
import 'package:ubuntu_frame_launcher/models/launcher_config.dart';

void main() {
  group('LauncherConfig', () {
    test('parses head, body and tail items', () {
      final config = LauncherConfig.fromMap({
        'UBUNTU_FRAME_LAUNCHER_HEAD_ITEMS_JSON': '["magnifier"]',
        'UBUNTU_FRAME_LAUNCHER_BODY_ITEMS_JSON': '["running"]',
        'UBUNTU_FRAME_LAUNCHER_TAIL_ITEMS_JSON': '["cursor-scale"]',
      });

      expect(config.headItems, ['magnifier']);
      expect(config.bodyItems, ['running']);
      expect(config.tailItems, ['cursor-scale']);
    });

    test('defaults each group to an empty list when unset', () {
      final config = LauncherConfig.fromMap({});

      expect(config.headItems, isEmpty);
      expect(config.bodyItems, isEmpty);
      expect(config.tailItems, isEmpty);
    });

    test('defaults body to empty when only head and tail are set', () {
      final config = LauncherConfig.fromMap({
        'UBUNTU_FRAME_LAUNCHER_HEAD_ITEMS_JSON': '["magnifier"]',
        'UBUNTU_FRAME_LAUNCHER_TAIL_ITEMS_JSON': '["cursor-scale"]',
      });

      expect(config.headItems, ['magnifier']);
      expect(config.bodyItems, isEmpty);
      expect(config.tailItems, ['cursor-scale']);
    });

    test('collects accessibility ids across head, body and tail', () {
      final config = LauncherConfig.fromMap({
        'UBUNTU_FRAME_LAUNCHER_HEAD_ITEMS_JSON': '["magnifier","running"]',
        'UBUNTU_FRAME_LAUNCHER_BODY_ITEMS_JSON': '["running","cursor-scale"]',
        'UBUNTU_FRAME_LAUNCHER_TAIL_ITEMS_JSON': '["output-filter"]',
      });

      expect(config.accessibilityOptionIds,
          ['magnifier', 'cursor-scale', 'output-filter']);
    });

    test('falls back to empty list on malformed json', () {
      final config = LauncherConfig.fromMap({
        'UBUNTU_FRAME_LAUNCHER_BODY_ITEMS_JSON': '"magnifier"]',
      });

      expect(config.bodyItems, isEmpty);
    });
  });
}

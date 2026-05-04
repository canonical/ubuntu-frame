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

import 'package:flutter/material.dart';

class AccessibilityOption {
  final String id;
  final IconData icon;
  final List<String> values;
  int _currentIndex = 0;

  AccessibilityOption({
    required this.id,
    required this.icon,
    required this.values,
  }) : assert(values.isNotEmpty);

  String get currentValue => values[_currentIndex];

  void setValueFromString(String value) {
    final index = values.indexOf(value);
    if (index == -1) return;
    _currentIndex = index;
  }

  void cycleForward() {
    _currentIndex = (_currentIndex + 1) % values.length;
  }

  void cycleBackward() {
    _currentIndex = (_currentIndex - 1 + values.length) % values.length;
  }
}

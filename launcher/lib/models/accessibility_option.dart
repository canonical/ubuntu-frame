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

import 'package:flutter/material.dart';

class AccessibilityOption {
  final String id;
  final String label;
  final IconData icon;
  final List<String> values;
  int _currentIndex;

  final _controller = StreamController<String>.broadcast();

  AccessibilityOption({
    required this.id,
    required this.label,
    required this.icon,
    required this.values,
    int currentIndex = 0,
  })  : assert(values.isNotEmpty),
        _currentIndex = currentIndex;

  int get currentIndex => _currentIndex;
  String get currentValue => values[_currentIndex];

  Stream<String> getStream() => _controller.stream;

  /// Sets the current value by matching [value] against [values].
  /// Returns true if a match was found, false otherwise.
  bool setValueFromString(String value) {
    final index = values.indexOf(value);
    if (index == -1) return false;
    _currentIndex = index;
    return true;
  }

  void cycleForward() {
    _currentIndex = (_currentIndex + 1) % values.length;
    _controller.add(values[_currentIndex]);
  }

  void cycleBackward() {
    _currentIndex = (_currentIndex - 1 + values.length) % values.length;
    _controller.add(values[_currentIndex]);
  }

  void dispose() {
    _controller.close();
  }
}

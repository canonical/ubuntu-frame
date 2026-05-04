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
import 'package:get_it/get_it.dart';
import 'package:ubuntu_frame_launcher/controllers/accessibility_controller.dart';
import 'package:ubuntu_frame_launcher/models/accessibility_option.dart';

class AccessibilityButton extends StatelessWidget {
  const AccessibilityButton({super.key});

  @override
  Widget build(BuildContext context) {
    final controller = GetIt.instance.get<AccessibilityController>();
    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        ...controller.options.map(
          (option) => Padding(
            padding: const EdgeInsets.only(bottom: 4),
            child: _OptionButton(
              option: option,
              onTap: () => controller.cycleForward(option),
              onLongPress: () => controller.cycleBackward(option),
            ),
          ),
        ),
      ],
    );
  }
}

class _OptionButton extends StatefulWidget {
  final AccessibilityOption option;
  final VoidCallback onTap;
  final VoidCallback onLongPress;

  const _OptionButton({
    required this.option,
    required this.onTap,
    required this.onLongPress,
  });

  @override
  State<_OptionButton> createState() => _OptionButtonState();
}

class _OptionButtonState extends State<_OptionButton> {
  bool _mouseOver = false;

  @override
  Widget build(BuildContext context) {
    return MouseRegion(
      onEnter: (_) => setState(() => _mouseOver = true),
      onExit: (_) => setState(() => _mouseOver = false),
      child: GestureDetector(
        onTap: widget.onTap,
        onLongPress: widget.onLongPress,
        child: AnimatedContainer(
          duration: const Duration(milliseconds: 120),
          width: 56,
          height: 56,
          decoration: BoxDecoration(
            borderRadius: BorderRadius.circular(14),
            color: Color.fromARGB(_mouseOver ? 63 : 20, 255, 255, 255),
          ),
          alignment: Alignment.center,
          child: Icon(
            widget.option.icon,
            color: Colors.white,
            size: 22,
          ),
        ),
      ),
    );
  }
}

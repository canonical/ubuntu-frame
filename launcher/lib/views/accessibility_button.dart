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
import 'package:get_it/get_it.dart';
import 'package:ubuntu_frame_launcher/controllers/accessibility_controller.dart';
import 'package:ubuntu_frame_launcher/models/accessibility_option.dart';
import 'package:ubuntu_frame_launcher/views/launcher_button.dart';

// Duration for the overall panel open/close animation.
const _kPanelDuration = Duration(milliseconds: 250);

// Duration for each option button's staggered fade-in.
const _kOptionFadeDuration = Duration(milliseconds: 180);

// Delay between each successive option button fading in.
const _kStaggerOffset = Duration(milliseconds: 40);

// Delay to let option fades begin before the panel closes.
const _kCollapseDelay = Duration(milliseconds: 80);

class AccessibilityButton extends StatefulWidget {
  const AccessibilityButton({super.key});

  @override
  State<AccessibilityButton> createState() => _AccessibilityButtonState();
}

class _AccessibilityButtonState extends State<AccessibilityButton>
    with TickerProviderStateMixin {
  final _controller = GetIt.instance.get<AccessibilityController>();

  // Drives the panel height (SizeTransition) and icon rotation.
  late final AnimationController _panelAnim;

  // One animation controller per option for staggered fades.
  late final List<AnimationController> _optionAnims;

  late final StreamSubscription<bool> _expandSubscription;

  // Incremented on every expand/collapse; lets async methods detect they
  // have been superseded by a newer call and bail out early.
  int _animGeneration = 0;

  @override
  void initState() {
    super.initState();

    _panelAnim = AnimationController(
      vsync: this,
      duration: _kPanelDuration,
    );

    _optionAnims = List.generate(
      _controller.options.length,
      (_) => AnimationController(
        vsync: this,
        duration: _kOptionFadeDuration,
      ),
    );

    _expandSubscription = _controller.getStream().listen(_onExpandedChanged);
  }

  @override
  void dispose() {
    _expandSubscription.cancel();
    _panelAnim.dispose();
    for (final a in _optionAnims) {
      a.dispose();
    }
    super.dispose();
  }

  void _onExpandedChanged(bool isExpanded) {
    if (isExpanded) {
      _playExpand();
    } else {
      _playCollapse();
    }
  }

  void _playExpand() async {
    final gen = ++_animGeneration;

    // Panel grows open first.
    _panelAnim.forward();

    // Then stagger each option button in.
    for (int i = 0; i < _optionAnims.length; i++) {
      _optionAnims[i].reset();
      await Future.delayed(_kStaggerOffset);
      if (!mounted || _animGeneration != gen) return;
      _optionAnims[i].forward();
    }
  }

  void _playCollapse() async {
    final gen = ++_animGeneration;

    // Fade options out quickly, then close the panel.
    for (final a in _optionAnims) {
      a.reverse();
    }
    await Future.delayed(_kCollapseDelay);
    if (!mounted || _animGeneration != gen) return;
    _panelAnim.reverse();
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        // The main accessibility icon button - always visible.
        StreamBuilder(
          stream: _controller.getStream(),
          builder: (context, _) => LauncherButton(
            onPressed: () {
              if (_controller.isExpanded) {
                _controller.collapse();
              } else {
                _controller.expand();
              }
            },
            active: _controller.isExpanded,
            child: AnimatedSwitcher(
              duration: const Duration(milliseconds: 200),
              transitionBuilder: (child, anim) => FadeTransition(
                opacity: anim,
                child: ScaleTransition(scale: anim, child: child),
              ),
              child: Icon(
                _controller.isExpanded
                    ? Icons.keyboard_arrow_down
                    : Icons.accessibility_new,
                key: ValueKey(_controller.isExpanded),
                color: Colors.white,
                size: 32,
              ),
            ),
          ),
        ),

        // The expanding panel: size + fade wraps the option list.
        SizeTransition(
          sizeFactor: CurvedAnimation(
            parent: _panelAnim,
            curve: Curves.easeInOut,
          ),
          axisAlignment: -1.0,
          child: FadeTransition(
            opacity: CurvedAnimation(
              parent: _panelAnim,
              curve: const Interval(0.3, 1.0, curve: Curves.easeIn),
            ),
            child: Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Column(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    const SizedBox(height: 4),
                    ..._controller.options.asMap().entries.map((entry) {
                      final i = entry.key;
                      final option = entry.value;
                      return Padding(
                        padding: const EdgeInsets.only(bottom: 4),
                        child: FadeTransition(
                          opacity: CurvedAnimation(
                            parent: _optionAnims[i],
                            curve: Curves.easeIn,
                          ),
                          child: _OptionButton(
                            option: option,
                            onTap: () => _controller.cycleForward(option),
                            onLongPress: () =>
                                _controller.cycleBackward(option),
                          ),
                        ),
                      );
                    }),
                  ],
                ),
              ],
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
  // true = incoming value slides in from the right (tap/forward),
  // false = incoming value slides in from the left (long press/backward).
  bool _slideFromRight = true;

  @override
  Widget build(BuildContext context) {
    // StreamBuilder is scoped to this option only, so cycling one
    // option does not cause sibling buttons to rebuild.
    return StreamBuilder(
      stream: widget.option.getStream(),
      builder: (context, _) {
        return Tooltip(
          message: '${widget.option.label}: ${widget.option.currentValue}',
          preferBelow: false,
          textStyle: const TextStyle(fontSize: 10, color: Colors.white),
          child: MouseRegion(
            onEnter: (_) => setState(() => _mouseOver = true),
            onExit: (_) => setState(() => _mouseOver = false),
            child: GestureDetector(
              onTap: () {
                setState(() => _slideFromRight = true);
                widget.onTap();
              },
              onLongPress: () {
                setState(() => _slideFromRight = false);
                widget.onLongPress();
              },
              child: AnimatedContainer(
                duration: const Duration(milliseconds: 120),
                width: 56,
                height: 56,
                decoration: BoxDecoration(
                  borderRadius: BorderRadius.circular(14),
                  color: Color.fromARGB(_mouseOver ? 63 : 20, 255, 255, 255),
                ),
                alignment: Alignment.center,
                child: Column(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    Icon(
                      widget.option.icon,
                      color: Colors.white,
                      size: 22,
                    ),
                    const SizedBox(height: 2),
                    AnimatedSwitcher(
                      duration: const Duration(milliseconds: 200),
                      // Incoming widget slides in from one side,
                      // outgoing widget slides out the opposite side.
                      transitionBuilder: (child, anim) {
                        final isIncoming =
                            child.key == ValueKey(widget.option.currentValue);
                        final incomingOffset =
                            Offset(_slideFromRight ? 1.0 : -1.0, 0.0);
                        final outgoingOffset =
                            Offset(_slideFromRight ? -1.0 : 1.0, 0.0);
                        return ClipRect(
                          child: SlideTransition(
                            position: Tween<Offset>(
                              begin:
                                  isIncoming ? incomingOffset : outgoingOffset,
                              end: Offset.zero,
                            ).animate(CurvedAnimation(
                              parent: anim,
                              curve: Curves.easeInOut,
                            )),
                            child: child,
                          ),
                        );
                      },
                      layoutBuilder: (currentChild, previousChildren) => Stack(
                        alignment: Alignment.center,
                        children: [
                          ...previousChildren,
                          if (currentChild != null) currentChild,
                        ],
                      ),
                      child: Text(
                        widget.option.currentValue,
                        key: ValueKey(widget.option.currentValue),
                        style: const TextStyle(
                          color: Colors.white70,
                          fontSize: 9,
                          fontFamily: 'Ubuntu',
                        ),
                        maxLines: 1,
                        overflow: TextOverflow.ellipsis,
                      ),
                    ),
                  ],
                ),
              ),
            ),
          ),
        );
      },
    );
  }
}

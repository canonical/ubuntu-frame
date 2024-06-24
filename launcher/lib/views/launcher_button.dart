import 'package:flutter/material.dart';

class LauncherButton extends StatefulWidget {
  final Widget child;
  final bool active;
  final VoidCallback onPressed;
  final VoidCallback? onMiddleClick;

  const LauncherButton(
      {super.key,
      required this.child,
      required this.onPressed,
      this.onMiddleClick,
      this.active = false});

  @override
  State<LauncherButton> createState() => _LauncherButtonState();
}

class _LauncherButtonState extends State<LauncherButton> {
  bool mouseOver = false;

  @override
  Widget build(BuildContext context) {
    return MouseRegion(
      onEnter: (event) => setMouseOver(true),
      onExit: (event) => setMouseOver(false),
      child: GestureDetector(
        onTap: widget.onPressed,
        onTertiaryTapDown: (details) => widget.onMiddleClick?.call(),
        child: Container(
          width: 64,
          height: 64,
          decoration: BoxDecoration(
            borderRadius: BorderRadius.circular(16),
            color: Color.fromARGB(
                widget.active
                    ? 127
                    : mouseOver
                        ? 63
                        : 0,
                255,
                255,
                255),
          ),
          alignment: Alignment.center,
          child: widget.child,
        ),
      ),
    );
  }

  void setMouseOver(bool mouseOver_) {
    setState(() => mouseOver = mouseOver_);
  }
}

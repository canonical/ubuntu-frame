import 'package:flutter/material.dart';
import 'package:get_it/get_it.dart';
import 'package:ubuntu_frame_launcher/models/desktop_file.dart';
import 'package:ubuntu_frame_launcher/views/app_icon.dart';
import 'package:ubuntu_frame_launcher/controllers/application_controller.dart';
import 'package:ubuntu_frame_launcher/controllers/window_controller.dart';
import 'launcher_button.dart';

class DockButton extends StatefulWidget {
  final DesktopFile desktopFile;

  const DockButton({super.key, required this.desktopFile});

  @override
  State<DockButton> createState() => _DockButtonState();
}

class _DockButtonState extends State<DockButton> {
  @override
  Widget build(BuildContext context) {
    return LauncherButton(
      onPressed: _onLeftClick,
      child: AppIcon(iconName: widget.desktopFile.icon ?? '', size: 48),
    );
  }

  void _onLeftClick() {
    final windowController = GetIt.instance.get<WindowController>();
    final applicationController = GetIt.instance.get<ApplicationController>();
    if (windowController.isOpen(widget.desktopFile)) {
      applicationController.focus(widget.desktopFile);
    }
  }
}

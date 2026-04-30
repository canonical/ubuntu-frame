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

import 'dart:io';

import 'package:flutter/material.dart';
import 'package:ubuntu_frame_launcher/services/wayland_window_watcher.dart';
import 'package:ubuntu_frame_launcher/controllers/window_controller.dart';
import 'package:ubuntu_frame_launcher/services/window_service.dart';
import 'services/desktop_file_manager.dart';
import 'package:get_it/get_it.dart';
import 'views/dock.dart';
import 'controllers/application_controller.dart';
import 'package:logging/logging.dart';
import 'package:ubuntu_frame_launcher/controllers/accessibility_controller.dart';
import 'package:ubuntu_frame_launcher/models/accessibility_option.dart';

void main() async {
  Logger.root.level = Level.ALL; // defaults to Level.INFO
  Logger.root.onRecord.listen((record) {
    print('${record.level.name}: ${record.time}: ${record.message}');
    if (record.error != null) print('  Error: ${record.error}');
    if (record.stackTrace != null) {
      print('  Stack trace:\n${record.stackTrace}');
    }
  });

  final logger = Logger("main");
  logger.info("Ubuntu Frame launcher is starting");

  final getIt = GetIt.instance;

  // Services
  getIt.registerLazySingleton<WindowService>(
      () => WindowService(WaylandWindowWatcherService()));
  getIt.registerLazySingleton<DesktopFileManager>(() => DesktopFileManager());
  // Controllers
  getIt.registerLazySingleton<ApplicationController>(() =>
      ApplicationController(
          getIt.get<WindowService>(), getIt.get<DesktopFileManager>()));
  getIt.registerLazySingleton<WindowController>(
      () => WindowController(getIt.get<WindowService>()));
  final allOptions = {
    'magnifier_enable': AccessibilityOption(
      id: 'magnifier_enable',
      label: 'Zoom',
      icon: Icons.zoom_in,
      values: ['true', 'false'],
    ),
    'output_filter': AccessibilityOption(
      id: 'output_filter',
      label: 'Screen Filter',
      icon: Icons.filter_b_and_w,
      values: ['none', 'grayscale', 'invert'],
    ),
    'cursor_scale': AccessibilityOption(
      id: 'cursor_scale',
      label: 'Cursor Scale',
      icon: Icons.mouse_outlined,
      values: ['1', '1.5', '2'],
    ),
  };

  const optionsEnvKey = 'UBUNTU_FRAME_LAUNCHER_ACCESSIBILITY_OPTIONS';
  final optionsEnv = Platform.environment[optionsEnvKey];
  final List<AccessibilityOption> activeOptions;
  if (optionsEnv != null && optionsEnv.isNotEmpty) {
    activeOptions = [];
    for (final id in optionsEnv.split(':')) {
      final option = allOptions[id];
      if (option != null) {
        activeOptions.add(option);
      } else {
        logger.warning('$optionsEnvKey: unknown option "$id", skipping');
      }
    }
  } else {
    activeOptions = allOptions.values.toList();
  }

  getIt.registerLazySingleton<AccessibilityController>(
      () => AccessibilityController(options: activeOptions));

  await getIt.get<AccessibilityController>().initialize();

  runApp(const LauncherApp());
}

class LauncherApp extends StatelessWidget {
  const LauncherApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      theme: ThemeData(
          colorScheme: ColorScheme.fromSeed(seedColor: Colors.black),
          primaryColor: Colors.black,
          useMaterial3: true,
          fontFamily: 'Ubuntu',
          iconTheme: const IconThemeData(color: Colors.white, size: 20),
          textTheme: const TextTheme(
            bodyMedium: TextStyle(
              color: Colors.white,
            ),
          )),
      home: const Launcher(),
    );
  }
}

class Launcher extends StatelessWidget {
  const Launcher({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
        body: Column(
      mainAxisSize: MainAxisSize.min,
      crossAxisAlignment: CrossAxisAlignment.start,
      children: <Widget>[Dock()],
    ));
  }
}

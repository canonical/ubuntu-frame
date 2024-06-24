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
import 'package:ubuntu_frame_launcher/services/wayland_window_watcher.dart';
import 'package:ubuntu_frame_launcher/controllers/window_controller.dart';
import 'package:ubuntu_frame_launcher/services/window_service.dart';
import 'services/desktop_file_manager.dart';
import 'package:get_it/get_it.dart';
import 'views/dock.dart';
import 'controllers/application_controller.dart';
import 'package:logging/logging.dart';

void main() async {
  Logger.root.level = Level.ALL; // defaults to Level.INFO
  Logger.root.onRecord.listen((record) {
    print('${record.level.name}: ${record.time}: ${record.message}');
  });

  final log = Logger("main");
  log.info("Ubuntu Frame launcher is starting");

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

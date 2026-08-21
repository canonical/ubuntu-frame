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

import 'dart:math' show max;

import 'package:flutter/material.dart';
import 'package:get_it/get_it.dart';
import 'package:ubuntu_frame_launcher/controllers/application_controller.dart';
import 'package:ubuntu_frame_launcher/models/launcher_config.dart';
import 'package:ubuntu_frame_launcher/views/accessibility_button.dart';
import 'package:ubuntu_frame_launcher/views/dock_button.dart';
import 'package:ubuntu_frame_launcher/views/stream_builder_with_future_initial_value.dart';

const double _dockWidthPx = 70;
const _dockPadding = EdgeInsets.fromLTRB(3, 6, 3, 6);

class Dock extends StatelessWidget {
  const Dock({super.key});

  @override
  Widget build(BuildContext context) {
    final config = GetIt.instance.get<LauncherConfig>();
    return Expanded(
      child: LayoutBuilder(
        builder: (context, constraints) {
          final fixedItemCount = [...config.headItems, ...config.tailItems]
              .where((i) => i != 'running')
              .length;
          // Each accessibility button is 56px + 4px bottom padding.
          const itemHeight = 60.0;
          final runningMaxHeight = max(
            0.0,
            constraints.maxHeight - _dockPadding.vertical - fixedItemCount * itemHeight,
          );
          return Container(
            color: Colors.black,
            width: _dockWidthPx,
            padding: _dockPadding,
            child: Column(
              children: [
                ..._buildItems(config.headItems, runningMaxHeight),
                const Spacer(),
                ..._buildItems(config.tailItems, runningMaxHeight),
              ],
            ),
          );
        },
      ),
    );
  }

  List<Widget> _buildItems(List<String> items, double runningMaxHeight) {
    return items.map<Widget>((item) {
      if (item == 'running') {
        return ConstrainedBox(
          constraints: BoxConstraints(maxHeight: runningMaxHeight),
          child: const _RunningAppsSection(),
        );
      }
      return AccessibilityButton(optionId: item);
    }).toList();
  }
}

class _RunningAppsSection extends StatelessWidget {
  const _RunningAppsSection();

  @override
  Widget build(BuildContext context) {
    final applicationController = GetIt.instance.get<ApplicationController>();
    return StreamBuilderWithFutureInitialValue(
      future: applicationController.getOpenApplications(),
      stream: applicationController.getOpenedAppsStream(),
      loader: const Row(),
      builder: (context, openedApps) {
        final dockButtons = openedApps
            .where((app) => app.id.isNotEmpty)
            .map((app) => DockButton(desktopFile: app))
            .toList();
        return SingleChildScrollView(child: Column(children: dockButtons));
      },
    );
  }
}

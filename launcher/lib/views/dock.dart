import 'package:flutter/material.dart';
import 'package:get_it/get_it.dart';
import 'package:ubuntu_frame_launcher/controllers/application_controller.dart';
import 'package:ubuntu_frame_launcher/views/dock_button.dart';
import 'package:ubuntu_frame_launcher/views/stream_builder_with_future_initial_value.dart';

const double _dockWidthPx = 70;
const _dockPadding = EdgeInsets.fromLTRB(3, 6, 3, 6);

class Dock extends StatelessWidget {
  final applicationControler = GetIt.instance.get<ApplicationController>();

  Dock({super.key});

  @override
  Widget build(BuildContext context) {
    return Expanded(
        child: Container(
            color: Colors.black,
            width: _dockWidthPx,
            padding: _dockPadding,
            child: StreamBuilderWithFutureInitialValue(
                future: applicationControler.getOpenApplications(),
                stream: applicationControler.getOpenedAppsStream(),
                loader: const Row(),
                builder: (context, openedApps) {
                  List<Widget> dockButtons = [];
                  for (final opened in openedApps) {
                    if (opened.id.isEmpty) {
                      continue;
                    }
                    dockButtons.add(DockButton(desktopFile: opened));
                  }

                  return SingleChildScrollView(
                      child: Column(children: dockButtons));
                })));
  }
}

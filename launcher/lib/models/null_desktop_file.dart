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

import 'package:ubuntu_frame_launcher/models/desktop_file.dart';

class NullDesktopFile extends DesktopFile {
  final String appId;
  NullDesktopFile(this.appId);

  @override
  String? get exec => null;

  @override
  String get icon => "application-x-executable";

  @override
  String get id => appId;

  @override
  void launch() {}

  @override
  String? get name => appId;

  @override
  bool get noDisplay => false;

  @override
  String? get path => null;

  @override
  bool get singleMainWindow => false;

  @override
  String? get type => null;
}

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
import 'package:xdg_icons/xdg_icons.dart';
import 'package:path/path.dart' as path;
import 'package:flutter_svg/flutter_svg.dart';

class AppIcon extends StatelessWidget {
  final String iconName;
  final int size;

  const AppIcon({super.key, required this.iconName, required this.size});

  @override
  Widget build(BuildContext context) {
    final extension = path.extension(iconName);
    if (extension == ".png" || extension == ".jpeg" || extension == ".jpg") {
      return Image(
          image: AssetImage(iconName),
          width: size.toDouble(),
          height: size.toDouble());
    }

    if (extension == ".svg") {
      return SvgPicture.asset(iconName,
          semanticsLabel: iconName,
          width: size.toDouble(),
          height: size.toDouble());
    }

    return XdgIcon(
        name: iconName,
        size: size,
        iconNotFoundBuilder: () {
          return XdgIcon(name: "application-x-executable", size: size);
        });
  }
}

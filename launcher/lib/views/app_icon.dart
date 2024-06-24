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

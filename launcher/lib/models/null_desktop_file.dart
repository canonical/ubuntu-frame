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

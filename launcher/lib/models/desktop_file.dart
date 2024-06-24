abstract class DesktopFile {
  String get id;
  String? get path;
  String? get type;
  String? get name;
  String get icon;
  bool get noDisplay;
  String? get exec;
  bool get singleMainWindow;
  void launch();
}

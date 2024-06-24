abstract class WindowHandle {
  void setMaximized();
  void unsetMaximized();
  void setMinimized();
  void unsetMinimized();
  void activate();
  void close();
  String getAppId();
  void setFullscreen();
  void unsetFullscreen();
}

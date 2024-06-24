import 'dart:math';
import 'dart:typed_data';

import 'package:ubuntu_frame_launcher/models/window_event.dart';
import 'package:ubuntu_frame_launcher/services/window_handle.dart';

import 'window_watcher_service.dart';
import 'package:wayland/wayland.dart';
import 'package:logging/logging.dart';

class WlrForeignTopLevelManager extends WaylandObject {
  @override
  String get interfaceName => "zwlr_foreign_toplevel_manager_v1";

  Function(int)? onTopLevel;
  Function()? onFinished;

  static const _closeRequest = 0;

  WlrForeignTopLevelManager(WaylandClient client, int id,
      {this.onTopLevel, this.onFinished})
      : super(client, id);

  void stop() {
    client.sendRequest(id, _closeRequest);
  }

  @override
  bool processEvent(int code, Uint8List payload) {
    const topLevelEvent = 0;
    const finishedEvent = 1;

    switch (code) {
      case topLevelEvent:
        var buffer = WaylandReadBuffer(payload);
        var id = buffer.readUint();
        onTopLevel?.call(id);
        return true;
      case finishedEvent:
        onFinished?.call();
        return true;
      default:
        return false;
    }
  }
}

class WlrForeignToplevelHandleV1 extends WaylandObject implements WindowHandle {
  final _logger = Logger("WlrForeignToplevelHandleV1");

  @override
  String get interfaceName => "zwlr_foreign_toplevel_handle_v1";

  WaylandSeat? seat;
  Function(WlrForeignToplevelHandleV1, String)? onName;
  Function(WlrForeignToplevelHandleV1, String)? onAppId;
  Function(WlrForeignToplevelHandleV1, WaylandOutput)? onOutputEnter;
  Function(WlrForeignToplevelHandleV1, WaylandOutput)? onOutputLeave;
  Function(WlrForeignToplevelHandleV1, List<int>)? onState;
  Function(WlrForeignToplevelHandleV1)? onDone;
  Function(WlrForeignToplevelHandleV1)? onClosed;

  String name = "";
  String appId = "";
  String desktopAppId = "";

  static const _maximizedRequest = 0;
  static const _unsetMaximizedRequest = 1;
  static const _minimizedRequest = 2;
  static const _unsetMinimizedRequest = 3;
  static const _activateRequest = 4;
  static const _closeRequest = 5;
  static const _setRectangleRequest = 6;
  static const _destroyRequest = 7;
  static const _setFullscreenRequest = 8;
  static const _unsetFullscreenRequest = 9;

  WlrForeignToplevelHandleV1(WaylandClient client, int id, this.seat,
      {this.onName,
      this.onAppId,
      this.onOutputEnter,
      this.onOutputLeave,
      this.onState,
      this.onDone,
      this.onClosed})
      : super(client, id);

  @override
  void setMaximized() {
    client.sendRequest(id, _maximizedRequest);
  }

  @override
  void unsetMaximized() {
    client.sendRequest(id, _unsetMaximizedRequest);
  }

  @override
  void setMinimized() {
    client.sendRequest(id, _minimizedRequest);
  }

  @override
  void unsetMinimized() {
    client.sendRequest(id, _unsetMinimizedRequest);
  }

  @override
  void activate() {
    if (seat == null) {
      _logger.shout("Unable to activate the window because the seat is null");
      return;
    }

    var payload = WaylandWriteBuffer();
    payload.writeUint(seat!.id);
    client.sendRequest(id, _activateRequest, payload.data);
  }

  @override
  void close() {
    client.sendRequest(id, _closeRequest);
  }

  void setRectangle(
      WaylandSurface surface, int x, int y, int width, int height) {
    var payload = WaylandWriteBuffer();
    payload.writeUint(surface.id);
    payload.writeInt(x);
    payload.writeInt(y);
    payload.writeInt(width);
    payload.writeInt(height);
    client.sendRequest(id, _setRectangleRequest);
  }

  void destroy() {
    client.sendRequest(id, _destroyRequest);
  }

  @override
  void setFullscreen() {
    client.sendRequest(id, _setFullscreenRequest);
  }

  @override
  void unsetFullscreen() {
    client.sendRequest(id, _unsetFullscreenRequest);
  }

  @override
  String getAppId() {
    return appId;
  }

  @override
  bool processEvent(int code, Uint8List payload) {
    const nameEvent = 0;
    const appIdEvent = 1;
    const outputEnterEvent = 2;
    const outputLeaveEvent = 3;
    const statesEvent = 4;
    const doneEvent = 5;
    const closedEvent = 6;

    switch (code) {
      case nameEvent:
        var buffer = WaylandReadBuffer(payload);
        name = buffer.readString();
        onName?.call(this, name);
        return true;
      case appIdEvent:
        var buffer = WaylandReadBuffer(payload);
        appId = buffer.readString();
        onAppId?.call(this, appId);
        return true;
      case outputEnterEvent:
        var buffer = WaylandReadBuffer(payload);
        var id = buffer.readUint();
        onOutputEnter?.call(this, WaylandOutput(client, id));
        return true;
      case outputLeaveEvent:
        var buffer = WaylandReadBuffer(payload);
        var id = buffer.readUint();
        onOutputLeave?.call(this, WaylandOutput(client, id));
        return true;
      case statesEvent:
        var buffer = WaylandReadBuffer(payload);
        var states = buffer.readUintArray();
        onState?.call(this, states);
        return true;
      case doneEvent:
        onDone?.call(this);
        return true;
      case closedEvent:
        onClosed?.call(this);
        return true;
      default:
        return false;
    }
  }
}

class WaylandWindowWatcherService extends WindowWatcherService {
  final client = WaylandClient();
  late WaylandRegistry registry;
  WlrForeignTopLevelManager? foreignTopLevelManager;
  WaylandSeat? seat;

  WaylandWindowWatcherService() {
    connect();
  }

  Future<void> connect() async {
    await client.connect();
    registry = client.getRegistry(onGlobal: _onGlobal);
  }

  Future<void> close() async {
    await client.close();
  }

  void _onGlobal(int name, String interface, int version) {
    if (interface == "zwlr_foreign_toplevel_manager_v1") {
      foreignTopLevelManager = WlrForeignTopLevelManager(
          client, registry.bind(name, interface, min(version, 2)),
          onTopLevel: _onTopLevel);
    } else if (interface == "wl_seat") {
      seat =
          WaylandSeat(client, registry.bind(name, interface, min(version, 4)));
    }
  }

  void _onTopLevel(int topLevel) {
    final handle = WlrForeignToplevelHandleV1(client, topLevel, seat,
        onClosed: _onWindowRemoved,
        onState: _onWindowState,
        onAppId: _onWindowAppId);

    _onWindowCreated(handle);
  }

  void _onWindowRemoved(WlrForeignToplevelHandleV1 handle) {
    controller.add(WindowEvent(WindowEventType.removed, handle));
  }

  void _onWindowCreated(WlrForeignToplevelHandleV1 handle) {
    controller.add(WindowEvent(WindowEventType.created, handle));
  }

  void _onWindowState(WlrForeignToplevelHandleV1 handle, List<int> state) {
    const applicationFocused = 2;
    if (state.contains(applicationFocused)) {
      controller.add(WindowEvent(WindowEventType.focused, handle));
    }
  }

  void _onWindowAppId(WlrForeignToplevelHandleV1 handle, String appId) {
    controller.add(WindowEvent(WindowEventType.appId, handle, appId));
  }
}

# ubuntu-frame-launcher
A flutter-based launcher for Ubuntu Frame.

## Install
```sh
flutter pub get
```

## Development
1 . Run `ubuntu-frame`:

```sh
WAYLAND_DISPLAY=wayland-98 ubuntu-frame --add-wayland-extensions zwlr_layer_shell_v1:zwlr_foreign_toplevel_manager_v1  
```

2. Run `launcher`:

```sh
cd launcher
WAYLAND_DISPLAY=wayland-98 flutter run
```
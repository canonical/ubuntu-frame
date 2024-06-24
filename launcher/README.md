# ubuntu-frame-launcher
A flutter-based launcher for Ubuntu Frame.

## Install
```sh
flutter pub get
```

## Development
1 . Run `miral-shell`:

```sh
WAYLAND_DISPLAY=wayland-98 miral-shell --add-wayland-extensions zwlr_layer_shell_v1:zwlr_foreign_toplevel_manager_v1  
```

2. Run `ubuntu-frame-launcher`:

```sh
WAYLAND_DISPLAY=wayland-98 flutter run
```
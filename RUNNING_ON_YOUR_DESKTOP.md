# Ubuntu Frame

**Disclaimer:** *This isn't about how to deploy on "Classic" linux systems such as Ubuntu Desktop or Ubuntu Server. For these systems see the `daemon` configuration option in [REFERENCE.md](REFERENCE.md)*

## Running on your desktop

By "running on your desktop" we mean that `ubuntu-frame` runs as an application window within your desktop environment. This is achieved by running it with an X11 backend (and it appears as a "Mir on X" window).

There are two ways to do this, and they achieve slightly different things.

1. Running as the current user; and,
2. Running as root (but hosted on your desktop environment).

### Running as the current user

This will allow you to test applications you intend to use with `ubuntu-frame` without first packaging them as a snap or dealing with the complications of running as `daemons` under snapcraft.

This is as easy as running:
```bash
$ ubuntu-frame
```
This will create a "Mir on X" window on your desktop and a (new) Wayland socket in `$XDG_RUNTIME_DIR`:
```bash
$ ls $XDG_RUNTIME_DIR/wayland*
/run/user/1000/wayland-0       /run/user/1000/wayland-1
/run/user/1000/wayland-0.lock  /run/user/1000/wayland-1.lock
```
This what happens on a Wayland based desktop. On an Xorg based desktop you'll only see the `wayland-0*` files and you should use `wayland-0` in the following example.

You can connect any Wayland supporting application (for example, one you are developing) using the `WAYLAND_DISPLAY` environment variable:
```bash
$ WAYLAND_DISPLAY=wayland-1 glmark2-wayland
```

You can supply also configuration options (see [REFERENCE.md](REFERENCE.md)) on the commandline. For example, for testing multiple displays:
```bash
ubuntu-frame --x11-output 800x600:800x600
```

### Running as root

This will create a "Mir on X" window on your desktop and a Wayland socket that root processes (such as `daemons`) can connect to. This is useful for testing the snap packaging of applications you plan to use with `ubuntu-frame`.

Running as root is complicated as desktops should normally reject connections from other users (even root).
To get this to work reliably create a script that automates the necessary logic:
```bash
#!/bin/sh
set -xe

frame="${1:-ubuntu-frame}"

sudo "${frame}" --help > /dev/null 2>&1 || true
sudo cp "${XAUTHORITY:-~/.Xauthority}" "/root/snap/${frame}/current/.Xauthority"
XAUTHORITY="/root/snap/${frame}/current/.Xauthority" exec sudo "${frame}"
```
When using this you will have to authorise `sudo` so please check you're happy with the script before doing that.

Calling this script `fake-frame` and placing it a convenient location (such as `~/bin`) means that you just have to type:
```bash
$ fake-frame
```

To use this you need to install and enable a suitable snap (possibly one you are working on):
```bash
$ snap install wpe-webkit-mir-kiosk
$ snap connect wpe-webkit-mir-kiosk:wayland
$ snap set wpe-webkit-mir-kiosk daemon=true
```

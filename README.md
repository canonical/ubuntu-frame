# Ubuntu Frame

## Description

The foundation for many embedded graphical display implementations. `ubuntu-frame` is a simple fullscreen shell (based on Wayland) used for kiosks, industrial displays, digital signage, smart mirrors, etc.

The application you choose (or provide) gets a fullscreen window (or windows) and input from touch, keyboard and mouse without needing to deal with the specific hardware.

## Connections
```sh
snap connect ubuntu-frame:desktop-launch
```

## Configuration

There are four snap configuration options:

* `daemon=[true|false]` enables the daemon (defaults to false on classic systems)
* `config=<options for the shell>`
* `display=<options for display layout>`
* `launcher=[true|false]`

The configuration options are described in detail in [the Ubuntu Frame reference](https://mir-server.io/docs/ubuntu-frame-configuration-options).

## Development

Developers working with Ubuntu Frame may find the following useful:

* [Run Ubuntu Frame in your Desktop Environment](https://mir-server.io/docs/run-ubuntu-frame-on-your-desktop)
* [Run Ubuntu Frame in a Virtual Machine](https://mir-server.io/docs/run-ubuntu-frame-in-a-virtual-machine)
* [Run Ubuntu Frame on your Device](https://mir-server.io/docs/run-ubuntu-frame-on-your-device)

## Further reading

Developers working with Ubuntu Frame may also find the following useful:

* [Ubuntu Frame How-to Guides](https://mir-server.io/docs/how-to-guides)

The following article describes some useful debugging techniques that can be applied when using `ubuntu-frame` as well as `mir-kiosk`.

* [Debugging Graphical Apps on Ubuntu Core](https://forum.snapcraft.io/t/debugging-graphical-apps-on-ubuntu-core/23671)

----
[![ubuntu-frame](https://snapcraft.io/ubuntu-frame/badge.svg)](https://snapcraft.io/ubuntu-frame)

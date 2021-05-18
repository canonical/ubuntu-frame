# Ubuntu Core Kiosk

## Description

`ubuntu-core-kiosk` provides the foundation for any graphical kiosk implementation. It provides a black screen with a mouse pointer, letting you run any application you want, instantly turning it into a kiosk application.

There are three snap configuration options:

* `daemon=[true|false]` enables the daemon (defaults to false on classic systems)
* `kiosk-config=<contents for ubuntu_core_kiosk.config>`
* `kiosk-display=<contents for ubuntu_core_kiosk.display>`

## Useful references

These references currently refer to `mir-kiosk` but you can use `ubuntu-core-kiosk` instead.

The configuration files are described here:

* [Configuring mir-kiosk: a masterclass](https://discourse.ubuntu.com/t/configuring-mir-kiosk-a-masterclass/)

These references for building kiosk snaps currently refer to `mir-kiosk` but you can use `ubuntu-core-kiosk` instead. 

* [Make a secure Ubuntu kiosk](https://ubuntu.com/tutorials/secure-ubuntu-kiosk)
* [Make a Wayland-native Kiosk snap](https://ubuntu.com/tutorials/wayland-kiosk)
* [Make a X11-based Kiosk Snap](https://ubuntu.com/tutorials/x11-kiosk)
* [Make a HTML5/Electron-based Kiosk Snap](https://ubuntu.com/tutorials/electron-kiosk)
* [HOWTO: Run your kiosk snap on your desktop](https://discourse.ubuntu.com/t/howto-run-your-kiosk-snap-on-your-desktop/)

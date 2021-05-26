# Ubuntu Core Kiosk

## Description

`ubuntu-core-kiosk` provides the foundation for any graphical kiosk implementation. It provides a black screen with a mouse pointer, letting you run any application you want, instantly turning it into a kiosk application.

There are three snap configuration options:

* `daemon=[true|false]` enables the daemon (defaults to false on classic systems)
* `kiosk-config=<contents for ubuntu_core_kiosk.config>`
* `kiosk-display=<contents for ubuntu_core_kiosk.display>`

The configuration options are described in detail in [REFERENCE.md](REFERENCE.md)

Developers working with Ubuntu Core Kiosk may also find the following useful:

* [RUNNING_IN_A_VM.md](RUNNING_IN_A_VM.md)
* [RUNNING_ON_YOUR_DESKTOP.md](RUNNING_ON_YOUR_DESKTOP.md)
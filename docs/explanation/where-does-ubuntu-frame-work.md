(where-does-ubuntu-frame-work)=

# Where does Ubuntu Frame work?

This document describes what are the requirements for running Ubuntu Frame.

______________________________________________________________________

Ubuntu Frame is a snap designed to provide a graphical shell on Linux based devices for a single application.

We recommend Ubuntu Core for a lot of purposes, but you should be able to use Ubuntu Frame on any Linux system with snapd installed and with "suitable graphics".

## Suitable graphics

Ubuntu Frame is based on Mir which {doc}`supports a range of graphics options <mir:explanation/mir-graphics-support>`. The current iteration of Ubuntu Frame does not support all of these, just "gbm-kms".

In general, this means that Ubuntu Frame can work with any driver providing KMS, `libgbm` and an EGL supporting [EGL_WL_bind_wayland_display ](https://registry.khronos.org/EGL/extensions/WL/EGL_WL_bind_wayland_display.txt).

### Default graphics

A default installation of Ubuntu Frame will install the open-source Mesa drivers (via the `mesa-core20` snap). Mesa provides gbm-kms support across a wide range of hardware.

### Bespoke graphics

Ubuntu Frame requires the graphics userspace to be supplied via {ref}`the-gpu-2404-snap-interface`.

We've done some "proof of concept" work with non-mesa drivers, but we are not currently in a position to provide these as snaps.

### Software graphics

Strictly speaking Ubuntu Frame itself does not require hardware graphics. Mir will use hardware graphics if available, but will “fall back” to software rendering.

However, for most applications on the typical IoT devices that Ubuntu Frame targets hardware graphics will be needed to achieve acceptable performance.

## Reference platforms

Ubuntu Frame should work on hardware that:

1. has a Mesa driver; and,
1. has the corresponding kernel drivers enabled

If you find there are problems on such a system then it is worth [filing a bug](https://github.com/canonical/ubuntu-frame/issues).

Because different drivers and hardware work differently we can only test and, if necessary, debug on systems to which we have access.

These are some systems where we have automated testing in place:

| Device              | Mesa driver | Video acceleration |
| ------------------- | ----------- | ------------------ |
| **Raspberry Pi 3b** | vc4         | No                 |
| **Raspberry Pi 4**  | V3D         | ?                  |
| **Intel**           | i915        | VAAPI              |
| **Nvidia**          | Nouveau     | VAAPI/VDPAU?       |
| **AMD**             | radeon      | VAAPI              |

______________________________________________________________________

**Notes on Ubuntu Server**

Ubuntu Frame fails to start when run on Ubuntu Server 21.10 in a default configuration. This can be fixed by changing the kernel device tree overlay to one with `kms`

*Note on Raspberry Pi 3 with Ubuntu Server 21.10*

Ref. [issue #39](https://github.com/canonical/ubuntu-frame/issues/39):

> In [Impish Release notes](https://discourse.ubuntu.com/t/impish-indri-release-notes/21951) I've followed the suggestion to change the `dtoverlay` setting in `config.txt` ... In `config.txt` I found `dtoverlay=dwc2` and changed it to `dtoverlay=vc4-fkms-v3d`, and now it displays.

*Note on Raspberry Pi 4 with Ubuntu Server 22.04*

Ref. [Run Ubuntu Frame on your Device](https://discourse.ubuntu.com/t/29377/2):

> It was great, thank you very much for your help. yes when edit dtoverlay (in /boot/firmware/config.txt) to vc4-kms-v3d-pi4 value, it works for me

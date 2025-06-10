(the-graphics-core22-snap-interface)=

# The graphics-core22 snap interface

This document describes how to use the `graphics-core22` Snap interface, what are the requirements to create a content provider snap as well as discusses the design of the interface.

## The basics

Snaps are software packages that are meant to bring all of their dependencies along. But to support graphical applications, userspace drivers matching the hardware used, and sometimes the kernel, are necessary. What's more, those drivers on some systems are not available as open source, or even at all - outside of commercial engagements. This makes it impractical, or even impossible, to have application snaps ship drivers supporting the breadth of graphics hardware available.

The long-term solution is for SnapD (the daemon managing snaps on your system) to have explicit support for this, and other hardware-specific pieces of software (kernel modules, firmware, udev rules etc.). While this is being worked on, we've designed a content interface that allows application snaps to use the graphics hardware by providing the userspace drivers and environment in a content snap.

This interface is an evolution of the [`graphics-core20` interface](/explanation/the-graphics-core20-snap-interface.md) - if your application uses `base: core20`, you'll need to rely on that, or move to the newer base.

## Consuming the interface

This section explains how graphical application snaps can consume this interface to enable graphics acceleration. We maintain a set of helpers that are the easiest to use, but if you have specific reasons, we also detail how to consume the provided userspace in your application snap.

### Using the helpers provided

The simplest way to enable your snap to consume the interface are the helpers we maintain in [gpu-snap](https://github.com/canonical/gpu-snap) repository.

There's just a few things you have to do in your `snap/snapcraft.yaml` to make use of it:

1. plug the `graphics-core22` interface (the wrapper assumes it's put under `$SNAP/graphics`):

   ```yaml
   plugs:
     graphics-core22:
       interface: content
       target: $SNAP/graphics
       default-provider: mesa-core22
   ```

1. [lay out](https://documentation.ubuntu.com/snapcraft/stable/reference/layouts/) these paths in your snap:

   ```yaml
   layout:
     /usr/share/libdrm:
       bind: $SNAP/graphics/libdrm
     /usr/share/drirc.d:
       symlink: $SNAP/graphics/drirc.d
   ```

   If your app needs X11 support:

   ```yaml
     /usr/share/X11/XErrorDB:
       symlink: $SNAP/graphics/X11/XErrorDB
     /usr/share/X11/locale:
       symlink: $SNAP/graphics/X11/locale
   ```

1. use [`bin/graphics-core22-wrapper`](https://github.com/canonical/gpu-snap/blob/main/bin/graphics-core22-wrapper) in your [`command-chain`](https://documentation.ubuntu.com/snapcraft/stable/reference/project-file/snapcraft-yaml/#apps.%3Capp-name%3E.command-chain)s:

   ```yaml
   apps:
     my-app:
       command-chain:
       - bin/graphics-core22-wrapper
       command: my-app
   ```

1. use [`bin/graphics-core22-cleanup`](https://github.com/canonical/gpu-snap/blob/main/bin/graphics-core22-cleanup) after priming any staged packages to avoid shipping any libraries already provided by the `graphics-core22` providers:

   ```yaml
   parts:
     my-app:
       stage-packages:
     # ...

     graphics-core22:
       after: [my-app]
       source: https://github.com/canonical/gpu-snap.git
       plugin: dump
       override-prime: |
         craftctl default
         ${CRAFT_PART_SRC}/bin/graphics-core22-cleanup mesa-core22 nvidia-core22
       prime:
       - bin/graphics-core22-wrapper
   ```

   You can override `$CRAFT_PRIME` if you have Mesa primed in a different location:

   ```
       override-prime: |
         craftctl default
         CRAFT_PRIME=${CRAFT_PRIME}/custom/prefix \
           ${CRAFT_PART_SRC}/bin/graphics-core22-cleanup mesa-core22 nvidia-core22
   ```

Your snap, when installed, will pull in the default [`mesa-core22`](https://snapcraft.io/mesa-core22) provider, which supports a wide range of hardware. It also supports Nvidia drivers installed with debs on your host system.

### Going the manual route

If, for whatever reason, you don't want to use the helpers, here is a description of the steps you should perform in your snap:

1. connect the `graphics-core22`, see above.
1. lay out the paths, see above.
1. wrap your apps with `<target>/bin/graphics-core22-provider-wrapper`. This script, coming from the provider side, is what sets up all the environment - paths to the libraries, drivers and any supporting files.
1. remove any libraries that are provided by the content providers (see [below](#libraries-shipped) for a list). _If_ you need to provide your own versions of any of those, you need to make sure they are ABI-compatible with Ubuntu 22.04.

## Creating a provider snap

The requirements for a snap providing the content are purposefully quite simple:

1. include a `bin/graphics-core22-provider-wrapper` that sets up all the environment and executes the provided arguments, usually:
   ```shell
   #!/bin/sh

   export VAR=value

   exec "$@"
   ```
1. it should support (include, in Ubuntu 22.04 ABI-compatible versions, and ensure the application can find them) as many of the [supported API](#supported-apis) libraries (and their dependencies) as possible/applicable
1. if your provider uses the Mesa stack:
   - provide the `drirc.d` content source holding the app-specific workarounds
1. if your provider supports X11:
   - provide the `X11/locale` and `X11/XErrorDB` content source with the appropriate assets
1. optionally, if there are Mir-specific workarounds required:
   - provide the `mir-quirks` content source, with any options needed.

The rest is left to the author of the provider snap. The default provider - [mesa-core22](https://github.com/canonical/mesa-core22) - is a good reference.

### Multi-architecture providers

In some cases, it may be desirable to include libraries for more than one architecture. The main use case would be supporting software for legacy architectures running on newer hardware that supports the legacy architecture (e.g. i386 on amd64, armhf on arm64). The default [mesa-core22](https://github.com/canonical/mesa-core22) implements that for the amd64 architecture.

The requirements remain as above - your wrapper needs to extend the environment for the additional architecture supported.

### Testing your provider snap

The [graphics-test-tools](https://snapcraft.io/graphics-test-tools) set of utilities will help you determine how well your provider works. Install and connect it to your provider and run to see what is supported and how well:

```shell
$ sudo snap install graphics-test-tools --channel 22/stable
graphics-test-tools (22/stable) 22.04 from Canonical✓ installed
$ sudo snap connect graphics-test-tools:graphics-core22 <your-snap>:graphics-core22
$ graphics-test-tools.drm-info
# ...
$ graphics-test-tools.eglinfo
# ...
```

Refer to the documentation of the individual tools to see what the results mean.

## Lists

### Supported APIs

- graphics
  - GL (libGL.so.1)
  - EGL (libEGL.so.1)
  - GLES (libGLESv2.so.2)
  - DRM (libdrm.so.2)
  - Vulkan (libvulkan.so.1)
  - GBM (libgbm.so.1)
- video acceleration
  - VA-API (libva.so.2)
    - libva-drm.so.2
    - libva-x11.so.2
    - libva-wayland.so.2
  - VDPAU (libvdpau.so.1)
- X11 support
  - libglx0
  - libx11-6
  - libx11-xcb1
  - libxcb-dri2-0
  - libxcb-dri3-0
  - libxcb-glx0,
  - libxcb-present0
  - libxcb-shm0
  - libxcb-sync1
  - libxcb-xfixes0
  - libxcb1
  - libxext6
  - libxfixes3
  - libxshmfence1
  - libxxf86vm1
- Wayland support
  - libwayland-client0
  - libwayland-server0
  - libwayland-egl1
  - libwayland-cursor0

### Libraries shipped

The lists of files shipped by a selection of snaps is maintained in the graphics-core22 repository here:

https://github.com/canonical/gpu-snap/tree/main/lists

The cleanup wrapper above uses that to prune the application snap from things it will receive through the content interface.

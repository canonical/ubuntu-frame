(the-gpu-2404-snap-interface)=

# The gpu-2404 snap interface

This document describes how to use the `gpu-2404` Snap interface, what are the requirements to create a content provider snap as well as discusses the design of the interface.

## The basics

Snaps are software packages that are meant to bring all of their dependencies along. But to support graphical applications, userspace drivers matching the hardware used, and sometimes the kernel, are necessary. What's more, those drivers on some systems are not available as open source, or even at all - outside of commercial engagements. This makes it impractical, or even impossible, to have application snaps ship drivers supporting the breadth of graphics hardware available.

The long-term solution is for SnapD (the daemon managing snaps on your system) to have explicit support for this, and other hardware-specific pieces of software (kernel modules, firmware, udev rules etc.). While this is being worked on, we've designed a content interface that allows application snaps to use the graphics hardware by providing the userspace drivers and environment in a content snap.

This interface is an evolution of the \{ref}\`\`graphics-core22` interface <the-graphics-core22-snap-interface>` - if your application uses `base: core22`, you'll need to rely on that, or move to the newer base. It was renamed to signify that all GPU functions, and not only graphics, are in scope; and that it's not only applicable to Ubuntu Core installations.

## Consuming the interface

This section explains how graphical application snaps can consume this interface to enable graphics acceleration. We maintain a set of helpers that are the easiest to use, but if you have specific reasons, we also detail how to consume the provided userspace in your application snap.

### Using the helpers provided

The simplest way to enable your snap to consume the interface are the helpers we maintain in the [gpu-snap](https://github.com/canonical/gpu-snap) repository.

There's just a few things you have to do in your `snap/snapcraft.yaml` to make use of it:

> :warning: A lot of these parts doesn't need to be done when you're using the following extensions:
>
> - `gnome`
> - `kde-neon-6`
> - `kde-neon-qt6`
>
> Except step 4, these extensions will set up all the other steps for your snap. You can check that out using this command:
>
> ```bash
> snapcraft expand-extensions
> ```
>
> Please checkout the code for your specific extension [here](https://github.com/canonical/snapcraft/tree/main/extensions/desktop)

{#gpu-2404-plug}

1. plug the `gpu-2404` interface (the wrapper assumes it's put under `$SNAP/gpu-2404`):

   ```yaml
   plugs:
     gpu-2404:
       interface: content
       target: $SNAP/gpu-2404
       default-provider: mesa-2404
   ```

{#gpu-2404-x11-layouts}

1. If your app needs X11 support, {doc}`lay out <snapcraft:reference/layouts>` these paths in your snap:

```yaml
  /usr/share/X11/XErrorDB:
    symlink: $SNAP/gpu-2404/X11/XErrorDB
```

1. use [`bin/gpu-2404-wrapper`](https://github.com/canonical/gpu-snap/blob/main/bin/gpu-2404-wrapper) in your <code>{doc}`command-chain <snapcraft:reference/project-file/snapcraft-yaml>`</code>s:

   ```yaml
   apps:
     my-app:
       command-chain:
       - bin/gpu-2404-wrapper
       command: my-app
   ```

{#gpu-2404-cleanup}

1. use [`bin/gpu-2404-cleanup`](https://github.com/canonical/gpu-snap/blob/main/bin/gpu-2404-cleanup) after priming any staged packages to avoid shipping any libraries already provided by the `gpu-2404` providers:

   ```yaml
   parts:
     my-app:
       stage-packages:
     # ...

     gpu-2404:
       after: [my-app]
       source: https://github.com/canonical/gpu-snap.git
       plugin: dump
       override-prime: |
         craftctl default
         ${CRAFT_PART_SRC}/bin/gpu-2404-cleanup mesa-2404
       prime:
       - bin/gpu-2404-wrapper
   ```

   You can override `$CRAFT_PRIME` if you have Mesa primed in a different location:

   ```
       override-prime: |
         craftctl default
         CRAFT_PRIME=${CRAFT_PRIME}/custom/prefix \
           ${CRAFT_PART_SRC}/bin/gpu-2404-cleanup mesa-2404
   ```

Your snap, when installed, will pull in the default [`mesa-2404`](https://snapcraft.io/mesa-2404) provider, which supports a wide range of hardware. It also supports Nvidia drivers installed on your host system.

### Migrating from `graphics-core20`

If your snap currently uses the `graphics-core20` interface, here are the steps when you're migrating to `base: core24`:

1. replace all references to `graphics-core20` with `gpu-2404`
1. replace all references to `mesa-core20` with `mesa-2404`
1. change the target of [the content interface](#gpu-2404-plug) to `$SNAP/gpu-2404`
1. remove all environment variables / paths pointing at `graphics` paths (the wrapper takes care of these):
   ```
   LD_LIBRARY_PATH
   LIBGL_DRIVERS_PATH
   LIBVA_DRIVERS_PATH
   __EGL_VENDOR_LIBRARY_DIRS
   ```
1. prepend `bin/gpu-2404-wrapper` to your apps' `command-chain:`
1. add [the X11 layouts](#gpu-2404-x11-layouts), if your app needs them
1. replace the `cleanup` part with [`gpu-2404` above](#gpu-2404-cleanup)

### Migrating from `graphics-core22`

If your snap currently uses the `graphics-2404` interface, here are the steps when you're migrating to `base: core24`:

1. replace all references to `graphics-2404` with `gpu-2404`
1. replace all references to `mesa-2404` with `mesa-2404`
1. change the target of [the content interface](#gpu-2404-plug) to `$SNAP/gpu-2404`
1. remove all the layouts - except for [the X11 ones](#gpu-2404-x11-layouts), if your app needs them

### Going the manual route

If, for whatever reason, you don't want to use the helpers, here is a description of the steps you should perform in your snap:

1. connect the `gpu-2404`, see above.
1. lay out the paths, see above.
1. wrap your apps with `<target>/bin/gpu-2404-provider-wrapper`. This script, coming from the provider side, is what sets up all the environment - paths to the libraries, drivers and any supporting files.
1. remove any libraries that are provided by the content providers (see {ref}`below <the-gpu-2404-snap-interface#libraries-shipped>` for a list). _If_ you need to provide your own versions of any of those, you need to make sure they are ABI-compatible with Ubuntu 24.04.

## Creating a provider snap

The requirements for a snap providing the content are purposefully quite simple:

1. include a `bin/gpu-2404-provider-wrapper` that sets up all the environment and executes the provided arguments, usually:
   ```shell
   #!/bin/sh

   export VAR=value

   exec "$@"
   ```
1. it should support (include, in Ubuntu 24.04 ABI-compatible versions, and ensure the application can find them) as many of the {ref}`supported API <the-gpu-2404-snap-interface#supported-apis>` libraries (and their dependencies) as possible/applicable
1. if your provider supports X11:
   - provide the `X11/XErrorDB` content source with the appropriate assets
1. optionally, if there are Mir-specific workarounds required:
   - provide the `mir-quirks` content source, with any options needed.

The rest is left to the author of the provider snap. The default provider - [mesa-2404](https://github.com/canonical/mesa-2404) - is a good reference.

### Multi-architecture providers

In some cases, it may be desirable to include libraries for more than one architecture. The main use case would be supporting software for legacy architectures running on newer hardware that supports the legacy architecture (e.g. i386 on amd64, armhf on arm64). The default [mesa-2404](https://github.com/canonical/mesa-2404) implements that for the amd64 architecture.

The requirements remain as above - your wrapper needs to extend the environment for the additional architecture supported.

### Testing your provider snap

The [graphics-test-tools](https://snapcraft.io/graphics-test-tools) set of utilities will help you determine how well your provider works. Install and connect it to your provider and run to see what is supported and how well:

```shell
$ sudo snap install graphics-test-tools --channel 24/stable
graphics-test-tools (24/stable) 24.04 from Canonical✓ installed
$ sudo snap connect graphics-test-tools:gpu-2404 <your-snap>:gpu-2404
$ graphics-test-tools.drm-info
# ...
$ graphics-test-tools.eglinfo
# ...
```

Refer to the documentation of the individual tools to see what the results mean.

## Lists

(the-gpu-2404-snap-interface#supported-apis)=

### Supported APIs

- graphics
  - GL (libGL.so.1)
  - EGL (libEGL.so.1)
  - GLESv1 (libGLESv1_CM.so.1)
  - GLESv2 (libGLESv2.so.2)
  - DRM (libdrm.so.2)
  - Vulkan (libvulkan.so.1)
  - GBM (libgbm.so.1)
- video acceleration
  - VA-API (libva.so.2)
    - libva-drm.so.2
    - libva-glx.so.2
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
  - libwayland-cursor0
  - libwayland-egl1
  - libwayland-server0

(the-gpu-2404-snap-interface#libraries-shipped)=

### Libraries shipped

The lists of files shipped by a selection of snaps is maintained in the gpu-snap repository here:

https://github.com/canonical/gpu-snap/tree/main/lists

The cleanup wrapper above uses that to prune the application snap from things it will receive through the content interface.

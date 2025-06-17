(the-graphics-core20-snap-interface)=

# The graphics-core20 snap interface

## The problem with snapping userspace graphics drivers

Over the past few years developing graphical snaps to run on Ubuntu Core we've encountered various issues. One of these is the need to make userspace graphics drivers available inside snap confinement. There's a partial solution in snapd that, on desktop systems can make these available from the host system but among the cases where this doesn't work is on Ubuntu Core.

Until recently, we have been including the mesa drivers inside the snap as these have the least licensing issues, and cover a wide range of hardware. But it is not an ideal solution:

- Both server (e.g. mir-kiosk) and client (e.g. mir-kiosk-kodi) snaps contain these drivers;
- Security updates to mesa require all the server and client snaps to be rebuilt and distributed; and
- There's no way to introduce other drivers, even if Mir supports them.

This led to some frustration as there are a number of {doc}`graphics stacks that Mir supports <mir:explanation/mir-graphics-support>` that were not [supported by mir-kiosk](https://discourse.ubuntu.com/t/where-does-mir-kiosk-work/22270). With [Ubuntu Frame](https://snapcraft.io/ubuntu-frame) we have adopted a more flexible approach.

### Introducing graphics-core20

Fundamentally, userspace graphics drivers are just files. And snapd has a way to share files between snaps: [content interfaces](https://snapcraft.io/docs/content-interface). Care is needed to ensure that the binaries are ABI compatible, but as the binaries come from a stable archive (20.04LTS) that isn't a problem here.

The Mir team are now maintaining a [mesa-core20](https://snapcraft.io/mesa-core20) content snap with the current mesa drivers from the archive:

- `lib` contains the mesa shared libraries (add to LD_LIBRARY_PATH)
- `drv` contains the mesa drivers (set LIBGL_DRIVERS_PATH/LIBVA_DRIVERS_PATH to this)
- `glvnd/egl_vendor.d` contains the mesa ICD (set \_\_EGL_VENDOR_LIBRARY_DIRS to this)
- `libdrm` contains mesa configuration for driver support (layout to /usr/share/libdrm)
- `drirc.d` contains mesa app-specific workarounds (layout to /usr/share/drirc.d)
- `etc/mir-quirks` contains any Mir configuration for driver support (none for mesa)

We've also built and tested a number of snaps to prove this approach. As well as setting the above environment variables, they need the following plug declaration:

```yaml
  graphics-core20:
    interface: content
    target: $SNAP/graphics
    default-provider: mesa-core20
```

There's also two mesa specific directories that needs to be bind mounted:

```yaml
layout:
  /usr/share/libdrm:  # Needed by mesa-core20 on AMD GPUs
    bind: $SNAP/graphics/libdrm
  /usr/share/drirc.d:  # Used by mesa-core20 for app specific workarounds
    bind: $SNAP/graphics/drirc.d
```

Finally, it is desirable to avoid shipping anything from mesa that gets pulled into the snap with a "cleanup" part similar to this:

```yaml
  cleanup:
    after: [kodi, mir-kiosk-snap-launch]
    plugin: nil
    build-snaps: [ mesa-core20 ]
    override-prime: |
      set -eux
      cd /snap/mesa-core20/current/egl/lib
      find . -type f,l -exec rm -f $SNAPCRAFT_PRIME/usr/lib/${SNAPCRAFT_ARCH_TRIPLET}/{} \;
      rm -fr "$SNAPCRAFT_PRIME/usr/lib/${SNAPCRAFT_ARCH_TRIPLET}/dri"
      for CRUFT in bug drirc.d glvnd libdrm lintian man; do
        rm -rf "$SNAPCRAFT_PRIME/usr/share/$CRUFT"
      done
```

(This example is taken from a [mir-kiosk-kodi PR](https://github.com/canonical/mir-kiosk-kodi/pull/22)).

### The `graphics-core20` libraries

As we've gained experience with this interface we've added components to the point we need to document the libraries that consumers can depend upon in `.../lib` and which providers must supply.
(As opposed to additional libraries that are here to support these).

#### Actual graphics interface

- libEGL.so.1
- libva.so.2
- libvulkan.so.1
- libGLESv2.so.2

#### Libraries we need for gbm/kms

- Libdrm.so.2
- libgbm.so.1

#### Wayland support

- libwayland-client.so.0
- libwayland-server.so.0

#### X support (optional)

- libX11-xcb.so.1
- libXau.so.6
- libxcb-dri2.so.0
- libxcb-dri3.so.0
- libxcb-present.so.0
- libxcb.so.1
- libxcb-sync.so.1
- libxcb-xfixes.so.0
- libXdmcp.so.6
- libxshmfence.so.1

### The result

With this approach:

- There's only one copy of the drivers in one snap;
- Mesa updates only require one snap to be rebuilt and distributed; and,
- It is possible to replace mesa with other (compatible) drivers

### Now and the future

We have been using this approach with [Ubuntu Frame](https://snapcraft.io/ubuntu-frame) both with mesa-core20 and other drivers providing KMS, `libgbm` and an EGL supporting [EGL_WL_bind_wayland_display](https://registry.khronos.org/EGL/extensions/WL/EGL_WL_bind_wayland_display.txt).

Use of the `graphics-core20` interface is fully integrated into both [Ubuntu Frame](https://snapcraft.io/ubuntu-frame) and the examples we provide for {ref}`packaging IoT GUIs <how-to-index>`.

Longer term, we're hopeful that the experience gained will feed into improvements in graphics support in snapd that make this unnecessary when packaging snaps.

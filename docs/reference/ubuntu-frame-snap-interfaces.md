(ubuntu-frame-snap-interfaces)=

# Ubuntu Frame snap interfaces

This document provides a reference for the snap interfaces available for Ubuntu Frame.

______________________________________________________________________

The current [snap interface](https://snapcraft.io/docs/supported-interfaces) connections can be checked by issuing:

```
$ snap connections ubuntu-frame
Interface              Plug                                Slot                                  Notes
content                -                                   ubuntu-frame:ubuntu-frame-diagnostic  -
content[gpu-2404]      ubuntu-frame:gpu-2404               mesa-2404:gpu-2404                    -
desktop-launch         ubuntu-frame:desktop-launch         -                                     -
hardware-observe       ubuntu-frame:hardware-observe       :hardware-observe                     -
login-session-control  ubuntu-frame:login-session-control  -                                     -
network-bind           ubuntu-frame:network-bind           :network-bind                         -
opengl                 ubuntu-frame:opengl                 :opengl                               -
wayland                -                                   ubuntu-frame:wayland                  -
wayland                ubuntu-frame:wayplug                -                                     -
x11                    ubuntu-frame:x11                    :x11                                  -
```

The above is the default set of connections upon installation. It serves the default use case of Frame running as a system service. You can change the connections through {ref}`a gadget snap <configuring-ubuntu-frame-through-a-gadget-snap>` or manually:

```
sudo snap connect ubuntu-frame:login-session-control
```

The `wayland` connection will be automatic if Frame is the only provider of the slot. On Classic systems this slot is provided by snapd, so upon installing a snap that has a `plugs: [wayland]`, a manual connection (to any `wayland` slot). The same applies if you have multiple snaps that have `slots: [wayland]` installed on a Core system:

```
sudo snap connect <snap>:wayland
# or, on Core
sudo snap connect <snap>:wayland <compositor>:wayland
```

______________________________________________________________________

The interface connections serve the following purposes:

| Interface                                                                          | Purpose                                                                   | Notes                                                                      |
| ---------------------------------------------------------------------------------- | ------------------------------------------------------------------------- | -------------------------------------------------------------------------- |
| ubuntu-frame-diagnostic                                                            | The {ref}`diagnostic screen <how-to-use-the-diagnostic-feature>` feature  |                                                                            |
| {ref}`graphics-core20 <the-graphics-core20-snap-interface>`                        | GPU userspace drivers on `20` track                                       |                                                                            |
| {ref}`graphics-core22 <the-graphics-core22-snap-interface>`                        | GPU userspace drivers on `22` track                                       |                                                                            |
| {ref}`gpu-2404 <the-gpu-2404-snap-interface>`                                      | GPU userspace drivers on `24` track                                       |                                                                            |
| [hardware-observe](https://snapcraft.io/docs/hardware-observe-interface)           | Avoiding excessive logging from libinput                                  |                                                                            |
| [login-session-control](https://snapcraft.io/docs/login-session-control-interface) | {ref}`Running as an unprivileged user <run-ubuntu-frame-unprivileged>`    | Not auto-connected, as the primary way to run Frame is as a system service |
| [network-bind](https://snapcraft.io/docs/network-bind-interface)                   | Operate as a X11 server                                                   |                                                                            |
| [opengl](https://snapcraft.io/docs/opengl-interface)                               | Access the GPU for hardware acceleration                                  |                                                                            |
| [wayland](https://snapcraft.io/docs/wayland-interface)                             | Act as a Wayland compositor                                               |                                                                            |
| [wayplug](https://snapcraft.io/docs/wayland-interface)                             | Supports the {ref}`launcher feature <ubuntu-frame-configuration-options>` |                                                                            |
| [x11](https://snapcraft.io/docs/x11-interface)                                     | Run nested on a X11 session                                               |                                                                            |

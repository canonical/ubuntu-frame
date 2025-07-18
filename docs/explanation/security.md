(security)=

# Security

This document aims to explain a number of aspects of security in the context of the Ubuntu Frame snap ecosystem (Frame itself, [ubuntu-frame-osk](https://snapcraft.io/ubuntu-frame-osk) and [ubuntu-frame-vnc](https://snapcraft.io/ubuntu-frame-vnc)).

______________________________________________________________________

## Threat model

We ran threat modeling for Mir itself (the [display server library](https://mir-server.io/) underpinning Frame) based on this snap stack, and maintain that documented there: {doc}`Mir Security documentation <mir:explanation/security>`.

## Cryptography

There is no cryptography used in Frame itself or the On-Screen Keyboard snap. No direct dependency on en/decryption, hashing or digital signatures.

The VNC snap is built on top of [wayvnc](https://github.com/any1/wayvnc), which has cryptographic features (password authentication, transport encryption) and that is handled through [gnutls](https://gnutls.org/) as [packaged and maintained in Ubuntu](https://packages.ubuntu.com/source/gnutls28). See {ref}`remote access documentation <how-to-use-remote-assistance#remote-access>` for more information.

## Hardening

### Snap connections

Review the snap connections between snaps on the system and disconnect those not essential to your deployment:

```
$ snap connections
Interface            Plug                           Slot                  Notes
# ... some examples
content[gpu-2404]    ubuntu-frame:gpu-2404          mesa-2404:gpu-2404    -
hardware-observe     ubuntu-frame:hardware-observe  :hardware-observe     -
opengl               ubuntu-frame:opengl            :opengl               -
wayland              -                              ubuntu-frame:wayland  -
```

Refer to {doc}`snapcraft:explanation/interfaces` and {ref}`Frame snap interfaces <ubuntu-frame-snap-interfaces>` documentation for more information.

### Wayland extensions

Avoid adding extensions to the ones allowed by `add-wayland-extensions`, as some of them may allow clients reading the screen contents or input events.

Refer to {ref}`Frame configuration reference <ubuntu-frame-configuration-options>` and the [wayland-protocols](https://gitlab.freedesktop.org/wayland/wayland-protocols/) repository for more information.

### VNC authentication

If using the VNC snap, It is recommended that you enable password authentication to avoid unauthorized access to the VNC socket on `localhost:5900`.

Refer to {ref}`ubuntu-frame-vnc configuration reference <how-to-use-remote-assistance>` for more information.

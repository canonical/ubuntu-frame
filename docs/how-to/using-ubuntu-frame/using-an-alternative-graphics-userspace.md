(using-an-alternative-graphics-userspace)=

# Using an alternative graphics userspace

## The issue

On "desktop" linux installations there are limited options for the graphics stack, and most systems will run with the "open" Mesa graphics drivers and/or Nvidia drivers.

The IoT world is a bit more varied, often with device specific drivers provided by the board manufacturer. [Ubuntu Frame](https://snapcraft.io/ubuntu-frame) is designed to work with graphics stacks that provide KMS, `libgbm` and an EGL supporting [EGL_WL_bind_wayland_display](https://www.khronos.org/registry/EGL/extensions/WL/EGL_WL_bind_wayland_display.txt).

It does this by using a Snap "content interface", [graphics-core20](/explanation/the-graphics-core20-snap-interface.md), that allows alternative drivers providing the facilities to be used. By default, [Ubuntu Frame](https://snapcraft.io/ubuntu-frame) will use the [mesa-core20](https://https:/snapcraft.io/mesa-core20) implementation of this interface which provides the Mesa drivers from the Ubuntu 20.04LTS Archive.

## The example

The default provider can be overridden in a number of ways. Manufacturers will ideally do this through a brand store; but, it can also be done "by hand" on a device and that's what we will describe here.

First we need an alternative `graphics-core20` provider. As you won't have access to a brand store we will use a test snap: [kisak-core20](https://snapcraft.io/kisak-core20). This is actually just a backport of a more recent version of Mesa, but it serves the purpose of illustrating this functionality.

Packaging graphics drivers as a snap isn't something we discuss here. There's details of the requirements in [The graphics-core20 Snap interface](/explanation/the-graphics-core20-snap-interface.md), and the [mesa-core20](https://https:/snapcraft.io/mesa-core20) and [kisak-core20](https://snapcraft.io/kisak-core20) snaps can be used as reference examples.

## Setup

For this example, I'm going to show the steps using a desktop Linux installation. If you are working on an Ubuntu Core device it is actually simpler as the snaps involved will be running automatically.

First install the following:

```plain
snap install ubuntu-frame
snap install wpe-webkit-mir-kiosk
snap connect wpe-webkit-mir-kiosk:wayland
snap install --edge kisak-core20
```

Now (on desktop only) install Frame-it

```plain
snap install --classic frame-it
```

Frame-it is a utility to make it simpler running Ubuntu Frame from a desktop environment.

## How to

You can see what's connected to the graphics-core20 interface as follows:

```plain
snap interfaces | grep graphics-core20
kisak-core20:graphics-core20  -
mesa-core20:graphics-core20   ubuntu-frame,wpe-webkit-mir-kiosk
```

First check that things are working by running:

```plain
frame-it snap run wpe-webkit-mir-kiosk.cog
```

![image|690x575](upload://ssgPPEX9Gg7fgQXYAiGrOms5UZE.jpeg)

You can close this.

Next we can change switch the graphics provider:

```plain
snap disconnect ubuntu-frame:graphics-core20
snap disconnect wpe-webkit-mir-kiosk:graphics-core20
snap connect ubuntu-frame:graphics-core20 kisak-core20
snap connect wpe-webkit-mir-kiosk:graphics-core20 kisak-core20
```

And show the changes:

```
$ snap interfaces | grep graphics-core20
kisak-core20:graphics-core20  ubuntu-frame,wpe-webkit-mir-kiosk
mesa-core20:graphics-core20   -
```

And prove that it still all works:

```plain
frame-it snap run wpe-webkit-mir-kiosk.cog
```

![image|690x575](upload://uXz6CIXzrDa4goPf0L9rybmXC15.jpeg)

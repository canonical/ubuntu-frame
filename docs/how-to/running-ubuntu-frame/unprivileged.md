(run-ubuntu-frame-unprivileged)=

# Run Ubuntu Frame unprivileged

This document describes how to run Ubuntu Frame and its clients unprivileged - as a user service - rather than the default system service as `root`.

______________________________________________________________________

## Rationale

When you install Ubuntu Frame on Ubuntu Core, it will start automatically as a system service. It's a simple solution, but has a handful of disadvantages:

- it runs as `root`
- as a result, its clients need to run as `root`
- the clients need to be specially crafted to run as system services

That means we lose the security layer of running as an unprivileged user, both for Frame itself, but even more importantly for its clients, which often run complex software like web views. It also means you can't take an existing snapped application from the Snap Store and use it as the client to Frame. And even if you modify that application to run as a system service, it running outside of a user session poses another potential problem.

## Solution

To have Frame support as many existing applications as possible, we'll run it as the compositor in a full user session - as if we replaced e.g. GNOME in the default Ubuntu installation. That user session will be started automatically on boot, and resource access will be mediated by [systemd-logind](https://www.freedesktop.org/software/systemd/man/latest/systemd-logind.service.html) - only the active session can access e.g. the GPU, the sound hardware and others.

The client will also run as part of that session, all managed by the [SystemD user manager](https://www.freedesktop.org/software/systemd/man/latest/user@.service.html). They will be autostarted on session startup.

You'll be able to run any Wayland application that's packaged as a snap, though some may expect more components of a user session (for example, [portals](https://github.com/flatpak/xdg-desktop-portal) or an audio server), which are out of scope for the Ubuntu Frame snap.

### The user session

SystemD has built-in facilities to start a user session. Let's configure one:

```
$ sudo systemctl edit --full --force user-session.service
```

Within the editor that opens, input these contents and exit:

```ini
[Service]
# This is what causes a user session to be allocated for the `ubuntu` user
User=ubuntu
PAMName=login
TTYPath=/dev/tty1

# A cheap way to wait indefinitely
ExecStart=/usr/bin/tail -f /dev/null
```

Start the service and you can confirm the session properties with `loginctl` (there may be more sessions listed, including e.g. your SSH one):

```
$ sudo systemctl start user-session.service
$ loginctl
SESSION  UID USER   SEAT  TTY
      1 1000 ubuntu seat0 tty1

1 sessions listed.
$ loginctl show-session --property Active 3
Active=yes
```

### Ubuntu Frame

We can now run Frame within the session. Let's configure a basic user service that start it:

```
$ systemctl --user edit --full --force ubuntu-frame.service
```

As above, input the contents and close the editor:

```ini
[Unit]
Before=xdg-desktop-autostart.target
BindsTo=graphical-session.target
[Service]
ExecStartPre=/usr/bin/dbus-update-activation-environment --systemd WAYLAND_DISPLAY=wayland-0
ExecStart=/snap/bin/ubuntu-frame
```

Start it, and you should see Frame's gradient background:

```
$ systemctl --user start ubuntu-frame.service
```

### Client applications

I'll go through a couple examples that showcase two ways to "bring" applications into the user session.

- _Using the app's `.desktop` file_
  All applications on Linux have a `.desktop` file that describes them - including what to execute to launch it. Another convention is that all `.desktop` files in `~/.config/autostart` are launched with the graphical session.
  We can use that through the [xdg-autostart-generator](https://www.freedesktop.org/software/systemd/man/latest/systemd-xdg-autostart-generator.html), which reads the `.desktop` files and generates user services.

  Let's install [flutter-gallery](https://snapcraft.io/flutter-gallery) and symlink its `.desktop` file to the autostart directory:

  ```
  $ sudo snap install flutter-gallery
  flutter-gallery v2.8.1-82-g358fe2dd7d from Flutter Team✓ installed
  $ mkdir --parents .config/autostart/
  $ ln --verbose --symlink \
      /var/lib/snapd/desktop/applications/flutter-gallery_flutter-gallery.desktop \
      .config/autostart/
  '.config/autostart/flutter-gallery_flutter-gallery.desktop' -> '/var/lib/snapd/desktop/applications/flutter-gallery_flutter-gallery.desktop'
  ```

  Now it's just a case of reloading the user manager so it picks that up and we can start it (note the quotes!):

  ```
  $ systemctl --user daemon-reload
  $ systemctl --user start 'app-flutter\x2dgallery_flutter\x2dgallery@autostart.service'
  # Stop it, if you want to try the next example
  $ systemctl --user stop 'app-flutter\x2dgallery_flutter\x2dgallery@autostart.service'
  ```

  You should see the Flutter Gallery running within Frame.

- _With a custom user service_
  If the app you want to use does not have a `.desktop` file, or for any other reason you want to use a custom unit, you just need an `ExecStart=` line. We'll use [graphics-test-tools](https://snapcraft.io/graphics-test-tools/) for that:

  ```
  $ sudo snap install graphics-test-tools
  $ systemctl --user edit --full --force glmark2.service
  ```

  Save those contents and exit:

  ```ini
  [Unit]
  After=ubuntu-frame.service
  [Service]
  ExecStart=/snap/bin/graphics-test-tools.glmark2-es2-wayland
  ```

  And start:

  ```
  $ systemctl --user start glmark2.service
  # Stop it again
  $ systemctl --user stop glmark2.service
  ```

  There, a prancing horse!

### Putting it all together

We could start the Frame service directly (instead of `tail`), but that would unnecessarily kill the user session. Instead we'll use a session target that will depend on all the pieces we want to run.

```
$ systemctl --user edit --full --force user-session.target
```

Input these and quit the editor:

```ini
[Unit]
# This will cause all .config/autostart .desktop files to start
Wants=ubuntu-frame.service xdg-desktop-autostart.target
# or the custom glmark2 one
# Wants=ubuntu-frame.service glmark2.service
```

Now you can replace the `tail` in `ExecStart` with this target:

```
$ sudo systemctl edit --full user-session.service
...
ExecStart=/usr/bin/systemctl --user start --wait user-session.target
...
```

Restart the user session service and things should all start up:

```
$ sudo systemctl restart user-session.service
```

Sprinkle `Restart=always` across the `[Service]` sections so things always come back up unless stopped:

```
$ sudo systemctl edit user-session.service
$ systemctl edit --user 'app-flutter\x2dgallery_flutter\x2dgallery@autostart.service'
$ systemctl edit --user --full glmark2.service
```

To start it on boot:

```
$ sudo systemctl add-wants graphical.target user-session.service
```

If you reboot now, the user session will start on boot, and with it Frame and the configured clients.

## Deployment

Bet you don't want to go through the above steps on the hundreds of devices you're going to deploy on. The way to avoid this with Ubuntu Core is to build a bespoke image fitting your solution. See {doc}`core:how-to-guides/image-creation/index` for a lot more information on this than we're going to cover.

### The gadget snap

From {doc}`core:how-to-guides/image-creation/build-a-gadget-snap`:

> Gadget snaps define and manipulate device-specific configuration and system properties

Rather than list all the changes to a gadget snap needed to build this solution, we'll maintain branches against stock gadgets for the PC and Pi platforms that you can modify to taste and go from there. We'll keep it heavily commented so it's clear what's happening where and why.

We'll rely on [cloud-init](https://cloudinit.readthedocs.io/en/latest/) to do the extra setup needed on first boot, with everything else being stock Ubuntu Core.

You can view the differences between the stock gadgets and our custom ones here, for the PC and Pi platforms, respectively:

https://github.com/canonical/pc-gadget/compare/22...MirServer:pc-gadget:22-frame

https://github.com/canonical/pi-gadget/compare/22-arm64...MirServer:pi-gadget:22-arm64-frame

To build it, just run Snapcraft within the checkout:

```
$ snapcraft
...
Created snap package pc_22-0.4_amd64.snap
```

There. Your gadget snap is ready.

### Building, testing and deploying the image

To build images from the gadget snaps we've prepared, we'll use [ubuntu-image](https://github.com/canonical/ubuntu-image) and [stock](https://github.com/canonical/models/) model {doc}`assertions <core:reference/assertions/model>`. Your solution may require custom models, but that's out of scope here.
Here are the assertions that interest us:

- [ubuntu-core-22-amd64-dangerous](https://github.com/canonical/models/blob/master/ubuntu-core-22-amd64-dangerous.model)
- [ubuntu-core-22-arm64-dangerous](https://github.com/canonical/models/blob/master/ubuntu-core-22-arm64-dangerous.model)
- [ubuntu-core-22-pi-arm64-dangerous](https://github.com/canonical/models/blob/master/ubuntu-core-22-pi-arm64-dangerous.model)

**NB**: they are "dangerous" because they allow inserting snaps when building the image. If you have the appropriate infrastructure (e.g. a {doc}`core:explanation/stores/dedicated-snap-store`), you can create and publish a properly signed model assertion instead.

To build the image, you run `ubuntu-image snap <model>`. To insert custom snaps, or additional ones from the store, pass `--snap <file> --snap <name>[=<channel>]`. You can read more about the available options in {doc}`ubuntu-image's manual <subiquity:reference/ubuntu-image>`.

We've wrapped all the above steps into a Makefile for easy consumption in the above branches. To build the image as-is, just run:

```
$ ./Makefile.frame
...
Created snap package pc_22-0.4_amd64.snap
...
2023-09-08 14:31:54 (21.4 MB/s) - 'ubuntu-core-22-amd64-dangerous.model' saved [1454/1454]
...
ubuntu-frame_amd64.img ready
```

There are a handful ways you can test that image - by {doc}`installing it on a device <core:tutorials/try-pre-built-images/install-on-a-device/index>`, or by {doc}`running it under QEMU <core:tutorials/try-pre-built-images/install-on-a-vm>`. Another approach is to use {doc}`virt-manager <server:how-to/virtualisation/virtual-machine-manager>`, creating the VM with the following command:

```
$ sudo virt-install --connect qemu:///session \
  --name ubuntu-frame \
  --memory 2048 \
  --vcpus 2 \
  --boot uefi \
  --os-variant ubuntu22.04 \
  --video virtio,accel3d=no \
  --graphics spice \
  --import --disk path=$PWD/ubuntu-frame_amd64.img,format=raw
```

## Summary

This approach makes for a very flexible solution, allowing the author of the image to control every facet of the deployment. You can easily import existing snap applications into the image and have it run, and restart automatically when things go wrong.

It does make for a complex setup, but it should simplify over time, particularly with SnapD introducing user services. A number of those could then be defined directly in the respective snaps.

More components could be brought into the session, including [Ubuntu Frame OSK](https://snapcraft.io/ubuntu-frame-osk), [Ubuntu Frame VNC](https://snapcraft.io/ubuntu-frame-vnc), and more. We'll cover that in the branches above.

### Refreshing snaps

With this solution, snaps won't refresh automatically, as it doesn't know how to restart the services when they're refreshed. You'll want to set up schedule to stop the user session, refresh the snaps and restart the session again at appropriate times.

When user services are first class citizens in SnapD, this will again get simpler, as you [can manage how and when do things get refreshed automatically](https://snapcraft.io/docs/managing-updates).

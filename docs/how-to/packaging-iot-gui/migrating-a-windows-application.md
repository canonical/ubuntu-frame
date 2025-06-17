(migrating-a-windows-application)=

# Migrating a Windows application

[Ubuntu Frame](https://mir-server.io/ubuntu-frame/) is the foundation for embedded displays. It provides a reliable, secure and easy way to embed your applications into a kiosk-style, IoT device, or digital signage solution. With Ubuntu Frame, the graphic application you choose or design gets a fullscreen window, a dedicated set of windows behaviours, input from touch, keyboard and mouse without needing to deal with the specific hardware, on-screen keyboard, and more.

Together with [Ubuntu Core](https://ubuntu.com/core), Ubuntu Frame provides all the infrastructure you need to securely deploy and maintain graphic applications on edge devices. And while Ubuntu Core maximizes performance and security of your apps, Ubuntu Frame is compatible with any Linux operating system that supports snaps.

This developer guide will show you how to deploy your graphic application that supports the [Wayland protocol](https://wayland.freedesktop.org/docs/html/) to work with Ubuntu Frame and Ubuntu Core. This guide is for developers looking to build kiosks, digital signage solutions, infotainment systems, IoT devices or any other applications that require a graphic interface running on a screen.

We will cover:

1. Setting up the tools and environment required to package and deploy your application on your desktop
1. Packaging the app as a snap and testing whether the snap works on your desktop
1. Packaging the snap for an IoT device and testing it on the device

If you want to learn how to install pre-built applications such as [wpe-webkit-mir-kiosk](https://snapcraft.io/wpe-webkit-mir-kiosk), [mir-kiosk-kodi](https://snapcraft.io/mir-kiosk-kodi/), or [Scummvm](https://snapcraft.io/scummvm), follow their official installation and configuration guides.

If you are new to Ubuntu Core, we recommend reading our {doc}`getting started document <core:tutorials/build-your-first-image/index>`. If you want to learn about building custom Ubuntu Core images, you could find information on the [snapcraft docs](https://snapcraft.io/docs/the-gadget-snap).

## Requirements

Developers use different tools and processes to build their graphic applications. For the purpose of this guide, we assume that you have an application that supports the Wayland protocol that you can test on your Linux-based desktop.

It is possible to work in a container or on a different computer (if snapd and X forwarding work well enough). But those options are outside the current scope.

For some of the later steps, you will need an [Ubuntu One account](https://login.ubuntu.com/). This will let you enable `remote-build` on your [Launchpad](https://launchpad.net/) account and publish on the [Snap Store](https://snapcraft.io).

## Setting up your test environment

**Ubuntu Frame** provides a tool for developers to simulate how their end application will look and respond in your development environment. So, you don’t need to work directly on your target device to perform the first design and usability iterations.

Open a terminal window and type:

```
sudo snap install ubuntu-frame --channel=22
```

[note status="channel=22"]
For Ubuntu Frame there are various channels corresponding to the snap bases that snaps are based on, in this case we use `--channel=22` which corresponds to `base: core22` which in turn refers to Ubuntu 22.04LTS.

````

**Frame-it** is a command-line utility for running snaps with Ubuntu Frame and is useful for testing on your development machine.

```
sudo snap install frame-it --classic
````

**Snapcraft** is a command-line utility for building snaps. This software allows users to build their own applications or software packages, and then publish them to the [Snap Store](https://snapcraft.io).

In the same terminal window type:

```
sudo snap install snapcraft --classic
```

If you don't have git installed, now is a good time to install it (on Ubuntu, use the command `sudo apt install git`).

## Packaging your application as a Snap

Now that you know how to confirm that an application is working with Ubuntu Frame, the next step is to use snap packaging to prepare the application for use on an IoT device. As before, this section will show you how to package your application together with some issues you might find and their troubleshoot.

### Snap packaging for IoT graphics

For use with [Ubuntu Core](https://ubuntu.com/core), your application needs to be packaged as a snap. This will also allow you to leverage Over The Air updates, automatic rollbacks, delta updates, update semantic channels, and more. If you don't use Ubuntu Core, but instead another form of Linux, we recommend you use snaps to get many of these advantages.

There's a lot of information about [packaging snaps online](https://ubuntu.com/tutorials/create-your-first-snap#1-overview), and the purpose here is not to teach about the {doc}`snapcraft <snapcraft:tutorials/craft-a-snap>` packaging tool or the [Snap Store](https://snapcraft.io/store). We will, instead, focus on the things that are special to IoT graphics.

Much of what you find online about packaging GUI applications as a snap refers to packaging for desktop. Some of that doesn't apply to IoT as Ubuntu Core and Ubuntu Server do not include everything a desktop installation does and the snaps need to run as {doc}`daemons <snapcraft:reference/project-file/snapcraft-yaml>` (background services) instead of being launched in a user session. In particular, for the time being, you should ignore various Snapcraft {doc}`extensions <snapcraft:how-to/extensions/index>` that help writing snap recipes that integrate with the desktop environment (e.g. using the correct theme) as they are not tested for use with Ubuntu Frame on Ubuntu Core.

Writing snap recipes without these extensions is not difficult as we'll illustrate for each of the example programs used in the previous section.

First, you will clone a repository containing a generic Snapcraft recipe for IoT graphics.

In the *same terminal window* you opened at the start of the last section, type:

```
git clone https://github.com/canonical/iot-example-graphical-snap.git
cd iot-example-graphical-snap
```

If you look in `snap/snapcraft.yaml`, you'll see a generic "snapcraft recipe" for an IoT graphics snap. This is where you will insert instructions for packaging your application. This is how the `.yaml` file looks like:

![image|690x575, 100%](8ba33b7d1bd1fc169b697a2a80d31d5962ebe905.jpeg)

The customised snapcraft recipe for each example described in this guide (i.e. GTK, Qt and SDL2) is on a corresponding branch in this repository:

```
$ git branch --list --remotes origin/22/*
  origin/22/Electron-quick-start
  origin/22/Flutter-demo
  origin/22/GTK3-adventure
  origin/22/GTK3-mastermind
  origin/22/Qt5-bomber
  origin/22/Qt5-bomber-first-try
  origin/22/Qt6-example
  origin/22/SDL2-neverputt
  origin/22/Wine-example
  origin/22/main
  origin/22/native-glmark2
  origin/22/x11-glxgears
```

[note status="22/"]
The "22" prefix refers to the snap bases that snaps are based on, in this case we use `22/` for branches using to `base: core22` (which in turn refers to Ubuntu 22.04LTS).

````

Once you have the customised snapcraft recipe you can snap your example applications.

Switch to the Wine example branch. Then use snapcraft to build the snap:

```
git checkout 22/Wine-example
snapcraft
````

Snapcraft is the packaging tool used to create snaps. We are not going to explore all its options here but, to avoid confusion, note that when you first run snapcraft, you will be asked "Support for 'multipass' needs to be set up. Would you like to do it now? \[y/N\]:", answer "yes".

After a few minutes, the snap will be built with a message like:

```
Snapped iot-example-graphical-snap_0+git.e656221_amd64.snap
```

You can then install and run the snap:

```
sudo snap install --dangerous *.snap
snap run iot-example-graphical-snap
```

The first time you run your snap with Ubuntu Frame installed, you are likely to see a warning:

```
$ snap run iot-example-graphical-snap
++ find /snap/iot-example-graphical-snap/x1/hacks -name 'setup-*'
+ for hack in $(find "${SNAP}/hacks" -name setup-\*)
+ . /snap/iot-example-graphical-snap/x1/hacks/setup-mir
++ set -x
++ export MIR_SERVER_PLATFORM_PATH=/snap/iot-example-graphical-snap/x1/usr/lib/x86_64-linux-gnu/mir/server-platform
++ MIR_SERVER_PLATFORM_PATH=/snap/iot-example-graphical-snap/x1/usr/lib/x86_64-linux-gnu/mir/server-platform
++ export MIR_SERVER_XWAYLAND_PATH=/snap/iot-example-graphical-snap/x1/usr/bin/Xwayland
++ MIR_SERVER_XWAYLAND_PATH=/snap/iot-example-graphical-snap/x1/usr/bin/Xwayland
+ for hack in $(find "${SNAP}/hacks" -name setup-\*)
+ . /snap/iot-example-graphical-snap/x1/hacks/setup-wayland-host
++ set -x
++ snapctl is-connected wayland
++ echo 'Wayland interface not connected!'
Wayland interface not connected!
++ exit 1
```

The WARNING "Wayland interface not connected!" is the key to the problem and comes from one of the scripts in the generic recipe. While developing your snap (that is, until your snap is uploaded to the store and any necessary “store assertions” granted), connecting many “interfaces” your snap uses needs to be done manually. Connect the `wayland` interface and try again:

```
/snap/iot-example-graphical-snap/current/bin/setup.sh
frame-it iot-example-graphical-snap
```

After a little time Frame's window should contain the example DNSBench app (we know it's not a great demo application, if you know about a better one, let us know!).

![obraz|690x437](055dae82692c95064bd6e15e1d4b8a91bcd04e50.png)

Close that. Your application has been successfully snapped.

### Packaging your own application

Runing a Windows application on Ubuntu requires the use of a compatibility layer - [Wine](https://www.winehq.org/). The snap uses a community-maintained set of utilities that make it easier to create a snap including the Windows application along with the elements required for Wine: [Sommelier Core](https://github.com/snapcrafters/sommelier-core). Refer to their documentation for more details of this layer, but some elements of the YAML are particularly interesting to use your app:

- installer URL, if your application should be downloaded and installed, rather than bundled with the snap:
  ```yaml
  INSTALL_URL: https://example.com/installer.exe
  ```
- installer flags, if the installer has a silent mode:
  ```yaml
  INSTALL_FLAGS: /silent
  ```
- the list of Wine tricks (e.g. libraries to be installed, see [the Wine tricks wiki page](https://gitlab.winehq.org/wine/wine/-/wikis/Winetricks)):
  ```yaml
  TRICKS: vcrun2019
  ```
- the executable to run:
  ```yaml
  # if it's installed in the Wine environment
  RUN_EXE: C:\path\to\installed\executable.exe
  # or if it's bundled with the snap
  RUN_EXE: $SNAP/executable.exe
  ```

Refer to [Sommelier Core README](https://github.com/snapcrafters/sommelier-core#variables) to see what other variables are available.

There is a lot more to say about getting Windows apps to run under Wine, but it's out of scope for this document.

## Building for and installing on a device

So far you explored the process for testing if your snapped application will work on Ubuntu Frame only using your desktop. While this accelerates the development process, you still need to consider the final board for your edge device. A lot of edge devices don’t use the amd64 architecture that is typical of development machines. Therefore, in this section, you will see how to build for and install on a device if your device uses a different architecture, using the SDL2 Neverputt application as an example. You will also see how to troubleshoot some common issues.

### Leveraging Snapcraft remote build tool

The simplest way to build your snap for other architectures is:

```
snapcraft remote-build
```

This uses the Launchpad build farm to build each of the architectures supported by the snap. This requires you to have a Launchpad account and to be okay uploading your snap source to a public location.

Once the build is complete, you can scp the .snap file to your IoT device and install using --dangerous.

### Installing on a device

For the sake of this guide, we are using a VM set up using the approach described in[ Ubuntu Core: Preparing a virtual machine with graphics support](https://ubuntu.com/tutorials/ubuntu-core-preparing-a-virtual-machine-with-graphics-support). Apart from the address used for scp and ssh this is the same as any other device and makes showing screenshots easier.

```
scp -P 10022 *.snap <username>@localhost:~
ssh -p 10022 <username>@localhost
snap install ubuntu-frame
snap install --dangerous *.snap
```

## Conclusion

From testing to deployment, this guide shows you how to use Ubuntu Frame to deploy your graphic applications. It covers topics as setting up the tools and environment on your desktop, testing if an application works with Ubuntu Frame, and packaging the application as a snap for an IoT device. It also included some issues you can encounter working with your applications and how to troubleshoot them.

We have also shown all the steps needed to get your snap running on a device. The rest of your development process is the same as for any other snap: uploading the snap to the store and installing on devices from there. There are just a few parting things to note:

Now that the Snap interfaces are configured, the application will automatically start when the system (re)starts.

Once you’ve uploaded the snap to the store, you can [request store assertions](https://forum.snapcraft.io/c/store-requests/19) to auto-connect any required interfaces. Alternatively, if you are building a Snap Appliance, then you can connect the interfaces in the “Gadget Snap”.

## Learn more

For more information about Ubuntu Frame please visit our [website](https://mir-server.io/ubuntu-frame).

You may also consider reading the following materials:

- {ref}`packaging-a-flutter-application`
- {ref}`make-a-secure-ubuntu-web-kiosk`.
- How to {ref}`enable on-screen keyboard support <ubuntu-frame-osk-documentation>` in Ubuntu Frame.

Need help in getting to market? [Contact us](https://ubuntu.com/internet-of-things/digital-signage#get-in-touch)

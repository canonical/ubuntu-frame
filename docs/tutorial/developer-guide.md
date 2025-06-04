(developer-guide)=

# Developer Guide

[Ubuntu Frame](https://mir-server.io/ubuntu-frame/) is the foundation for embedded displays. It provides a reliable, secure and easy way to embed your applications into a kiosk-style, IoT device, or digital signage solution. With Ubuntu Frame, the graphic application you choose or design gets a fullscreen window, a dedicated set of windows behaviours, input from touch, keyboard and mouse without needing to deal with the specific hardware, on-screen keyboard, and more.

Together with [Ubuntu Core](https://ubuntu.com/core), Ubuntu Frame provides all the infrastructure you need to securely deploy and maintain graphic applications on edge devices. And while Ubuntu Core maximises performance and security of your apps, Ubuntu Frame is compatible with any Linux operating system that supports snaps.

This developer guide will show you how to deploy your graphic application that supports the [Wayland protocol](https://wayland.freedesktop.org/docs/html/) to work with Ubuntu Frame and Ubuntu Core. This guide is for developers looking to build kiosks, digital signage solutions, infotainment systems, IoT devices or any other applications that require a graphic interface running on a screen.

We will cover:

1. Setting up the tools and environment required to package and deploy your application on your desktop
1. Testing if an application works with Ubuntu Frame on your desktop
1. Troubleshooting some common issues
1. Packaging the app as a snap and testing whether the snap works on your desktop
1. Packaging the snap for an IoT device and testing it on the device

If you want to learn how to install pre-built applications such as [wpe-webkit-mir-kiosk](https://snapcraft.io/wpe-webkit-mir-kiosk), [mir-kiosk-kodi](https://snapcraft.io/mir-kiosk-kodi/), or [Scummvm](https://snapcraft.io/scummvm), follow their official installation and configuration guides.

Note: This guide will not cover how to build an application using a toolkit that supports Wayland (there are many). And while it is possible to [package X11](https://discourse.ubuntu.com/t/making-x11-applications-talk-wayland/18631)-based applications to work on Ubuntu Core, this guide will not cover this either. We will also not cover how to upload your snap to the snap store, nor building custom Ubuntu Core images with pre-configured snaps. That is documented on [snapcraft.io/docs](https://snapcraft.io/docs).

If you are new to Ubuntu Core, we recommend reading our [getting started document](https://ubuntu.com/core/docs/getting-started). If you want to learn about building custom Ubuntu Core images, you could find information on the [snapcraft docs](https://snapcraft.io/docs/gadget-snap).

![image|388x581](upload://aykYNrjU60rwuUt34z2u1zrx66k.jpeg)

## Requirements

Developers use different tools and processes to build their graphic applications. For the purpose of this guide, we assume that you have an application that supports the Wayland protocol that you can test on your Linux-based desktop.

It is possible to work in a container or on a different computer (if snapd and X forwarding work well enough). But those options are outside the current scope.

For some of the later steps, you will need an [Ubuntu One account](https://login.ubuntu.com/). This will let you enable `remote-build` on your [Launchpad](https://launchpad.net/) account and publish on the [Snap Store](https://snapcraft.io).

![image|689x268](upload://894ZGy6nsTSlyR3kxrk2CfzSrLx.png)

## Setting up your test environment

**Ubuntu Frame** provides a tool for developers to simulate how their end application will look and respond in your development environment. So, you don’t need to work directly on your target device to perform the first design and usability iterations.

Open a terminal window and type:

```plain
sudo snap install ubuntu-frame --channel=22
```

[note status="channel=22"]
For Ubuntu Frame there are various channels corresponding to the snap bases that snaps are based on, in this case we use `--channel=22` which corresponds to `base: core22` which in turn refers to Ubuntu 22.04LTS.

````

**Frame-it** is a command-line utility for running snaps with Ubuntu Frame and is useful for testing on your development machine.

```plain
sudo snap install frame-it --classic
````

**Snapcraft** is a command-line utility for building snaps. This software allows users to build their own applications or software packages, and then publish them to the [Snap Store](https://snapcraft.io).

In the same terminal window type:

```plain
sudo snap install snapcraft --classic
```

If you don't have git installed, now is a good time to install it (on Ubuntu, use the command `sudo apt install git`).

## Checking your application works with Ubuntu Frame

There can be problems with both getting your application to work well with Ubuntu Frame and getting your application to work in a snap. To avoid confusion, we recommend first testing your application with Ubuntu Frame before packaging it as a snap. In this section, you will test your application, explore some common issues you might run into, and learn how to fix them.

### Testing your application with Frame-it

You can use Electron, Flutter, Qt, or any other toolkit or programming language to develop your graphic application. There is no sole path for checking all of them. Instead, this guide will use some example applications using GTK, QT, and SDL2.

The examples used here are game applications, such as Mastermind, Neverputt, and Bomber. We’ve chosen these applications as they are easily installable and are designed to work without a full desktop session. But they can be replaced by your kiosk application, industrial GUI, smart fridge GUI, digital sign and more.

The first step is to download the application and execute it:

#### GTK Example: Mastermind

```plain
sudo apt install gnome-mastermind
frame-it gnome-mastermind
```

Now Frame's window should contain the "Mastermind" game.

![image|690x575](upload://zjKIC8nBpT1QdvW2Y93PTBDuhv2.jpeg)

If your application doesn’t appear in the Ubuntu Frame window or look right at this stage, then this is the time to work out the fix, before packaging as a snap.

Close Mastermind (`Ctrl-Q`) and try the next example:

#### Qt Example: Bomber

```plain
sudo apt install bomber
frame-it bomber
```

Now Frame's window should contain the "Bomber" game as shown in the next image.

![image|690x574](upload://fb08ypmlNuZFtnBg65Iw6AzFBD8.jpeg)

Close that (`Ctrl-Q`) and try the next example.

#### SDL2 Example: Neverputt

```plain
sudo apt install neverputt
frame-it neverputt
```

Now Frame's window should contain the "Neverputt" game. You’ll see is that the game doesn’t fill the display. That’s because the application doesn’t understand Ubuntu Frame telling it to fill the screen. This is a problem with some applications and, in this case, can be fixed by editing `~/.neverball/neverballrc` [sic] to say “fullscreen 1” and restarting the game. (The same file works for neverball, but you may need to figure out the right configuration option for your application.) This is what you will see:

![image|690x575](upload://qsInLi6cXvNjZCH8AH5fCdrzC9i.jpeg)

You can now close the app.

## Packaging your application as a Snap

Now that you know how to confirm that an application is working with Ubuntu Frame, the next step is to use snap packaging to prepare the application for use on an IoT device. As before, this section will show you how to package your application together with some issues you might find and their troubleshoot.

### Snap packaging for IoT graphics

For use with [Ubuntu Core](https://ubuntu.com/core), your application needs to be packaged as a snap. This will also allow you to leverage Over The Air updates, automatic rollbacks, delta updates, update semantic channels, and more. If you don't use Ubuntu Core, but instead another form of Linux, we recommend you use snaps to get many of these advantages.

There's a lot of information about [packaging snaps online](https://ubuntu.com/tutorials/create-your-first-snap#1-overview), and the purpose here is not to teach about the [snapcraft](https://snapcraft.io/docs/snapcraft-overview) packaging tool or the [Snap Store](https://snapcraft.io/store). We will, instead, focus on the things that are special to IoT graphics.

Much of what you find online about packaging GUI applications as a snap refers to packaging for desktop. Some of that doesn't apply to IoT as Ubuntu Core and Ubuntu Server do not include everything a desktop installation does and the snaps need to run as [daemons](https://snapcraft.io/docs/services-and-daemons) (background services) instead of being launched in a user session. In particular, for the time being, you should ignore various Snapcraft [extensions](https://snapcraft.io/docs/snapcraft-extensions) that help writing snap recipes that integrate with the desktop environment (e.g. using the correct theme) as they are not tested for use with Ubuntu Frame on Ubuntu Core.

Writing snap recipes without these extensions is not difficult as we'll illustrate for each of the example programs used in the previous section.

First, you will clone a repository containing a generic Snapcraft recipe for IoT graphics.

In the *same terminal window* you opened at the start of the last section, type:

```plain
git clone https://github.com/MirServer/iot-example-graphical-snap.git
cd iot-example-graphical-snap
```

If you look in `snap/snapcraft.yaml`, you'll see a generic "snapcraft recipe" for an IoT graphics snap. This is where you will insert instructions for packaging your application. This is how the `.yaml` file looks like:

![image|690x575](upload://jVi5IdEqljarHP0XyJjgOhrmtKd.jpeg)

The customised snapcraft recipe for each example described in this guide (i.e. GTK, Qt and SDL2) is on a corresponding branch in this repository:

```plain
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

### Toolkit examples

#### GTK Example: Mastermind

Switch to the GTK example branch. Then use snapcraft to build the snap:

```plain
git checkout 22/GTK3-mastermind
snapcraft
````

Snapcraft is the packaging tool used to create snaps. We are not going to explore all its options here but, to avoid confusion, note that when you first run snapcraft, you will be asked "Support for 'multipass' needs to be set up. Would you like to do it now? \[y/N\]:", answer "yes".

After a few minutes, the snap will be built with a message like:

```plain
Created snap package iot-example-graphical-snap_0+git.fc3da3d_amd64.snap
```

You can then install and run the snap:

```plain
sudo snap install --dangerous iot-example-graphical-snap_0+git.fc3da3d_amd64.snap
frame-it iot-example-graphical-snap
```

The first time you run your snap with Ubuntu Frame installed, you are likely to see a warning:

```plain
WARNING: wayland interface not connected! Please run: /snap/iot-example-graphical-snap/current/bin/setup.sh

(gnome-mastermind:639854): Gtk-WARNING **: 12:30:27.215: cannot open display:
[2023-08-09 12:30:27.218285] < - debug - > mirserver: Handling Terminated from pid=639737
[2023-08-09 12:30:27.218522] < -warning- > mirserver: wl_surface@12 destroyed before associated role
```

The first WARNING is the key to the problem and comes from one of the scripts in the generic recipe. While developing your snap (that is, until your snap is uploaded to the store and any necessary “store assertions” granted), connecting any “interfaces” your snap uses needs to be done manually. As the message suggests, there’s a helper script for this. Run it and try again:

```plain
/snap/iot-example-graphical-snap/current/bin/setup.sh
frame-it iot-example-graphical-snap
```

Now Frame’s window should contain the “Mastermind” game.

![image|690x575](upload://c8D1ubEYF9IKdIQgRPAHEOtDQBI.png)

Close that (`Ctrl-Q`) and try the next example:

#### Qt Example: Bomber

To avoid confusion, delete the .snap file created with the previous example:

```plain
rm *.snap
```

Now switch to the Qt first-try example branch. Then build, install, and run the snap:

```plain
git checkout 22/Qt5-bomber-first-try
snapcraft
sudo snap install --dangerous *.snap
frame-it iot-example-graphical-snap
```

If you’ve been paying attention, you’ll notice that the branch name `Qt5-bomber-first-try` is not what you might expect. This is to show you the sort of problem you might encounter:

```plain
Warning: Ignoring XDG_SESSION_TYPE=wayland on Gnome. Use QT_QPA_PLATFORM=wayland to run on Wayland anyway.
QSocketNotifier: Can only be used with threads started with QThread
kf.dbusaddons: DBus session bus not found. To circumvent this problem try the following command (with bash):
    export $(dbus-launch)
```

This didn’t work as the bomber application requires a DBus “session bus”. To solve this issue, you can provide one within the snap using dbus-run-session. You can see exactly how this is done by comparing branches:

```
git diff 22/Qt5-bomber-first-try origin/22/Qt5-bomber

```

Or you can just switch to the 22/Qt5-bomber example branch. Then build, install and run the snap:

```plain
git checkout 22/Qt5-bomber
snapcraft
sudo snap install --dangerous *.snap
frame-it iot-example-graphical-snap
```

Now Frame's window should contain the "Bomber" game.

![image|690x575](upload://3uy0A96vsQTQDfYVLkzIXBaFGlc.jpeg)

Close that (`Ctrl-Q`) and try the next example:

#### SDL2 Example: Neverputt

To avoid confusion, delete the .snap file created with the previous example:

```plain
rm *.snap
```

Now switch to the SDL2 example branch. Then build, install, and run the snap:

```plain
git checkout 22/SDL2-neverputt
snapcraft
sudo snap install --dangerous *.snap
frame-it iot-example-graphical-snap
```

Now Ubuntu Frame's window should contain the "Neverputt" game.

But you are likely to see a warning as Neverputt uses additional plugs that have not been connected:

```plain
WARNING: hardware-observe interface not connected! Please run: /snap/iot-example-graphical-snap/current/bin/setup.sh
WARNING: joystick interface not connected! Please run: /snap/iot-example-graphical-snap/current/bin/setup.sh
ALSA lib conf.c:4120:(snd_config_update_r) Cannot access file /usr/share/alsa/alsa.conf
Failure to initialize SDL (Could not initialize UDEV)
```

You have warnings requesting that the setup script is run. This is because the Neverputt snap needs some interfaces that you haven’t connected yet, and this could be the case with other applications that you build. Identifying the interfaces needed by your application and adding them to the snapcraft.yaml recipe is one of the things you need to do at this stage.

Run the setup script to connect the missing interfaces, and try again:

```plain
/snap/iot-example-graphical-snap/current/bin/setup.sh
frame-it iot-example-graphical-snap
```

Now Frame's window should contain the "Neverputt" game.

![image|690x575](upload://r7frB6bMFVvOqQeY5HJrE5Ly3pe.jpeg)

Close that. Your application has been successfully snapped.

### Packaging your own application

When packaging an application there are many issues to address: what needs to be in the snap, how does the runtime environment need to be configured and what interfaces are needed.

You might get some inspiration from the examples we’ve given. You can see the customisation used in each example using git diff for example:

```plain
git diff 22/main 22/SDL2-neverputt
```

You’ll see, for example, the `SDL_VIDEODRIVER` settings and the `neverballrc` file in this example. But we can’t go into all the possibilities and tools in this guide. There are helpful docs and forums on the [Snapcraft website](https://snapcraft.io/).

## Building for and installing on a device

So far you explored the process for testing if your snapped application will work on Ubuntu Frame only using your desktop. While this accelerates the development process, you still need to consider the final board for your edge device. A lot of edge devices don’t use the amd64 architecture that is typical of development machines. Therefore, in this section, you will see how to build for and install on a device if your device uses a different architecture, using the SDL2 Neverputt application as an example. You will also see how to troubleshoot some common issues.

### Leveraging Snapcraft remote build tool

The simplest way to build your snap for other architectures is:

```plain
snapcraft remote-build
```

This uses the Launchpad build farm to build each of the architectures supported by the snap. This requires you to have a Launchpad account and to be okay uploading your snap source to a public location.

Once the build is complete, you can scp the .snap file to your IoT device and install using --dangerous.

For the sake of this guide, we are using a VM set up using the approach described in[ Ubuntu Core: Preparing a virtual machine with graphics support](https://ubuntu.com/tutorials/ubuntu-core-preparing-a-virtual-machine-with-graphics-support). Apart from the address used for scp and ssh this is the same as any other device and makes showing screenshots easier.

```plain
scp -P 10022 *.snap <username>@localhost:~
ssh -p 10022 <username>@localhost
snap install ubuntu-frame
snap install --dangerous *.snap
```

You'll see the Ubuntu Frame grayscale background once that instals, but (if you've been following the steps precisely) you won't see Neverputt start:

```plain
$ snap logs  iot-example-graphical-snap
2022-06-23T16:17:51Z iot-example-graphical-snap.iot-example-graphical-snap[4210]: WARNING: hardware-observe interface not connected! Please run: /snap/iot-example-graphical-snap/current/bin/setup.sh
2022-06-23T16:17:51Z iot-example-graphical-snap.iot-example-graphical-snap[4210]: WARNING: audio-playback interface not connected! Please run: /snap/iot-example-graphical-snap/current/bin/setup.sh
2022-06-23T16:17:51Z iot-example-graphical-snap.iot-example-graphical-snap[4210]: WARNING: joystick interface not connected! Please run: /snap/iot-example-graphical-snap/current/bin/setup.sh
2022-06-23T16:17:51Z iot-example-graphical-snap.iot-example-graphical-snap[4210]: Failure to initialize SDL (Could not initialize UDEV)
2022-06-23T16:17:51Z systemd[1]: snap.iot-example-graphical-snap.iot-example-graphical-snap.service: Succeeded.
2022-06-23T16:17:51Z systemd[1]: snap.iot-example-graphical-snap.iot-example-graphical-snap.service: Scheduled restart job, restart counter is at 5.
2022-06-23T16:17:51Z systemd[1]: Stopped Service for snap application iot-example-graphical-snap.iot-example-graphical-snap.
2022-06-23T16:17:51Z systemd[1]: snap.iot-example-graphical-snap.iot-example-graphical-snap.service: Start request repeated too quickly.
2022-06-23T16:17:51Z systemd[1]: snap.iot-example-graphical-snap.iot-example-graphical-snap.service: Failed with result 'start-limit-hit'.
2022-06-23T16:17:51Z systemd[1]: Failed to start Service for snap application iot-example-graphical-snap.iot-example-graphical-snap.
```

All these WARNING messages give the clue: you’re still developing the snap and interfaces are not yet being connected automatically. So, connect the missing interfaces and manually start the daemon:

```plain
/snap/iot-example-graphical-snap/current/bin/setup.sh
snap start iot-example-graphical-snap
```

You should see Neverputt starting.

![image|690x575](upload://qDTS1ykgoG6WGlm2Yu1u0gLMnOo.jpeg)

## Conclusion

From testing to deployment, this guide shows you how to use Ubuntu Frame to deploy your graphic applications. It covers topics as setting up the tools and environment on your desktop, testing if an application works with Ubuntu Frame, and packaging the application as a snap for an IoT device. It also included some issues you can encounter working with your applications and how to troubleshoot them.

We have also shown all the steps needed to get your snap running on a device. The rest of your development process is the same as for any other snap: uploading the snap to the store and installing on devices from there. There are just a few parting things to note:

Now that the Snap interfaces are configured, the application will automatically start when the system (re)starts.

Once you’ve uploaded the snap to the store, you can [request store assertions](https://forum.snapcraft.io/c/store-requests/19) to auto-connect any required interfaces. Alternatively, if you are building a Snap Appliance, then you can connect the interfaces in the “Gadget Snap”.

## Learn more

For more information about Ubuntu Frame please visit our [website](https://mir-server.io/ubuntu-frame).

You may also consider reading the following materials:

- How to [run Flutter applications on Ubuntu Core](https://ubuntu.com/tutorials/run-flutter-applications-on-ubuntu-core#1-overview)
- How to leverage existing snaps to [build a webkiosk](https://ubuntu.com/tutorials/secure-ubuntu-kiosk#1-overview).
- How to [configure audio on Ubuntu Core](https://github.com/MirServer/iot-example-graphical-snap/wiki/How-to-configure-audio-on-Ubuntu-Core)
- How to [enable on-screen keyboard support](https://discourse.ubuntu.com/t/on-screen-keyboard-support-in-ubuntu-frame/25840) in Ubuntu Frame.

Need help in getting to market? [Contact us](https://ubuntu.com/internet-of-things/digital-signage#get-in-touch)

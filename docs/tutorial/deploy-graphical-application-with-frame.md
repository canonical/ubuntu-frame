(tutorial-deploy-graphical-application)=

# Deploy your graphical application

[Ubuntu Frame](https://mir-server.io/ubuntu-frame/) is the foundation for embedded displays. It provides a reliable, secure and easy way to embed your applications into a kiosk-style, IoT device, or digital signage solution. With Ubuntu Frame, the graphic application you choose or design gets a fullscreen window, a dedicated set of windows behaviours, input from touch, keyboard and mouse without needing to deal with the specific hardware, on-screen keyboard, and more.

Together with [Ubuntu Core](https://ubuntu.com/core), Ubuntu Frame provides all the infrastructure you need to securely deploy and maintain graphic applications on edge devices. And while Ubuntu Core maximizes performance and security of your apps, Ubuntu Frame is compatible with any Linux operating system that supports snaps.

This tutorial helps you deploy your graphic application that supports the [Wayland protocol](https://wayland.freedesktop.org/docs/html/) to work with Ubuntu Frame and Ubuntu Core. This guide is for developers looking to build kiosks, digital signage solutions, infotainment systems, IoT devices or any other applications that require a graphic interface running on a screen.

Note that this tutorial does not cover how to build an application using a toolkit that supports Wayland or to {ref}`package X11 <packaging-an-x11-based-application>`-based applications to work on Ubuntu Core. It also does not cover how to upload your snap to the snap store, nor building custom Ubuntu Core images with pre-configured snaps. For information on these topics, see [Snapcraft documentation](https://snapcraft.io/docs).

If you want to learn how to install pre-built applications such as [wpe-webkit-mir-kiosk](https://snapcraft.io/wpe-webkit-mir-kiosk), [mir-kiosk-kodi](https://snapcraft.io/mir-kiosk-kodi/), or [Scummvm](https://snapcraft.io/scummvm), follow their official installation and configuration guides.

Here's a summary of what this tutorial covers:

1. Setting up the tools and environment required to package and deploy your application on your desktop
1. Testing if an application works with Ubuntu Frame on your desktop
1. Troubleshooting some common issues
1. Packaging the app as a snap and testing whether the snap works on your desktop
1. Packaging the snap for an IoT device and testing it on the device

## Preparation

If you are new to Ubuntu Core, we recommend reading {doc}`core:tutorials/build-your-first-image/index`. If you want to learn about building custom Ubuntu Core images, you could find information on the [snapcraft docs](https://snapcraft.io/docs/the-gadget-snap).

For the purpose of this tutorial, we assume that you have an application that supports the Wayland protocol that you can test on your Linux-based desktop.

For some of the later steps, you will need an [Ubuntu One account](https://login.ubuntu.com/). This will let you enable `remote-build` on your [Launchpad](https://launchpad.net/) account and publish on the [Snap Store](https://snapcraft.io).

### Setting up your test environment

**Ubuntu Frame** provides a tool to simulate how your end application will look and respond in your development environment. So, you don’t need to work directly on your target device to perform the first design and usability iterations.

Install Ubuntu Frame:

    sudo snap install ubuntu-frame --channel=24

Ubuntu Frame has various channels corresponding to the snap bases, we use `--channel=24` here which corresponds to `base: core24` which in turn refers to Ubuntu 24.04 LTS.

Install Frame-it, a command-line utility for running snaps with Ubuntu Frame and testing on your development machine:
    sudo snap install frame-it --classic

Install Snapcraft, a command-line utility for building snaps. This software allows users to build their own applications or software packages, and then publish them to the [Snap Store](https://snapcraft.io):

In the same terminal window type:

    sudo snap install snapcraft --classic

If you don't have git installed, now is a good time to install it (on Ubuntu, use the command `sudo apt install git`).

## Checking your application works with Ubuntu Frame

There can be problems with both getting your application to work well with Ubuntu Frame and getting your application to work in a snap. To avoid confusion, we recommend first testing your application with Ubuntu Frame before packaging it as a snap. In this section, you will test your application, explore some common issues you might run into, and learn how to fix them.

You can any toolkit or framework such as Flutter, Electron, Qt to develop your graphic application. There is no sole path for checking all of them. Instead, this tutorial will focus on some example applications.

The examples used here are game applications, such as Mastermind, Neverputt, and Bomber. We’ve chosen these applications as they are easily installable and are designed to work without a full desktop session. But they can be replaced by your kiosk application, industrial GUI, smart fridge GUI, digital sign and more.

The first step is to download the application and execute it - let's use the GTK example, Mastermind:

```
sudo apt install gnome-mastermind
frame-it gnome-mastermind
```

Now Frame's window should contain the "Mastermind" game.

![image|690x575](f787b8fe63197afac80c6f50cbeb2753d7584ee8.jpeg)

If your application doesn’t appear in the Ubuntu Frame window or look right at this stage, then this is the time to work out the fix, before packaging as a snap.

Similarly, you can try the Qt example, "Bomber" and the SDL example, "Neverputt".

With Neverputt, you are probably wondering why the game doesn’t fill the display. That’s because the application doesn’t understand Ubuntu Frame telling it to fill the screen. This is a problem with some applications and, in this case, can be fixed by editing `~/.neverball/neverballrc` [sic] to say “fullscreen 1” and restarting the game. (The same file works for neverball, but you may need to figure out the right configuration option for your application.) Once fixed, this is what you will see:

![image|690x575](b97782d3fb25d700a2f0f087f758be88d72f11d0.jpeg)

You can now close the app.

## Packaging your application as a Snap

When you have confirmed that an application is working with Ubuntu Frame, the next step is to use snap packaging to prepare the application for use on an IoT device.

### Snap packaging for IoT graphics

For use with [Ubuntu Core](https://ubuntu.com/core), your application needs to be packaged as a snap. This will also allow you to leverage over-the-air updates, automatic rollbacks, delta updates, update semantic channels, and more. If you don't use Ubuntu Core, but instead another form of Linux, we recommend you use snaps to get many of these advantages.

There's a lot of information about [packaging snaps online](https://ubuntu.com/tutorials/create-your-first-snap#1-overview), and the purpose of this tutorial is not to teach about the {doc}`snapcraft <snapcraft:tutorials/craft-a-snap>` packaging tool or the [Snap Store](https://snapcraft.io/store). We will, instead, focus on the things that are special to IoT graphics.

Much of what you find online about packaging GUI applications as a snap refers to packaging for desktop. Some of that doesn't apply to IoT as Ubuntu Core and Ubuntu Server do not include everything a desktop installation does and the snaps need to run as {doc}`daemons<snapcraft:reference/project-file/anatomy-of-snapcraft-yaml>` (background services) instead of being launched in a user session. In particular, for the time being, you should ignore various Snapcraft {doc}`extensions <snapcraft:how-to/extensions/index>` that help writing snap recipes that integrate with the desktop environment (e.g. using the correct theme) as they are not tested for use with Ubuntu Frame on Ubuntu Core.

Writing snap recipes without these extensions is not difficult.

In the *same terminal window* you opened at the start of the last section, clone a repository containing a generic Snapcraft recipe for IoT graphics:

```
git clone https://github.com/canonical/iot-example-graphical-snap.git
cd iot-example-graphical-snap
```

If you look in `snap/snapcraft.yaml`, you'll see a generic "snapcraft recipe" for an IoT graphics snap. This is where you will insert instructions for packaging your application. This is how the `.yaml` file looks like:

![image|690x575](8ba33b7d1bd1fc169b697a2a80d31d5962ebe905.jpeg)

The customised snapcraft recipe for each example described in this tutorial (i.e. GTK, Qt and SDL2) is on a corresponding branch in this repository:

```
$ git branch --list --remotes origin/24/*

```

origin/24/Electron-quick-start
origin/24/Flutter-demo
origin/24/GTK3-adventure
origin/24/GTK3-mastermind
origin/24/Qt5-bomber
origin/24/Qt5-bomber-first-try
origin/24/Qt6-example
origin/24/SDL2-neverputt
origin/24/main
origin/24/native-glmark2
origin/24/x11-glxgears

```{tip}
The "24" prefix refers to the snap bases, in this case we use `24/` for branches using to `base: core24`.
```

Once you have the customised snapcraft recipe, you can snap your example toolkit applications.

### GTK example: Mastermind

Switch to the GTK example branch. Then use snapcraft to build the snap:

```
git checkout 24/GTK3-mastermind
snapcraft
```

```{tip}
When you first run snapcraft, you will be asked "Support for 'multipass' needs to be set up. Would you like to do it now? \[y/N\]:", answer "yes".
```

After a few minutes, the snap will be built with a message like:

```
Created snap package iot-example-graphical-snap_0+git.fc3da3d_amd64.snap
```

You can then install and run the snap:

```
sudo snap install --dangerous iot-example-graphical-snap_0+git.fc3da3d_amd64.snap
frame-it iot-example-graphical-snap
```

The first time you run your snap with Ubuntu Frame installed, you are likely to see a warning:

```
WARNING: wayland interface not connected! Please run: /snap/iot-example-graphical-snap/current/bin/setup.sh

(gnome-mastermind:639854): Gtk-WARNING **: 12:30:27.215: cannot open display:
```

The first WARNING is the key to the problem and comes from one of the scripts in the generic recipe. While developing your snap (that is, until your snap is uploaded to the store and any necessary “store assertions” granted), connecting any “interfaces” your snap uses needs to be done manually. As the message suggests, there’s a helper script for this. Run it and try again:

```
/snap/iot-example-graphical-snap/current/bin/setup.sh
frame-it iot-example-graphical-snap
```

Now Frame’s window should contain the “Mastermind” game.

![image|690x575](5513cee9e8851b25d97704a32153fbe513087bca.png)

Close the window (`Ctrl-Q`) and try the next example:

### Qt example: Bomber

Now, try the Qt application. To avoid confusion, delete the .snap file created with the previous example:

```
rm *.snap
```

Now switch to the Qt first-try example branch. Then build, install, and run the snap:

```
git checkout 24/Qt5-bomber-first-try
snapcraft
sudo snap install --dangerous *.snap
frame-it iot-example-graphical-snap
```

If you’ve been paying attention, you’ll notice that the branch name `Qt5-bomber-first-try` is not what you might expect. This is to show you the sort of problem you might encounter:

```
Warning: Ignoring XDG_SESSION_TYPE=wayland on Gnome. Use QT_QPA_PLATFORM=wayland to run on Wayland anyway.
QSocketNotifier: Can only be used with threads started with QThread
kf.dbusaddons: DBus session bus not found. To circumvent this problem try the following command (with bash):
    export $(dbus-launch)
```

This didn’t work as the bomber application requires a DBus “session bus”. To solve this issue, you can provide one within the snap using dbus-run-session. You can see exactly how this is done by comparing branches:

```
git diff 24/Qt5-bomber-first-try origin/24/Qt5-bomber

```

Or you can just switch to the 24/Qt5-bomber example branch. Then build, install and run the snap:

```
git checkout 24/Qt5-bomber
snapcraft
sudo snap install --dangerous *.snap
frame-it iot-example-graphical-snap
```

Now Frame's window should contain the "Bomber" game.

![image|690x575](187a8b5c29c0fc1069c37b1e6a41861a86dadd42.jpeg)

Close the window (`Ctrl-Q`) and try the next example:

### SDL2 example: Neverputt

Delete the .snap file created with the previous example:

```
rm *.snap
```

Now switch to the SDL2 example branch. Then build, install, and run the snap:

```
git checkout 24/SDL2-neverputt
snapcraft
sudo snap install --dangerous *.snap
frame-it iot-example-graphical-snap
```

Now Ubuntu Frame's window should contain the "Neverputt" game.

But you are likely to see a warning as Neverputt uses additional plugs that have not been connected:

```
WARNING: hardware-observe interface not connected! Please run: /snap/iot-example-graphical-snap/current/bin/setup.sh
WARNING: joystick interface not connected! Please run: /snap/iot-example-graphical-snap/current/bin/setup.sh
ALSA lib conf.c:4120:(snd_config_update_r) Cannot access file /usr/share/alsa/alsa.conf
Failure to initialize SDL (Could not initialize UDEV)
```

You have warnings requesting that the setup script is run. This is because the Neverputt snap needs some interfaces that you haven’t connected yet, and this could be the case with other applications that you build. Identifying the interfaces needed by your application and adding them to the snapcraft.yaml recipe is one of the things you need to do at this stage.

Run the setup script to connect the missing interfaces, and try again:

```
/snap/iot-example-graphical-snap/current/bin/setup.sh
frame-it iot-example-graphical-snap
```

Now Frame's window should contain the "Neverputt" game.

![image|690x575](be0c79621a30e294669b5954316cfbb96d0c3de8.jpeg)

Close the window. Your application has been successfully snapped.

## Packaging your own application

When packaging an application, there are many issues to address: what needs to be in the snap, how does the runtime environment need to be configured and what interfaces are needed.

You might get some inspiration from the examples we’ve given. You can see the customisation used in each example using git diff for example:

```
git diff 24/main 24/SDL2-neverputt
```

You’ll see, for example, the `SDL_VIDEODRIVER` settings and the `neverballrc` file in this example. For additional information, there are helpful docs and forums on the [Snapcraft website](https://snapcraft.io/).

## Building for and installing on a device

So far you explored the process for testing if your snapped application will work on Ubuntu Frame only using your desktop. While this accelerates the development process, you still need to consider the final board for your edge device. A lot of edge devices don’t use the AMD64 architecture that is typical of development machines. So let's see how to build for and install on a device if your device uses a different architecture, using the SDL2 Neverputt application as an example.

The simplest way to build your snap for other architectures is to leverage the Snapcraft remote build tool:

```
snapcraft remote-build
```

The remote build tool uses the Launchpad build farm to build each of the architectures supported by the snap. To do this, you need a Launchpad account and consent to uploading your snap source to a public location.

Once the build is complete, you can scp the .snap file to your IoT device and install using --dangerous.

In this example, we are using a VM set up using the approach described in[ Ubuntu Core: Preparing a virtual machine with graphics support](https://ubuntu.com/tutorials/ubuntu-core-preparing-a-virtual-machine-with-graphics-support). Apart from the address used for scp and ssh, this is the same as any other device and makes showing screenshots easier.

```
scp -P 10022 *.snap <username>@localhost:~
ssh -p 10022 <username>@localhost
snap install ubuntu-frame
snap install --dangerous *.snap
```

You'll see the Ubuntu Frame grayscale background once that installs, but you won't see Neverputt start. Look at the logs to find out why:

```
$ snap logs iot-example-graphical-snap
# these are abridged
WARNING: hardware-observe interface not connected! Please run: /snap/iot-example-graphical-snap/current/bin/setup.sh
WARNING: audio-playback interface not connected! Please run: /snap/iot-example-graphical-snap/current/bin/setup.sh
WARNING: joystick interface not connected! Please run: /snap/iot-example-graphical-snap/current/bin/setup.sh
Failure to initialize SDL (Could not initialize UDEV)
```

You’re still developing the snap and interfaces are not yet being connected automatically. So, connect the missing interfaces and manually start the daemon:

```
/snap/iot-example-graphical-snap/current/bin/setup.sh
snap start iot-example-graphical-snap
```

You should see Neverputt starting.

![image|690x575](babb31bb8373e8350ed028413d1d3f3b43b6b300.jpeg)

## Next Steps

The rest of your development process is the same as for any other snap: uploading the snap to the store and installing on devices from there.

Now that the Snap interfaces are configured, the application will automatically start when the system (re)starts.

Once you’ve uploaded the snap to the store, you can [request store assertions](https://forum.snapcraft.io/c/store-requests/19) to auto-connect any required interfaces. Alternatively, if you are building a Snap Appliance, then you can connect the interfaces in the “Gadget Snap”.

## Learn more

For more information about Ubuntu Frame, visit our [website](https://mir-server.io/ubuntu-frame). 

Other useful guides to learn about Ubuntu Frame are:
- {ref}`packaging-a-flutter-application`
- How to leverage existing snaps to {ref}`build a webkiosk <make-a-secure-ubuntu-web-kiosk>`
- How to {ref}`enable on-screen keyboard support <ubuntu-frame-osk-documentation>` in Ubuntu Frame

Need help in getting to market? [Contact us](https://ubuntu.com/internet-of-things/digital-signage#get-in-touch)

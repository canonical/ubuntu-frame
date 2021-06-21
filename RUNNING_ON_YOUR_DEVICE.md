# Ubuntu Frame

This is a quick introduction to running Ubuntu Frame on a device.

## Setup your device

You need either Ubuntu Core or a version of Linux supporting snaps.

1. Setting up Ubuntu Core is described in [Get Ubuntu for IoT](https://ubuntu.com/download/iot)
2. Setting up snapd on other versions of Linux is described in [Installing snapd](https://snapcraft.io/docs/installing-snapd)

Follow the instructions to install Ubuntu Core/snapd and ssh into your device.

## Install Ubuntu Frame

You should now have a command prompt reading something like `<your‑user>@yourdevice:~$`. Now install Ubuntu Frame with the following command:

    $ snap install ubuntu-frame

If you are using Ubuntu Core, you can skip this next step: On other versions of Linux, Ubuntu Core does not start by default. To change this default run the following command:

    $ snap set ubuntu-frame daemon=true

Once ubuntu-frame starts the installation completes the display should show a graduated grey screen. This is the default Ubuntu Frame wallpaper. This can be changed using the “config” snap configuration option:

    $ snap set ubuntu-frame config="
    wallpaper-top=0x92006a
    wallpaper-bottom=0xdd4814
    "

The `wallpaper-top` and `wallpaper-bottom` are RGB values. There are a lot of configuration options to control the kiosk display, but they are all set using `config` and `display` (more details in [REFERENCE.md](REFERENCE.md))

## Install a Web Kiosk

Still in your ssh session, install web kiosk:

    $ snap install wpe-webkit-mir-kiosk

Once the installation completes the display should show the WPE website. This can be changed using the “url” snap configuration option:

    $ snap set wpe-webkit-mir-kiosk url=https://mir-server.io

This will show the Mir Server website.

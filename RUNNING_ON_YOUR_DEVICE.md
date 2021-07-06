# Ubuntu Frame

This is a quick reference for running Ubuntu Frame on a device.

## Setup your device

You need either [Ubuntu Core](https://ubuntu.com/core) or a version of Linux supporting snaps.

1. Setting up Ubuntu Core is described in [Get Ubuntu for IoT](https://ubuntu.com/download/iot)
2. Setting up snapd on other versions of Linux is described in [Installing snapd](https://snapcraft.io/docs/installing-snapd)

Follow the instructions to install Ubuntu Core/snapd and ssh into your device.

## Install Ubuntu Frame

You should now have a command prompt reading something like `<your‑user>@yourdevice:~$`. Now install Ubuntu Frame with the following command:

    $ snap install ubuntu-frame

(Note: Until ubuntu-frame is released you will need to append --beta to this command.)

If you are using Ubuntu Core, you can skip this next step. On other versions of Linux, Ubuntu Frame does not start by default. To change this default run the following command:

    $ snap set ubuntu-frame daemon=true

Once Ubuntu Frame starts the display should show a graduated grey screen. This is the default Ubuntu Frame wallpaper. This can be changed using the “config” snap configuration option:

    $ snap set ubuntu-frame config="
    wallpaper-top=0x92006a
    wallpaper-bottom=0xdd4814
    "

The `wallpaper-top` and `wallpaper-bottom` are RGB values. There are a lot of configuration options to control the kiosk display, but they are all set using `config` and `display` (more details in [REFERENCE.md](REFERENCE.md))

## Install a Web Kiosk

Still in your ssh session, install web kiosk:

    $ snap install wpe-webkit-mir-kiosk

If you are using Ubuntu Core, you can skip this next step. On other versions of Linux, wpe-webkit-mir-kiosk does not start by default. To change this default run the following command:

    $ snap set wpe-webkit-mir-kiosk daemon=true

Once installed and configured the display should show the WPE website.

The website can be changed using the “url” snap configuration option:

    $ snap set wpe-webkit-mir-kiosk url=https://mir-server.io
    $ snap start wpe-webkit-mir-kiosk

This will show the Mir Server website.

## Next steps

For more detail about running and configuring Ubuntu Frame, and for resources to help build your own snaps to work with it see [REFERENCE.md](REFERENCE.md).

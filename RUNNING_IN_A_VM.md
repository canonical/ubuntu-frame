# Ubuntu Core Kiosk

This is a quick introduction to running Ubuntu Core Kiosk in a virtual machine.

## Setup a VM
We need to be able to run the VM and, as the default build of qemu on Ubuntu doesn’t support graphics, we will use a handy snap that enables this:

    $ sudo snap install qemu-virgil
    $ sudo snap connect qemu-virgil:kvm

We also download an Ubuntu Core image. It’s currently easiest to get core18 working (as core20 requires UEFI and our qemu backend doesn't support UEFI booting). On an x86 computer download the `ubuntu-core-18-amd64.img.xz` image from here:

[https://cdimage.ubuntu.com/ubuntu-core/18/stable/current/](https://cdimage.ubuntu.com/ubuntu-core/18/stable/current/)

Now, uncompress the image and move it to a convenient location:

    $ unxz ~/Downloads/ubuntu-core-18-amd64.img.xz
    $ mv ~/Downloads/ubuntu-core-18-amd64.img ~/snap/qemu-virgil/common/
    $ qemu-virgil -enable-kvm -m 512 -device virtio-vga,virgl=on\
     -display sdl,gl=on -netdev user,id=ethernet.0,hostfwd=tcp::10022-:22\
     -device rtl8139,netdev=ethernet.0 -soundhw ac97\
     ~/snap/qemu-virgil/common/ubuntu-core-18-amd64.img

This will create a QEMU window on your desktop and you need to follow through the prompts to initialise the VM with your launchpad account. (If you don’t have a launchpad account, get one and set up a public SSH key.)

Connect to the VP using ssh from a terminal window (ignore the address shown in the QEMU window and use the port set in the above command):

    $ ssh -P 10022 <your‑user>@localhost

## Install Ubuntu Core Kiosk
You should now have a command prompt reading something like `<your‑user>@localhost:~$`. Now install Ubuntu Core Kiosk with the following command:

    $ snap install ubuntu-core-kiosk

Once the installation completes the QEMU window should show a graduated grey screen. This is the default Ubuntu Core Kiosk wallpaper. This can be changed using the “kiosk-config” snap configuration option:

    $ snap set ubuntu-core-kiosk kiosk-config="
    wallpaper-top=0x92006a
    wallpaper-bottom=0xdd4814
    "

The `wallpaper-top` and `wallpaper-bottom` are RGB values. There are a lot of configuration options to control the kiosk display, but they are all set using `kiosk-config` and `kiosk-display` (more details in [REFERENCE.md](REFERENCE.md))

## Install a Web Kiosk

Still in your ssh session, install web kiosk:

    $ snap install wpe-webkit-mir-kiosk

Once the installation completes the QEMU window should show the WPE website. This can be changed using the “url” snap configuration option:

    $ snap set wpe-webkit-mir-kiosk url=https://mir-server.io

This will show the Mir Server website.

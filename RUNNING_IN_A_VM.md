# Ubuntu Frame

This is a quick reference for running Ubuntu Frame in a virtual machine.

## Setup a VM
We need to be able to run the VM and, as the default build of qemu on Ubuntu doesn’t support graphics, we will use a handy snap that enables this:

    $ sudo snap install qemu-virgil
    $ sudo snap connect qemu-virgil:kvm
    $ qemu-virgil --version

We also download an Ubuntu Core image from [https://cdimage.ubuntu.com/ubuntu-core/20/stable/current/](https://cdimage.ubuntu.com/ubuntu-core/20/stable/current/)

Now, uncompress the image and move it to a convenient location:

    $ unxz ~/Downloads/ubuntu-core-20-amd64.img.xz
    $ mv ~/Downloads/ubuntu-core-20-amd64.img ~/snap/qemu-virgil/common/
    $ qemu-virgil -enable-kvm -m 512 -device virtio-vga,virgl=on\
     -display sdl,gl=on -netdev user,id=ethernet.0,hostfwd=tcp::10022-:22\
     -device rtl8139,netdev=ethernet.0\
     -drive file=/snap/qemu-virgil/current/usr/share/qemu/edk2-x86_64-code.fd,if=pflash,format=raw,unit=0,readonly=on\
     ~/snap/qemu-virgil/common/ubuntu-core-20-amd64.img

This will create a QEMU window on your desktop and you need to follow through the prompts to initialise the VM with your launchpad account. (If you don’t have a launchpad account, get one and set up a public SSH key.)

Connect to the VP using ssh from a terminal window (ignore the address shown in the QEMU window and use the port set in the above command):

    $ ssh -p 10022 <your‑user>@localhost

## Install Ubuntu Frame
You should now have a command prompt reading something like `<your‑user>@localhost:~$`. Now install Ubuntu Frame with the following command:

    $ snap install ubuntu-frame


Once the installation completes the QEMU window should show a graduated grey screen. This is the default Ubuntu Frame wallpaper. This can be changed using the “config” snap configuration option:

    $ snap set ubuntu-frame config="
    wallpaper-top=0x92006a
    wallpaper-bottom=0xdd4814
    "

The `wallpaper-top` and `wallpaper-bottom` are RGB values. There are a lot of configuration options to control Ubuntu Frame, but they are all set using `config` and `display` (more details in [REFERENCE.md](REFERENCE.md))

## Install a Web Kiosk

Still in your ssh session, install web kiosk:

    $ snap install wpe-webkit-mir-kiosk

Once the installation completes the QEMU window should show the WPE website.

_Note [2021-10-08]: there seems to be a problem with wpe-webkit-mir-kiosk and some websites: https://gitlab.com/glancr/wpe-webkit-snap/-/issues/27_

_Note [2021-10-12]: As a workaround, use `wpe-webkit-frame-temp` (e.g. `snap install --edge wpe-webkit-frame-temp`) until problems are resolved_

The website can be changed using the “url” snap configuration option:

    $ snap set wpe-webkit-mir-kiosk url=https://mir-server.io

This will show the Mir Server website.

## Next steps

For more detail about running and configuring Ubuntu Frame, and for resources to help build your own snaps to work with it see [REFERENCE.md](REFERENCE.md).

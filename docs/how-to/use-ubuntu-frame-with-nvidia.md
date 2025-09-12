(how-to-use-ubuntu-frame-with-nvidia)=

# How to use Ubuntu Frame with Nvidia

This document outlines how to enable support for Nvidia GPUs for use with Ubuntu Frame.

If you require more outputs, or higher performance, you might decide to use Nvidia GPUs in your solution.
Nvidia only provides proprietary drivers for their GPUs, so there's additional steps required to get it working.

## System preparation

The steps and requirements differ by the operating system in use, choose the one matching your deployment:

`````{tab-set}
````{tab-item} Ubuntu
:sync: ubuntu

On classic Ubuntu, just install the drivers on the host.
For example, to install the `570` driver version, issue the following:

```shell
sudo ubuntu-drivers install nvidia:570
```

You can find out more at Ubuntu Server's {ref}`server:nvidia-drivers-installation`.
````

````{tab-item} Ubuntu Core 22
:sync: ubuntu-core22

On Ubuntu Core 22, the drivers come with the kernel snap, but need to be assembled on device for licensing reasons.
To do this, install the [nvidia-assemble](https://snapcraft.io/nvidia-assemble) snap:

```shell
sudo snap install nvidia-assemble --track 22
```

The userspace libraries needed for software to communicate with the kernel drivers are supplied by the [nvidia-core22](https://snapcraft.io/nvidia-core22) snap:

```shell
sudo snap install nvidia-core22
```

You will need to make sure Ubuntu Frame is from the `22` track:
```shell
sudo snap refresh ubuntu-frame --channel 22
```

### Avoid the mesa-core22 snap

It might be that you already have another provider of [the `graphics-core22` interface](the-graphics-core22-snap-interface) installed - likely the default of [mesa-core22](https://snapcraft.io/mesa-core22),
in which case you'll want to disconnect Ubuntu Frame (and any application snaps) from it, and connect to the Nvidia one:

```shell
sudo snap disconnect ubuntu-frame:graphics-core22
sudo snap connect ubuntu-frame:graphics-core22 nvidia-core22
```

To avoid having to do the above, make sure nvidia-core22 is installed before any snaps consuming the interface.
They will get connected to the only provider on the system by default.

If you'd like to fully switch over, unintall the Mesa snap and connect all the consumers to the Nvidia provider:

```shell
sudo snap remove mesa-core22
for plug in $( snap connections --all | grep :graphics-core22 | awk '{ print $2 }' ); do
    sudo snap connect "$plug" nvidia-core22
done
```
````

````{tab-item} Ubuntu Core 24
:sync: ubuntu-core24

On Ubuntu Core 24, the drivers and userspace libraries come in components of the kernel snap.
To find out which version is available

```shell
sudo snap set pc-kernel nvidia-stream=570
```

You will need to make sure Ubuntu Frame is from the `24` track:
```shell
sudo snap refresh ubuntu-frame --channel 24
```
````
`````

## Verification

First, to verify that your setup works, reboot to load the new drivers:

```shell
sudo reboot
```

Next, check that the Nvidia driver was loadded and the devices node is there:

```shell
$ sudo dmesg | grep NVRM
[  830.693443] NVRM: loading NVIDIA UNIX x86_64 Kernel Module  570.158.01  Tue Apr  8 12:41:17 UTC 2025
$ ls -l /dev/nvidia
/dev/nvidia
```

Check Ubuntu Frame logs for its use of Nvidia.
You want to see `GLRenderer` detailing the GPU and driver in use:

```shell
$ snap logs -n1000 ubuntu-frame | grep -E "GL (vendor|renderer|version):"
... GLRenderer: GL vendor: NVIDIA Corporation
... GLRenderer: GL renderer: NVIDIA GeForce RTX 2070 Super/PCIe/SSE2
... GLRenderer: GL version: OpenGL ES 3.2 NVIDIA 570.158.01
```

Finally, run [`graphics-test-tools.glmark2-es2-wayland`](https://snapcraft.io/graphics-test-tools) to check that clients can use it, too:

`````{tab-set}
````{tab-item} Ubuntu
:sync: ubuntu
```shell
sudo snap install graphics-test-tools
```
````
````{tab-item} Ubuntu Core 22
:sync: ubuntu-core22

```shell
sudo snap install graphics-test-tools --channel 22
```
````
````{tab-item} Ubuntu Core 24
:sync: ubuntu-core24

```shell
sudo snap install graphics-test-tools --channel 24
```
````
`````

```shell
$ sudo graphics-test-tools.glmark2-es2-wayland
=======================================================
    glmark2 2023.01
=======================================================
    OpenGL Information
    GL_VENDOR:      NVIDIA Corporation
    GL_RENDERER:    NVIDIA GeForce RTX 2070 Super/PCIe/SSE2
    GL_VERSION:     OpenGL ES 3.2 NVIDIA 570.158.01
# ...
```

## Conclusion

Ubuntu Frame supports Nvidia GPUs across Ubuntu and Ubuntu Core,
delivering the most value and performance out of your choice of hardware.

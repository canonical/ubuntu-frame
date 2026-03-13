(how-to-use-ubuntu-frame-with-nvidia)=

# How to use Ubuntu Frame with Nvidia

This document outlines how to enable support for Nvidia GPUs for use with Ubuntu Frame.

If you require more outputs, or higher performance, you might decide to use Nvidia GPUs in your solution.
Nvidia only provides proprietary drivers for their GPUs, so there's additional steps required to get it working.

## System preparation

On classic Ubuntu, it is enough to install the drivers on the host.
For example, to install the `570` driver version, issue the following:

```shell
sudo ubuntu-drivers install nvidia:570
```

You can find out more at Ubuntu Server's {ref}`server:nvidia-drivers-installation`.

## Verification

First, to verify that your setup works, reboot to load the new drivers:

```shell
sudo reboot
```

Next, check that the Nvidia driver was loaded and the devices node is there:

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

```shell
sudo snap install graphics-test-tools
```

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

Ubuntu Frame supports Nvidia GPUs, delivering the most value and performance out of your choice of hardware.

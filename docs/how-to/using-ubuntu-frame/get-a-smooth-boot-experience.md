(how-to-get-a-smooth-boot-experience)=
# How to get a smooth boot experience

This document walks through the steps needed to achieve a flicker-free, smooth boot experience with Frame on Ubuntu Core.

---

[note status="Version 123-mir2.15.0+dev399"]
This is only possible with Ubuntu Frame 123-mir2.15.0+dev399 and later
```

## Enabling the boot splash

This guide assumes that you enable the boot splash in your Ubuntu Core image, so head on to [the relevant documentation](https://ubuntu.com/core/docs/splash-screen) if you didn't, yet.

## Installing Ubuntu Frame

If your image doesn't yet have Ubuntu Frame, you'll need to install it:

```shell
$ sudo snap install ubuntu-frame --edge
```

And maybe a client:

```shell
$ sudo snap install wpe-webkit-mir-kiosk
```

## Modifying service dependencies and configuration

By default, Plymouth (the daemon responsible for the splash screen) quits at the end of the boot process. We need to prevent that, but make it release the GPU resources so that Frame can take over.

To do this, we'll add the following changes to the Ubuntu Frame daemon service, either via `sudo systemctl edit snap.ubuntu-frame.daemon.service` or just dropping these lines into the file:

```ini
# /etc/systemd/system/snap.ubuntu-frame.daemon.service.d/override.conf
[Unit]
Conflicts=plymouth-quit.service
After=plymouth-quit.service
OnFailure=plymouth-quit.service

[Service]
ExecStartPre=-/usr/bin/plymouth deactivate
ExecStartPost=/usr/bin/sleep 10
ExecStartPost=-/usr/bin/plymouth quit --retain-splash
```

Another change we want to do is tell Ubuntu Frame to not draw its wallpaper, and run on the first virtual terminal:

```shell
$ sudo snap set ubuntu-frame config="
wallpaper=false
vt=1
"
```

If you have other configuration, just amend it to include the above values. You can reboot now and see that the first frame drawn is that of the application.

## Embedding in an image

Refer to the relevant documentation for how to [configure Frame](/reference/configuring-ubuntu-frame-through-a-gadget-snap.md) and then [build images](https://ubuntu.com/core/docs/image-building), but here are the snippets specific to this solution:

- The required configuration in the gadget snap:
  ```yaml
  # gadget.yaml
  defaults:
    BPZbvWzvoMTrpec4goCXlckLe2IhfthK:  # this is ubuntu-frame's `snap-id`
      config: |
        vt=1
        wallpaper=false
  ```

- One way to include the necessary service changes is by [including cloud-init configuration](https://ubuntu.com/core/docs/gadget-snaps#setup) in the gadget snap:
   ```
   # cloud.conf
   #cloud-config
   datasource_list: [NoCloud]

  write_files:
   - path: '/etc/systemd/system/snap.ubuntu-frame.daemon.service.d/override.conf'
     content: |
       [Unit]
       Conflicts=plymouth-quit.service
       After=plymouth-quit.service
       OnFailure=plymouth-quit.service

       [Service]
       ExecStartPre=-/usr/bin/plymouth deactivate
       ExecStartPost=/usr/bin/sleep 10
       ExecStartPost=-/usr/bin/plymouth quit --retain-splash
   ```

## Display configuration

If you configure Ubuntu Frame with a non-default display resolution, you'll see the display re-configure between the splash and Frame. You'll have to modify the boot loader resolution to avoid that, but that's out of scope for this document.

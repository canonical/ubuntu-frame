(migrating-from-mir-kiosk-to-ubuntu-frame)=

# Migrating from mir-kiosk to Ubuntu Frame

This document explains how to migrate from the legacy mir-kiosk display server to Ubuntu Frame.

______________________________________________________________________

[Ubuntu Frame](https://mir-server.io/ubuntu-frame/) is the recommended way to enable embedded graphics on Ubuntu Core. It replaces mir-kiosk and provides an improved experience. New features will only be added to Ubuntu Frame, while mir-kiosk will remain maintained until we can decommission it.

Some existing improvements are:

1. Support for an on-screen keyboard
1. A more powerful and easier configuration system
1. A more polished “developer story”
1. `idle-timeout` option for power saving
1. A `ubuntu-frame.screenshot` utility

Ubuntu Frame was announced in October 2021, so it is already a true and proven solution. Your migration from mir-kiosk should be planned as soon as possible. This document aims to help you migrate to the new solution as a drop-in replacement.

## Update an existing installation

The simplest way to migrate a device that you have shell access to, is to disable mir-kiosk, install Ubuntu Frame and transfer the configuration over:

```
snap stop mir-kiosk
snap install ubuntu-frame
snap set ubuntu-frame config="$( cat /var/snap/mir-kiosk/current/miral-kiosk.config )"
snap set ubuntu-frame display="$( cat /var/snap/mir-kiosk/current/miral-kiosk.display )"
snap set ubuntu-frame daemon="$( snap get mir-kiosk daemon )"
snap disable mir-kiosk
# or, to remove it completely
snap remove --purge mir-kiosk
```

Your device should now be running Ubuntu Frame configured identically to mir-kiosk.

You will also need to connect your application to the `ubuntu-frame` snap and potentially restart it:

```
snap connect <your-application>:wayland ubuntu-frame
snap restart <your-application>
```

Below is a handy script that you can run on your target, optionally passing your application name as argument. If anything goes wrong, it will try and bring back mir-kiosk.

```shell
#!/bin/bash

set -ex

! snap list mir-kiosk && echo "No current mir-kiosk installation found!" && exit 1

APP=$1
! snap list ${APP} && echo "${APP} snap not found!" && exit 2

_exit() {
  if [ $? -ne 0 ]; then
    set +e
    snap disable ubuntu-frame
    snap enable mir-kiosk
    snap restart mir-kiosk
    if [ -n "${APP}" ]; then
      snap connect ${APP}:wayland mir-kiosk
      snap restart ${APP}
    fi
  fi
}

trap _exit EXIT

snap stop mir-kiosk
snap install ubuntu-frame
snap set ubuntu-frame config="$( cat /var/snap/mir-kiosk/current/miral-kiosk.config )"
snap set ubuntu-frame display="$( cat /var/snap/mir-kiosk/current/miral-kiosk.display )"
snap set ubuntu-frame daemon="$( snap get mir-kiosk daemon )"
snap disable mir-kiosk

if [ -n "${APP}" ]; then
  snap connect ${APP}:wayland ubuntu-frame
  snap restart ${APP}
fi
```

## Use a custom image

If you used a custom model and image, you just need to replace mir-kiosk with ubuntu-frame in your model assertion and provide your configuration options in the new format. An example config looked like follows:

```yaml
defaults:
  rW4inp7UbJb1YBxWr6SVebxa3Yv7K1Vm:  # snap-id for mir-kiosk
    cursor: software
    daemon: true
    display-layout: default
    vt: 4
    display-config: |
      layouts:
        default:
          cards:
          - card-id: 0
            HDMI-A-1:
              # ...
```

You will need to convert it to look like so:

```yaml
defaults:
  BPZbvWzvoMTrpec4goCXlckLe2IhfthK:  # snap-id for ubuntu-frame
    daemon: true
    config: |
      cursor=software
      display-layout=default
      vt=4
    display: |
      layouts:
        default:
          cards:
          - card-id: 0
            HDMI-A-1:
              # ...
```

A big benefit of Ubuntu Frame over mir-kiosk is that under the `config` key you can provide any of the {ref}`supported Ubuntu Frame options <ubuntu-frame-configuration-options>`, which previously required a much more complex approach with preparation scripts. You can now build the new image / model and deploy it to the device or proceed with a remodel.

(iot-gui-snaps-made-easy)=

# IoT GUI snaps made easy

The following assumes some familiarity with using snapcraft to package snaps, and concentrates on the specifics of snapping graphical snap intended to work with, for example, the `ubuntu-frame` snap on Ubuntu Core.

This is a peek "under the hood" of the `iot-example-graphical-snap` examples. If you are looking for a "how to" about packaging an IoT GUI based on a common graphics toolkit then you should look at the " Packaging applications as IoT GUIs section of {ref}`Ubuntu Frame How-to Guides <how-to-index>`.

## IoT GUIs

Essentially IoT GUI snaps are Wayland applications packaged as a snap. Compared with running applications on a desktop, these do not care about integration with desktop themes and are expected to run as single fullscreen applications.

There are various types of application that fall into this category. As well as bespoke "kiosks" e.g. [wpe-webkit](https://snapcraft.io/wpe-webkit-mir-kiosk), there are also things that might be embedded e.g. [Kodi](https://snapcraft.io/mir-kiosk-kodi) and games e.g. [scummvm](https://snapcraft.io/mir-kiosk-scummvm).

## Dancing with Wayland, dancing with daemons

The [iot-example-graphical-snap](https://github.com/canonical/iot-example-graphical-snap) repository incorporates the experience gained writing a number of snaps to work with Ubuntu Frame and simplified the process. The result of this is a simple approach to snapping an IoT GUI.

### The "wayland interface dance"

Snaps use "interfaces" to access capabilities of the system and one of these is the `wayland` interface. Snaps have their own `$XDG_RUNTIME_DIR` but Wayland expects to use a file in this location to connect to the server. As a result, every Wayland based snap needs logic to make this work.

After writing this logic a few times, we extracted it into a `wayland-launch` helper script:

```sh
#!/bin/sh
set -e

for PLUG in %PLUGS%; do
  if ! snapctl is-connected ${PLUG}
  then
    echo "WARNING: ${PLUG} interface not connected! Please run: /snap/${SNAP_INSTANCE_NAME}/current/bin/setup.sh"
  fi
done

if ! command -v inotifywait > /dev/null
then
    echo "ERROR: inotifywait could not be found, mir-kiosk-snap-launch expects:"
    echo " . . :     stage-packages:"
    echo " . . :        - inotify-tools"
    exit 1
fi

wait_for()
{
  until
    inotifywait --event create "$(dirname "$1")"&
    inotify_pid=$!
    [ -O "$1" ]
  do
    wait "${inotify_pid}"
  done
  kill "${inotify_pid}"
}

real_xdg_runtime_dir=$(dirname "${XDG_RUNTIME_DIR}")
real_wayland=${real_xdg_runtime_dir}/${WAYLAND_DISPLAY:-wayland-0}

# On core systems may need to wait for real XDG_RUNTIME_DIR
wait_for "${real_xdg_runtime_dir}"
wait_for "${real_wayland}"

mkdir -p "$XDG_RUNTIME_DIR" -m 700
ln -sf "${real_wayland}" "$XDG_RUNTIME_DIR"
ln -sf "${real_wayland}.lock" "$XDG_RUNTIME_DIR"
unset DISPLAY

exec "$@"
```

This creates a link from the snap's `$XDG_RUNTIME_DIR` to the real one in the user's `$XDG_RUNTIME_DIR`.

This "dance" (by design) works equally well on Ubuntu Core, running as a daemon and on Classic systems running in a user session.

### The "daemon dance"

There is a difference between the way that snaps are run on Ubuntu Core and on Classic systems. On Ubuntu Core, snaps are expected to start automatically as daemons, on a Classic system they are started by the user.

To accommodate this I've introduced a `daemon` snap option, and on installation I set this according to the type of system. (Like other options this can then be changed using `snap set`.)

This option is used by the `configure` hook (another script):

```bash
#!/bin/bash
set -euo pipefail

daemon=$(snapctl get daemon)
case "$daemon" in
  true)
    # start the daemon
    if snapctl services "$SNAP_INSTANCE_NAME" | grep -q inactive; then
      snapctl start --enable "$SNAP_INSTANCE_NAME" 2>&1 || true
    fi
    ;;
  false)
    # stop the daemon
        snapctl stop --disable "$SNAP_INSTANCE_NAME" 2>&1 || true
    ;;
  *)
    echo "ERROR: Set 'daemon' to one of true|false"
    exit 1
    ;;
esac
```

The effect of this is that the snap will be disabled when `daemon=false` and enabled when `daemon=true`. This default is set in the `install` and `post-refresh` hooks.

## The end result

If you build a snap based on the `iot-example-graphical-snap` examples it will have all the basics needed to work with Ubuntu Frame. (It will probably also run on a Wayland desktop, but that isn't the point.)

Here some examples that use these techniques:

| Snap Store                                                  | Github                                         | Comment |
| ----------------------------------------------------------- | ---------------------------------------------- | ------- |
| [mir-kiosk-kodi](https://snapcraft.io/mir-kiosk-kodi)       | https://github.com/canonical/mir-kiosk-kodi    |         |
| [mir-kiosk-scummvm](https://snapcraft.io/mir-kiosk-scummvm) | https://github.com/canonical/mir-kiosk-scummvm | SDL2    |

#!/bin/bash

# This script runs outside of snap confinement as a wrapper around the
# confined desktop session.
snap_cmd="$1"
snap_name="$(echo "$snap_cmd" | cut -d . -f 1)"

# Set up PATH and XDG_DATA_DIRS to allow calling snaps
if [ -f /snap/snapd/current/etc/profile.d/apps-bin-path.sh ]; then
    source /snap/snapd/current/etc/profile.d/apps-bin-path.sh
fi

export XDG_CURRENT_DESKTOP=ubuntu:GNOME
export GSETTINGS_BACKEND=keyfile
export MOZ_ENABLE_WAYLAND=1

dbus-update-activation-environment --systemd --all

# Don't set this in our own environment, since it will make
# gnome-session believe it is running in X mode
dbus-update-activation-environment --systemd WAYLAND_DISPLAY=wayland-0

# Symlink the Wayland socket from the snap's private directory
ln -sf "snap.$snap_name/wayland-0" $XDG_RUNTIME_DIR/wayland-0

exec "$@"

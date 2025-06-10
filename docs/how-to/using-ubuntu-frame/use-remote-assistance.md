(how-to-use-remote-assistance)=

# How to use remote assistance

This document describes how to use the remote assistance function in Ubuntu Frame.

## The `ubuntu-frame-vnc` snap

The remote assistance feature of Ubuntu Frame is provided by [the ubuntu-frame-vnc snap](https://snapcraft.io/ubuntu-frame-vnc). This needs to run on the same device as Ubuntu Frame and, under the hood, it uses [wayvnc](https://github.com/any1/wayvnc) built and configured to work with Ubuntu Frame.

You can install the snap:

```
$ snap install ubuntu-frame-vnc
```

### Using as a daemon

The default on Ubuntu Core, using as a daemon allows you to always have the remote access enabled. To turn the daemon feature on or off, use:

```
$ sudo snap set ubuntu-frame-vnc daemon=true
# or
$ sudo snap set ubuntu-frame-vnc daemon=false
```

### Running manually

To run the server manually, just issue:

```
$ ubuntu-frame-vnc
# Ctrl+C to stop
```

**NB**: If you're running Frame on a Wayland desktop, by default this will try and connect to your desktop rather than Frame. On Ubuntu 22.04, for example, the symptom is:

```
$ ubuntu-frame-vnc
wl_registry@2: error 0: invalid version for global wl_output (5): have 2, wanted 3
ERROR: Compositor doesn't support screencopy! Exiting.
ERROR: Failed to initialise wayland
```

To make sure you're using the same `WAYLAND_DISPLAY`, run all the components with the same value of the variable:

```
$ WAYLAND_DISPLAY=wayland-99 ubuntu-frame
# in another terminal
$ WAYLAND_DISPLAY=wayland-99 ubuntu-frame-vnc
```

## Authentication

[note status="Version 82"]
This feature is only available from version `82` onward on the `24` track.

````

To improve security of the connection, you can configure the daemon to require a username and password - it's recommended to use `read -s` to avoid the password getting into your shell history:
```
$ snap set ubuntu-frame-vnc username=user password=$( read -s P; echo P )
# type your password and press Enter
````

The daemon will restart and require the provided username and password on connection.

**NB**: not all clients support this mechanism, see {ref}`how-to-use-remote-assistance#client-compatibility` below.

To disable again, set both username and password to empty:

```
$ snap set ubuntu-frame-vnc username= password=
```

(how-to-use-remote-assistance#remote-access)=

## Remote access

Rather than expose an extra attack surface, `ubuntu-frame-vnc` is configured to only listen on `localhost`, on port 5900 (the default VNC port). It is up to the operator to expose it securely to the outside or provide other means of access.

SSH port forwarding is a common approach. On your host, issue:

```
$ ssh -L 5900:localhost:5900 <user>@<hostname>
```

You will then be able to connect to `localhost` with any VNC client running on your host.

______________________________________________________________________

Here is the same, wrapped in an [example](https://github.com/AlanGriffiths/frame-it/blob/master/frame-it/frame-it-vnc) using the `frame-it` snap and `gvncviewer`:

```
$ sudo snap install frame-it
$ sudo apt install gvncviewer
...
$ frame-it.vnc <user>@<hostname>
```

(how-to-use-remote-assistance#client-compatibility)=

## Client compatibility

The following VNC client software was tested against this:

- [Remmina](https://remmina.org/) (authentication not supported)
- [TigerVNC](https://tigervnc.org/)
- [RealVNC](https://www.realvnc.com/en/)
- *not working*: Finder on macOS ([any1/neatvnc#48](https://github.com/any1/neatvnc/issues/48))

## Debugging

When things don't work as expected, the general rules apply when debugging snaps.

The first place to look would be `snap logs ubuntu-frame-vnc`. Use `-n <number>` to see more history.

You can also use `[sudo] snap run --shell ubuntu-frame-vnc` to get into the environment of the VNC server and poke around. Use `${SNAP}/usr/local/bin/wayvnc` to run it, and provide configuration options, like `--log-level trace` to get more information.

Please file any issues you find, with as much context as possible, on [the GitHub page](https://github.com/canonical/ubuntu-frame-vnc/issues).

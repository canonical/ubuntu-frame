(how-to-run-your-iot-gui-on-your-desktop)=

# How to run your IoT GUI on your desktop

When developing things it is nice to have all your familiar development tools to hand. And the same is true when snapping applications to run on [Ubuntu Frame](https://snapcraft.io/ubuntu-frame).

Instead of running everything on an Ubuntu Core device and having to use ssh only to miss some key development tool; Or having to switch to another VT on your development machine and have processes you're trying to use suspend; Or trying to find another laptop to ssh into your own machine... wouldn't it be nice to simply have Ubuntu Frame running on a window on your desktop?

![image|690x388](61cc9ea2639b30c91169b8c70970fa84dd72e7f0.jpeg)

It is as easy as 1, 2, 3...

## 0. Install your snap

We'll use `wpe-webkit-mir-kiosk` as an example here, but you can use your own snap, or a different one from the store. The only requirement is that it is intended as a IoT GUI as snaps intended for desktop use may work differently.

```
snap install wpe-webkit-mir-kiosk
snap connect wpe-webkit-mir-kiosk:wayland
```

## 1. Install frame-it

```
snap install frame-it --classic
```

> Frame-it is a collection of scripts intended to make development and testing with Ubuntu Frame simpler.
>
> - `frame-it.check` checks Ubuntu Frame is installed plus options frame-it uses
> - `frame-it gnome-calendar` will run `gnome-calendar` using Ubuntu Frame
> - `snap set frame-it shell-app=gnome-calendar` sets the `shell-app` option
> - `frame-it.shell` will run Ubuntu Frame using the `shell-app` option
> - `sudo frame-it.root-on-x11` will run Ubuntu Frame as root using X11
>
> When `shell-app` is set a login shell is provided from the greeter: Logging into "frame-it" will run Ubuntu Frame using the `shell-app` option as a user session

## 2. Run frame-it.check

```
$ frame-it.check
Checking: Ubuntu Frame installation (needed for all commands)
 . OK . : Ubuntu Frame is installed
Checking: Ubuntu Frame has login-session-control (needed to run a login shell)
 . OK . : ubuntu-frame:login-session-control is connected
Checking: shell-app option (needed for frame-it.shell and to run a login shell)
 WARNING: shell-app not set. Please run the following (with the app you choose):
  snap set frame-it shell-app=...
```

If you don't have Ubuntu Frame installed already, Frame-it will prompt you with the commands to install it. Ignore any "WARNING" messages about `shell-app` for now as that's an advanced feature we don't need.

## 3. Frame-it "your IoT GUI"

There are two ways you can use Frame-it to run your application.

### 3.1 frame-it

The first simply runs it as the current user (and can also be used for non-snapped applications that use Wayland). This doesn't test everything that might need testing on an IoT device, but is more convenient for tests of your UI.

```
frame-it snap run wpe-webkit-mir-kiosk.cog
```

(If you're using your own snap, it probably won't have or need the `.cog` extension. I don't know why `wpe-webkit-mir-kiosk` has that.)

![image|690x388](2c53266b106281dd566cd57af61664483d992085.png)

### 3.2 Running your "daemon"

Normally, when deployed on an IoT device your snap will run as a daemon (which makes the user "root", not your current user). To test this mode of operation we need to run Ubuntu Frame as root, and start the daemon for your snap.

Running Ubuntu Frame as root requires running one of the Frame-it scripts as root:

```
sudo frame-it.root-on-x11
```

![image|690x388](e9bb7ff0cf4efa76589bcbeaa2d87131d18aaf52.png)

Running your snap's daemon may take a couple of commands. Many IoT GUI snaps don't try to run their daemons by default unless installed on Ubuntu Core and use a `daemon` configuration option to control this.

```
$ snap set wpe-webkit-mir-kiosk daemon=true
$ snap restart wpe-webkit-mir-kiosk.daemon
```

(If you're using your own snap, it probably won't have or need the `.daemon` extension. I don't know why `wpe-webkit-mir-kiosk` has that.)

![image|690x388](61cc9ea2639b30c91169b8c70970fa84dd72e7f0.jpeg)

## And that's it!

*Note: Not all snaps need the* `sudo snap set mir-kiosk-apps daemon=true` *command, but snaps packaged using mir-kiosk-snap-launch (and run-daemon) default to not running the daemon when not on Ubuntu Core.*

______________________________________________________________________

## Note: On Frame-it being a "classic" snap

It is a classic snap, which gives it full access to your system, but you can review all the scripts to see what they are doing:

```
$ find /snap/frame-it/current/ -type f -executable
/snap/frame-it/current/frame-it
/snap/frame-it/current/frame-it-check
/snap/frame-it/current/frame-it-root-on-x11
/snap/frame-it/current/frame-it-shell
/snap/frame-it/current/meta/hooks/configure
/snap/frame-it/current/meta/hooks/remove
/snap/frame-it/current/snap/hooks/configure
/snap/frame-it/current/snap/hooks/remove
```

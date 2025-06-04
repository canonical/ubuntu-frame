(ubuntu-frame-osk-documentation)=
# Ubuntu Frame OSK Documentation

This document describes how to work with the Ubuntu Frame OSK

---

Ubuntu Frame OSK is the easiest way to add on-screen keyboard support to Ubuntu Frame. To create Ubuntu Frame OSK, we selected Squeekboard, an on-screen keyboard built by Purism for the Librem 5. We chose Squeekboard because it’s modern, stable, and actively maintained. Maintainers and contributors also made Squeekboard really easy to work with, plus it supports a large and growing number of languages and layouts, such as US, German, Russian, Arabic, and many more.

## Setup System

Make sure you’ve installed and refreshed [Ubuntu Frame](https://mir-server.io/ubuntu-frame/).

For the on-screen keyboard to work, you’ll need to use it with an app that supports a text input Wayland protocol (`zwp_text_input_v3` or `zwp_text_input_v2`). Some apps and toolkits known to work are: Firefox, wpe-webkit-mir-kiosk, GTK, Flutter and Qt. Legacy X11 apps do not work with the OSK.

## Install Ubuntu Frame OSK

To install the snap, run:
```
sudo snap install ubuntu-frame-osk
```

If on a classic system, make sure it’s connected to the Wayland interface:
```
snap connect ubuntu-frame-osk:wayland
```

On Ubuntu Core, Ubuntu Frame OSK runs automatically on startup. By default it does not do this on classic systems. If you want to turn this behavior on or off:
```
sudo snap set ubuntu-frame-osk daemon=true
sudo snap set ubuntu-frame-osk daemon=false
```

You can also manually run Ubuntu Frame:
```
ubuntu-frame-osk
```

Once Ubuntu Frame, an OSK-supporting app and Ubuntu Frame OSK are all running on the same Wayland display, the OSK should appear whenever you click a text field.

## Configuration

### Daemon

To enable or disable the OSK daemon (which makes it run automatically on startup):
```
sudo snap set ubuntu-frame-osk daemon=true
sudo snap set ubuntu-frame-osk daemon=false
```

### Theme
You can switch between light and dark mode with the `theme` option:
```
sudo snap set ubuntu-frame-osk theme=dark
sudo snap set ubuntu-frame-osk theme=light
```

### Layout

To list all available languages/keyboard layouts:
```
ubuntu-frame-osk.list-layouts
```

To change the layout, set the `layout` option to the desired ID. For example, to use Arabic:
```
sudo snap set ubuntu-frame-osk layout=ara
```

You can enable multiple layouts. For example, to set layouts used in Switzerland:
```
sudo snap set ubuntu-frame-osk layout=ch,ch+fr,us
```

The first is used by default, and the other’s are available in the menu opened by the layout button in the bottom left. The emoji and terminal layouts are always available.

### Modifying the layout

If the existing layout does not work for you, you can provide a custom one. It needs to be placed in `$SNAP_COMMON/squeekboard/keyboards`, following [the file/folder layout from Squeekboard upstream](https://gitlab.gnome.org/World/Phosh/squeekboard/-/tree/v1.17.1/data/keyboards).

See this for upstream documentation on layouts:

https://gitlab.gnome.org/World/Phosh/squeekboard/-/blob/master/doc/layouts.md

**NB**: this may break with updates to ubuntu-frame-osk, so tread carefully.

Here's an example how you can modify the number layout for a narrow (portrait) "us" layout (if you run on a landscape device, make it `us_wide.yaml` - you should see in `snap logs ubuntu-frame-osk` which layout it's trying to load in your case):

```shell
$ sudo snap run --shell ubuntu-frame-osk.list-layouts \
   -c 'mkdir --parents --verbose $SNAP_COMMON/squeekboard/keyboards/number'
mkdir: created directory '/root/snap/ubuntu-frame-osk/302/.local'
mkdir: created directory '/root/snap/ubuntu-frame-osk/302/.local/share'
mkdir: created directory '/root/snap/ubuntu-frame-osk/302/.local/share/squeekboard'
mkdir: created directory '/root/snap/ubuntu-frame-osk/302/.local/share/squeekboard/keyboards'
mkdir: created directory '/root/snap/ubuntu-frame-osk/302/.local/share/squeekboard/keyboards/number'

$ curl --silent https://gitlab.gnome.org/World/Phosh/squeekboard/-/raw/v1.17.1/data/keyboards/number/us.yaml \
  | sudo snap run --shell ubuntu-frame-osk.list-layouts \
      -c 'cat > $SNAP_COMMON/squeekboard/keyboards/number/us.yaml'

$ sudo editor $( \
    sudo snap run --shell ubuntu-frame-osk.list-layouts \
      -c 'echo $SNAP_COMMON/squeekboard/keyboards/number/us.yaml' )
```

Modify the layout to your liking and restart the OSK:
```shell
sudo snap restart ubuntu-frame-osk
```

(how-to-set-up-digital-signage)=
# How to set up digital signage

## Introduction

This document walks through an example of deploying a digital signage solution on Ubuntu Core, with Ubuntu Frame as the display server. It uses [Dashkiosk](https://github.com/vincentbernat/dashkiosk/), a simple dashboard manager to control the displays, and [WPE WebKit distributed as a snap](https://snapcraft.io/wpe-webkit-mir-kiosk) to render them.

## Outline

For consistency, we'll call the hardware that you connect the displays to - the *receivers*. The software that renders the content - the *renderers*. The Dashkiosk instance - the *server*. And the content displayed - the *dashboards*.

Dashkiosk is a simple web server at which you need to point the renderers. You can manage as many displays as you need, for example by running it on some central server hardware - but you then need to ensure connectivity between the renderers and the server (like using a reverse proxy to listen outside of `localhost`), which is out of scope for this guide.

A dashboard can be anything that a web browser can render - a web page, an image, or a video. The receivers need network access to whatever's hosting the dashboards, as the server only tells the renderers what URL to display, but doesn't participate in the rendering. Creating the dashboards is also out of scope for this guide - refer to [Dashkiosk documentation](https://dashkiosk.readthedocs.io/en/latest/usage.html#about-the-dashboards) for more detail.

## Setting up
### The server

We've [packaged Dashkiosk as a snap](https://snapcraft.io/dashkiosk), so the installation is as easy as:

```shell
sudo snap install dashkiosk
```
You can then reach the administration panel on port 9400, under `/admin`, for example http://127.0.0.1:9400/admin, if you installed it locally. You will see that there are no displays yet.
![Dashkiosk — Administration interface|690x459, 50%](upload://3GWeX7UeUF1XZ4V9yN5E9JhdSdQ.jpeg)

If you point your browser at http://127.0.0.1:9400/receiver, you will see some default images being rendered, and a display will show up in the administration panel. The receiver will get a random serial generated and you can give it a name and set some other properties. You can find out more in [Dashkiosk display configuration](https://dashkiosk.readthedocs.io/en/latest/usage.html#display-configuration).
![Dashkiosk — Receiver|690x459, 50%](upload://sEsdgWC9AF8loqLNMUiuRvGiTNp.jpeg)![Dashkiosk — Administration interface|690x459, 50%](upload://jaRaFceaMupfFkFxxBhKwqTnTFG.jpeg)

You can configure the server by editing `/var/snap/dashkiosk/current/config.json`. You can change the port, add basic authentication and see [Dashkiosk configuration](https://dashkiosk.readthedocs.io/en/latest/configuration.html#json-configuration-file) for a reference. After you change anything, restart the server to apply the changes:
```shell
sudo snap restart dashkiosk
```

### The renderer

To have a web browser running fullscreen on a display, we'll use the WPE kiosk snap hosted on Ubuntu Frame. To install those:

```shell
sudo snap install ubuntu-frame wpe-webkit-mir-kiosk
```

1. On [Ubuntu Core](https://ubuntu.com/core) you should soon see the WPE website displayed
  Now, let's hide the cursor and point the renderer at Dashkiosk:
   ```shell
   sudo snap set ubuntu-frame="cursor=null"
   sudo snap set wpe-webkit-mir-kiosk url=http://localhost:9400/receiver
   ```
   That's it! Every time your device boots it will display the dashboards configured in Dashkiosk.

   You can read [Where does Ubuntu Frame work?](/explanation/where-does-ubuntu-frame-work.md) to find the right hardware for you.

2. On classic Ubuntu, you will need a bit more set up
  Allow the WPE snap to connect to Frame:
   ```shell
   sudo snap connect wpe-webkit-mir-kiosk:wayland ubuntu-frame
   ```

   If testing on your desktop, you'll need X11 support and run:
   ```shell
   WAYLAND_DISPLAY=wayland-99 ubuntu-frame --cursor null & \
     sleep 1; wpe-webkit-mir-kiosk.cog http://localhost:9400/receiver &

   # To stop it
   kill %1 %2
   ```
   ![Ubuntu Frame|690x424](upload://w8s6xrgbYxJbJhxCZCEpcoHZpQD.jpeg)

   Or,  if you're running things on dedicated hardware, you can make it permanent, just like on Core above:
   ```shell
   sudo snap set ubuntu-frame daemon=true
   sudo snap set wpe-webkit-mir-kiosk daemon=true
   sudo snap set wpe-webkit-mir-kiosk url=http://localhost:9400/receiver
   ```

### The dashboards

For completion, as mentioned above, setting up the dashboards is out of scope for this guide. You should refer to [the Dashkiosk documentation](https://dashkiosk.readthedocs.io/en/latest/usage.html#about-the-dashboards) - but just as an example, I'll replace `/unassigned` in the "Unassigned" group's single dashboard with a [random dashboard example](https://share.geckoboard.com/dashboards/56R2XOCUMXD5MNV4).
![Dashkiosk — Administration interface|690x459, 50%](upload://6snybc8bcac2uJgCnfsySe0zKhu.png) ![Ubuntu Frame|690x424, 50%](upload://xCr2hjXXxeX6EkXXdf5mtkeFn12.png)

## Summary

In this example we've deployed a very simple setup for digital signage solutions based on Ubuntu Core and Ubuntu Frame. It may well fulfill your use case - but if it doesn't, Ubuntu Frame can help bridge any application to your display.

We're working on supporting multiple displays driven from the same hardware, so you can reduce cost and complexity. We'll update this document when this is possible.

(how-to-configure-ubuntu-frame-for-multiple-outputs)=
# How to configure Ubuntu Frame for multiple outputs

This document describes all the available options for using Ubuntu Frame with multiple displays.

## The default

By default Ubuntu Frame will show the same content on all outputs. This works well for the simple case of one application showing one fullscreen window on multiple displays.

![IMG_0353|690x517](upload://5ZTJJ9kooEw4BLqZWKVz1nD4aWF.jpeg) 

But sometimes this is not what is wanted, and Ubuntu Frame can do a lot more.

[note status="Version 98-mir2.13.0"]
The features described below were introduced in release 98-mir2.13.0
```

## The `display` and `display-layouts` configuration options

By setting the `display` configuration option you can control very precisely how multiple outputs are used. However, there's a lot of detail and it may not be obvious how to proceed. Because the configuration depends on the graphics cards and displays attached to your system the first step is to find the configuration for your hardware.

When it starts, Ubuntu Frame will, unless it is already set, populate the `display` configuration. This is the output from one of my test systems:

```yaml
$ snap get ubuntu-frame display
layouts:
# keys here are layout labels (used for atomically switching between them).
# The yaml anchor 'the_default' is used to alias the 'default' label

  default:
    cards:
    # a list of cards (currently matched by card-id)

    - card-id: 0
      eDP-1:
        # This output supports the following modes: 2560x1440@60.0
        #
        # Uncomment the following to enforce the selected configuration.
        # Or amend as desired.
        #
        state: enabled	# {enabled, disabled}, defaults to enabled
        mode: 2560x1440@60.0	# Defaults to preferred mode
        position: [0, 0]	# Defaults to [0, 0]
        orientation: normal	# {normal, left, right, inverted}, defaults to normal
        scale: 1
        group: 0	# Outputs with the same non-zero value are treated as a single display

      DisplayPort-1:
        # (disconnected)

      HDMI-A-1:
        # (disconnected)

      DisplayPort-2:
        # (disconnected)

      HDMI-A-2:
        # This output supports the following modes: 1024x600@60.0, 1920x1080@60.0,
        # 1600x900@60.0, 1366x768@59.9, 1280x720@60.0, 1280x720@59.9
        #
        # Uncomment the following to enforce the selected configuration.
        # Or amend as desired.
        #
        state: enabled	# {enabled, disabled}, defaults to enabled
        mode: 1024x600@60.0	# Defaults to preferred mode
        position: [0, 0]	# Defaults to [0, 0]
        orientation: normal	# {normal, left, right, inverted}, defaults to normal
        scale: 1
        group: 0	# Outputs with the same non-zero value are treated as a single display

  side_by_side:
    cards:
    # a list of cards (currently matched by card-id)

    - card-id: 0
      eDP-1:
        # This output supports the following modes: 2560x1440@60.0
        #
        # Uncomment the following to enforce the selected configuration.
        # Or amend as desired.
        #
        state: enabled	# {enabled, disabled}, defaults to enabled
        mode: 2560x1440@60.0	# Defaults to preferred mode
        position: [0, 0]	# Defaults to [0, 0]
        orientation: normal	# {normal, left, right, inverted}, defaults to normal
        scale: 1
        group: 0	# Outputs with the same non-zero value are treated as a single display

      DisplayPort-1:
        # (disconnected)

      HDMI-A-1:
        # (disconnected)

      DisplayPort-2:
        # (disconnected)

      HDMI-A-2:
        # This output supports the following modes: 1024x600@60.0, 1920x1080@60.0,
        # 1600x900@60.0, 1366x768@59.9, 1280x720@60.0, 1280x720@59.9
        #
        # Uncomment the following to enforce the selected configuration.
        # Or amend as desired.
        #
        state: enabled	# {enabled, disabled}, defaults to enabled
        mode: 1024x600@60.0	# Defaults to preferred mode
        position: [2560, 0]	# Defaults to [0, 0]
        orientation: normal	# {normal, left, right, inverted}, defaults to normal
        scale: 1
        group: 0	# Outputs with the same non-zero value are treated as a single display

```
Note that, unless your setup matches mine, the exact details will differ on your system.

This is somewhat verbose, but you can see two `layouts`, the `default` and `side_by_side`. Within each of the `layouts` there are two outputs: `eDP-1` and `HDMI-A-2` and the settings used for the `layout` and a comment explaining the options Ubuntu Frame supports for each of them.

We can switch between these layouts (and any others you add to the `display` configuration) by setting the `display-layout` configuration variable. For example:

```plain
snap set ubuntu-frame display-layout=side_by_side
```
![IMG_0472|690x517](upload://ymnxEpUL97BS1VrPZ4ESzXI1izD.jpeg) 

With these changes the displays are "side by side" and the cursor can be moved from one display to another. You'll also see that the test application is only running on one output. It is possible to assign applications to outputs and we'll discuss that later.

## Changing the `display` configuration

I find it easiest to pipe the `display` configuration to a file and make the changes in a text editor and then pass the file content to the configuration option.

First, pipe the above output to a file and, for reference, make a backup of the original:

    $ snap get ubuntu-frame display > my-uf-display-configuration
    $ cp my-uf-display-configuration{,~original}

Next edit the file with your editor of choice. Now, I'm going to show changing the scale of `eDP-1` to "2" (which, because it has twice the pixels, makes the "height" of the displays the same) in both layouts. In the `side_by_side` layout it is also necessary to change the position of the `DisplayPort-1` output to match the new logical size of the first output:

```diff
diff --unified my-uf-display-configuration{~original,}
--- my-uf-display-configuration~original	2023-03-23 11:26:31.482290742 +0000
+++ my-uf-display-configuration	2023-03-23 12:24:36.974409249 +0000
@@ -17,7 +17,7 @@
         mode: 2560x1440@60.0	# Defaults to preferred mode
         position: [0, 0]	# Defaults to [0, 0]
         orientation: normal	# {normal, left, right, inverted}, defaults to normal
-        scale: 1
+        scale: 2
         group: 0	# Outputs with the same non-zero value are treated as a single display
 
       DisplayPort-1:
@@ -58,7 +58,7 @@
         mode: 2560x1440@60.0	# Defaults to preferred mode
         position: [0, 0]	# Defaults to [0, 0]
         orientation: normal	# {normal, left, right, inverted}, defaults to normal
-        scale: 1
+        scale: 2
         group: 0	# Outputs with the same non-zero value are treated as a single display
 
       DisplayPort-1:
@@ -79,7 +79,7 @@
         #
         state: enabled	# {enabled, disabled}, defaults to enabled
         mode: 1024x600@60.0	# Defaults to preferred mode
-        position: [2560, 0]	# Defaults to [0, 0]
+        position: [1280, 0]	# Defaults to [0, 0]
         orientation: normal	# {normal, left, right, inverted}, defaults to normal
         scale: 1
         group: 0	# Outputs with the same non-zero value are treated as a single display
```

You will likely have to make slightly different changes on your system, but the approach should be clear.

And here's the command to apply these changes:

```sh
$ snap set ubuntu-frame display="`cat my-uf-display-configuration`"
```

## A client window for each output

With the displays "side by side" as described in the previous section it should be possible to install and run an application snap on each output.

To specify which output an application appears on we need to add the snap name to the output within the layout. For example:

```diff
$ diff --unified my-uf-display-configuration{~original,}
--- my-uf-display-configuration~original	2023-03-23 11:26:31.482290742 +0000
+++ my-uf-display-configuration	2023-03-23 12:23:42.190975690 +0000
@@ -17,7 +17,7 @@
         mode: 2560x1440@60.0	# Defaults to preferred mode
         position: [0, 0]	# Defaults to [0, 0]
         orientation: normal	# {normal, left, right, inverted}, defaults to normal
-        scale: 1
+        scale: 2
         group: 0	# Outputs with the same non-zero value are treated as a single display
 
       DisplayPort-1:
@@ -49,6 +49,7 @@
 
     - card-id: 0
       eDP-1:
+        snap-name: mir-kiosk-kodi
         # This output supports the following modes: 2560x1440@60.0
         #
         # Uncomment the following to enforce the selected configuration.
@@ -58,7 +59,7 @@
         mode: 2560x1440@60.0	# Defaults to preferred mode
         position: [0, 0]	# Defaults to [0, 0]
         orientation: normal	# {normal, left, right, inverted}, defaults to normal
-        scale: 1
+        scale: 2
         group: 0	# Outputs with the same non-zero value are treated as a single display
 
       DisplayPort-1:
@@ -71,6 +72,7 @@
         # (disconnected)
 
       HDMI-A-2:
+        snap-name: iot-example-graphical-snap
         # This output supports the following modes: 1024x600@60.0, 1920x1080@60.0,
         # 1600x900@60.0, 1366x768@59.9, 1280x720@60.0, 1280x720@59.9
         #
@@ -79,7 +81,7 @@
         #
         state: enabled	# {enabled, disabled}, defaults to enabled
         mode: 1024x600@60.0	# Defaults to preferred mode
-        position: [2560, 0]	# Defaults to [0, 0]
+        position: [1280, 0]	# Defaults to [0, 0]
         orientation: normal	# {normal, left, right, inverted}, defaults to normal
         scale: 1
         group: 0	# Outputs with the same non-zero value are treated as a single display
```

And, again, apply these changes:

```sh
$ snap set ubuntu-frame display="`cat my-uf-display-configuration`"
```

This will move the test application to the second monitor; and, if I install `mir-kiosk-kodi` that will run on the main monitor.

![IMG_0473|690x517](upload://4kI4w6b2ggSX84Hddw6lug3NUzp.jpeg) 

![IMG_0474|690x517](upload://rGv8J0Ur2zFtiKiQZzeGEZIT3I2.jpeg) 

There is also a `surface-title` key that can be set, but as windows tend to change their title that is of limited usefulness.

### Using display properties

[note status="Version 243-mir2.21"]
This feature is only available from version 243-mir2.21 onward
```

Rather than configuring _outputs_, you can configure _displays_, by way of the `displays:` map for each layout. In the startup configuration you'll see a commented-out section like:
```yaml
    # displays:
    # A list of display configurations matched by the displays' properties
    #
    # These take the same display options as above,
    # and take precedence over the port-based configuration.
    #
    # - vendor: BOE
    #   model: 
    #   product: 2399
    #   serial: 0
    #
    # - vendor: DEL
    #   model: DELL U2413
    #   product: 61512
    #   serial: 1095650636
    #
```

You can provide as many of those as needed in your setup, and - for each connected display - the first one in the list, **that exactly matches all given properties**, will be applied.

All of those matchers are _optional_, so you could have a configuration matching all displays, selecting the same mode whatever the output they're connected to:
```yaml
    displays:
    - mode: 1920x1080@60.0
```

Or, you could change the orientation of a display model you know should be vertical:
```yaml
    displays:
    - vendor: VEN
      model: Model 1
      orientation: vertical
```

**NOTE**:  `displays:` take precedence over `cards:`, so best make it more specific, than less so.

## A client window spanning outputs

Instead of assigning applications to outputs we can also put both outputs into a group so that one application will appear across them. Still within the `side_by_side` layout we remove the `snap-name` keys that we had in the last section and set `group` to 1:

```diff
$ diff --unified my-uf-display-configuration{~original,}
--- my-uf-display-configuration~original	2023-03-23 11:26:31.482290742 +0000
+++ my-uf-display-configuration	2023-03-23 12:17:26.790860939 +0000
@@ -17,7 +17,7 @@
         mode: 2560x1440@60.0	# Defaults to preferred mode
         position: [0, 0]	# Defaults to [0, 0]
         orientation: normal	# {normal, left, right, inverted}, defaults to normal
-        scale: 1
+        scale: 2
         group: 0	# Outputs with the same non-zero value are treated as a single display
 
       DisplayPort-1:
@@ -58,8 +58,8 @@
         mode: 2560x1440@60.0	# Defaults to preferred mode
         position: [0, 0]	# Defaults to [0, 0]
         orientation: normal	# {normal, left, right, inverted}, defaults to normal
-        scale: 1
-        group: 0	# Outputs with the same non-zero value are treated as a single display
+        scale: 2
+        group: 1	# Outputs with the same non-zero value are treated as a single display
 
       DisplayPort-1:
         # (disconnected)
@@ -79,7 +79,7 @@
         #
         state: enabled	# {enabled, disabled}, defaults to enabled
         mode: 1024x600@60.0	# Defaults to preferred mode
-        position: [2560, 0]	# Defaults to [0, 0]
+        position: [1280, 0]	# Defaults to [0, 0]
         orientation: normal	# {normal, left, right, inverted}, defaults to normal
         scale: 1
-        group: 0	# Outputs with the same non-zero value are treated as a single display
+        group: 1	# Outputs with the same non-zero value are treated as a single display
```
Now, Ubuntu Frame treats the "side by side" windows as one large display:

![image|690x517](upload://ylO3iX94PvkRMvZdlHcngKWu3Dl.jpeg)

## Custom client placement

[note status="Version 243-mir2.21"]
This feature is only available from version 243-mir2.21 onward
```
You can also be in full control over the placement of the clients. With the layout-level `applications:` entry, you can directly place application windows and give them a custom size:
```yaml
layouts:
  default:
    cards:
      - card-id: 0
        # ...

    applications:
      # Two applications side by side on the first display.
      - snap-name: wpe-webkit-mir-kiosk
        position: [0, 0]
        size: [1720, 1080]
      - snap-name: wpe-webkit-mir-kiosk_two
        position: [1720, 0]
        size: [200, 1080]

      # Two overlapping applications positioned on second display.
      - snap-name: mir-kiosk-scummvm
        position: [2368, 448]
        size: [1024, 768]
      - surface-title: "My fun application"
        position: [1920, 0]
        size: [800, 600]
```
Each entry may contain the following keys:

* `snap-name` - name of snap providing this application. Optional (see below).
* `surface-title` - title of window providing this application. Optional (see below).
* `position` - the X and Y co-ordinates of the window in logical pixels. Required.
* `size` - the width and height of the window in logical pixels. Required.

Either `snap-name` or `surface-title` must be set. Both being set is invalid configuration.

Each entry in the configuration is checked if it matches `snap-name` or `surface-title`. If a match occurs, the window is placed in the `position` with the given `size`.

![obraz|800x675](upload://2vGr0LG4KzpAUwWXgMDZKSIMlUh.png)

**Note**: An experimental snapd feature, [parallel installs](https://snapcraft.io/docs/parallel-installs), allows you to run more than one instance of the same snap, configured separately and isolated from each other. That's why the `_two` suffix allows me to run the WPE kiosk twice.


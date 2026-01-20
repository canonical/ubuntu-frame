(how-to-use-the-diagnostic-feature)=

# The diagnostic feature

Ubuntu Frame has an easy-to-use diagnostic screen that can be used to inform the user of any issues that have occurred when running your application. It allows the application to define which information is most useful to display to diagnose and fix any bring-up issues or runtime errors.

## Setup system

Make sure to install or refresh [Ubuntu Frame](https://mir-server.io/ubuntu-frame/).

```
snap install ubuntu-frame
```

> For demonstration purposes in this document, we will be using [a simple snap](https://github.com/AlanGriffiths/ubuntu-frame-diagnostic) whose only purpose is to display user-inputted text on the diagnostic screen.

To install the demo snap:

```
snap install frame-diagnostic --edge
```

## Running the demo

Launch Ubuntu Frame:

```
ubuntu-frame
```

In another terminal, run the demo snap:

```
frame-diagnostic "Hello, world!"
```

![A Hello World! diagnostic message](diagnostic-hello.jpeg)

Now, without closing Frame, try writing something else.

```
frame-diagnostic "Goodbye, world!"
```

![A Goodbye, World! diagnostic message](diagnostic-goodbye.jpeg)

The text will update automatically when the diagnostic text changes.

## Using with your application

To learn how to implement Ubuntu Frame's diagnostic screen, take a look at the demo app's [`snap/snapcraft.yaml`](https://github.com/AlanGriffiths/ubuntu-frame-diagnostic/blob/master/snap/snapcraft.yaml) and notice the following entries:

```yaml
plugs:
  ubuntu-frame-diagnostic:
    interface: content
    content: diagnostic-text
    target: $SNAP_COMMON/diagnostic
    default-provider: ubuntu-frame

environment:
  DIAGNOSTIC_FILE: $SNAP_COMMON/diagnostic/diagnostic.txt
```

`plugs` allows access to predetermined resources. One with `interface: content` allows for sharing data between snaps. Here, there is a plug called `ubuntu-frame-diagnostic` that allows writing to the diagnostic file within Frame. In the `target` line, the directory inside the Ubuntu Frame snap that holds the diagnostic file is linked to this snap's `$SNAP_COMMON` directory.

The `environment` section creates an environment variable named `DIAGNOSTIC_FILE` that points to the `diagnostic.txt` file which Frame reads from.

In [`script/write_diagnostic`](https://github.com/AlanGriffiths/ubuntu-frame-diagnostic/blob/master/script/write-diagnostic), you can see the following:

```
#!/bin/sh
echo "$*" > "${DIAGNOSTIC_FILE}"
```

The only thing that the script does is write the user-inputted text to `DIAGNOSTIC_FILE`.

To enable diagnostic reporting in your application:

> 1. Create a plug to link with Frame's `ubuntu-frame-diagnostic` slot
> 1. Create an environment variable pointing to Frame's diagnostic file
> 1. Write diagnostic information to the file located at the environment variable

The diagnostic screen will automatically update when it detects changes to the diagnostic file.

## Configuration Options

To learn about Ubuntu Frame Diagnostic's various configuration options, please refer to {ref}`the configuration options reference <ubuntu-frame-configuration-options.md#snap-configuration-options>`.

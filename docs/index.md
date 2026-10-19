---
myst:
  html_meta:
    description: Documentation for Ubuntu Frame, Canonical's open-source Wayland display server for embedded Linux devices, kiosks, digital signage, and IoT graphical applications.
---

# Ubuntu Frame

Ubuntu Frame is an open-source Wayland display server for embedded Linux devices that need a graphical interface, such as kiosks, digital signage, and interactive displays. Built on Mir and distributed as a snap, it manages display output and input devices and provides a controlled connection between applications and the screen.

Frame reduces the platform-specific work involved in integrating graphics, input, window behaviour, and application communication. Device makers and operators can run full-screen applications built with common graphical toolkits and web technologies, while using Ubuntu Frame's touch and input support, optional on-screen keyboard, and security controls.

Ubuntu Frame can run on Ubuntu Core or another Linux distribution that supports snaps. It is intended for teams building and operating single-purpose graphical devices, where a focused display stack can simplify development and deployment without requiring a full desktop environment.

## In this documentation

The documentation covers the product from first evaluation through deployment and maintenance. Browse by area:

- **Getting started and application development**: [Create a secure web kiosk](tutorial/make-a-secure-ubuntu-web-kiosk.md) • [Deploy a graphical application](tutorial/deploy-graphical-application-with-frame.md) • [Package an application for IoT](how-to/packaging-iot-gui/packaging-an-application.md) • [Run an IoT GUI on your desktop](how-to/using-ubuntu-frame/run-your-iot-gui-on-your-desktop.md)
- **Displays and graphics**: [Where Ubuntu Frame works](explanation/where-does-ubuntu-frame-work.md) • [Run Frame on a device](how-to/running-ubuntu-frame/on-your-device.md) • [Configure multiple outputs](how-to/using-ubuntu-frame/configure-ubuntu-frame-for-multiple-outputs.md) • [Use snap graphics](how-to/use-snap-graphics.md) • [Use NVIDIA graphics](how-to/use-ubuntu-frame-with-nvidia.md) • [Set up digital signage](how-to/using-ubuntu-frame/set-up-digital-signage.md)
- **Security and deployment**: [Security concepts](explanation/security.md) • [Snap interfaces](reference/ubuntu-frame-snap-interfaces.md) • [Configuration options](reference/ubuntu-frame-configuration-options.md) • [Configure Frame through a gadget snap](reference/configuring-ubuntu-frame-through-a-gadget-snap.md) • [Get a smooth boot](how-to/using-ubuntu-frame/get-a-smooth-boot-experience.md) • [Use remote assistance](reference/ubuntu-frame-vnc-documentation.md)
- **Concepts and components**: [Build a product with Ubuntu Frame](explanation/building-your-product.md) • [IoT GUI snaps](explanation/iot-gui-snaps-made-easy.md) • [On-screen keyboard](reference/ubuntu-frame-osk-documentation.md) • [VNC remote assistance](reference/ubuntu-frame-vnc-documentation.md)
- **Maintenance and migration**: [Diagnostic feature](reference/the-diagnostic-feature.md) • [Migrate from Mir Kiosk](how-to/migrating-from-mir-kiosk-to-ubuntu-frame.md) • [Release notes](release-notes.md)

For the complete section indexes, see {ref}`tutorial-index`, {ref}`how-to-index`, {ref}`explanation-index`, and {ref}`reference-index`.

## How this documentation is organised

The documentation follows the Diátaxis approach, with each type of content serving a different need:

- **Tutorials** guide you through a learning activity, from a first web kiosk to deploying your own graphical application.
- **How-to guides** provide task-focused instructions for running, configuring, and packaging Ubuntu Frame deployments.
- **Explanations** describe concepts and context, including security, graphics support, and product design.
- **Reference** documents describe configuration, interfaces, diagnostics, and supporting components.
- **Release notes** record notable changes in each Ubuntu Frame release.

The thematic groups above show what the documentation covers; they do not change the purpose or arrangement of these content types. Choose a tutorial to learn by doing, a how-to for a specific task, an explanation to understand why something works as it does, or reference material when you need precise details.

## Project and community

Ubuntu Frame is developed and maintained by Canonical and is open source. The source code, issue tracker, and contribution workflow are available in the [Ubuntu Frame GitHub repository](https://github.com/canonical/ubuntu-frame). Bug reports, feature requests, and proposed fixes are welcome; please follow the repository's contribution guidance and the Ubuntu [Code of conduct](https://ubuntu.com/community/ethos/code-of-conduct).

Join product discussions on [Ubuntu Discourse](https://discourse.ubuntu.com/tag/ubuntu-frame) or the [Mir community Matrix room](https://matrix.to/#/#mir-server:matrix.org). For commercial enquiries, contact [Canonical](https://canonical.com/contact-us). Support terms depend on the Ubuntu platform and support offering used for a deployment; see [Ubuntu Core](https://ubuntu.com/core) for platform details.

```{toctree}
---
hidden:
---
tutorial/index
how-to/index
explanation/index
reference/index
release-notes
```

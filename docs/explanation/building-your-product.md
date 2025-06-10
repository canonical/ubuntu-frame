(building-your-product)=

# Building your product

## With Ubuntu Frame and Ubuntu Core

You've built an application and packaged it with a snap for use with Ubuntu Frame. Now you want to deliver the whole thing to your customers.

What you need is an Ubuntu Core image. It's a pre-configured bootable image that includes one or more snaps to provide a specific set of features: {doc}`core:how-to-guides/using-ubuntu-core`.

### Which base?

You should make an explicit decision on which `base` to build your appliance with. Corresponding to each recent LTS there is a `base` for snaps:

| `base`   | Corresponding LTS |
| -------- | ----------------- |
| `core22` | Ubuntu 22.04LTS   |
| `core20` | Ubuntu 20.04LTS   |
| `core18` | Ubuntu 18.04LTS   |
| `core`   | Ubuntu 16.04LTS   |

As well as providing more recent libraries and tools to work with, that means support for the base is limited by support for the LTS. For example, The core20/20.04 version is supported until April 2030.

Ubuntu Frame supports both `base: core20` and `base: core22`.

For `base: core20` there is a `20` track using `graphics-core20` (with a `mesa-core20` default provider). This is in maintenance mode and will only receive security updates.

For `base: core22` there is a `22` track using `graphics-core22` (with a `mesa-core22` default provider). This is the basis for ongoing development.

Your application snap can also make use of either `base: core20` or `base: core22` (and the corresponding `graphics-coreXX` content interface.

In principle, it is possible to mix & match snaps with different bases but that comes at a cost having multiple base and graphics snaps installed.

Ubuntu Frame also has a default `latest` track. This is deprecated in favour of appliances explicitly choosing either `base: core20` or `base: core22`.

### Maintaining your snap

You also need to consider maintenance of your snap and testing with changes to the other snaps you are packaged with.

The snap store will send you emails if there is a security update to any of the packages included in your snap. You should be ready to for a "no-change" repackaging of your snap to pick up these updates.

If you've based your snap on one of our tutorials be aware that we sometimes update these. Either because we find a better approach, or because we update things. (At the time of writing, we'll soon be updating our examples from `base: core20` to `base: core22`.)

There's more documentation on managing your snaps here:

https://snapcraft.io/docs

### Call for testing (Ubuntu Frame)

As part of the release cycle of Ubuntu Frame we test it with the snaps we maintain and those we have developed with customers. Following that we promote the release to the `candidate` and announce a "call for testing". You also should run your testing devices on the `candidate` channel, in which we keep our next stable for at least a week before promoting to `stable`.

https://discourse.ubuntu.com/c/project/mir/15

### Branding

If you are a commercial entity, not an individual packager, then you may well wish to publish your snap(s) from a brand account:

https://snapcraft.io/docs/store-brand-accounts

## Commercial options

You can gain explicit control over the versions of snaps distributed to your devices with a "Brand Store":

https://ubuntu.com/core/docs/dedicated-snap-stores

If you want official support for your snap and our QA pipeline replicate your exact setup, please contact our support team at https://ubuntu.com/support.

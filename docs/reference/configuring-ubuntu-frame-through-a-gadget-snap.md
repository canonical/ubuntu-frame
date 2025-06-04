(configuring-ubuntu-frame-through-a-gadget-snap)=
# Configuring Ubuntu Frame through a gadget snap

The [snapcraft documentation](https://snapcraft.io/docs/gadget-snap) describes gadget snaps in detail. The only additional thing we'll show here is an example showing how to set the above configuration options for `ubuntu-frame`:

```yaml
defaults:
  BPZbvWzvoMTrpec4goCXlckLe2IhfthK:
    display: |
      layouts:
        default:
          cards:
            - card-id: 0
              unknown-1:
                orientation: inverted
    config: |
      wallpaper-top=0x92006a
      wallpaper-bottom=0xdd4814
      add-wayland-extensions=zwp_pointer_constraints_v1:zwp_relative_pointer_manager_v1
    daemon: true
```


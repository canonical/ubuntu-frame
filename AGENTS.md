# Repository Guide

## Workshop Environment

Identify the Workshop environment by checking whether `$USER` is `workshop`. `.workshop/dev.yaml` documents the toolchain setup and build configuration for reference. Do not try to invoke its Workshop actions from inside the Workshop environment. Build storage is mounted from `$HOME/build`.

- C++/Frame CMake build directory: `$HOME/build/src`.
- Flutter launcher build directory: `$HOME/build/launcher`.
- The `.workshop/dev.yaml` actions show how Frame and launcher builds are configured; treat them as reference, not commands to run inside Workshop.
- The Frame GUI action requires the Workshop desktop plug and a working `$DISPLAY`; check availability before trying to launch it.

Outside Workshop, do not assume these build directories or Workshop actions exist. Inspect the project build configuration and available environment before building.
If the user directs you to use Workshop or its actions from outside Workshop, follow the [use-workshop skill](https://github.com/canonical/use-workshop-skill/tree/main/.github/skills/use-workshop).

## C++ Guidelines

- Follow the [Mir C++ guide](https://canonical.com/mir/docs/page/contributing/reference/cppguide/).

## Validation

- Run repository lint hooks with `pre-commit run --all-files`.
- Validate documentation with `make -C docs html`; this target fails on Sphinx warnings.
- Use `make -C docs spelling` for documentation spelling checks and `make -C docs lint-md` for Markdown linting.
- Build C++ targets in Workshop with `cmake --build "$HOME/build/src"`. Run focused CTest suites when possible; for hardware-independent CEC tests, use `ctest --test-dir "$HOME/build/src" --output-on-failure -R '^(Cec|LibCec)'`.

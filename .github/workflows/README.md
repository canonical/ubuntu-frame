# Reusable snap workflows

The reusable workflows build one architecture or close one snap channel per
call. Callers own their event triggers, permissions, concurrency, matrix and
any dependencies between jobs. Other repositories can call them using
`canonical/ubuntu-frame/.github/workflows/reusable-snap.yml@<ref>` and
`canonical/ubuntu-frame/.github/workflows/reusable-close-snap.yml@<ref>`.
Pin `<ref>` to a reviewed commit.

## Building

`reusable-snap.yml` requires `architecture`. It selects `ubuntu-latest` for
amd64 and riscv64, `ubuntu-26.04-arm` for arm64, and the self-hosted labels
`self-hosted`, the architecture, and `medium` otherwise. Set `runner` to
override this selection, for example to use `ubuntu-latest` for a Launchpad
remote build. Remote builds require the optional `launchpad-credentials` secret
and explicit `launchpad-accept-public-upload: true`.

The caller can configure `timeout-minutes` (default 90), `repository`, `ref`,
`checkout-lfs`, `fetch-depth` (default 0), `snapcraft-channel`, `review`,
`review-opts`, `publish`, `publish-channel` and `upload-artifact`. The
`snapcraft-token` secret is required when publishing. Publishing defaults to
false; callers must restrict it to trusted events, such as pull requests from
the same repository. The caller must grant `contents: read`.
Explicitly map the secrets in each caller; secrets are not passed automatically.

Optional `prepare` and `test` inputs contain Bash commands executed after
checkout and after the build respectively. Both receive `SNAP_ARCHITECTURE`;
the test also receives `SNAP_PATH` and `SNAP_NAME`. These commands run on the
build runner and can access its files, so use trusted commands and do not
interpolate untrusted event data into them. For architecture-specific tests,
callers can set `test` conditionally in their matrix.

The `snap-name` workflow output is the built filename. A runner's `SNAP_PATH`
is not available to later jobs: set `upload-artifact: true` and download the
artifact by its name if another job needs the file. Matrix callers should not
rely on a single workflow output to collect all matrix results.
Jobs that need several individual matrix statuses or additional steps on the
same runner may need their existing orchestration rather than a direct
replacement with this workflow.

## Closing

`reusable-close-snap.yml` requires `channel` and accepts `name` (defaults to
the caller repository name), `snapcraft-channel` and the `snapcraft-token`
secret. Like the original close workflows, errors are ignored by default;
set `continue-on-error: false` to make them fail the job. Call it with a
matrix of names and channels to close multiple snaps.

The local `snap.yml` and `close-snap.yml` demonstrate how to call these
workflows, including same-repository pull request publishing and matching
per-pull-request channels.
Calling a reusable workflow changes the displayed check names; update any
required status checks when migrating.

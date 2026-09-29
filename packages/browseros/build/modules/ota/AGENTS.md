# `build/modules/ota/` — Server OTA appcast generation and publishing

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`packages/browseros/build/modules/`](../AGENTS.md).

## What's here

The OTA update path for the **BrowserOS Server** binary (not the
browser). It signs and notarizes per-platform server bundles, zips
them, builds or extends a Sparkle appcast that preserves previously
published items, and uploads the result to the CDN. This area has its
own signing code rather than reusing `../sign/`, because OTA signing
operates on already-built artifacts on a non-build machine.

`SERVER_PLATFORMS` in `common.py` is the platform list; the appcast
seed XML lives in `../../config/appcast/`.

## Contents

| File | What it does |
|---|---|
| `common.py` | `SPARKLE_NS`, `SERVER_PLATFORMS` (5 targets), `APPCAST_TEMPLATE`, `ENCLOSURE_TEMPLATE`, `class SignedArtifact`, `class ExistingAppcast`, `find_server_resources_dir()`, `parse_existing_appcast()`, `generate_server_appcast()`, `create_server_bundle_zip()`, `get_appcast_path(channel)`. |
| `sign_binary.py` | `sign_macos_binary()`, `verify_macos_signature()`, `_resolve_notarization_credentials()`, `_submit_notarization()`, `notarize_macos_binary()`, `notarize_macos_zip()`, `sign_windows_binary()`, `sign_server_bundle_macos()`, `sign_server_bundle_windows()`. |
| `server.py` | `class ServerOTAModule(CommandModule)` — `produces = ["server_ota_artifacts", "server_appcast"]`, `description = "Create and upload BrowserOS Server OTA update"`. |
| `bundle_test.py` | Unit tests for `create_server_bundle_zip()` and `find_server_resources_dir()`. |
| `__init__.py` | Re-exports everything plus `AVAILABLE_MODULES = {"server_ota": ServerOTAModule}`. |

## Rules

**OTA1 — Server signing is deliberately separate from `../sign/`.**
`sign_binary.py` signs files in place on a release machine; the
`../sign/` modules sign a Chromium app bundle during the build. Do not
merge them and do not "de-duplicate" by making one call the other.

**OTA2 — `SERVER_PLATFORMS` is the platform contract.** Each entry
carries `name`, `binary`, `target`, `os`, `arch`; `target` is the R2
path segment, `arch` uses `x86_64` (not `x64`). Adding a platform
means adding an entry here plus an `r2_key` in
`../../config/download_resources.yaml`.

**OTA3 — Appcast generation must preserve history.**
`parse_existing_appcast()` reads the seed/current appcast and
`generate_server_appcast()` re-emits prior `<item>` entries alongside
the new one. Never generate an appcast from `APPCAST_TEMPLATE` alone
in a release path — that silently drops every previous version.

**OTA4 — The channel is an argument, and the function default is `alpha`.**
`get_appcast_path(channel="alpha")` returns
`../../config/appcast/appcast-server.alpha.xml` for `"alpha"` and
`appcast-server.xml` for anything else. Pass the channel explicitly; do not
rely on the default.

**OTA5 — Notarization credentials come from `EnvConfig`**, resolved by
`_resolve_notarization_credentials()`. Never read them from the appcast
directory or a local file.

**OTA6 — `verify_macos_signature()` exists for a reason.** After
`sign_macos_binary()`, confirm the signature before submitting to
notarization; a bad signature wastes a full notarization round trip.

## Workflows

**Publishing a Server OTA release**
1. The per-platform server bundles are downloaded from R2 into a temp
   root, laid out as `<binaries_dir>/<target>/resources/`
   (`find_server_resources_dir()`); nothing needs to be staged in
   `packages/browseros/resources/` by hand.
2. `browseros ota server release` — signs, notarizes (macOS), zips,
   generates the appcast, uploads.
3. Verify the published appcast, not the local one.

**Inspecting supported platforms**
1. `browseros ota server list-platforms`.
2. Cross-check against `SERVER_PLATFORMS` in `common.py`.

**Generating an appcast without uploading**
1. `browseros ota server release-appcast`.
2. It reads the seed from `../../config/appcast/` and merges the new
   item, preserving history.

**Verifying signing credentials work**
1. `browseros ota test-signing`.
2. On macOS this must run in a session with keychain access — see
   `../../docs/nightly-macos-ci.md` for the `launchd` caveat.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — all 14 module areas.
- [`../../config/appcast/AGENTS.md`](../../config/appcast/AGENTS.md) — the seed appcast files.
- [`../sign/AGENTS.md`](../sign/AGENTS.md) — browser signing (a separate code path).
- [`../../common/sparkle.py`](../../common/sparkle.py) — `sparkle_sign_file()`.
- [`../../cli/ota.py`](../../cli/ota.py) — the `browseros ota` commands.

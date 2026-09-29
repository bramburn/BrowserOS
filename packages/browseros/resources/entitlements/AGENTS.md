# `resources/entitlements/` — macOS codesign entitlements and Sparkle keys

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`packages/browseros/`](../AGENTS.md).

## What's here

The Apple codesign entitlement plists applied to BrowserOS components during
build phase 4, plus one non-plist fragment. They are consumed *directly* by
`build/modules/sign/macos.py` (app bundle, helper apps, server binaries) and
by `build/common/server_binaries.py`; they are not listed in
`build/config/copy_resources.yaml` and are never copied into the Chromium
tree.

`Info.plist.additions` is a key/value fragment (not a full plist) holding the
Sparkle update-framework keys that get merged into the built
`BrowserOS.app/Contents/Info.plist`.

## Contents

| File | Applied to | Keys |
|---|---|---|
| `app-entitlements.plist` | `BrowserOS.app` (main bundle) | `device.audio-input`, `device.bluetooth`, `device.camera`, `device.print`, `device.usb`, `personal-information.location`, `personal-information.photos-library` |
| `browseros-executable-entitlements.plist` | `browseros_server`, `bun` | `cs.allow-jit`, `cs.allow-unsigned-executable-memory`, `cs.disable-executable-page-protection`, `cs.allow-dyld-environment-variables`, `cs.disable-library-validation` |
| `helper-gpu-entitlements.plist` | GPU helper app | `cs.allow-jit` |
| `helper-renderer-entitlements.plist` | renderer helper app | `cs.allow-jit` |
| `helper-plugin-entitlements.plist` | plugin host helper app | `cs.allow-unsigned-executable-memory`, `cs.disable-library-validation` |
| `lima-vz-entitlements.plist` | `limactl` (VM driver) | `security.virtualization` |
| `Info.plist.additions` | merged into `BrowserOS.app` plist | `SUPublicEDKey`, `CrProductDirName`, `SUEnableAutomaticChecks`, `SUScheduledCheckInterval` (3600), `SUAllowsAutomaticUpdates`, `SUAutomaticallyUpdate` |

## Rules

**EN1 — Filenames are the contract.** `sign/macos.py` looks up
`helper-renderer-entitlements.plist`, `helper-gpu-entitlements.plist`,
`helper-plugin-entitlements.plist` by literal name, and `SignSpec` in
`server_binaries.py` stores bare filenames. Renaming a file here silently
signs the component without its entitlements (a warning at best).

**EN2 — `browseros-executable-entitlements.plist` is shared by two
binaries.** `browseros_server` and `bun` use the identical plist. Splitting
them requires editing the `MACOS_SERVER_BINARIES` table, not duplicating
the file.

**EN3 — `cs.disable-library-validation` and
`cs.allow-dyld-environment-variables` weaken the hardened runtime.** They
exist because `browseros_server` loads a Bun runtime and dylibs from its own
bundle. Removing them breaks signing verification of the server binary.

**EN4 — `Info.plist.additions` is a fragment, not a plist.** It has no
`<?xml?>` header and no `<dict>` wrapper — only `<key>`/`<value>` pairs.
Do not add XML boilerplate; the merge inserts the pairs into an existing
`dict`.

**EN5 — `SUPublicEDKey` must match the EdDSA key used to sign the update
feed.** Changing the appcast key without changing the signing key (or vice
versa) makes Sparkle reject every update. The signing side is
`../../build/common/sparkle.py` (`sparkle_sign_file()`, driven by the
`SPARKLE_PRIVATE_KEY` in `ctx.env`); there is no
`tools/release/generate_update_manifests.py` in this repo.

**EN6 — Entitlements are not a copy resource.** Adding a `copy_operations`
entry for this directory would push plists into the Chromium tree for no
reason. `ctx.get_entitlements_dir()` (`build/common/context.py:431`) is the
only supported lookup.

**EN7 — Missing entitlements fail the sign step loudly, not silently.**
`sign_binary.py:385-388` raises `Missing entitlements for <name>` when a
`SignSpec` names a file that is not on disk. That is the expected failure
mode; do not "fix" it by setting `entitlements: None`.

## Workflows

**Adding an entitlement for a new bundled binary**
1. Create the `.plist` here.
2. Add a `SignSpec(identifier_suffix, options, "<file>.plist")` entry to
   `MACOS_SERVER_BINARIES` in
   [`../../build/common/server_binaries.py`](../../build/common/server_binaries.py).
3. Both the phase-4 sign path and the OTA path pick it up automatically
   (that is the stated purpose of that module).

**Changing the Sparkle check cadence**
1. Edit `SUScheduledCheckInterval` in `Info.plist.additions`.
2. Leave `SUPublicEDKey` untouched unless you are also rotating the
   manifest signing key.
3. Confirm the built app's plist carries the new value before packaging.

**Debugging a codesign rejection on macOS**
1. `Missing entitlements for <path>` → the `SignSpec` filename and this
   directory disagree (EN1).
2. `resource fork, Finder information` → the plist picked up extended
   attributes; re-clone rather than re-`chmod`.
3. `signature not valid for hardened runtime` → an entitlement was removed
   from `browseros-executable-entitlements.plist` (EN3).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `resources/` overview (RS6, signing
  context).
- [`../../build/common/server_binaries.py`](../../build/common/server_binaries.py) —
  `SignSpec` table consumed by both sign and OTA.
- [`../../build/modules/sign/macos.py`](../../build/modules/sign/macos.py) —
  applies these plists to the app and helpers.
- [`../../build/modules/ota/sign_binary.py`](../../build/modules/ota/sign_binary.py) —
  the `Missing entitlements` guard.
- [`../../build/common/sparkle.py`](../../build/common/sparkle.py) — Sparkle
  framework setup that pairs with `Info.plist.additions`.

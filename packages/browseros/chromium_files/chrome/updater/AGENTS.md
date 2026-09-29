# `chrome/updater/` — updater branding (`branding.gni`)

> Part of [`../../AGENTS.md`](../../AGENTS.md) in
> [`chromium_files/`](../../AGENTS.md).

## What's here

`branding.gni` — the branding variable set consumed by Chromium's updater
(macOS Keystone / GoogleUpdate3 COM clients, Windows installer, metainstaller)
as built for BrowserOS. The `is_chrome_branded` branch delegates to
`//chrome/updater/internal/branding_google.gni`; the `else` branch is the
BrowserOS set and is the one this fork builds.

`build/modules/resources/chromium_replace.py` copies it verbatim onto
`<chromium_src>/chrome/updater/branding.gni`.

## Contents

`branding.gni` (109 lines), in four blocks:

| Block | Contents |
|---|---|
| Names + bundle ids | `browser_name`, `browser_product_name`, `mac_browser_bundle_identifier`, `mac_updater_bundle_identifier`, `keystone_bundle_identifier` |
| macOS updater | `keystone_app_name`, `privileged_helper_*`, `updater_app_icon_path`, `mac_team_identifier` |
| App ids / GUIDs | `updater_appid`, `browser_appid`, `qualification_appid`, `prefs_access_mutex`, `setup_mutex_prefix`, plus ~30 `*GUID` COM interface constants |
| Update config | `update_check_url` (empty), `updater_event_logging_url` (empty), `extra_args_is_chrome_branded`, `grdfile_name` |

## Rules

**UP1 — The ~30 COM GUIDs are Microsoft-registered and must not be
regenerated.** `UpdaterLegacyLibGUID`, `IGoogleUpdate3Web*`, `IPolicyStatus*`,
`IProcessLauncher*`, etc. are the CLSID/IID pairs the Windows update client
registers. Changing any of them produces a client that registers a fresh
object every run — new CLSIDs, orphaned registrations, updater state loss.

**UP2 — `update_check_url` and `updater_event_logging_url` are empty by
design.** This GN file is not where the fork's update endpoints live. The
overlay contains no `components/update_client/` patch, and there is no
`tools/release/generate_update_manifests.py`; the Server update path is the
Sparkle appcast pipeline in `../../../build/modules/ota/` (appcast XML,
signing in `../../../build/common/sparkle.py`). Filling these in without a
signed-payload server points the updater at nothing.

**UP3 — `mac_team_identifier` is the literal string `PLACEHOLDER`, and nothing
rewrites it.** `build/modules/sign/macos.py` signs with its own certificate,
identifier and entitlements plists; it never touches this GN variable. Leave
the placeholder unless you also have a matching Developer ID to hard-code.

**UP4 — `updater_app_icon_path` must match a real copied icon.** It points at
`//chrome/app/theme/chromium/mac/app.icns`, which is produced by the
`resources` module from `../../../../resources/icons/mac/app.icns`. Renaming
that asset without updating this line breaks the macOS updater bundle.

**UP5 — `crx_pkhash` is empty, and that is intentional.** The fork does not
pin an extension public-key hash here. Do not copy a value from
`branding_google.gni`.

**UP6 — `updater_copyright` is a hardcoded year** (`Copyright 2020
BrowserOS.`), unlike the `@LASTCHANGE_YEAR@` token used in
`../app/theme/chromium/BRANDING.release`. Update it by hand when you
rebrand; there is no substitution pass over this file.

**UP7 — Keep this file under the `branding` feature in `build/features.yaml`.**
It is listed explicitly, so `browseros dev annotate` groups it with the other
identity files.

## Workflows

**Pointing the macOS updater at a different Sparkle feed**
1. `SUPublicEDKey` and `CrProductDirName` are the keys this fork sets, in
   `../../../../chromium_patches/chrome/app/app-Info.plist`; no `SUFeedURL`
   override is patched, so the feed URL comes from the Sparkle appcast the
   build publishes.
2. Change `SUPublicEDKey` to the matching EdDSA public key — the same key
   `build/common/sparkle.py` signs DMGs with.
3. Leave `keystone_app_name` and `mac_team_identifier` alone.

**Adding a new GUID-bearing updater interface**
1. Add the constant next to its siblings, keeping the
   `<Iface>GUID` / `<Iface>UserGUID` / `<Iface>SystemGUID` triple.
2. Copy the value from the Microsoft-registered set — never mint a new one.

**Verifying the copy**
1. Run `browseros build -m chromium_replace`.
2. Confirm `<chromium_src>/chrome/updater/branding.gni` changed and the
   build reported one more replaced file.

## Cross-references

- [`../../AGENTS.md`](../../AGENTS.md) — `chromium_files/` rules (CF1, CF2).
- [`../app/theme/chromium/AGENTS.md`](../app/theme/chromium/AGENTS.md) —
  browser product identity that must match `browser_name` here.
- [`../enterprise_companion/AGENTS.md`](../enterprise_companion/AGENTS.md) —
  the sibling branding file.
- [`../../../build/modules/sign/macos.py`](../../../build/modules/sign/macos.py) —
  certificate, identifier and entitlements used at signing time.
- [`../../../../AGENTS-build.md`](../../../../../AGENTS-build.md) — the
  Omaha-4 / appcast / `latest.json` update pipeline.

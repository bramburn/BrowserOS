# `chrome/enterprise_companion/` — enterprise companion branding

> Part of [`../../AGENTS.md`](../../AGENTS.md) in
> [`chromium_files/`](../../AGENTS.md).

## What's here

A single GN file, `branding.gni`, that defines the branding variables for
Chrome's enterprise companion app (the policy / device-management helper) as
built for BrowserOS. It is a copy of the upstream file with the
`is_chrome_branded` `else` branch rewritten to BrowserOS values, plus the
unchanged device-management endpoint block at the bottom.

`build/modules/resources/chromium_replace.py` copies it verbatim onto
`<chromium_src>/chrome/enterprise_companion/branding.gni`.

## Contents

`branding.gni` (43 lines) — two branches plus four unconditional URLs:

| Branch | Key values |
|---|---|
| `is_chrome_branded` (Chrome) | `Chrome_Enterprise_Companion`, Google names, `com.google.ChromeEnterpriseCompanion` |
| `else` (BrowserOS, active) | crash product `BrowserOS_Enterprise_Companion`, appid `{d6acc642-8982-441d-949b-312d5ccb559f}`, names `BrowserOS` / `browseros` / `BROWSEROS`, keystone app `BrowserOSSoftwareUpdate`, bundle id `com.browseros.BrowserOSEnterpriseCompanion` |

Unconditional (both branches): `enterprise_companion_device_management_server_url`,
`..._realtime_reporting_url`, `..._encrypted_reporting_url`,
`enterprise_companion_event_logging_url` — all still Google endpoints.

## Rules

**EC1 — Only edit the `else` branch.** The `is_chrome_branded` branch is
kept for overlay/import comparisons; changing it has no effect on a
BrowserOS build and breaks a Chrome-branded one.

**EC2 — `enterprise_companion_appid` is a policy identity, not a cosmetic
string.** It is referenced by Chrome policy registration; reusing an existing
Google GUID would collide with the Chrome install. The BrowserOS value
`{d6acc642-8982-441d-949b-312d5ccb559f}` is already distinct — leave it.

**EC3 — Crash upload still points at Google's staging endpoint.**
`enterprise_companion_crash_upload_url` is
`https://clients2.google.com/cr/staging_report`. Routing BrowserOS crash
reports to a private backend means changing this line (and the matching
override in
`chromium_patches/chrome/app/chrome_crash_reporter_client*.cc`) together, not
one alone.

**EC4 — The four device-management URLs are intentional leftovers.** They
point at `m.google.com` and `chromereporting-pa.googleapis.com`. Changing
them to a non-existent host silently breaks Chrome enterprise policy
enrolment; if you must repoint them, do it in
`chromium_patches/` so the diff is reviewable.

**EC5 — Keep `import("//build/config/chrome_build.gni")` first.** The file
depends on `is_chrome_branded` being defined by that import; removing it
turns both branches into a GN unresolved-variable error.

**EC6 — Keep the file listed under the `branding` feature in
`build/features.yaml`.** `chrome/enterprise_companion/branding.gni` is named
explicitly there, so `browseros dev annotate` groups it with the other
identity changes.

## Workflows

**Pointing enterprise-companion crash reports at your own collector**
1. Change `enterprise_companion_crash_upload_url` in this file.
2. Change `GetUploadUrl()` in
   `../../../../chromium_patches/chrome/app/chrome_crash_reporter_client.cc`
   (and the Windows client) so the main browser matches.
3. Re-run `browseros build -m chromium_replace` and confirm the copy count.

**Rebranding the companion app**
1. Update `enterprise_companion_crash_product_name` and the three
   `enterprise_companion_company_short_name*` variants (they must agree on
   case: `BrowserOS` / `browseros` / `BROWSEROS`).
2. Update `enterprise_companion_product_full_name` and its
   `_dashed_lowercase` form together.
3. Update `mac_enterprise_companion_bundle_identifier` in step with
   `MAC_BUNDLE_ID` in `../app/theme/chromium/BRANDING.release`.

## Cross-references

- [`../../AGENTS.md`](../../AGENTS.md) — `chromium_files/` rules (CF1, CF2).
- [`../updater/AGENTS.md`](../updater/AGENTS.md) — the sibling branding file.
- [`../app/theme/chromium/AGENTS.md`](../app/theme/chromium/AGENTS.md) —
  browser product identity.
- [`../../../build/features.yaml`](../../../build/features.yaml) —
  `branding` feature block.
- [`../../../chromium_patches/chrome/app/AGENTS.md`](../../../chromium_patches/chrome/app/AGENTS.md) —
  crash-reporter upload URL overrides.

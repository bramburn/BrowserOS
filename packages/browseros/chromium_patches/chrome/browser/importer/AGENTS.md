# `importer/` — Chrome-profile import (cookies + extensions)

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/`) in
> [`packages/browseros/`](../../../../AGENTS.md).

## What's here

The browser-process half of the import pipeline, and the fork adds two new
data types to it. Both bridges gain handlers for them:
`in_process_importer_bridge.{h,cc}` gets `SetCookie()` and `SetExtensions()`,
and `external_process_importer_client.{h,cc}` gets `OnCookieImportReady()` and
`OnExtensionsImportReady()`. The browser side lands them:
`profile_writer.{h,cc}` gains `AddCookie()` and `AddExtensions()` (the latter
is an explicitly "silent installer via webstore without any prompt or bubble"),
and `importer_list.cc` gains the detection logic that decides whether a source
Chrome profile has anything to import. `importer_uma.cc` adds the
`TYPE_CHROME` histogram bucket.

## Contents

```
importer/
├── importer_list.{cc}                  ← DetectChromeProfiles(), per-profile service
│                                         detection (Bookmarks / History / Login Data /
│                                         Cookies / Preferences), HasExtensionsToImport()
│                                         parsing extensions.settings with
│                                         base::JSON_PARSE_CHROMIUM_EXTENSIONS
├── profile_writer.{h,cc}               ← AddCookie(), AddExtensions() (silent webstore
│                                         install, no prompt or bubble)
├── in_process_importer_bridge.{h,cc}   ← SetCookie(), SetExtensions()
├── external_process_importer_client.{h,cc} ← OnCookieImportReady(),
│                                         OnExtensionsImportReady()
└── importer_uma.cc                     ← IMPORTER_METRICS_CHROME = 8 for TYPE_CHROME
```

## Rules

**IMP1 — `IMPORTER_METRICS_CHROME = 8` is a persisted histogram value.**
`importer_uma.cc` appends it after `IMPORTER_METRICS_EDGE = 7` with the
upstream comment "Never remove any existing values, as this enum is used to
bucket a UMA histogram". Append only; never renumber.

**IMP2 — Both bridges must be updated together.**
`SetCookie`/`SetExtensions` (in-process) and
`OnCookieImportReady`/`OnExtensionsImportReady` (external process) are the two
sides of the same `user_data_importer` mojo interface, patched separately
under `../../../components/user_data_importer/`. A one-sided change compiles
and then silently drops the data.

**IMP3 — `AddExtensions()` is a silent install by design.** The comment in
`profile_writer.cc` is explicit: "Silent installer via webstore without any
prompt or bubble." Do not route it through a normal install path that prompts;
that changes the import contract.

**IMP4 — Extension detection is deliberately narrow and lenient.**
`HasExtensionsToImport()` counts an extension only when it is **not**
`was_installed_by_default` **and** `from_webstore` is true. The inline comment
notes that disabled extensions are still counted ("we're being lenient here,
importing disabled ones too for now"). Tightening or loosening this changes
which users get a non-empty import dialog.

**IMP5 — Both `Preferences` and `Secure Preferences` are probed.**
`DetectChromeProfiles()` sets the `EXTENSIONS` service bit when either file has
importable extensions. Dropping the `Secure Preferences` check loses the
majority of Chrome installs, where that is where the extension list lives.

**IMP6 — Parsing uses `base::JSONReader::ReadDict(..., JSON_PARSE_CHROMIUM_EXTENSIONS)`.**
Chrome's on-disk prefs are not strict JSON (trailing commas, `//` comments);
the flag is what makes the read succeed. A plain `ReadDict()` returns
`std::nullopt` on a real Chrome profile.

**IMP7 — `DetectChromeProfiles()` runs under a
`base::ScopedBlockingCall`.** It reads files and parses JSON synchronously; the
scoped-blocking wrapper is required and must not be removed.

## Workflows

**Adding a new importable data type**
1. Add the enum to `user_data_importer` under
   `../../../components/user_data_importer/`.
2. Add `On…ImportReady()` to `external_process_importer_client.{h,cc}` and
   `Set…()` to `in_process_importer_bridge.{h,cc}`.
3. Add the browser-side writer to `profile_writer.{h,cc}`.
4. Detect it in `importer_list.cc` (`base::PathExists` on the source file plus,
   for JSON-backed types, the `JSON_PARSE_CHROMIUM_EXTENSIONS` read).
5. Add the checkbox in
   `../resources/settings/people_page/import_data_dialog.html` and the
   `BrowserProfile` field in `import_data_browser_proxy.ts`.
6. Extract every touched file and list them under one `features.yaml` block.

**Adding a UMA bucket for an importer type**
1. Append the next free value to the `ImporterTypeMetrics` enum in
   `importer_uma.cc`.
2. Add the `case user_data_importer::TYPE_X:` to
   `LogImporterUseToMetrics()`.
3. Update the mirrored `enums.xml` under
   `../../../tools/metrics/histograms/metadata/` in the same change (LINT).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/` overlay.
- [`../../../components/user_data_importer/AGENTS.md`](../../../components/user_data_importer/AGENTS.md)
  — the mojo interface both bridges implement.
- [`../resources/settings/people_page/AGENTS.md`](../resources/settings/people_page/AGENTS.md)
  — the import dialog checkboxes and `BrowserProfile` fields.
- [`../../../chrome/common/extensions/api/AGENTS.md`](../../../chrome/common/extensions/api/AGENTS.md)
  — allowlist rows for `kImportDialogExtensions` / `kImportDialogCookies`.
- [`../../../../build/features.yaml`](../../../../build/features.yaml) — patch manifest.

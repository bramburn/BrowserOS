# `chrome/browser/ui/webui/settings/` — settings page BrowserOS handlers

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/webui/`) in
> [`packages/browseros/`](../../../../../../AGENTS.md).

## What's here

The browser-process side of `chrome://settings`: the BrowserOS metrics
message handler, the Chrome import dialog handler, the localized-string
provider additions, and the pref/handler registrations.

## Contents

```
settings/
├── browseros_metrics_handler.{cc,h}     ← NEW: SettingsPageUIHandler that accepts the
│                                           "logBrowserOSMetric" WebUI message and
│                                           forwards it to browseros_metrics
├── import_data_handler.cc               ← maps prefs::kImportDialogExtensions /
│                                           kImportDialogCookies to
│                                           user_data_importer::EXTENSIONS / COOKIES,
│                                           and reports both back to the page
├── settings_ui.cc                       ← registers both import-dialog prefs
│                                           (default true) and
│                                           AddSettingsPageUIHandler(
│                                               std::make_unique<BrowserOSMetricsHandler>())
└── settings_localized_strings_provider.cc
                                        ← adds "aboutBrowserOSVersion" (from
                                           version_info::GetBrowserOSVersionNumber()) and
                                           importDialogExtensions / importDialogCookies
                                           IDS mappings
```

## Rules

**SW1 — New settings handlers are `SettingsPageUIHandler` subclasses
registered in `settings_ui.cc`.** `AddSettingsPageUIHandler(
std::make_unique<…>())` is the only registration path. A handler not
registered here never receives messages.

**SW2 — Message names are wire contracts.** `logBrowserOSMetric` is the
exact string the settings page sends. Renaming it silently breaks the page.

**SW3 — Import-dialog prefs default to `true` in `settings_ui.cc`.** The
test data in `chrome/test/data/webui/settings/import_data_dialog_test.ts`
explicitly clears both, so changing the defaults requires updating that
test too.

**SW4 — The Mojo struct field names (`extensions`, `cookies`) must match
the TypeScript.** `import_data_handler.cc` writes
`browser_profile.Set("extensions", …)` and
`browser_profile.Set("cookies", …)`; the dialog template in
`chrome/browser/resources/settings/people_page/import_data_dialog.html` reads
the same keys.

**SW5 — Strings need three edits.** `IDS_` message in
`chrome/app/settings_strings.grdp`, mapping in
`settings_localized_strings_provider.cc`, use in the Lit/TS page.

**SW6 — `aboutBrowserOSVersion` comes from
`version_info::GetBrowserOSVersionNumber()`.** The upstream `version_info.h`
is patched outside this tree (the `branding` feature). Do not hard-code a
version string here.

**SW7 — Owned by three different feature blocks.** `settings_ui.cc`,
`settings_localized_strings_provider.cc`, and `import_data_handler.cc` are
under `chrome-importer`; `browseros_metrics_handler.*` is under `metrics`;
the localized-strings provider is under `branding`. Check
`build/features.yaml` before attributing a commit.

## Workflows

**Adding a new settings message handler**
1. Create `<name>_handler.{cc,h}` in this directory, extending
   `settings::SettingsPageUIHandler`.
2. Override `RegisterMessages()` and call
   `web_ui()->RegisterMessageCallback(...)`.
3. Register it in `settings_ui.cc` with
   `AddSettingsPageUIHandler(std::make_unique<…>())`.
4. Add the sources to `chrome/browser/ui/BUILD.gn` (not
   `chrome/browser/ui/webui/BUILD.gn`).
5. Add the paths to the owning feature block in
   `packages/browseros/build/features.yaml`.

**Adding an import data type**
1. Add the pref name in `chrome/common/pref_names.h`.
2. Register the default in `settings_ui.cc`.
3. Map it to a `user_data_importer::*` bit in `import_data_handler.cc`.
4. Add the `IDS_SETTINGS_IMPORT_*_CHECKBOX` string in
   `chrome/app/settings_strings.grdp` and map it in
   `settings_localized_strings_provider.cc`.
5. Update `chrome/test/data/webui/settings/import_data_dialog_test.ts`.

**Showing a BrowserOS version string**
Already wired: `html_source->AddString("aboutBrowserOSVersion", …)` in
`settings_localized_strings_provider.cc`. Change the *version source*
(`version_info`), not this provider.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/webui/`.
- [`../../../browseros/metrics/AGENTS.md`](../../../browseros/metrics/AGENTS.md)
  — the metrics service this handler forwards to.
- [`../../../../app/AGENTS.md`](../../../../app/AGENTS.md) — the
  `IDS_SETTINGS_IMPORT_*` GRIT strings.
- [`../../../../common/importer/AGENTS.md`](../../../../common/importer/AGENTS.md)
  — the import IPC contract.
- [`../../../../test/data/webui/settings/AGENTS.md`](../../../../test/data/webui/settings/AGENTS.md)
  — the dialog test that must stay in sync.
- [`../../../../../../AGENTS.md`](../../../../../../AGENTS.md) —
  `packages/browseros/`.

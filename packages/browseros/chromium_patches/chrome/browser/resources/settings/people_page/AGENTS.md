# `resources/settings/people_page/` — import dialog extensions + cookies

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

The Chrome "Import data" dialog. Two files, both adding the same two data
types. `import_data_browser_proxy.ts` extends the `BrowserProfile` interface
with `extensions: boolean;` and `cookies: boolean;` alongside the existing
`bookmarks`, `history`, `passwords`, `search` and `autofillFormData` fields.
`import_data_dialog.html` adds two `settings-checkbox` elements —
`#importDialogExtensions` and `#importDialogCookies` — each bound to
`prefs.import_dialog_extensions` / `prefs.import_dialog_cookies` and each
`hidden="[[!selected_.extensions]]"` / `hidden="[[!selected_.cookies]]"` so
they only appear when the source profile actually has that data.

## Contents

```
people_page/
├── import_data_browser_proxy.ts   ← interface BrowserProfile { … extensions: boolean;
│                                    cookies: boolean; }
└── import_data_dialog.html        ← + <settings-checkbox id="importDialogExtensions">
                                      + <settings-checkbox id="importDialogCookies">
                                    both no-set-pref, hidden on !selected_.X
```

## Rules

**PP1 — The interface field and the checkbox are one change.**
`import_data_browser_proxy.ts` declares what the C++ side reports;
`import_data_dialog.html` binds what the user can tick. A field with no
checkbox is invisible; a checkbox with no field is permanently hidden.

**PP2 — Both checkboxes carry `no-set-pref`.**
They reflect what will be imported, not a user preference — same as the
neighbouring `importAutofillFormData` checkbox. Adding `set-pref` makes the
dialog write prefs it should only read.

**PP3 — Visibility is driven by `selected_`, not by a static flag.**
`hidden="[[!selected_.extensions]]"` and `hidden="[[!selected_.cookies]]"`.
The values come from `DetectChromeProfiles()` in `../../../importer/importer_list.cc`,
which sets the `user_data_importer::EXTENSIONS` and `COOKIES` service bits by
probing the source profile's files. A checkbox that shows unconditionally
promises data the importer may not have.

**PP4 — The prefs must be allowlisted for `settingsPrivate`.**
`kImportDialogExtensions` and `kImportDialogCookies` were added to
`../../../extensions/api/settings_private/prefs_util.cc` (inside the
ChromeOS guard). Removing them there while the HTML still binds them produces
a runtime allowlist rejection, not a compile error.

**PP5 — Both new items go after `importAutofillFormData`, before the closing
`</div>`.** The dialog is a flat list of `settings-checkbox` rows; inserting
elsewhere breaks the visual order the import types have on other platforms.

**PP6 — `import_data_browser_proxy.ts` is a type-only file.**
It declares `BrowserProfile`; the C++ that fills it is the
`ImportDataBrowserProxy` moom in
`../../../../chrome/common/importer/`. A field here with no moom field is
always `undefined` and the checkbox never shows.

## Workflows

**Adding a new importable data type to the dialog**
1. Add the `user_data_importer` enum and the service bit in
   `../../../../components/user_data_importer/`.
2. Detect it in `../../../importer/importer_list.cc` (set the bit only when
   the source file exists).
3. Add the `BrowserProfile` field in `import_data_browser_proxy.ts`.
4. Add the moom field in `../../../../chrome/common/importer/`.
5. Add a `settings-checkbox` in `import_data_dialog.html` bound to a new
   `prefs.import_dialog_*` key, with `no-set-pref` and a `hidden` binding.
6. Register the new pref in
   `../../../extensions/api/settings_private/prefs_util.cc`.
7. Extract all six diffs into one `features.yaml` block.

**Debugging a checkbox that never appears**
1. Confirm the source profile has the data (PP3) — the checkbox is hidden, not
   broken.
2. Confirm the `BrowserProfile` field is filled by the C++ proxy (PP6).
3. Confirm the `import_dialog_*` pref is allowlisted (PP4).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `settings/` app, routes and exports.
- [`../../../importer/AGENTS.md`](../../../importer/AGENTS.md) —
  `DetectChromeProfiles()` and the service bits.
- [`../../../extensions/api/settings_private/AGENTS.md`](../../../extensions/api/settings_private/AGENTS.md)
  — the `kImportDialog*` allowlist rows.
- [`../../../../common/importer/AGENTS.md`](../../../../common/importer/AGENTS.md)
  — the moom that carries `BrowserProfile`.
- [`../../../../../../build/features.yaml`](../../../../../../build/features.yaml) —
  patch manifest.

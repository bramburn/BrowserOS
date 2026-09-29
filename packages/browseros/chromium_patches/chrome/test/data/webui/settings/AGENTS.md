# `chrome/test/data/webui/settings/` — import dialog test data

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/test/data/webui/`) in
> [`packages/browseros/`](../../../../../../AGENTS.md).

## What's here

The only BrowserOS WebUI test in the overlay:
`import_data_dialog_test.ts`. It exercises the Chrome import dialog in
`chrome://settings/peoplePage`, whose BrowserOS additions are the
**Extensions** and **Login sessions** checkboxes.

## Contents

```
settings/
└── import_data_dialog_test.ts   ← each dialog assertion now sets
                                   cookies: false,
                                   extensions: false,
                                   (three dialog setups updated)
```

## Rules

**IS1 — The test mirrors the dialog's default-selected state.** BrowserOS
registers `prefs::kImportDialogCookies` and `prefs::kImportDialogExtensions`
defaulting to `true`, and the test explicitly clears both (`cookies: false,
extensions: false`) so the assertions are about the dialog's mechanics, not
its defaults. If you change a default in
`chrome/browser/ui/webui/settings/settings_ui.cc`, update this file.

**IS2 — Checkbox names are the pref string constants.**
`chrome/common/pref_names.h` defines
`kImportDialogExtensions = "import_dialog_extensions"` and
`kImportDialogCookies = "import_dialog_cookies"`. The TypeScript keys
(`extensions`, `cookies`) in this test are the Mojo struct field names
written by `import_data_handler.cc`; both spellings must be kept aligned.

**IS3 — This is a diff patch.** It is a `diff --git` against the Chromium
`import_data_dialog_test.ts`; a rewrite of the upstream test is not
possible here.

## Workflows

**Adding a new import data type**
1. Add the pref in `chrome/common/pref_names.h`.
2. Register it in
   `chrome/browser/ui/webui/settings/settings_ui.cc` (`RegisterProfilePrefs`).
3. Map it to a `user_data_importer::*` bit in
   `chrome/browser/ui/webui/settings/import_data_handler.cc`.
4. Add the checkbox message to `chrome/app/settings_strings.grdp`.
5. Add the field to this test's dialog setups.

**Running the test**
Run it through Chromium's WebUI TS test target
(`out/Default/chrome --test-webui-tests`) against the
`chrome://settings/peoplePage` harness. There is no BrowserOS-local runner.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/test/data/webui/`.
- [`../../../../browser/ui/webui/settings/AGENTS.md`](../../../../browser/ui/webui/settings/AGENTS.md)
  — the handler and prefs this test covers.
- [`../../../../app/AGENTS.md`](../../../../app/AGENTS.md) — the GRIT
  checkbox strings.
- [`../../../../../../AGENTS.md`](../../../../../../AGENTS.md) —
  `packages/browseros/`.

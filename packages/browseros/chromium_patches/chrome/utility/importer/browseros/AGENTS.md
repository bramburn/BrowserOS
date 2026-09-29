# `chrome/utility/importer/browseros/` — the Chrome importer

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/utility/importer/`) in
> [`packages/browseros/`](../../../../../AGENTS.md).

## What's here

The complete Google Chrome → BrowserOS import implementation, in
`namespace browseros_importer` (copyright AKW Technology Inc). One thin
orchestrator (`chrome_importer.*`) fans out to six per-data-type importers
— history, bookmarks, passwords, cookies, autofill, extensions — plus a
shared decryptor with a per-platform implementation and a Chrome-time
conversion helper.

Every file here is a `new file mode` patch: these are **new files added to
the Chromium tree**, not replacements.

## Contents

```
browseros/
├── BUILD.gn                     ← source_set("browseros"); adds
│                                   chrome_decryptor_mac.mm + Security.framework on mac,
│                                   chrome_decryptor_win.cc + crypt32.lib on win
├── chrome_importer.{cc,h}       ← orchestrator; derives chrome/utility/importer/importer.h
├── chrome_importer_utils.{cc,h} ← ChromeTimeToBaseTime (µs since Windows epoch)
├── chrome_decryptor.{cc,h}      ← interface; .cc is a Linux/ChromeOS stub that logs
├── chrome_decryptor_mac.mm      ← Keychain key retrieval, PBKDF2, AES-128-CBC
├── chrome_decryptor_win.cc      ← DPAPI key retrieval, AES-256-GCM
├── chrome_history_importer.{cc,h}    ← reads Chrome "History" SQLite
├── chrome_bookmarks_importer.{cc,h}  ← parses Chrome "Bookmarks" JSON
├── chrome_password_importer.{cc,h}  ← reads "Login Data" via the decryptor
├── chrome_cookie_importer.{cc,h}    ← reads "Cookies" via the decryptor; owns
│                                       ImportedCookieEntry
├── chrome_autofill_importer.{cc,h}  ← reads "Web Data" autofill rows
└── chrome_extensions_importer.{cc,h}← reads Chrome's Preferences/Extensions JSON
```

## Rules

**BI1 — Credential files are only read through `chrome_decryptor.*`.**
Passwords and cookies must never see a raw key. Add new credential-reading
code behind the decryptor interface, not with a direct crypto call.

**BI2 — One decryptor per platform, same header.** `chrome_decryptor.h`
exposes a fixed API; `chrome_decryptor_mac.mm`, `chrome_decryptor_win.cc`,
and the `chrome_decryptor.cc` Linux/ChromeOS stub implement it. Linux/ChromeOS
currently **log and fail** — do not present Linux credential import as
working.

**BI3 — Build-gate new platform code in this `BUILD.gn`,** under `if (is_mac)`
or `if (is_win)`, and add the matching system library
(`Security.framework` / `crypt32.lib`) in the same hunk. A source added
without its library fails only on that platform.

**BI4 — Chrome timestamps are microseconds since the Windows epoch.**
Always convert with `browseros_importer::ChromeTimeToBaseTime()` from
`chrome_importer_utils.h`; `0` means "unset" and is special-cased.

**BI5 — SQL access is read-only and via `sql::Database` + `sql::Statement`.**
Open Chrome profile files with `OpenDatabase` helpers, never copy them to a
writable location. Chrome may be running; a copy-and-open approach races.

**BI6 — `chrome_importer.cc` is an orchestrator, not a container.** It
includes each per-type importer, calls them, and reports progress with
`IDS_*` progress strings. Do not move parsing logic into it.

**BI7 — `ImportedCookieEntry` here is the typemap source for
`chrome/common/importer/profile_import.mojom`.** Changing its field set
requires a matching Mojo struct and a conversion in
`../../utility/importer/external_process_importer_bridge.cc`.

## Workflows

**Adding a new Chrome data type to import**
1. Create `chrome_<x>_importer.{cc,h}` in `namespace browseros_importer`,
   with a `…(const base::FilePath& user_data_dir, …)` entry point.
2. Add the two files to the `sources` list in this directory's `BUILD.gn`.
3. Call it from `chrome_importer.cc` and report progress.
4. Add `Set<Thing>` to `chrome/common/importer/importer_bridge.h`, the Mojo
   struct, the bridge conversion, and the gmock override.
5. Add the checkbox in
   `chrome/browser/ui/webui/settings/import_data_handler.cc` and
   `chrome/common/pref_names.h`.

**Adding credential support for a new platform**
1. Create `chrome_decryptor_<platform>.<ext>` implementing
   `chrome_decryptor.h`.
2. Gate it in this `BUILD.gn` and add the platform's crypto library.
3. Leave `chrome_decryptor.cc` as the fallback stub for platforms without
   one.
4. Update the platform support note in the `AGENTS.md` of the parent
   `chrome/utility/` tree.

**Debugging a failed import**
1. Check `chrome_importer.cc` for which sub-importers actually ran.
2. Check the platform decryptor: Linux/ChromeOS logs "not supported".
3. Check `chrome/common/importer/profile_import_process_param_traits_macros.h`
   maps `TYPE_CHROME`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/utility/importer/`.
- [`../../BUILD.gn`](../../BUILD.gn) — the `deps +=` that pulls this target
  into `static_library("utility")`.
- [`../../../common/importer/AGENTS.md`](../../../common/importer/AGENTS.md)
  — bridge / Mojo / mock contract.
- [`../../../browser/ui/webui/settings/AGENTS.md`](../../../browser/ui/webui/settings/AGENTS.md)
  — settings-page import dialog.
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) —
  `packages/browseros/`.

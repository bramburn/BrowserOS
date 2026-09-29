# `chrome/utility/importer/` — utility-process importer bridge

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/utility/`) in
> [`packages/browseros/`](../../../../AGENTS.md).

## What's here

The Chromium-side importer plumbing in the utility process: the external
process bridge that forwards results back to the browser over Mojo, the
importer factory that maps a `user_data_importer::TYPE_*` to a concrete
importer class, and a unit test that mocks the new browser-side import
bridge.

## Contents

```
importer/
├── external_process_importer_bridge.{cc,h}  ← implements SetCookie() (Mojo
│                                              ImportedCookieEntry → util types) and
│                                              SetExtensions(vector<string>)
├── importer_creator.cc                     ← case user_data_importer::TYPE_CHROME:
│                                              return new ChromeImporter();
├── bookmarks_file_importer_unittest.cc     ← MOCK_METHOD overrides for SetCookie and
│                                              SetExtensions on the importer bridge
└── browseros/                              ← the ChromeImporter implementation
```

## Rules

**UI1 — `TYPE_CHROME` maps to `browseros_importer::ChromeImporter`, not the
upstream `ChromeImporter`.** `importer_creator.cc` includes
`chrome/utility/importer/browseros/chrome_importer.h` and returns
`new ChromeImporter()`. Both classes derive from
`chrome/utility/importer/importer.h`; the browseros one adds extensions and
credential decryption.

**UI2 — `external_process_importer_bridge.cc` is the serialization
boundary.** Cookie fields are converted field-by-field, and all four
`*_utc` timestamps use
`ToDeltaSinceWindowsEpoch().InMicroseconds()`. A field added to
`chrome/common/importer/profile_import.mojom` must be converted here or it
arrives at the browser as default-valued.

**UI3 — SameSite, Priority, and SourceScheme enums are translated, not
memcpy'd.** The Mojo enums and
`browseros_importer::ImportedCookieEntry` enums are parallel but distinct
types; a direct cast is a bug when upstream reorders them.

**UI4 — The unittest's `MOCK_METHOD` list must track
`chrome/common/importer/importer_bridge.h`.** A new pure virtual without a
mock override breaks every test that constructs `MockImporterBridge`.

**UI5 — Do not add Chromium UI dependencies to this directory.** It is
utility-process code; it compiles into `static_library("utility")`.

## Workflows

**Wiring a new imported data type end to end**
1. Add the Mojo struct in `chrome/common/importer/profile_import.mojom`.
2. Add the C++ struct under
   [`browseros/`](browseros/AGENTS.md).
3. Add `virtual void Set<Thing>(...) = 0;` to
   `chrome/common/importer/importer_bridge.h`.
4. Implement the Mojo conversion in `external_process_importer_bridge.cc`.
5. Add the `MOCK_METHOD` override in
   `chrome/utility/importer/bookmarks_file_importer_unittest.cc`.
6. Expose a checkbox in
   `chrome/browser/ui/webui/settings/import_data_handler.cc`.

**Switching Chrome import to a different importer**
1. Edit the `case user_data_importer::TYPE_CHROME:` arm in
   `importer_creator.cc`.
2. Add the new target to
   `chrome/utility/BUILD.gn` deps (currently
   `//chrome/utility/importer/browseros`).

## Cross-references

- [`browseros/AGENTS.md`](browseros/AGENTS.md) — the importer implementation.
- [`../AGENTS.md`](../AGENTS.md) — `chrome/utility/`.
- [`../../common/importer/AGENTS.md`](../../common/importer/AGENTS.md) — the
  IPC contract these implement.
- [`../../browser/importer/external_process_importer_client.cc`](../../browser/importer/external_process_importer_client.cc)
  — browser-process client.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `packages/browseros/`.

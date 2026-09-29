# `chrome/common/importer/` — import IPC contract

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/common/`) in
> [`packages/browseros/`](../../../../AGENTS.md).

## What's here

The shared contract between the browser process and the utility-process
importer: the abstract bridge interface, the Mojo struct for an imported
cookie, its cross-process param traits, and a gmock mock used by browser
side unit tests. It is the only place the cookie/extension import channels
are declared.

## Contents

```
importer/
├── importer_bridge.h                    ← adds SetCookie(ImportedCookieEntry) and
│                                          SetExtensions(vector<string>) as pure virtuals
├── mock_importer_bridge.h               ← gmock subclass; includes the cookie importer header
├── profile_import.mojom                 ← struct ImportedCookieEntry { SameSite, Priority,
│                                          SourceScheme, host_key, name, value, path,
│                                          *_utc times as µs since Windows epoch, ... }
└── profile_import_process_param_traits_macros.h
                                         ← adds a BrowserOS row mapping to
                                           user_data_importer::TYPE_CHROME
```

## Rules

**IM1 — The three sides must change together.** `importer_bridge.h` declares
the virtuals, `profile_import.mojom` carries the payload across processes,
and `mock_importer_bridge.h` implements them. Adding a method to only one of
them fails to compile in the utility process or breaks every test that uses
the mock.

**IM2 — `ImportedCookieEntry` is a type-map pair, not a coincidence.**
`chrome/common/importer/profile_import.mojom` says it is "Typemapped to
`browseros_importer::ImportedCookieEntry`". The C++ struct lives in
`chrome/utility/importer/browseros/chrome_cookie_importer.h`; the field set
must stay in sync, and the Mojo type must be registered in that header's
`MOJO_TYPEMAP` (or equivalent) declaration.

**IM3 — Times cross the wire as microseconds since the Windows epoch.**
`external_process_importer_bridge.cc` converts with
`ToDeltaSinceWindowsEpoch().InMicroseconds()`. Do not change the wire unit
without changing both sides.

**IM4 — The mock must override the new methods explicitly.** Tests that use
`MockImporterBridge` fail at link/compile time if a pure virtual is
unimplemented — that is intentional. Use
`MOCK_METHOD1(SetCookie, void(const browseros_importer::ImportedCookieEntry&))`
as the existing template.

**IM5 — `profile_import_process_param_traits_macros.h` is a macro table.**
New rows must keep the same two-argument shape; this header is included by
generated code that will not tolerate a stray comma or comment.

## Workflows

**Adding a new imported data type**
1. Add the struct to `profile_import.mojom`.
2. Declare a matching C++ struct in the corresponding
   `chrome/utility/importer/browseros/chrome_*_importer.h`.
3. Add `virtual void Set<Thing>(...) = 0;` to `importer_bridge.h`.
4. Add the `MOCK_METHOD` override in `mock_importer_bridge.h`.
5. Implement the Mojo conversion in
   `chrome/utility/importer/external_process_importer_bridge.cc`.

**Wiring a new importer source type**
1. Add the `user_data_importer::TYPE_*` row to
   `profile_import_process_param_traits_macros.h`.
2. Return the importer from
   `chrome/utility/importer/importer_creator.cc`.
3. Expose the new service checkboxes in
   `chrome/browser/ui/webui/settings/import_data_handler.cc`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/common/`.
- [`../../utility/importer/AGENTS.md`](../../utility/importer/AGENTS.md) —
  the utility-process implementations of this contract.
- [`../../browser/ui/webui/settings/AGENTS.md`](../../browser/ui/webui/settings/AGENTS.md)
  — settings-page import dialog handler.
- [`../../browser/importer/`](../../browser/importer/external_process_importer_client.cc)
  — browser-process client side.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `packages/browseros/`.

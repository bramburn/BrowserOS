# `components/user_data_importer/` — Chrome as an import source

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

Two header patches that extend Chromium's importer enums for the "import from
Google Chrome" feature. A new importer type (`TYPE_CHROME`) is registered, and
the importable-item bitmask gains an `EXTENSIONS` bit (Chrome is the only
source BrowserOS imports extensions from). A `VisitSource` value records that
a history entry came from Chrome.

Feature block: **`chrome-importer`**.

## Contents

```
user_data_importer/
└── common/
    ├── importer_data_types.h   ← EXTENSIONS = 1 << 7, ALL re-derived, VISIT_SOURCE_CHROME_IMPORTED = 4
    └── importer_type.h         ← TYPE_CHROME = 7
```

The implementation of the Chrome importer lives under
`chrome/utility/importer/browseros/chrome_*.cc` and is registered in
`chrome/browser/importer/importer_list.cc` — both in `chrome/`, out of this
folder.

## Rules

**UDI1 — `ALL` must be re-derived whenever a bit is added.** The hunk changes
`ALL = (1 << 7) - 1` to `(1 << 8) - 1`. Forgetting this makes "select all"
silently skip the new category — a data-loss-shaped bug that only shows on
the summary screen.

**UDI2 — The `COOKIES` comment removal is deliberate.** Upstream carries
`COOKIES = 1 << 2,  // Not supported yet.`; BrowserOS supports cookie import
(via `chrome/utility/importer/browseros/chrome_decryptor_*.cc`), so the
comment is dropped. Don't restore it.

**UDI3 — `TYPE_EDGE` is inside `#if BUILDFLAG(IS_WIN)`; `TYPE_CHROME` is
not.** Chrome's profile layout is cross-platform, so `TYPE_CHROME` must stay
outside the buildflag guard. Moving it inside breaks mac/Linux imports.

**UDI4 — Enum values are append-only.** `TYPE_CHROME = 7` and
`VISIT_SOURCE_CHROME_IMPORTED = 4` sit at the end of their enums. Never
renumber.

**UDI5 — The two headers must change together conceptually.** `ImporterType`
and `ImportItem` are independent enums, but a new importer that adds an item
bit without an importer entry, or vice-versa, produces a half-wired feature.

## Workflows

**Adding another importable category (e.g. `PASSWORDS_V2`)**
1. Add the bit to `ImportItem` in `importer_data_types.h`.
2. Re-derive `ALL`.
3. Add a `VisitSource` entry for the new source browser.
4. Add the matching `TYPE_<BROWSER>` to `importer_type.h`.
5. Implement the reader under `chrome/utility/importer/browseros/` and register
   it in `chrome/browser/importer/importer_list.cc`.
6. Add the metrics variant to
   `chromium_patches/tools/metrics/histograms/metadata/sql/histograms.xml`.

**Adding a new source browser**
1. `TYPE_<BROWSER> = <next>` in `importer_type.h` (outside buildflag guards
   unless genuinely platform-only).
2. `VISIT_SOURCE_<BROWSER>_IMPORTED = <next>` in `importer_data_types.h`.
3. New `chrome_utility/importer/browseros/chrome_<browser>_importer.{h,cc}`.
4. `<variant name="<Browser>Importer" .../>` in the SQL histogram metadata.
5. UI entry in `chrome/browser/importer/importer_list.cc` and
   `chrome/app/settings_strings.grdp`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `components/` subtree rules.
- [`common/AGENTS.md`](common/AGENTS.md) — the leaf folder.
- [`../../tools/metrics/histograms/metadata/sql/AGENTS.md`](../../tools/metrics/histograms/metadata/sql/AGENTS.md) — the `<variant>` mirror.
- [`../../AGENTS.md`](../../AGENTS.md) — package overview.

# `components/user_data_importer/common/` — importer enums for Chrome import

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

Two header diffs — the enum declarations that make "import from Google Chrome"
expressible. `importer_type.h` adds the source browser; `importer_data_types.h`
adds what can be imported and where a visit came from.

Feature block: **`chrome-importer`**.

## Contents

```
common/
├── importer_data_types.h
│   ├── ImportItem:     + EXTENSIONS = 1 << 7, ALL re-derived to (1 << 8) - 1
│   │                   - "COOKIES = 1 << 2, // Not supported yet." comment
│   └── VisitSource:    + VISIT_SOURCE_CHROME_IMPORTED = 4
└── importer_type.h
    └── ImporterType:   + TYPE_CHROME = 7   (outside the IS_WIN guard)
```

## Rules

**UDIC1 — `ALL = (1 << 8) - 1` must be updated in lockstep with any new bit.**
This is the single highest-risk line in the folder: getting it wrong does not
crash, it silently omits data the user asked for.

**UDIC2 — Keep the trailing comment on `ALL`.** `// All the bits should be 1,
hence the -1.` explains the arithmetic and prevents a well-meaning "cleanup".

**UDIC3 — `TYPE_CHROME` is outside `#if BUILDFLAG(IS_WIN)`.** Do not move it
inside alongside `TYPE_EDGE`; the Chrome profile format is cross-platform.

**UDIC4 — These headers are consumed by the out-of-process importer bridge**
(`chrome/common/importer/profile_import.mojom` and
`profile_import_process_param_traits_macros.h`, both under `chrome/`). Changing
an enum here changes the mojom contract — the `.mojom` and its traits macros
must move in the same change.

**UDIC5 — No `.cc` files belong here.** Implementations live in
`chrome/utility/importer/browseros/`. This folder is declarations only.

## Workflows

**Adding an importable item**
1. Insert the bit before `ALL` in `importer_data_types.h`.
2. Increment the `ALL` mask.
3. Add the `VisitSource` if a new source is involved.
4. Extract both headers; keep them under `chrome-importer`.
5. Update the mojom/traits under `chrome/common/importer/` if the bit crosses
   the process boundary.

**Debugging "cookies/extensions are missing after import"**
1. Check the item bit is set in the caller's mask.
2. Check `ALL` includes it.
3. Check `chrome/utility/importer/browseros/chrome_decryptor_*.cc` can actually
   decrypt the source profile (Chrome's App-Bound Encryption on Windows is a
   known blocker on newer Chrome versions).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `components/user_data_importer/` rules.
- [`../../tools/metrics/histograms/metadata/sql/AGENTS.md`](../../../tools/metrics/histograms/metadata/sql/AGENTS.md) — the `<variant>` mirror.
- [`../../../AGENTS.md`](../../../AGENTS.md) — `components/` subtree rules.

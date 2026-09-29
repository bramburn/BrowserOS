# `series_patches/` — GNU-Quilt ordered patch sets

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`packages/browseros/`](../AGENTS.md).

## What's here

The last-resort patch surface: ordered `.patch` files applied with
`git apply -p1` against the Chromium checkout, driven by `series*` files
instead of by `features.yaml`. Currently six patches — one cross-platform
(MV2 extension support) and five Windows-only, all carried over from the
Ungoogled Chromium project.

The consumer is `build/modules/patches/series_patches.py`, registered as the
`series_patches` module. **It is not in the `--prep` phase**: `cli/build.py`
runs the module list for prep without it and logs
"⚠️ --prep does NOT apply series_patches". It must be invoked explicitly with
`browseros build -m series_patches`, and the release configs
(`build/config/release.{windows,macos,macos.arm64,macos.arm64.noupload,linux}.yaml`)
list `series_patches` in their module sequence.

## Contents

```
series_patches/
├── series               ← always applied; 1 entry
├── series.windows       ← 5 entries
├── series.linux         ← comments only (no patches)
├── series.macos         ← comments only (no patches)
├── ungoogled-chromium/
│   └── extensions-manifestv2.patch
└── windows/ungoogled-chromium/
    ├── windows-disable-rcpy.patch
    ├── windows-fix-rc.patch
    ├── windows-fix-command-ids.patch
    ├── windows-disable-download-warning-prompt.patch
    └── windows-fix-rc-terminating-error.patch
```

`get_series_files()` applies `series` first, then `series.<platform>` where
platform is `windows` / `linux` / `macos` from `get_platform()`.

## Rules

**SP1 — Adding a patch means adding a line to a `series*` file.** A `.patch`
on disk that no series file names is silently never applied —
`apply_series_patches_impl` only walks the parsed series lists.

**SP2 — Order is the whole point; do not reorder.** `series` (common) is
applied before `series.<platform>`. Within a file, lines are applied top to
bottom. Reordering changes conflict outcomes and is not caught by any test.

**SP3 — `series` and `series.<platform>` use different conventions and
both work.** The parser (`parse_series`) skips lines starting with `#`,
strips inline ` #` comments, and ignores blanks. The actual files use
full-line comments throughout; inline comments are supported but unused.

**SP4 — Platform file names must match `get_platform()` exactly.**
`windows`, `linux`, `macos` — lowercase, no `darwin`, no `win32`. A
mismatched name means the file is never read.

**SP5 — Patches are applied with `-p1 --ignore-whitespace`, then retried
with `--3way`.** A patch that needs both is a sign it should be a
`chromium_patches/` entry instead. `-p1` means the diff must carry the
`a/` `b/` prefix.

**SP6 — Prefer `chromium_patches/`.** That tree gets one commit per feature
via `browseros dev annotate`, is listed in `features.yaml`, and runs inside
`--prep`. This tree gets none of that: no per-feature commit, no manifest
entry, and a separate manual invocation. Adding a sixth patch here should
be a deliberate decision, not a default.

**SP7 — Any conflict in a series file aborts the module.** One failure
raises `RuntimeError(f"Failed to apply {len(failed)} series patches")` and
stops the build. There is no partial-continue.

**SP8 — `series.linux` and `series.macos` are empty by design.** They hold
the header comments only. Do not delete them — `get_series_files` tolerates
their absence, but keeping the placeholder documents the intended slot and
means adding a platform patch needs no new file-creation decision.

## Workflows

**Adding a cross-platform patch**
1. Put the `.patch` under a thematic subdirectory (existing: `ungoogle-chromium/`).
2. Add its path to `series`, with a comment if the ordering matters.
3. Run `browseros build -m series_patches` on a clean Chromium checkout.
4. If it conflicts, convert it to a `chromium_patches/` entry and remove the
   series line (SP6).

**Adding a Windows-only patch**
1. Put it under `windows/ungoogled-chromium/`.
2. Add the path to `series.windows` in the intended order.
3. Run `browseros build -m series_patches` on Windows. It will not run
   anywhere else (SP4).

**Debugging "No patches listed in series files"**
1. The platform-specific file is named for a different platform, or the
   common `series` parsed to zero entries (SP1/SP4).
2. Check indentation — a leading space means the line does not start with
   `#` and is not blank, so it is parsed as a patch path.

**Debugging a single-patch failure**
1. The fallback is `--3way`, so a persistent failure means the surrounding
   Chromium code moved. Regenerate the diff against the current
   `BASE_COMMIT` rather than loosening the context lines.
2. `Patch file not found: <path>` in the log means the series file lists a
   path that is not on disk.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `packages/browseros/` (F4: series
  patches are last-resort).
- [`../build/modules/patches/series_patches.py`](../build/modules/patches/series_patches.py) —
  the parser and applier.
- [`../build/cli/build.py`](../build/cli/build.py) — the `series_patches`
  module registration and the `--prep` warning.
- [`../build/config/release.windows.yaml`](../build/config/release.windows.yaml) —
  release module ordering that includes this module.
- [`ungoogle-chromium/AGENTS.md`](ungoogle-chromium/AGENTS.md) — the
  cross-platform MV2 patch.
- [`windows/ungoogled-chromium/AGENTS.md`](windows/ungoogled-chromium/AGENTS.md) —
  the Windows patch set.
- [`../../AGENTS-build.md`](../../../AGENTS-build.md) — patch-system overview.

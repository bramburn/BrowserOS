# `series_patches/windows/` — Windows platform patch slot

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`series_patches/`](../AGENTS.md).

## What's here

The Windows platform layer of the series-patch system: a single
subdirectory, `ungoogled-chromium/`, holding the five Windows-only patches
listed in [`../series.windows`](../series.windows). This folder itself
contains no patches and no config — it exists so the patches can be grouped
by platform and named after their upstream origin.

## Contents

```
windows/
└── ungoogled-chromium/     ← 5 patches, all listed in ../series.windows
```

All five patches are applied, in this order, only on Windows:
`windows-disable-rcpy` → `windows-fix-rc` → `windows-fix-command-ids` →
`windows-disable-download-warning-prompt` →
`windows-fix-rc-terminating-error`.

## Rules

**WS1 — This folder is a namespace; it is never scanned directly.**
`apply_series_patches_impl` walks only the paths listed in `series*` files.
Adding a patch here without a `series.windows` line does nothing (SP1 in
the parent).

**WS2 — `../series.windows` is the only thing that scopes this folder to
Windows.** `get_series_files()` derives the platform suffix from
`get_platform()`, so these patches never run on Linux or macOS. Nothing else
in the build filters them.

**WS3 — The common `series` file runs first.** Any future cross-platform
patch lands before these, so a Windows patch that depends on one must be
added to `series.windows` and must tolerate whatever the common patch did to
the same files.

**WS4 — Platform patches still need `-p1` and a `--3way` retry.** The
applier is identical to the common path; there is no Windows-specific
handling.

## Workflows

**Adding a Windows-only patch**
1. Put it under `ungoogled-chromium/` (or a new thematic subdirectory
   alongside it).
2. Add the path to [`../series.windows`](../series.windows) in the intended
   order.
3. Run `browseros build -m series_patches` on a Windows host.

**Reviewing the full Windows delta in one read**
1. Read [`../series.windows`](../series.windows) for the applied order.
2. Read each patch under `ungoogled-chromium/` for the change.
3. Cross-check the icon references against
   `../../resources/icons/win/AGENTS.md` — three of these patches edit
   `.rc` files that name those icons.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `series_patches/` rules.
- [`../series.windows`](../series.windows) — the ordered Windows list.
- [`ungoogled-chromium/AGENTS.md`](ungoogled-chromium/AGENTS.md) — the five
  patches in detail.
- [`../ungoogle-chromium/AGENTS.md`](../ungoogle-chromium/AGENTS.md) — the
  cross-platform patch that also runs on Windows.
- [`../../resources/icons/win/AGENTS.md`](../../resources/icons/win/AGENTS.md) —
  icons referenced by the `.rc` patches.

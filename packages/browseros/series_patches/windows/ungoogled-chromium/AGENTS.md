# `series_patches/windows/ungoogled-chromium/` — Windows Ungoogled patches

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`series_patches/windows/`](../AGENTS.md).

## What's here

Five Windows-only patches inherited from Ungoogled Chromium, applied in the
order listed in [`../../series.windows`](../../series.windows) and only on
Windows. Together they make the Windows toolchain build without Google's
resources and strip the Chrome-only surfaces that a Chromium-branded build
would otherwise try to compile.

## Contents

| Patch (applied in this order) | Files touched | Effect |
|---|---|---|
| `windows-disable-rcpy.patch` | `build/toolchain/win/tool_wrapper.py`, `chrome/app/chrome_dll.rc`, `chrome/app/chrome_exe.rc`, `chrome/installer/setup/setup.rc` | replaces `rc.py` with MSVC `rc.exe`; forces `#if 0` on every `BUILDFLAG(GOOGLE_CHROME_BRANDING)` / `GOOGLE_CHROME_FOR_TESTING_BRANDING` branch so Chromium icons and `chromium_b06a12530me7r` identity are used; `#if BUILDFLAG(ENABLE_PRINTING)` → `#if 1` |
| `windows-fix-rc.patch` | `chrome/browser/web_applications/chrome_pwa_launcher/chrome_pwa_launcher_exe.rc` | forces the non-Google PWA launcher icon |
| `windows-fix-command-ids.patch` | `chrome/app/chrome_command_ids.h` | every `BUILDFLAG(IS_CHROMEOS)` / `IS_LINUX` / `GOOGLE_CHROME_BRANDING` guard becomes `#if (0)` — MSVC's resource compiler truncates macro names at 31 characters |
| `windows-disable-download-warning-prompt.patch` | `components/download/internal/common/download_item_impl.cc` | `SetDangerType` writes `DOWNLOAD_DANGER_TYPE_NOT_DANGEROUS` unconditionally, disabling the "this file may be dangerous" prompt |
| `windows-fix-rc-terminating-error.patch` | `build/build_config.h` | guard becomes `#if !defined(BUILD_BUILD_CONFIG_H_) && !defined(RC_INVOKED)` so the C preprocessor does not run inside `rc.exe` |

## Rules

**WU1 — `windows-disable-rcpy` must be applied first.** The other four
patches are hand-edited workarounds for MSVC `rc.exe` semantics; they only
make sense once `rc.py` has been replaced by `rc.exe`. Reordering the
`series.windows` lines breaks that chain.

**WU2 — `windows-fix-rc-terminating-error` and `windows-disable-rcpy` are a
pair.** One makes `rc.exe` parse headers, the other stops it. Neither is
useful alone; do not remove one "because it looks redundant".

**WU3 — These patches duplicate `features.yaml` entries.** The `windows-patches`
feature block in `../../build/features.yaml` already lists
`build/build_config.h`, `build/toolchain/win/tool_wrapper.py`,
`chrome/app/chrome_dll.rc`, `chrome/app/chrome_exe.rc`, and
`chrome/browser/web_applications/chrome_pwa_launcher/chrome_pwa_launcher_exe.rc`
— the same files these series patches edit. Both surfaces can write the same
Chromium file. Keep them consistent, and prefer the `features.yaml` route
for anything that does not genuinely need the series ordering (SP6 in the
parent).

**WU4 — `#if 0` substitutions are load-bearing, not cosmetic.** Every
`BUILDFLAG(GOOGLE_CHROME_BRANDING)` replaced with `#if 0` forces the
`#else` Chromium branch. Restoring the original `BUILDFLAG` text breaks the
resource compile, because `rc.exe` truncates the macro name at 31 chars
(that is the entire reason `windows-fix-command-ids` exists).

**WU5 — `windows-disable-download-warning-prompt` is a security
behaviour change.** It makes the browser treat every download as safe
regardless of the scanner's verdict. It is inherited from Ungoogled
Chromium, not a BrowserOS decision; do not extend it to other download
paths without an explicit decision.

**WU6 — These run only when the module is invoked.** `series_patches` is
not part of `--prep`; a build that forgets `browseros build -m series_patches`
compiles the *unpatched* Windows resources. Check the phase-2 log for the
"does NOT apply series_patches" warning.

## Workflows

**Re-basing the Windows set after a Chromium bump**
1. Check out the new tag in `<chromium_src>`.
2. Run `browseros build -m series_patches` and read the per-patch result
   lines.
3. Regenerate each failing patch against the new tree, keeping the same
   filename and the same `series.windows` order.
4. Re-sync the overlapping entries in
   [`../../build/features.yaml`](../../../build/features.yaml) (WU3).

**Adding a new Windows-only change**
1. Prefer a `chromium_patches/` entry under the `windows-patches` feature —
   it gets a commit and runs in `--prep`.
2. Use a series patch only if it depends on the `.rc.exe` chain above.
3. If a series patch is unavoidable, add it to `../../series.windows` in the
   right position and document the dependency here.

**Debugging a resource-compile failure in the ninja phase**
1. Confirm `series_patches` actually ran in this build (WU6).
2. Look for a `BUILDFLAG` guard that was not converted to `#if 0` (WU4).
3. Check `build/build_config.h` for a missing `RC_INVOKED` exclusion
   (WU2).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `series_patches/windows/` slot rules.
- [`../../AGENTS.md`](../../AGENTS.md) — `series_patches/` rules (SP5, SP6).
- [`../../series.windows`](../../series.windows) — the applied order.
- [`../../../build/modules/patches/series_patches.py`](../../../build/modules/patches/series_patches.py) —
  the applier.
- [`../../../build/features.yaml`](../../../build/features.yaml) — the
  overlapping `windows-patches` feature block.
- [`../../../resources/icons/win/AGENTS.md`](../../../resources/icons/win/AGENTS.md) —
  the `.ico` files these `.rc` patches point at.

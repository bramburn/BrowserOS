# `build/modules/compile/` — `autoninja` and the macOS universal pipeline

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`packages/browseros/build/modules/`](../AGENTS.md).

## What's here

Two compile strategies. `standard.py` is the ordinary path: write
`chrome/VERSION`, run `autoninja -C <out_dir> chrome chromedriver`,
move the produced app bundle to the canonical BrowserOS name, and
register `built_app`. `universal.py` is the macOS arm64+x64 path: it
runs the *entire* pipeline — resources, configure, compile, sign,
package, upload — once per architecture, then merges the two apps,
then signs, packages and uploads the universal result.

## Contents

| File | What it does |
|---|---|
| `standard.py` | `class CompileModule(CommandModule)` — `produces = ["built_app"]`, `requires = []`, `description = "Build BrowserOS using autoninja"`. Plus `build_target(ctx, target)` for one-off targets such as `mini_installer`. |
| `universal.py` | `class UniversalBuildModule(CommandModule)` — `produces = ["dmg_arm64", "dmg_x64", "dmg_universal"]`. `UNIVERSAL_ARCHITECTURES = ["arm64", "x64"]`. Methods `_clean_build_directories`, `_create_arch_context`, `_create_universal_context`, `_merge_universal`. |
| `__init__.py` | Re-exports `CompileModule`, `UniversalBuildModule`, `build_target`. |

## Rules

**CMP1 — `autoninja` runs with `cwd=ctx.chromium_src` and a relative
`-C ctx.out_dir`.** Both matter: `out_dir` is relative to the Chromium
root, and the cwd must be the Chromium root for `depot_tools` discovery.

**CMP2 — `autoninja.bat` on Windows, `autoninja` elsewhere.** Use the
`IS_WINDOWS()` branch; do not hardcode.

**CMP3 — `CompileModule` renames the app bundle to
`ctx.get_app_path()` after the build.** Downstream modules resolve
`BrowserOS.app` / `chrome.exe` / `BrowserOS` by that name via
`ctx.BROWSEROS_APP_NAME`. If the rename fails silently the packaging
modules will look in the wrong place.

**CMP4 — `universal_build` owns sign, package and upload for
universal releases.** `config/release.macos.yaml` lists *only*
`universal_build` after the patch phase — do not also list
`sign_macos` / `package_macos` / `upload` in that config, or the app
gets signed and packaged twice.

**CMP5 — Per-architecture contexts must set `ctx._fixed_app_path`.**
Without it, `get_app_path()` auto-detects the universal directory and
the x64 build resolves to the arm64 app. `_create_arch_context()`
exists solely for this.

**CMP6 — Upload failures inside `universal_build` are non-fatal and
logged as warnings.** This is the one place in the tree where an
exception is caught and downgraded. Keep the comment explaining it if
you touch that block.

**CMP7 — `validate()` checks `args.gn` exists before compiling.** A
missing `configure` step surfaces here as "Build not configured - args.gn
not found", not as a mysterious autoninja failure.

## Workflows

**Running a normal single-arch build**
1. `browseros build -m configure,compile` (or a full config recipe).
2. `CompileModule` writes `chrome/VERSION` from
   `ctx.browseros_chromium_version` (four-part only — a different shape
   logs a warning and skips).
3. autoninja produces `out/<dir>/chrome` + `chromedriver`; the bundle is
   moved to `out/<dir>/BrowserOS{,.app,.exe}` and registered as
   `built_app`.

**Running the macOS universal release**
1. `browseros build --config build/config/release.macos.yaml --chromium-src <src>`.
2. `validate()` requires macOS, the presence of
   `../package/universalizer_patched.py`, and a configured signing
   environment (`MACOS_CERTIFICATE_NAME` + notarization credentials).
3. Output: `BrowserOS_{version}_{arm64,x64,universal}_signed.dmg`.

**Building a one-off ninja target**
1. Call `build_target(ctx, "mini_installer")` from Python, or use the
   `mini_installer` path in `../sign/windows.py::build_mini_installer()`.
2. Do not add it to a pipeline — targets are not modules.

**Debugging a failed universal build**
1. The three output directories are `out/Default_arm64`,
   `out/Default_x64`, `out/Default_universal`; all are wiped at the
   start of the run.
2. Check the per-arch log section for resources / configure / compile
   before blaming the merge step.
3. Merge failures come from `../package/merge.py::merge_architectures()`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — all 14 module areas.
- [`../setup/AGENTS.md`](../setup/AGENTS.md) — the `configure` step that writes `args.gn`.
- [`../package/AGENTS.md`](../package/AGENTS.md) — `merge.py` and `universalizer_patched.py`, invoked from here.
- [`../sign/AGENTS.md`](../sign/AGENTS.md) — `MacOSSignModule`, invoked internally by `universal_build`.
- [`../config/AGENTS.md`](../../config/AGENTS.md) — the GN args autoninja reads.

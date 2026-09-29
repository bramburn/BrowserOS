# `build/modules/package/` — platform packagers and universal merge

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`packages/browseros/build/modules/`](../AGENTS.md).

## What's here

Turns a compiled `<out_dir>` into a distributable: a DMG on macOS, an
installer plus a portable ZIP on Windows, an AppImage plus a `.deb` on
Linux. `merge.py` and `universalizer_patched.py` handle the macOS
universal-binary case, invoked by `../compile/universal.py`.

The area has no `__init__.py` — it is a PEP 420 namespace package.

## Contents

| File | What it does |
|---|---|
| `macos.py` | `class MacOSPackageModule` — `produces = ["dmg"]`. `create_dmg()`, `sign_dmg()`, `notarize_dmg()`, `create_signed_notarized_dmg()`, `package_universal()`. Uses Chromium's `pkg-dmg` from `ctx.get_pkg_dmg_path()`. |
| `windows.py` | `class WindowsPackageModule` — `produces = ["installer", "installer_zip"]`. `_create_installer()`, `_create_portable_zip()`, plus standalone `build_mini_installer()`, `create_installer()`, `create_portable_zip()`, `get_target_cpu()`, `create_files_cfg_package()` (unimplemented stub). |
| `linux.py` | `class LinuxPackageModule` — `produces = ["appimage", "deb"]`. `LINUX_ARCHITECTURE_CONFIG`, `LINUX_HOST_APPIMAGETOOL`, `get_linux_architecture_config()`, `get_host_appimagetool()`, `copy_browser_files()`, `create_desktop_file()`, `copy_icon()`, `prepare_appdir()`, `download_appimagetool()`, `create_appimage()`, `create_launcher_script()`, `create_control_file()`, `create_postinst_script()`, `create_prerm_script()`, `create_apparmor_profile()`, `create_metainfo_file()`. |
| `merge.py` | `merge_architectures()`, `create_minimal_context()`, `merge_sign_package()`, `handle_merge_command()`. |
| `universalizer_patched.py` | Standalone universal-binary builder (vendored + patched upstream script). `universalize()`, `_merge_info_plists()`, `_universalize()`, `_is_macho_file()`, `_get_architectures()`, `main()`. Run as a subprocess with `sys.executable`. |
| `linux_test.py` | Unit tests for `LINUX_ARCHITECTURE_CONFIG` and `get_host_appimagetool()`. |

## Rules

**PKG1 — Every packager registers its artifacts with
`ctx.artifact_registry`.** `package_windows` does
`ctx.artifact_registry.add("installer", ...)`. The `upload` module
reads from the registry, so skipping this means nothing gets uploaded.

**PKG2 — `get_linux_architecture_config()` and
`get_host_appimagetool()` are keyed on different things — do not
conflate them.** The first is keyed on `ctx.architecture` (the *target*
arch: `appimage_arch`, `deb_arch`). The second is keyed on
`get_platform_arch()` (the *host* arch), because `appimagetool` must run
locally. Cross-compiling arm64 from an x64 host still needs the x86_64
tool. This distinction is the most common source of broken Linux
cross-builds.

**PKG3 — Both Linux artifacts are attempted even if one fails.**
`execute()` raises only if *both* AppImage and `.deb` fail; a single
success logs a warning. Preserve that partial-success behaviour.

**PKG4 — `chrome_sandbox` gets SUID (`0o4755`) in the AppImage and
`0o4755` via `postinst` in the .deb; `0o755` elsewhere.** The AppArmor
profile in `create_apparmor_profile()` is not optional on Ubuntu 23.10+:
without it the Chromium sandbox cannot create user namespaces and the
browser fatals on launch (GitHub issue #165, cited in the docstring).

**PKG5 — `merge.py` shells out to `universalizer_patched.py` with
`sys.executable`.** Do not replace it with an in-process import; the
script is also invoked standalone and validated by path in
`../compile/universal.py::validate()`.

**PKG6 — `create_files_cfg_package()` in `windows.py` is a stub** that
logs "FILES.cfg packaging not yet implemented" and returns `False`. It
is not wired to any module. Don't advertise it as a supported path.

**PKG7 — Windows packaging depends on `mini_installer.exe` existing
in `<out_dir>`.** It is built by `../sign/windows.py::build_mini_installer()`
or via `build_target(ctx, "mini_installer")`. `validate()` fails with
that exact message if it is missing.

## Workflows

**Packaging macOS**
1. `browseros build -m package_macos` after `sign_macos`.
2. `MacOSPackageModule` locates Chromium's `pkg-dmg` via
   `ctx.get_pkg_dmg_path()`; a missing `pkg-dmg` means the
   `--chromium-src` path is wrong.
3. Output: `releases/<version>/BrowserOS_<version>_macos.dmg`.

**Packaging Windows**
1. `browseros build -m package_windows` (or `sign.windows.yaml`, which
   runs both).
2. Produces `<version>.exe` and a ZIP containing that same installer.
3. Both register as `installer` / `installer_zip`.

**Packaging Linux**
1. `browseros build -m package_linux`.
2. `download_appimagetool()` fetches the *host* arch tool into
   `build/tools/` on first run.
3. The `.deb` carries `postinst` / `prerm` scripts that set the SUID
   bit, load/unload the AppArmor profile, and register
   `update-alternatives` for `x-www-browser` and `gnome-www-browser`.

**Producing a universal macOS binary**
1. Use `browseros build -m universal_build` rather than driving
   `merge.py` directly — the module handles context setup, signing,
   packaging and upload around the merge.
2. `handle_merge_command()` exists for the standalone merge-only CLI
   path.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — all 14 module areas.
- [`../compile/AGENTS.md`](../compile/AGENTS.md) — `universal_build`, which drives the merge.
- [`../sign/AGENTS.md`](../sign/AGENTS.md) — the signing step that must precede packaging.
- [`../storage/AGENTS.md`](../storage/AGENTS.md) — consumes the registry entries written here.
- [`../../common/context.py`](../../common/context.py) — `get_dist_dir()`, `get_artifact_name()`, `get_pkg_dmg_path()`.

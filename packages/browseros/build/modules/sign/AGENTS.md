# `build/modules/sign/` — code signing (macOS, Windows, Linux, Sparkle)

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`packages/browseros/build/modules/`](../AGENTS.md).

## What's here

Four signing modules. `macos.py` is the substantial one: codesign
every nested component with the right identifier and entitlements,
sign the app bundle, notarize with `xcrun notarytool`, and staple.
`windows.py` signs with SSL.com CodeSignTool in three steps, and
rebuilds `mini_installer.exe` between signing the executables and
signing the installer. `linux.py` is an intentional no-op.
`sparkle.py` signs finished DMGs with the Sparkle Ed25519 key for
auto-update.

Which third-party binaries get signed is declared once, in
`../../common/server_binaries.py`, and consumed by both this area and
`../ota/`.

The area has no `__init__.py` — PEP 420 namespace package.

## Contents

| File | What it does |
|---|---|
| `macos.py` | `class MacOSSignModule` — `produces = ["signed_app"]`, `requires = ["built_app"]`. `get_browseros_server_binary_info()`, `unlock_keychain()`, `check_signing_environment()`, `check_environment()`, `find_components_to_sign()`, `get_identifier_for_component()`, `get_signing_options()`, `sign_component()`, `sign_all_components()`, `verify_signature()`, `notarize_app()`, `sign_app()`, `sign_universal()`. |
| `windows.py` | `class WindowsSignModule` — `produces = ["signed_installer"]`, `requires = ["built_app"]`. `get_browseros_server_binary_paths()`, `build_mini_installer()`, `sign_with_codesigntool()`. |
| `linux.py` | `class LinuxSignModule` — `produces = []`, `requires = []`. No-op. Plus `sign_universal()` and `check_signing_environment()` returning `True`. |
| `sparkle.py` | `class SparkleSignModule` — `produces = ["sparkle_signatures"]`. `sign_dmgs_with_sparkle()`. Delegates to `../../common/sparkle.py::sparkle_sign_file()`. |

## Rules

**SG1 — Per-binary signing metadata lives in
`../../common/server_binaries.py`, not here.** `MACOS_SERVER_BINARIES`
maps binary stem → `SignSpec(identifier_suffix, options, entitlements)`;
`WINDOWS_SERVER_BINARIES` is a relative-path list. Adding a bundled
binary means editing that file, which updates both this area and
`../ota/`.

**SG2 — `LinuxSignModule` is a deliberate no-op, and must stay
registered.** `config/release.linux.yaml` omits `sign_linux` entirely,
but the class exists so `--list` and the platform helpers
(`_get_sign_module()` in `../../cli/build.py`) resolve on every OS. Its
`validate()` is `pass` and `execute()` only logs.

**SG3 — All three platform modules are imported unconditionally.**
Platform gating lives in `validate()` via `IS_MACOS()` / `IS_WINDOWS()`.
Do not convert these to conditional imports.

**SG4 — Windows signing is three ordered steps and step 2 is not
optional.** `sign chrome.exe` + server binaries → **build
`mini_installer.exe`** → sign `mini_installer.exe`. The installer must
be built after the executables are signed or it will bundle unsigned
ones.

**SG5 — `unlock_keychain()` is called before macOS signing** and needs
keychain access. This is why codesign fails with "User interaction not
allowed" under a `launchd`-started CI runner — see
`../../docs/nightly-macos-ci.md`.

**SG6 — Verify after signing, before notarizing.**
`verify_signature()` exists; a bad signature costs a full notarization
round trip.

**SG7 — Sparkle signing scans `ctx.get_dist_dir()` for `*.dmg` and
signs every one it finds.** It also stores the result in
`ctx.artifacts["sparkle_signatures"]` for the upload module, in
addition to per-file registry entries. Keep both.

**SG8 — Required credentials come from `ctx.env` only**
(`MACOS_CERTIFICATE_NAME`, `PROD_MACOS_NOTARIZATION_*`,
`CODE_SIGN_TOOL_PATH`, `ESIGNER_USERNAME`, `ESIGNER_PASSWORD`,
`ESIGNER_TOTP_SECRET`, `SPARKLE_PRIVATE_KEY`). `validate()` fails fast
with a named list of what is missing; do not defer that to execute time.

## Workflows

**Signing a macOS release**
1. `browseros build -m sign_macos` (or let `universal_build` call it
   internally).
2. `sign_all_components()` walks the bundle deepest-first, using
   `find_components_to_sign()` to identify nested binaries and
   `get_identifier_for_component()` to derive each bundle id.
3. `notarize_app()` submits, polls, and staples.
4. `ctx.artifact_registry.add("signed_app", ...)`.

**Signing on Windows**
1. `browseros build --config build/config/sign.windows.yaml` against an
   already-built `out/Default`.
2. Requires the four `ESIGNER_*` / `CODE_SIGN_TOOL_PATH` variables.
3. Output: signed `mini_installer.exe`, registered as `signed_installer`.

**Signing DMGs for auto-update**
1. `browseros build -m package_macos,sparkle_sign`.
2. Requires `SPARKLE_PRIVATE_KEY`.
3. Signatures are registered for the `upload` module.

**Debugging a codesign failure in CI**
1. Check the runner has keychain access (not `launchd`, not SSH-only).
2. Check `MACOS_CERTIFICATE_NAME` matches an identity in the keychain.
3. Check `unlock_keychain()` ran — it uses the keychain password from
   `ctx.env`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — all 14 module areas.
- [`../../common/server_binaries.py`](../../common/server_binaries.py) — the shared sign metadata.
- [`../package/AGENTS.md`](../package/AGENTS.md) — packaging, which follows signing.
- [`../compile/AGENTS.md`](../compile/AGENTS.md) — `universal_build`, which calls this area internally.
- [`../../docs/AGENTS.md`](../../docs/AGENTS.md) — the macOS CI keychain constraints.

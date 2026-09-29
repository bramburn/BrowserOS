# `chrome/install_static/` — Windows install identity

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/`) in
> [`packages/browseros/`](../../../AGENTS.md).

## What's here

One patch: `chromium_install_modes.h`, the compile-time struct that decides
what BrowserOS installs as on Windows — directory name, registry keys,
ProgIDs, URL scheme, and the CLSIDs Windows integration needs.

## Contents

```
install_static/
└── chromium_install_modes.h   ← ChromiumInstallMode struct fields:
                                 kProductPathName L"BrowserOS"
                                 app GUID {CF887152-7CB8-4393-84CC-1BACF0EDE1D1}
                                 base_app_name / base_app_id "BrowserOS"
                                 browser_prog_id_prefix "BOSHTML"
                                 direct_launch_url_scheme "browseros"
                                 pdf_prog_id_prefix "BOSPDF"
                                 toast_activator_clsid, elevator_clsid
```

## Rules

**IS1 — The browser ProgID prefix is `BOSHTML`, not `CHTML`.** Anything that
registers or looks up `.html` associations on Windows depends on this
string. Changing it orphans existing installs.

**IS2 — The URL scheme is `browseros://`, matching
`kBrowserProcessExecutableName` in `../common/chrome_constants.cc`.** These
two must stay in lockstep or the installed browser cannot be launched by
its own scheme.

**IS3 — Custom CLSIDs are deliberate.** `toast_activator_clsid`
({0xE76CCE76,…}) and `elevator_clsid` ({0x29ED629C,…}) replace upstream
Chrome's so that toast notifications and elevation target BrowserOS, not a
co-installed Chrome. Reusing an upstream GUID causes cross-product
interference on the same machine.

**IS4 — Do not add fields without checking `mini_installer/`.** The
`chrome.release` file in `../installer/mini_installer/` packages the
BrowserOS server directory alongside the browser; both are driven from this
identity block plus the Python packager in
`packages/browseros/build/modules/package/windows.py`.

**IS5 — Linux and macOS are unaffected here.** `install_static` is
Windows-only by definition; platform identity for the other two lives in
`../common/chrome_paths*.cc` and `../app/app-Info.plist`.

## Workflows

**Changing the installed product name**
1. Update `kProductPathName`, `base_app_name`, and `base_app_id` together.
2. Grep the Python packager for the same string — a mismatch produces an
   install directory that no version can update.
3. Re-run packaging and check the resulting `%LOCALAPPDATA%` path.

**Adding a new shell integration (file association, protocol, toast)**
1. Add the `InstallMode` field here.
2. Set it on every mode in the same file (there is more than one).
3. Verify the GUID is BrowserOS-specific, not a copied Chrome GUID.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/` root overlay.
- [`../common/AGENTS.md`](../common/AGENTS.md) — executable name and paths
  that must match this identity block.
- [`../app/AGENTS.md`](../app/AGENTS.md) — macOS bundle identity.
- [`../installer/mini_installer/AGENTS.md`](../installer/mini_installer/AGENTS.md)
  — the manifest that ships the server directory.
- [`../../../AGENTS.md`](../../../AGENTS.md) — `packages/browseros/`.

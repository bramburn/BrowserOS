# `chrome/installer/mini_installer/` — `chrome.release` manifest

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/installer/`) in
> [`packages/browseros/`](../../../../AGENTS.md).

## What's here

A single patch file: `chrome.release`, the Windows mini-installer manifest.
It lists the files and directories that must be copied into the versioned
install directory alongside the browser binary. BrowserOS uses it to ship
the bundled MCP server tree.

## Contents

```
mini_installer/
└── chrome.release   ← appends two wildcard rules under a "BrowserOS Server"
                        comment:
                        BrowserOSServer\default\resources\*.*
                        BrowserOSServer\default\resources\bin\*.*
                        each mapping to %(VersionDir)s\BrowserOSServer\...
```

## Rules

**MI1 — Source and destination share the `%(VersionDir)s` prefix.** Both
sides of each rule are version-directory relative. A destination that
omits `%(VersionDir)s` writes outside the versioned folder and the updater
will not see the files.

**MI2 — Use `\*.*` wildcards, not individual filenames.** The server tree
changes shape between releases (new binaries, new resources). Enumerating
files here means a silent omission on the next version.

**MI3 — The installer's resource set is also produced by the Python
packager.** `packages/browseros/build/modules/package/windows.py` and
`build/config/copy_resources.yaml` must list the same tree, or the build
produces a binary that installs without its server.

**MI4 — Keep BrowserOS additions at the end of the file, under a comment
header.** The patch is a context diff; inserting mid-file increases the
chance of a context mismatch on the next Chromium rebase.

## Workflows

**Adding a runtime payload to the Windows install**
1. Append a `dir\*.*: %(VersionDir)s\dir\` rule to `chrome.release`.
2. Add the matching copy operation to
   `packages/browseros/build/config/copy_resources.yaml` (with `os:` /
   `arch:` filters).
3. Confirm the directory is populated in `<chromium_src>/out/Default/chrome-win/`.

**Verifying a fresh install**
1. Install to a clean `%LOCALAPPDATA%` profile.
2. Check `<app>/<version>/BrowserOSServer/default/resources/bin/` exists and
   contains the server binary.
3. Check the first-launch registry entries match
   [`../../install_static/chromium_install_modes.h`](../../install_static/chromium_install_modes.h).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/installer/`.
- [`../../install_static/AGENTS.md`](../../install_static/AGENTS.md) — Windows
  product identity this manifest ships alongside.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `packages/browseros/`.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — repo root
  (installation UX is roadmap priority #3).

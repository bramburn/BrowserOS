# `chrome/installer/` — installer overlay root

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/`) in
> [`packages/browseros/`](../../../AGENTS.md).

## What's here

A container directory. There are **no patch files directly in
`chrome/installer/`** — all installer content lives in the single
subdirectory:

- [`mini_installer/`](mini_installer/AGENTS.md) — `chrome.release`, the
  file manifest that tells the mini installer which extra directories to
  copy next to the browser.

Installer *branding* is not here. The Windows product identity lives in
[`../install_static/AGENTS.md`](../install_static/AGENTS.md); the Python
packagers live in `packages/browseros/build/modules/package/`.

## Rules

**IN1 — Do not add files to this directory.** The mini installer manifest
is the only installer file Chromium applies from this tree; anything else
belongs in `mini_installer/` or in the Python packaging module.

**IN2 — Installer changes are only half the change.** The companion half is
`packages/browseros/build/modules/package/windows.py` (or `macos.py` /
`linux.py`) plus `packages/browseros/config/copy_resources.yaml`. A
`chrome.release` entry that no packager knows about is ignored at package
time even though it is correct in the source tree.

**IN3 — Cross-check the strategic roadmap before editing installer files.**
The repo root `AGENTS.md` ranks "Installation UX / UI" as priority #3 and
gates MCP/agent work behind #1–#4. Installer patches are high-risk
rollback candidates.

## Workflows

**Shipping a new runtime next to the browser**
1. Add the directory + wildcard to
   [`mini_installer/chrome.release`](mini_installer/AGENTS.md).
2. Make the Python packager copy the same tree.
3. Verify the installed layout under
   `%LOCALAPPDATA%/<vendor>/<app>/<version>/`.

**Debugging a missing file in an install**
1. Check `chrome.release` for the wildcard rule.
2. Check `copy_resources.yaml` for the `os:`/`arch:` filter.
3. Only then suspect the patch overlay.

## Cross-references

- [`mini_installer/AGENTS.md`](mini_installer/AGENTS.md) — the only child.
- [`../AGENTS.md`](../AGENTS.md) — `chrome/` root overlay.
- [`../install_static/AGENTS.md`](../install_static/AGENTS.md) — Windows
  product identity.
- [`../../../AGENTS.md`](../../../AGENTS.md) — `packages/browseros/`.
- [`../../../AGENTS.md`](../../../AGENTS.md) — repo root, strategic
  roadmap and installer-UX priority.

# `browseros/bundled_extensions/` — CRX build payload

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

Two files, both patches that create new files in the Chromium tree. `BUILD.gn`
is the whole payload: it copies three `.crx` files plus `bundled_extensions.json`
into the build output so BrowserOS can install them on first run before it ever
touches the network. `.gitignore` ignores `*.crx` and `*.json`, so the actual
binaries are **not** in this repository — they are fetched during the build and
land in the checkout, not here.

The three extensions are the Agent
(`bflpfmnmnokmjhmgnolecpppdbdophmk.crx`), the Bug Reporter
(`adlpneommgkgeanpaekgoaolcpncohkf.crx`) and the Controller
(`nlnihljpboknmfagkikhkdblbedophja.crx`).

## Contents

```
bundled_extensions/
├── BUILD.gn       ← _bundled_extensions_sources (4 paths, CRX named by
│                    extension ID) + a platform split:
│                      !is_mac  → copy() into
│                                 $root_out_dir/browseros_extensions/
│                      is_mac   → bundle_data() into
│                                 <framework>/Resources/browseros_extensions/
└── .gitignore     ← *.crx, *.json
```

## Rules

**BE1 — The CRX filename is the extension ID.** The three names in
`_bundled_extensions_sources` are the IDs that
`../core/browseros_constants.h:kAgentExtensionId` /
`kBugReporterExtensionId` / `kControllerExtensionId` also declare. Renaming one
without the other produces an extension that the browser protects
(`IsBrowserOSExtension`) but never installs.

**BE2 — The two platform branches must stay symmetric.**
On non-mac the payload goes to `$root_out_dir/browseros_extensions/`; on mac it
goes to `{{bundle_contents_dir}}/Resources/browseros_extensions/`. The
`BrowserOSExtensionInstaller` looks for the directory next to the executable
(`server_utils::GetExecutionDir()` in `../server/browseros_server_utils.cc`),
so changing one branch without the other breaks macOS installs only.

**BE3 — Do not commit a `.crx` or `bundled_extensions.json` here.** The
`.gitignore` in this very directory excludes both patterns. A committed CRX
would be a binary blob in a patch overlay and would break
`browseros dev extract` (it diffs text).

**BE4 — `bundled_extensions.json` is consumed, not defined, here.** It is
listed in `sources` and copied, but its schema is owned by
`../extensions/browseros_extension_installer.cc`; the file itself is
produced by the build. Check the installer before changing its shape.

**BE5 — This directory has no `BUILD.gn` deps on BrowserOS C++.** It is pure
data staging. Wiring it to code would create a build-order cycle with
`//chrome/browser/browseros:browseros_bundled_extensions`.

## Workflows

**Adding a fourth bundled extension**
1. Add the CRX to `_bundled_extensions_sources` in `BUILD.gn`, named
   `<extension-id>.crx`.
2. Add its ID to `kBrowserOSExtensions[]` in `../core/browseros_constants.h`
   and set pinned / labelled / contextual-side-panel flags there.
3. Add it to the build step that fetches `bundled_extensions.json`.
4. Ensure the extension's `manifest.json` has an `update_url` so the
   maintainer's `ForceUpdateCheck()` can replace it later.

**Debugging a missing extension on macOS**
1. Confirm the framework bundle has
   `Contents/Resources/browseros_extensions/<id>.crx` (BE2).
2. Check `--disable-browseros-extensions` is not set (declared in
   `../core/browseros_switches.h`).
3. Check `chrome://browseros-extensions` logging from
   `../extensions/browseros_extension_installer.cc`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `browseros/` root overlay.
- [`../extensions/AGENTS.md`](../extensions/AGENTS.md) — the installer that
  reads this payload.
- [`../core/AGENTS.md`](../core/AGENTS.md) — extension ID table.
- [`../server/AGENTS.md`](../server/AGENTS.md) — `server_utils::GetExecutionDir()`,
  the anchor for the output path.
- [`../AGENTS.md`](../AGENTS.md) — the `browseros_bundled_extensions` group.

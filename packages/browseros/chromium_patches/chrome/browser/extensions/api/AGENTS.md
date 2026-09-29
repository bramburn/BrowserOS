# `extensions/api/` — API subpackages

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

A directory with no files of its own. It exists only to group the API
subpackages that the fork touches, and its four subdirectories are compiled by
the parent target `//chrome/browser/extensions:extensions` — the entries in
`../BUILD.gn` are relative (`"api/browser_os/browser_os_api.cc"`, …), not
absolute, so there is nothing to list here.

## Contents

```
api/
├── browser_os/       ← new browserOS.* extension API (4 file pairs)
├── debugger/         ← debugger_api.cc
├── settings_private/ ← prefs_util.cc
└── side_panel/       ← sidePanel.browseros* functions
```

## Rules

**API0 — This directory holds no sources.** `browser_os/`, `debugger/`,
`settings_private/` and `side_panel/` are the only children, and each of their
files is listed individually in `../BUILD.gn`. A new subpackage needs a
`BUILD.gn` entry there, not a local build file.

**API1 — Do not add a fifth subpackage without checking
`chrome/common/extensions/api/`.** The schema JSON that generates the
`api_registration` table lives under `chrome/common/extensions/api/`, not here.
This tree only holds the hand-written C++ side.

**API2 — A subpackage here must be reachable from an extension.** Every
function needs a `DECLARE_EXTENSION_FUNCTION` and a schema entry, or it is dead
code that still compiles.

## Workflows

**Adding a new API subpackage**
1. Create `api/<name>/` and put the `.cc`/`.h` there.
2. Add each path to the `sources` list of `source_set("extensions")` in
   `../BUILD.gn` using a path relative to this directory.
3. Add the schema in `chrome/common/extensions/api/` so the generated registry
   can see it.
4. Extract the diffs and add the paths to the matching `features.yaml` block.

**Finding where an existing API function is wired**
1. Grep the `sources` list in `../BUILD.gn` for the subdirectory name.
2. For `browser_os`, the hand-registered entry point is
   `../chrome_extensions_browser_api_provider.cc`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `extensions/` overlay and `BUILD.gn`.
- [`browser_os/AGENTS.md`](browser_os/AGENTS.md) — the `browserOS.*` API.
- [`debugger/AGENTS.md`](debugger/AGENTS.md) — debugger infobar suppression.
- [`settings_private/AGENTS.md`](settings_private/AGENTS.md) — the pref allowlist.
- [`side_panel/AGENTS.md`](side_panel/AGENTS.md) — `sidePanel.browseros*`.
- [`../../../../../build/features.yaml`](../../../../../build/features.yaml) —
  patch manifest.

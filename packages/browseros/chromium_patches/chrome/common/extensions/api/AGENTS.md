# `chrome/common/extensions/api/` — `browserOS` + `sidePanel` WebExtension APIs

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/common/extensions/`) in
> [`packages/browseros/`](../../../../../AGENTS.md).

## What's here

The declarative half of the BrowserOS extension surface: two WebExtension
IDL files, the two Chromium feature-JSON files that gate them, and the GN
list that makes GN actually compile them. This directory is the contract
that `chrome/browser/extensions/api/browser_os/` implements.

## Contents

```
api/
├── browser_os.idl              ← namespace browserOS: choosePath, accessibility
│                                  tree, interactive snapshot, page structure, click,
│                                  inputText, clear, and more
├── side_panel.idl              ← adds BrowserosToggleOptions / BrowserosIsOpenOptions
│                                  + browserosToggle() / browserosIsOpen() to sidePanel
├── _api_features.json          ← per-method feature entries, all
│                                  contexts: ["privileged_extension"]
├── _permission_features.json   ← "browserOS": channel stable, extension+platform_app
└── api_sources.gni             ← adds "browser_os.idl" to the API sources list
```

## Rules

**AX1 — Every method needs an `_api_features.json` entry.** A method in the
`.idl` with no feature entry will not compile. Each entry must name
`"dependencies": ["permission:browserOS"]` and
`"contexts": ["privileged_extension"]`.

**AX2 — Register new `.idl` files in `api_sources.gni`.** The list is
alphabetical inside the `enable_extensions` block. An unregistered IDL is
silently not generated — you get a missing-mojom link error, not a
compile error in the IDL.

**AX3 — The permission is `browserOS` (capital OS), the permission *ID* is
`kBrowserOS`.** `_permission_features.json` uses the string `"browserOS"`;
`chrome/common/extensions/permissions/chrome_api_permissions.cc` maps
`APIPermissionID::kBrowserOS` to the same string. Case must match exactly.

**AX4 — `side_panel.idl` additions are additive dictionaries.** The BrowserOS
types (`BrowserosToggleOptions`, `BrowserosToggleResult`,
`BrowserosIsOpenOptions`, `BrowserosToggleCallback`,
`BrowserosIsOpenCallback`) were appended to the existing `sidePanel`
namespace. Do not reorder or rename upstream entries — a reordered
WebIDL changes generated bindings for every extension.

**AX5 — Keep the `browserOS` namespace self-contained.** The implementation
lives in `namespace browserOS` in
`chrome/browser/extensions/api/browser_os/browser_os_api.cc`; IDL and C++
names must match one-for-one.

## Workflows

**Adding a `browserOS` method**
1. Add the `callback`/return signature to `browser_os.idl`.
2. Add the method key to `_api_features.json` with
   `permission:browserOS` + `privileged_extension`.
3. Implement + register in
   `chrome/browser/extensions/api/browser_os/browser_os_api.cc`.
4. Add the new source to `chrome/browser/extensions/BUILD.gn`.

**Adding a side-panel toggle variant**
1. Add the options/result dictionary and `callback` to `side_panel.idl`.
2. Add the `[nodisc] static void` declaration inside the `sidePanel` namespace.
3. Implement in `chrome/browser/extensions/api/side_panel/side_panel_api.cc`
   (its `side-panel-fixes` patch).

**Changing who can call a BrowserOS API**
1. Edit `_api_features.json` `contexts` for that method.
2. Confirm `chrome_api_permissions.cc` still registers the permission.
3. Re-check the `settings_overridden_params_providers.cc` skip-list in
   `chrome/browser/ui/extensions/`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/common/extensions/`.
- [`../permissions/AGENTS.md`](../permissions/AGENTS.md) — permission map.
- [`../../../browser/extensions/api/browser_os/browser_os_api.h`](../../../browser/extensions/api/browser_os/browser_os_api.h)
  — C++ side of `browser_os.idl`.
- [`../../../browser/extensions/api/side_panel/side_panel_api.cc`](../../../browser/extensions/api/side_panel/side_panel_api.cc)
  — C++ side of `side_panel.idl`.
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) — `packages/browseros/`.

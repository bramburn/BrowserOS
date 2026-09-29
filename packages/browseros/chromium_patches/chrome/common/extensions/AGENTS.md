# `chrome/common/extensions/` — extension API surface

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/common/`) in
> [`packages/browseros/`](../../../../AGENTS.md).

## What's here

A pure container directory. There are **no patch files directly in
`chrome/common/extensions/`** — it holds two subdirectories that carry all
the content:

- [`api/`](api/AGENTS.md) — the `browserOS` and `sidePanel` WebExtension API
  definitions (`browser_os.idl`, `side_panel.idl`), their permission
  feature JSON, and the `api_sources.gni` registration.
- [`permissions/`](permissions/AGENTS.md) — the `browserOS` permission entry
  in `chrome_api_permissions.cc`.

## Rules

**EX1 — An `.idl` here is useless on its own.** Four files must change
together: the `.idl` (`api/`), the `_api_features.json` entry
(`api/`), the `_permission_features.json` entry (`api/`), and the
`APIPermissionID` mapping (`permissions/`). A feature flag that lists only
some of them produces a build error in `api/` or a silently missing method.

**EX2 — Every API is `contexts: ["privileged_extension"]`.** BrowserOS
APIs are not exposed to ordinary web extensions. Do not add a broader
context.

**EX3 — The `.idl` must be registered in `api_sources.gni`.** Unregistered
IDL is not compiled; the failure appears as a missing generated
`browser_os.mojom`.

## Workflows

**Adding a method to the `browserOS` API**
1. Declare it in [`api/browser_os.idl`](api/AGENTS.md) with a `callback` type
   if it is async.
2. Add a `_api_features.json` entry naming the method and its
   `permission:browserOS` dependency.
3. Implement in
   [`../../browser/extensions/api/browser_os/`](../../browser/extensions/api/browser_os/browser_os_api.cc)
   and add the sources to `../../browser/extensions/BUILD.gn`.

**Adding a new permission name**
1. Add it to `api/_permission_features.json` with an explicit `channel`.
2. Map it in [`permissions/chrome_api_permissions.cc`](permissions/AGENTS.md)
   against the new `APIPermissionID` from
   `extensions/common/mojom/api_permission_id.mojom`.

## Cross-references

- [`api/AGENTS.md`](api/AGENTS.md) — IDL + feature JSON.
- [`permissions/AGENTS.md`](permissions/AGENTS.md) — permission registration.
- [`../AGENTS.md`](../AGENTS.md) — `chrome/common/`.
- [`../../browser/extensions/api/browser_os/browser_os_api.h`](../../browser/extensions/api/browser_os/browser_os_api.h)
  — C++ implementation side.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `packages/browseros/`.

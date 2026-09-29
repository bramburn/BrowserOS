# `extensions/common/mojom/` — `kBrowserOS` permission id

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A single-line addition to Chromium's `APIPermissionID` mojom enum:

```
kGlicPrivate = 265,
kBrowserOS = 266,      // ← added
```

`266` is the id backing the `browserOs` extension API, so the BrowserOS
controller extension is gated by a first-class permission rather than an ad-hoc
check.

Feature block: **`api`**.

## Contents

```
mojom/
└── api_permission_id.mojom   ← @@ -290,6 +290,7 @@ : + kBrowserOS = 266
```

The enum is declared immediately before the "Add new entries at the end of the
enum and be sure to update the `ExtensionPermission3` enum in ..." comment.

## Rules

**EXM1 — Append only.** The enum is mirrored in two other places, both of which
must move in the same change:
- `tools/metrics/histograms/metadata/extensions/enums.xml` —
  `<int value="266" label="kBrowserOS"/>` in `ExtensionPermission3`.
- `chrome/common/extensions/permissions/chrome_api_permissions.cc` — the
  human-readable permission name (under `chrome/`).

**EXM2 — The upstream comment above the enum is a hard instruction.** "Add new
entries at the end of the enum and be sure to update the `ExtensionPermission3`
enum" is enforced by
`tools/metrics/histograms/update_extension_permissions.py`.

**EXM3 — Mojom enum values are ABI.** They are serialised between the browser
and extension processes. Never renumber, never reorder.

**EXM4 — Adding an id here is not enough to ship a permission.** The
`browserOs` API surface also needs an IDL entry in
`chrome/common/extensions/api/browser_os.idl`, registration in
`api_sources.gni`, and feature entries in `_api_features.json` /
`_permission_features.json`. All under `chrome/`.

**EXM5 — Value 266 is the current high-water mark.** The next BrowserOS
permission is `267`. Check upstream before assuming 266 is still free.

## Workflows

**Adding a new extension permission**
1. Append `kFoo = <next>` to `APIPermissionID` here.
2. Add `<int value="<next>" label="kFoo"/>` to
   `chromium_patches/tools/metrics/histograms/metadata/extensions/enums.xml`.
3. Add the display name to `chrome_api_permissions.cc`.
4. Declare the API in the relevant `.idl` and register it in `api_sources.gni`.
5. Add the permission to `_permission_features.json` if it should be
   warning-only.
6. Register every path under the `api` feature in `features.yaml`.

**Verifying the permission is enforced**
1. Load the extension without the permission → the API must be undefined.
2. Grant it → `chrome.runtime.lastError` reports an unknown permission until
   the enum and the permission name agree.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `extensions/common/` rules.
- [`../../AGENTS.md`](../../AGENTS.md) — `extensions/` subtree rules.
- [`../../../tools/metrics/histograms/metadata/extensions/AGENTS.md`](../../../tools/metrics/histograms/metadata/extensions/AGENTS.md) — the `ExtensionPermission3` mirror.
- [`../../../build/features.yaml`](../../../../build/features.yaml) — the `api` block.

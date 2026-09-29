# `chrome/common/extensions/permissions/` — permission name map

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/common/extensions/`) in
> [`packages/browseros/`](../../../../../AGENTS.md).

## What's here

One patch file. `chrome_api_permissions.cc` gains a single table entry
mapping the Chromium `APIPermissionID` enum to its WebExtension permission
string.

## Contents

```
permissions/
└── chrome_api_permissions.cc   ← adds {APIPermissionID::kBrowserOS, "browserOS"}
```

## Rules

**PM1 — The entry string must equal the `_permission_features.json` key.**
`"browserOS"` here, `"browserOS"` in
[`../api/_permission_features.json`](../api/_permission_features.json). A mismatch compiles
cleanly and then denies every call at runtime.

**PM2 — `APIPermissionID::kBrowserOS` must exist upstream-of-this-patch.**
The enum lives in `extensions/common/mojom/api_permission_id.mojom`, which
is patched under the `api` feature in `build/features.yaml` — outside this
directory. If you add an ID here, that mojom patch is part of the same
change.

**PM3 — One line per permission; no helper logic.** The table is a flat
`std::pair` list consumed by Chromium's permission machinery. Do not add
conditional logic or comments inside the initializer list.

## Workflows

**Adding a new BrowserOS permission**
1. Add the enumerator to `extensions/common/mojom/api_permission_id.mojom`.
2. Add the string to `../api/_permission_features.json` with a `channel`.
3. Add the `{APIPermissionID::kX, "x"}` row here.
4. Reference it from `_api_features.json` `dependencies`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/common/extensions/`.
- [`../api/AGENTS.md`](../api/AGENTS.md) — IDL and feature JSON.
- [`../../importer/AGENTS.md`](../../importer/AGENTS.md) — sibling
  `chrome/common/` subtree.
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) — `packages/browseros/`.

# `content/public/browser/` — `GetTargetTabId` hook

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

The public half of BrowserOS's tab-identity feature. A new `virtual` on
`DevToolsManagerDelegate` lets an embedder report which browser tab and window
a `WebContents` belongs to; the default implementation declines, so
non-Chrome embedders are unaffected.

```
devtools_manager_delegate.h   + virtual bool GetTargetTabId(WebContents*, int* tab_id, int* window_id);
devtools_manager_delegate.cc  + bool DevToolsManagerDelegate::GetTargetTabId(...) { return false; }
```

Feature block: **`cdp-api`**.

## Contents

```
browser/
├── devtools_manager_delegate.h    ← +8 lines (doc comment + virtual)
└── devtools_manager_delegate.cc   ← +6 lines (out-of-line default)
```

The Chrome override is `chrome/browser/devtools/chrome_devtools_manager_delegate.{h,cc}`.

## Rules

**CPBB1 — Return value is the contract; out-params are only valid on `true`.**
The caller in `content/browser/devtools/protocol/target_handler.cc` reads
`tab_id`/`window_id` only after a `true` return. Document any new out-param the
same way.

**CPBB2 — The default must return `false`, not a sentinel id.** Returning `0`
would make every non-Chrome embedder report tab 0 for every target.

**CPBB3 — `window_id` is optional by design.** A tab can exist without a known
window. The caller guards with `window_id >= 0`; do not change that to a
truthiness check (window `0` is valid).

**CPBB4 — The doc comment is the API contract for all embedders.** It states
that embedders supporting tab identity should override to populate
`tabId`/`windowId` in `TargetInfo`, and that `false` means "no tab identity".
Keep it accurate.

**CPBB5 — No `#include` churn.** This header is included widely in
`content/public/browser`; a new include costs build time across the tree.

## Workflows

**A CDP client reports missing `tabId`**
1. Confirm the Chrome delegate overrides `GetTargetTabId` and returns `true`
   for real tabs.
2. Confirm the `WebContents` maps to a `TabStripModel` tab that is not a
   prerender / no state.
3. Confirm `domains/Target.pdl` declares the fields `experimental optional`.
4. Confirm the build regenerated `protocol.target.pdl` bindings.

**Changing the signature**
1. Update the header virtual.
2. Update the `.cc` default.
3. Update `BuildTargetInfo` in
   `content/browser/devtools/protocol/target_handler.cc`.
4. Update the Chrome override.
5. Re-extract all four files.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `content/public/` rules.
- [`../../AGENTS.md`](../../AGENTS.md) — `content/` subtree rules.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — package overview.
- [`../../../../third_party/blink/public/devtools_protocol/domains/AGENTS.md`](../../../third_party/blink/public/devtools_protocol/domains/AGENTS.md) — the `Target.pdl` field definitions.

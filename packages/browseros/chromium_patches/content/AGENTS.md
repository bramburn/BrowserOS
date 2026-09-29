# `content/` — CDP tab identity plumbing

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../chromium_patches/AGENTS.md`](../../chromium_patches/AGENTS.md).

## What's here

Three patch files in Chromium's `content/` layer, all serving one goal: let a
DevTools target carry the browser tab and window it belongs to. This is the
foundation of the CDP surface the BrowserOS agent drives — `Target.getTargets`
returns a `tabId`/`windowId` so the agent can map a CDP target to a user-visible
tab.

Two of the three are `content/public/browser` (the embedder-facing delegate
API); the third is `content/browser/devtools` (the protocol handler that calls
it).

Features: `cdp-api` for the two `content/public/browser` files, `cdp-fixes` for
`content/browser/devtools/protocol/target_handler.cc`.

## Contents

```
content/
├── browser/
│   └── devtools/
│       └── protocol/
│           └── target_handler.cc   ← populates TargetInfo.tabId/windowId; silences attach/detach events
└── public/
    └── browser/
        ├── devtools_manager_delegate.h   ← + virtual bool GetTargetTabId(WebContents*, int*, int*)
        └── devtools_manager_delegate.cc   ← default impl returns false
```

The Chrome-side implementation of the new virtual lives at
`chrome/browser/devtools/chrome_devtools_manager_delegate.{h,cc}` (under
`chrome/`, not this folder).

## Rules

**CN1 — Header and default implementation are one change.** A new `virtual` in
`content/public/browser/devtools_manager_delegate.h` without the matching
definition in the `.cc` is a link error in every embedder that instantiates the
delegate.

**CN2 — Out-of-line default returning `false` is the contract.** The base
implementation in `.cc` returns `false` so non-Chrome embedders are unaffected.
Do not move it inline to the header.

**CN3 — The two `int*` out-params are a deliberate choice.** `tab_id` and
`window_id` are pointers so the delegate can leave `window_id` unset; the
caller checks `window_id >= 0` before setting it. Don't change the signature
to a struct or `std::optional` without updating `target_handler.cc` too.

**CN4 — `Target.pdl` must declare `tabId`/`windowId` before this compiles.**
`target_handler.cc` calls `target_info->SetTabId(...)`; the generated setters
come from
`third_party/blink/public/devtools_protocol/domains/Target.pdl`. Protocol
definition and handler change together — see
[`third_party/blink/AGENTS.md`](../third_party/blink/AGENTS.md).

**CN5 — The commented-out `TargetInfoChanged` calls are intentional.**
`DevToolsAgentHostAttached` / `DevToolsAgentHostDetached` no longer emit
`TargetInfoChanged`, suppressing target churn for the agent. Re-enabling them
will flood any CDP client with spurious events. Leave the comments in place so
the next reader knows it was a decision.

**CN6 — Do not add `content/` patches for agent features.** `content/` is the
embedder-agnostic layer; BrowserOS-specific behaviour belongs in `chrome/`.
This folder exists only because the tab-identity hook genuinely has to live at
the `content` boundary.

## Workflows

**Adding a new field to `TargetInfo`**
1. Add the optional property to
   `third_party/blink/public/devtools_protocol/domains/Target.pdl`.
2. Add a `virtual` on `DevToolsManagerDelegate` in
   `content/public/browser/devtools_manager_delegate.h`.
3. Add the out-of-line default in the `.cc`.
4. Populate it in `BuildTargetInfo` in
   `content/browser/devtools/protocol/target_handler.cc`.
5. Override it in
   `chrome/browser/devtools/chrome_devtools_manager_delegate.{h,cc}`.
6. Register all paths under `cdp-api` in `features.yaml`.

**Debugging "tabId is absent from Target.getTargets"**
1. Confirm the `Target.pdl` field is `experimental optional` — a required field
   breaks every non-tab target (workers, browser).
2. Confirm `BuildTargetInfo` only sets it when `host->GetWebContents()` is
   non-null (worker targets have none).
3. Confirm the Chrome delegate override returns `true`.
4. Confirm the session's `Target.setDiscoverTargets` is on.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — overlay conventions.
- [`../third_party/blink/AGENTS.md`](../third_party/blink/AGENTS.md) — the `.pdl` protocol definitions.
- [`../../AGENTS.md`](../../AGENTS.md) — package overview.
- [`../../build/features.yaml`](../../build/features.yaml) — the `cdp-api` and `cdp-fixes` blocks.

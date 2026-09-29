# `content/browser/devtools/protocol/` — target tab identity

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

One file, two changes, both about letting a CDP client map a target to a
browser tab.

1. In `BuildTargetInfo`, ask the `DevToolsManagerDelegate` for the target's
   `tabId`/`windowId` and set them on the `TargetInfo` when the delegate
   supplies them.
2. `DevToolsAgentHostAttached` and `DevToolsAgentHostDetached` no longer call
   `TargetInfoChanged(host)` — the calls are commented out, suppressing target
   churn for BrowserOS's CDP client.

Feature block: **`cdp-fixes`**.

## Contents

```
protocol/
└── target_handler.cc
    ├── BuildTargetInfo()             +13 lines: delegate->GetTargetTabId(...)
    ├── DevToolsAgentHostAttached()   TargetInfoChanged(host) → commented out
    └── DevToolsAgentHostDetached()   TargetInfoChanged(host) → commented out
```

## Rules

**CTP1 — The comment form is deliberate; do not tidy it.** The two
`// TargetInfoChanged(host);` lines are the record of a behavioural change.
Reformatting them to `#if 0` or deleting them loses that.

**CTP2 — `int tab_id, window_id;` are intentionally uninitialised.** The
delegate contract is "returns false ⇒ do not read the out-params", so the
callers only read them after a `true` return. Initialising them to 0 would be
harmless but changes the diff shape against upstream.

**CTP3 — `window_id >= 0` is a real guard.** A target can have a tab identity
but no containing window. Setting a negative `window_id` would surface a
garbage value over the wire.

**CTP4 — This depends on the new `Target.pdl` fields.** `SetTabId` /
`SetWindowId` are generated from
`third_party/blink/public/devtools_protocol/domains/Target.pdl`. Protocol and
handler must ship together.

**CTP5 — This file is high-drift.** `target_handler.cc` changes frequently
upstream (MPArch, prerender, service-worker target work). On a version bump,
re-extract rather than hand-resolving hunk offsets.

**CTP6 — Don't silence more events here.** Disabling further
`TargetInfoChanged` calls would break legitimate clients. If a CDP client
misbehaves, fix the client (`packages/browseros-agent/apps/server/src/browser/backends/cdp.ts`).

## Workflows

**Adding another identity field to `TargetInfo`**
1. Add the `experimental optional` property to `domains/Target.pdl`.
2. Extend the `GetTargetTabId` signature in
   `content/public/browser/devtools_manager_delegate.{h,cc}`.
3. Populate it in `BuildTargetInfo` here.
4. Override it in `chrome/browser/devtools/chrome_devtools_manager_delegate.{h,cc}`.
5. Register all paths under `cdp-api` / `cdp-fixes`.

**Verifying tab identity end to end**
1. Start the browser with a remote-debugging port.
2. `curl http://127.0.0.1:<port>/json/list` — page targets should now carry a
   `tabId`.
3. Confirm worker/service-worker targets have no `tabId` (they have no
   `WebContents`).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `content/browser/devtools/` rules.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — package overview.
- [`../../../../third_party/blink/public/devtools_protocol/domains/AGENTS.md`](../../../../third_party/blink/public/devtools_protocol/domains/AGENTS.md) — the `Target.pdl` definition.

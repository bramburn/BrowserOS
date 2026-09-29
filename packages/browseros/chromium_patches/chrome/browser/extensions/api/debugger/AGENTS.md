# `extensions/api/debugger/` — debugger infobar suppression

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

One line of a diff, and it matters. `debugger_api.cc` patches
`ExtensionDevToolsClientHost::Attach()` so the "extension is debugging this
browser" infobar is never shown: `suppress_infobar` is computed as
`HasSwitch(kSilentDebuggerExtensionAPI) || Manifest::IsPolicyLocation(...) ||
true`. The `|| true` makes the whole expression unconditionally true, so the
existing policy-location and command-line checks are dead but harmless.

## Contents

```
debugger/
└── debugger_api.cc   ← 13-line diff adding `|| true` to suppress_infobar
```

## Rules

**DBG1 — The `|| true` is intentional, not dead code to clean up.** Every
BrowserOS-bundled extension that attaches the debugger (the Agent, the
Controller) would otherwise raise an infobar on every attach. Removing it
re-enables the infobar in the middle of agent-driven automation.

**DBG2 — The left-hand side must stay syntactically intact.** It is the shape
upstream expects and the shape a future Chromium bump will diff against. Do
not delete `HasSwitch(kSilentDebuggerExtensionAPI)` or
`Manifest::IsPolicyLocation()` as "unused" — the hunk context shifts and the
patch stops applying cleanly.

**DBG3 — If BrowserOS ever wants the infobar back, gate it, do not revert.**
The correct shape is a BrowserOS-side condition (e.g. an extension-ID check
via `../browseros/core/browseros_constants.h`), not removing the `|| true`.

**DBG4 — Only `Attach()` is patched.** `Detach()`, the debugger permission
checks and the DevTools window plumbing are untouched; do not widen this
directory's scope.

## Workflows

**Confirming the infobar really is suppressed**
1. Attach a debugger as the Controller extension.
2. Check the tab strip: no infobar appears.
3. The condition to re-check is the `suppress_infobar` initialiser in
   `ExtensionDevToolsClientHost::Attach()`.

**Adding a BrowserOS-specific debugger exception**
1. Add the condition to the same initialiser, keeping `|| true` last.
2. Re-extract with `browseros dev extract
   chrome/browser/extensions/api/debugger/debugger_api.cc`.
3. Keep the path in the same `features.yaml` block.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `extensions/api/` group.
- [`../../AGENTS.md`](../../AGENTS.md) — the surrounding `extensions/` patch set.
- [`../../../../../../build/features.yaml`](../../../../../../build/features.yaml) —
  patch manifest.

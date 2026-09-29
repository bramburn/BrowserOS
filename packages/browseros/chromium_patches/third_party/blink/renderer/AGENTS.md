# `third_party/blink/renderer/` — Blink runtime patches

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

The Blink *runtime* half of the overlay, as opposed to
`../public/devtools_protocol/` which holds the CDP schema. Exactly one renderer
file is patched: `navigator.cc`, hard-coding `navigator.webdriver` to `false`
so pages cannot detect CDP automation.

Feature block: **`misc`**.

## Contents

```
renderer/
└── core/
    └── frame/
        └── navigator.cc   ← Navigator::webdriver() { return false; }
```

## Rules

**BLR1 — The Blink renderer is the highest-churn code in Chromium.** Blink CLs
land daily; `navigator.cc` in particular is touched by privacy, spec, and
fingerprinting work. Expect this patch to conflict on every version bump and
re-extract rather than hand-fix.

**BLR2 — One renderer patch is the budget, not a coincidence.** Every additional
Blink runtime patch multiplies rebase cost across the whole Chromium upgrade
cycle. New Blink-level behaviour should go in `chrome/` unless it genuinely
cannot.

**BLR3 — `navigator.webdriver` is a browser-fingerprinting surface.** Changing
it changes what every site observes. It is a deliberate product decision for the
agent product, not an optimisation.

## Workflows

**Rebasing `navigator.cc` after a Chromium bump**
1. `git checkout <BASE_COMMIT> -- third_party/blink/renderer/core/frame/navigator.cc`
   inside `<chromium_src>`.
2. Find the current `Navigator::webdriver()` implementation.
3. Replace the body with `return false;`.
4. `browseros dev extract third_party/blink/renderer/core/frame/navigator.cc`.
5. Confirm `probe::ApplyAutomationOverride` is no longer called (upstream may
   have inlined or renamed it).

**Evaluating a request to add a Blink runtime patch**
1. Can it live in `chrome/browser/` (an embedder hook) instead?
2. If not, estimate the rebase cost and record it in the feature's
   `description:` in `features.yaml`.
3. Add a `// BrowserOS:` comment at the change site so future rebases can
   identify the hunk quickly.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `third_party/blink/` rules.
- [`core/AGENTS.md`](core/AGENTS.md) — next level down.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — package overview.

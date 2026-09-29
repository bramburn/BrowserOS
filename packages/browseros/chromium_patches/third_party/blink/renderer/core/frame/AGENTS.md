# `third_party/blink/renderer/core/frame/` — hide the webdriver flag

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

One file, one hunk. `Navigator::webdriver()` in Blink's frame code returns a
bare `false`, replacing Chromium's two-source detection:

```cpp
bool Navigator::webdriver() const {
  return false;
}
```

Upstream checks `RuntimeEnabledFeatures::AutomationControlledEnabled()` and then
`probe::ApplyAutomationOverride(GetExecutionContext(), automation_enabled)`.
Both are removed.

Feature block: **`misc`**.

## Contents

```
frame/
└── navigator.cc   ← @@ -98,12 +98,7 @@ : webdriver() body replaced
```

## Rules

**FRM1 — Do not restore either removed call.** Reinstating
`AutomationControlledEnabled()` or `probe::ApplyAutomationOverride` re-enables
the `navigator.webdriver === true` signal that every bot-detection library
checks. The BrowserOS agent drives a real browser over CDP and needs this to
stay false.

**FRM2 — The comment above the function ("// If true then the browser is
controlled by automation") is upstream and stays.** Only the body changed.

**FRM3 — Expect a conflict on every Chromium bump.** `navigator.cc` is a
frequent Blink privacy CL target. Reset to `BASE_COMMIT` and re-apply; never
adjust hunk offsets by hand.

**FRM4 — This is one half of agent anti-detection; the other half is
client-side.** Remaining detection vectors (missing `window.chrome`, timing,
`Runtime.enable` artefacts) are handled in
`packages/browseros-agent/apps/server/src/browser/backends/cdp.ts`, not here.

**FRM5 — Add a `// BrowserOS:` comment if you ever touch adjacent code in this
file.** The current hunk has no marker, which makes it harder to spot during a
rebase. Add one for anything new.

## Workflows

**Rebasing after a Chromium version bump**
1. In `<chromium_src>`: `git checkout <BASE_COMMIT> -- third_party/blink/renderer/core/frame/navigator.cc`
2. Locate the current `Navigator::webdriver()` body.
3. Replace the entire body with `return false;`.
4. `browseros dev extract third_party/blink/renderer/core/frame/navigator.cc`
5. Verify the signature hasn't gained a parameter — a new argument would mean
   the base-class contract changed and the override must change with it.

**Verifying at runtime**
- In the built browser's devtools console on any page:
  `navigator.webdriver` → must be `false`.

**A site still detects the agent**
1. Confirm `navigator.webdriver` is `false` (this patch).
2. Check for other signals — `window.chrome`, `Notification.permission`,
   `navigator.plugins.length`, iframe `contentWindow` quirks.
3. Those are CDP client behaviours, not Blink patches. Do not add more Blink
   patches to fix them (BLR2).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `blink/renderer/core/` rules.
- [`../../AGENTS.md`](../../AGENTS.md) — `blink/renderer/` rules.
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) — package overview.
- [`../../../../../build/features.yaml`](../../../../../../build/features.yaml) — the `misc` block.

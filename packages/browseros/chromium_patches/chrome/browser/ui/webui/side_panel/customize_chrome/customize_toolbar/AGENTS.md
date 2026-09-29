# `.../customize_chrome/customize_toolbar/` — customize-toolbar action mapping

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/webui/side_panel/customize_chrome/`)
> in [`packages/browseros/`](../../../../../../../../AGENTS.md).

## What's here

The mapping between Chromium's `actions::ActionId` and the mojom
`ActionId` used by the customize-your-side-panel UI, plus the list of
actions BrowserOS contributes. Three files: the mojom, the handler, and the
handler's unit test.

## Contents

```
customize_toolbar/
├── customize_toolbar.mojom                 ← adds ActionId values
│                                             kShowThirdPartyLlm,
│                                             kShowClashOfGpts
├── customize_toolbar_handler.cc            ← three additions:
│     • case kActionSidePanelShowThirdPartyLlm: → mojom::ActionId::kShowThirdPartyLlm
│     • case kActionSidePanelShowClashOfGpts:  → mojom::ActionId::kShowClashOfGpts
│       (and the two reverse cases)
│     • add_action(kActionSidePanelShowThirdPartyLlm,
│                   CategoryId::kYourChrome);
│       add_action(kActionSidePanelShowClashOfGpts, CategoryId::kYourChrome);
└── customize_toolbar_handler_unittest.cc   ← asserts
      prefs::kPinSplitTabButton is true in a fresh profile
```

## Rules

**CT1 — The mapping must be bidirectional.** The handler needs both
`ActionId → mojom::ActionId` and `mojom::ActionId → ActionId` cases. A
one-way mapping lets the action appear in the list but not be applied when
the user toggles it.

**CT2 — BrowserOS actions are categorized `kYourChrome`.** Not a new
category, not `kChromeLabs`. A new BrowserOS action should follow the same
assignment unless there is a product reason not to.

**CT3 — The mojom enum is append-only in practice.**
`kShowThirdPartyLlm` and `kShowClashOfGpts` were appended after the upstream
values. Inserting in the middle renumbers the generated enum and can
invalidate a stored selection.

**CT4 — The unit test also covers a pref default outside this feature.**
`customize_toolbar_handler_unittest.cc` asserts
`prefs::kPinSplitTabButton` is true, which is set in
`chrome/browser/ui/browser_ui_prefs.cc` (`browser_ui_prefs.cc` also
registers it default-true). Both are `chromium-ui-fixes`/`vertical-tabs`
territory; keep the assertion when changing that default.

**CT5 — The mojom needs regeneration after any enum change.** This is done
by Chromium's build, not by hand; a stale generated header produces a
mismatch between `customize_toolbar_handler.cc` and the enum.

## Workflows

**Adding a BrowserOS action to the customize panel**
1. Append the value to `ActionId` in `customize_toolbar.mojom`.
2. Add both `case` arms in `customize_toolbar_handler.cc`.
3. `add_action(..., CategoryId::kYourChrome)`.
4. Add the `E(...)` row in
   `chrome/browser/ui/actions/chrome_action_id.h`.
5. Add the `V(...)` row in
   `chrome/browser/ui/side_panel/side_panel_entry_id.h`.
6. Add the action to the default pinned list in
   `chrome/browser/ui/toolbar/toolbar_pref_names.cc` (behind its feature).
7. Extend `customize_toolbar_handler_unittest.cc` if the mapping is
   non-trivial.

**Debugging "my action is missing from the customize panel"**
1. Is it in the mojom enum?
2. Are both `case` arms present?
3. Was it passed to `add_action(...)`?
4. Is the action ID in `chrome_action_id.h` at all?

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `customize_chrome/`.
- [`../../../../../../../../AGENTS.md`](../../../../../../../../AGENTS.md) — `webui/side_panel/`.
- [`../../../../actions/AGENTS.md`](../../../../actions/AGENTS.md) — action IDs.
- [`../../../../side_panel/AGENTS.md`](../../../../side_panel/AGENTS.md) —
  entry IDs and panel prefs.
- [`../../../../../../../../AGENTS.md`](../../../../../../../../AGENTS.md) —
  `packages/browseros/`.

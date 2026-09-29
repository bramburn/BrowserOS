# `chrome/browser/ui/views/toolbar/` — pinned action buttons and labels

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/views/`) in
> [`packages/browseros/`](../../../../../../AGENTS.md).

## What's here

The view side of the pinned toolbar: the button class that can render a
text label next to its icon, and the container that owns the pinned /
popped-out buttons, calls `EnsureAlwaysPinnedActions()`, and refreshes
labels when the visibility pref changes.

## Contents

```
toolbar/
├── BUILD.gn                          ← adds
│     "//chrome/browser/browseros/core:action_utils"
│     "//chrome/browser/browseros/core:prefs"
│     to source_set("impl") deps
├── pinned_action_toolbar_button.{cc,h} ← label rendering for BrowserOS actions;
│                                       UpdateLabelVisibility(); GetBrowser()
└── pinned_toolbar_actions_container.{cc,h}
                                     ← calls model_->EnsureAlwaysPinnedActions() at
│                                       construction; UpdateAllLabels();
│                                       OnLabelsVisibilityChanged() override
```

## Rules

**VTB1 — Labels come from the container's `ActionItem`, not the button.**
`PinnedActionToolbarButton` asks
`container_->GetActionItemFor(action_id)` for the text and only uses it
when `browseros::IsBrowserOSAction(action_id)`. Non-BrowserOS actions keep
icon-only rendering.

**VTB2 — Label visibility is a profile pref, not a per-window setting.**
`browseros::ShouldShowToolbarLabels(profile->GetPrefs())` is read through
`GetBrowser()`. The button null-checks `browser_ && browser_->profile()`
because the button can exist before the Browser is attached.

**VTB3 — `EnsureAlwaysPinnedActions()` is called once, from the
container's constructor.** This is the model's contract
(`chrome/browser/ui/toolbar/pinned_toolbar/`). Do not call it from a button
or from `UpdateAllLabels()`.

**VTB4 — `UpdateAllLabels()` must walk both lists.** It iterates
`pinned_buttons_` *and* `popped_out_buttons_`. Updating only one leaves
stale labels on the popped-out row.

**VTB5 — `OnLabelsVisibilityChanged()` is the model→view notification.**
The model's virtual hook is empty by default; the container overrides it and
calls `UpdateAllLabels()`. Keep the override one line.

**VTB6 — Both `browseros/core` deps are required** (`action_utils` for
`IsBrowserOSAction` / `ShouldShowToolbarLabels`, `prefs` for the pref
constants). This target is `source_set("impl")`; a missing dep fails only
here.

## Workflows

**Turning toolbar labels on or off**
Flip `browseros.show_toolbar_labels`
(`chrome/browser/browseros/core/browseros_prefs.h`). The model's
`PrefChangeRegistrar` fires, `OnLabelsVisibilityPrefChanged()` runs, the
container's `OnLabelsVisibilityChanged()` override fires, and
`UpdateAllLabels()` re-renders.

**Adding a label to a new BrowserOS action**
1. Add the action to `kBrowserOSNativeActionIds` in
   `chrome/browser/browseros/core/browseros_action_utils.h`.
2. Register the action with a display name in
   `chrome/browser/ui/browser_actions.cc`.
3. `IsBrowserOSAction()` returns true and the button renders the label.

**Debugging a stale label**
1. Confirm the pref observer fired in
   `pinned_toolbar_actions_model.cc`.
2. Confirm the button is in `pinned_buttons_` or `popped_out_buttons_`.
3. Confirm the container's `GetActionItemFor(action_id)` returns an item
   with a name.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/views/`.
- [`../../toolbar/AGENTS.md`](../../toolbar/AGENTS.md) — default pinned list
  and toolbar pref names.
- [`../../toolbar/pinned_toolbar/AGENTS.md`](../../toolbar/pinned_toolbar/AGENTS.md)
  — the model that fires the label hook.
- [`../../../browseros/core/AGENTS.md`](../../../browseros/core/AGENTS.md) —
  `IsBrowserOSAction()`, `ShouldShowToolbarLabels()`.
- [`../../../../../../AGENTS.md`](../../../../../../AGENTS.md) —
  `packages/browseros/`.

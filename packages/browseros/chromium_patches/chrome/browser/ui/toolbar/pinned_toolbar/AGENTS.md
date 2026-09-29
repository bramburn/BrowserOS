# `chrome/browser/ui/toolbar/pinned_toolbar/` — reactive pinned-action model

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/toolbar/`) in
> [`packages/browseros/`](../../../../../../AGENTS.md).

## What's here

The `PinnedToolbarActionsModel` patch: it makes the pinned-action list
react to BrowserOS visibility prefs and drops the automatic Chrome Labs
pin. This is the model; the view side is
[`../views/toolbar/`](../../views/AGENTS.md).

## Contents

```
pinned_toolbar/
├── BUILD.gn                    ← adds //chrome/browser/browseros/core:action_utils
│                                 and .../core:prefs to the source_set deps
├── pinned_toolbar_actions_model.h   ← adds:
│     void EnsureAlwaysPinnedActions();
│     void OnBrowserOSVisibilityPrefChanged();
│     void OnLabelsVisibilityPrefChanged();
│     virtual void OnLabelsVisibilityChanged() {}   // observer hook
└── pinned_toolbar_actions_model.cc  ← PrefChangeRegistrar observes
      browseros::prefs::kShowLLMChat, kShowLLMHub  → OnBrowserOSVisibilityPrefChanged
      browseros::prefs::kShowToolbarLabels          → OnLabelsVisibilityPrefChanged
      Chrome Labs is no longer auto-pinned for new profiles
```

## Rules

**PT1 — Three visibility prefs, two observers.** `kShowLLMChat` and
`kShowLLMHub` drive `OnBrowserOSVisibilityPrefChanged()`;
`kShowToolbarLabels` drives `OnLabelsVisibilityPrefChanged()`. They are
different concerns (visibility vs. labels) and must not be merged.

**PT2 — `OnLabelsVisibilityChanged()` is a virtual observer hook with an
empty default.** Subclasses (the views container) override it. Do not give
it a body — an empty default is what lets non-view users of the model skip
the notification.

**PT3 — `EnsureAlwaysPinnedActions()` is called from the container, not the
model constructor.** `PinnedToolbarActionsContainer` calls it at
construction so the toolbar, not the model, owns the initial pinning
decision. Do not move the call.

**PT4 — Observer callbacks are `base::Unretained(this)`.** The
`PrefChangeRegistrar` is a member, so it is destroyed with the model and
the unowned-this callbacks cannot outlive it. Do not store the registrar
anywhere else.

**PT5 — The two `browseros/core` deps are load-bearing.** The model calls
`browseros::IsBrowserOSPinnedExtension()` (action_utils) and reads
`kShowLLMChat` / `kShowLLMHub` / `kShowToolbarLabels` (prefs). Removing
either dep fails only this target.

**PT6 — Chrome Labs removal must stay in sync with
`../toolbar_pref_names.cc`.** Both control the default pinned list; a
divergence gives new and migrated profiles different toolbars.

## Workflows

**Hiding a BrowserOS panel from the toolbar**
1. Flip the corresponding `browseros::prefs::kShow*` pref
   (`chrome/browser/browseros/core/browseros_prefs.h`).
2. The model's `PrefChangeRegistrar` fires and re-evaluates pinning.
3. No code change is needed — do not add a bespoke observer.

**Adding a new label-visibility hook consumer**
1. Override `OnLabelsVisibilityChanged()` in the subclass.
2. Do not modify the model's own label handling.

**Adding a new always-pinned action**
1. Add it to `browseros_action_utils.h`'s native set or the extension set.
2. `EnsureAlwaysPinnedActions()` picks it up.
3. Confirm the default-pinned list in `../toolbar_pref_names.cc` agrees.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/toolbar/`.
- [`../../views/toolbar/AGENTS.md`](../../views/toolbar/AGENTS.md) — the container
  that calls `EnsureAlwaysPinnedActions()` and implements the label hook.
- [`../../../browseros/core/AGENTS.md`](../../../browseros/core/AGENTS.md) —
  prefs and action utils.
- [`../../../../../../AGENTS.md`](../../../../../../AGENTS.md) —
  `packages/browseros/`.

# `chrome/browser/ui/views/side_panel/extensions/` — contextual panel helpers

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/views/side_panel/`)
> in [`packages/browseros/`](../../../../../../../AGENTS.md).

## What's here

The views-layer implementations of the contextual (tab-specific) side-panel
helpers declared in
[`../../../extensions/extension_side_panel_utils.h`](../../../extensions/extension_side_panel_utils.h),
plus the auto-pinning of BrowserOS extensions to the toolbar.

## Contents

```
extensions/
├── extension_side_panel_utils.cc   ← implements
│     IsContextualExtensionSidePanelOpen(BrowserWindowInterface*,
│         content::WebContents*, const ExtensionId&)
│     ToggleContextualExtensionSidePanel(BrowserWindowInterface&,
│         content::WebContents&, const ExtensionId&, std::optional<bool>)
│     Resolves SidePanelEntry::Key(kExtension, extension_id) against the
│     active tab; auto-registers a contextual entry if none exists
└── extension_side_panel_manager.cc ← on extension install, if
      browseros::IsBrowserOSPinnedExtension(extension->id()):
      PinnedToolbarActionsModel::Get(profile_)->UpdatePinnedState(
          extension_action_id, true);
      logs "browseros: Auto-pinning BrowserOS extension: <id>"
```

## Rules

**EXV1 — The declarations are in a different directory.** The two function
signatures live in
`chrome/browser/ui/extensions/extension_side_panel_utils.h`; only the bodies
are here. Change both together or the link breaks.

**EXV2 — These functions are contextual-only.** They build
`SidePanelEntry::Key(SidePanelEntry::Id::kExtension, extension_id)` and
match against the *active tab's* `WebContents`. Global panel entries are out
of scope; use `SidePanelService` for those.

**EXV3 — `ToggleContextualExtensionSidePanel` auto-registers.** If no
contextual entry exists for the tab, one is created rather than failing.
This is what makes the extension API's `browserosToggle` work on a fresh
tab. Preserve the `desired_state == std::nullopt` → "toggle current state"
semantics.

**EXV4 — Null `BrowserWindowInterface` or `WebContents` returns `false` and
logs a warning.** Both entry points are defensive; callers pass
`GetActiveTabInterface()->GetContents()` which can be null during teardown.

**EXV5 — Auto-pinning is a one-way, install-time action.**
`extension_side_panel_manager.cc` pins on extension install only. There is
no unpin on uninstall, and no user-facing toggle for the pin itself — it
comes from `IsBrowserOSPinnedExtension()` in
`chrome/browser/browseros/core/browseros_constants.h`.

**EXV6 — Keep the `browseros:` log prefix** on both files; it is the
diagnostic anchor for pinned-extension and panel-toggle issues.

## Workflows

**Adding auto-pinning for a new BrowserOS extension**
1. Mark it in the BrowserOS extension set
   (`chrome/browser/browseros/core/browseros_constants.h`).
2. Nothing else is needed — `IsBrowserOSPinnedExtension()` drives it.

**Calling the helpers from a new site**
1. Include `chrome/browser/ui/extensions/extension_side_panel_utils.h` (not
   this directory's `.cc`).
2. Pass the active tab's `WebContents` and the `ExtensionId`.
3. Pass `std::nullopt` to toggle, `true`/`false` to force a state.

**Debugging "extension panel toggle does nothing"**
1. Confirm the extension is a BrowserOS extension
   (`IsBrowserOSExtension`).
2. Confirm the action is bound to
   `CreateBrowserosToggleSidePanelActionCallback` in
   `chrome/browser/ui/side_panel/side_panel_action_callback.cc`.
3. Check the `browseros:` log lines in this directory for the actual
   lookup result.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/views/side_panel/`.
- [`../../../extensions/AGENTS.md`](../../../extensions/AGENTS.md) — the
  declarations.
- [`../../../side_panel/side_panel_action_callback.cc`](../../../side_panel/side_panel_action_callback.cc)
  — the toolbar callback that calls these.
- [`../../../toolbar/pinned_toolbar/AGENTS.md`](../../../toolbar/pinned_toolbar/AGENTS.md)
  — the model `UpdatePinnedState()` writes to.
- [`../../../../browseros/core/AGENTS.md`](../../../../browseros/core/AGENTS.md)
  — `IsBrowserOSPinnedExtension()`.
- [`../../../../../../../AGENTS.md`](../../../../../../../AGENTS.md) —
  `packages/browseros/`.

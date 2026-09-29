# `chrome/browser/ui/views/extensions/` — side-panel focus handling

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/views/`) in
> [`packages/browseros/`](../../../../../../AGENTS.md).

## What's here

One patch: `extension_view_views.cc`. When an extension-hosted side panel
belongs to a BrowserOS extension, the view requests focus once its render
widget host actually exists.

## Contents

```
extensions/
└── extension_view_views.cc   ← after load, if
                                 host_->extension_host_type() ==
                                     extensions::mojom::ViewType::kExtensionSidePanel
                                 and browseros::IsBrowserOSExtension(
                                     host_->extension_id()):
                                   RequestFocus();
```

## Rules

**VE1 — Focus is requested twice on purpose.** `PopulateSidePanel()` in
Chromium calls `RequestFocus()` before the `RenderWidgetHostView` exists,
which is a no-op. This patch re-requests after the host is created. Removing
the first call is fine; removing this one loses keyboard focus.

**VE2 — The check is double-gated:** extension-side-panel host type *and*
`browseros::IsBrowserOSExtension(extension_id())`. Non-BrowserOS extension
side panels keep upstream focus behaviour.

**VE3 — The extension ID set is the single source of truth.**
`IsBrowserOSExtension()` reads the constants in
`chrome/browser/browseros/core/browseros_constants.h` (`kAgentExtensionId`,
`kControllerExtensionId`, `kBugReporterExtensionId`, …). Add an ID there, not
here.

**VE4 — Focus steals from the web contents.** This runs on every load of a
BrowserOS side panel; do not extend it to panels the user is not looking
at.

## Workflows

**Adding focus behaviour for a new panel host type**
1. Add a `ViewType` comparison in the same block.
2. Keep the `IsBrowserOSExtension` gate unless the behaviour should be
   universal.
3. Re-check `browseros_constants.h` for the extension ID set.

**Debugging "side panel opens but has no keyboard focus"**
1. Confirm `host_->extension_host_type()` is
   `kExtensionSidePanel` at this point.
2. Confirm `IsBrowserOSExtension(host_->extension_id())` returns true —
   the ID must be in the BrowserOS set.
3. Confirm the call is after the render widget host is attached, not in
   `PopulateSidePanel()`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/views/`.
- [`../../../browseros/core/AGENTS.md`](../../../browseros/core/AGENTS.md)
  — `IsBrowserOSExtension()` and the extension IDs.
- [`../../extensions/AGENTS.md`](../../extensions/AGENTS.md) — the
  browser-layer side-panel helpers.
- [`../side_panel/extensions/AGENTS.md`](../side_panel/extensions/AGENTS.md)
  — the views-side toggle implementations.
- [`../../../../../../AGENTS.md`](../../../../../../AGENTS.md) —
  `packages/browseros/`.

# `chrome/browser/ui/extensions/` — extension UI glue

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/`) in
> [`packages/browseros/`](../../../../../AGENTS.md).

## What's here

Two patches at the extension/UI boundary: the declarations for
contextual (tab-specific) side-panel open/toggle helpers, and the
settings-override dialog suppression for BrowserOS extensions.

## Contents

```
extensions/
├── extension_side_panel_utils.h           ← declares
│     bool IsContextualExtensionSidePanelOpen(BrowserWindowInterface*,
│         content::WebContents*, const ExtensionId&);
│     bool ToggleContextualExtensionSidePanel(BrowserWindowInterface&,
│         content::WebContents&, const ExtensionId&,
│         std::optional<bool> desired_state);
│     (implemented in browser/ui/views/side_panel/extensions/extension_side_panel_utils.cc)
└── settings_overridden_params_providers.cc ← returns std::nullopt for
      browseros::IsBrowserOSExtension(extension->id()) so no dialog is shown
```

## Rules

**EXU1 — The implementations are NOT in this directory.** Both functions
are declared here and defined in
[`../views/side_panel/extensions/extension_side_panel_utils.cc`](../views/side_panel/extensions/extension_side_panel_utils.cc).
The header even says so. Editing this header without editing that `.cc`
breaks the link.

**EXU2 — These helpers handle only the contextual (tab-specific) panel.**
They deliberately do not touch global side-panel entries. A caller that
wants global behaviour must go through `SidePanelService` /
`SidePanelEntry` directly.

**EXU3 — `desired_state == std::nullopt` means "toggle".** Passing
`std::optional<bool>` rather than a plain `bool` is what lets the
`browserOS.sidePanel.browserosToggle` extension API implement
open/close/toggle from one entry point.

**EXU4 — BrowserOS extensions are silently exempt from the
settings-overridden dialog.** The check is
`browseros::IsBrowserOSExtension(extension->id())` and returns
`std::nullopt` (no dialog) with a `LOG(INFO)`. Do not surface a dialog for
the agent, controller, or bug-reporter extensions — they are first-party.

**EXU5 — New extension-facing helpers here must stay views-free.** This
header is included from the browser layer; the implementation belongs in
`browser/ui/views/`.

## Workflows

**Toggling an extension side panel from C++**
1. Call
   `ToggleContextualExtensionSidePanel(browser_window, web_contents, id,
   std::nullopt)`.
2. Check the returned `bool` for the resulting open state.
3. If the panel is not registered for the tab, it is auto-registered by the
   implementation.

**Suppressing a dialog for a new first-party extension**
1. Add the extension ID to the BrowserOS extension set in
   `chrome/browser/browseros/core/browseros_constants.h`
   (`kAgentExtensionId`, `kControllerExtensionId`,
   `kBugReporterExtensionId`, …).
2. `IsBrowserOSExtension()` picks it up automatically; no change is needed
   in this directory.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/`.
- [`../views/side_panel/extensions/AGENTS.md`](../views/side_panel/extensions/AGENTS.md)
  — the implementations and extension auto-pinning.
- [`../../browseros/core/AGENTS.md`](../../browseros/core/AGENTS.md) —
  `IsBrowserOSExtension()` and the extension-ID constants.
- [`../side_panel/AGENTS.md`](../side_panel/AGENTS.md) — toolbar action
  callback for pinned BrowserOS extensions.
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) —
  `packages/browseros/`.

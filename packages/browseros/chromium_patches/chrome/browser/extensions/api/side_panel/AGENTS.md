# `extensions/api/side_panel/` — `sidePanel.browseros*` functions

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

Two new extension API functions, `sidePanel.browserosToggle` and
`sidePanel.browserosIsOpen`, plus the two `BrowserOSService` methods behind
them. Unlike stock `sidePanel.open`/`close`, these operate on a *tab* rather
than the global side panel: they take an explicit `tab_id`, and
`BrowserosToggleSidePanelForTab` auto-registers contextual panel options for
that tab if none exist, then toggles. The result is `base::expected<bool,
std::string>` — a new state, or an error string.

## Contents

```
side_panel/
├── side_panel_api.h        ← SidePanelBrowserosToggleFunction
│                              ("sidePanel.browserosToggle")
│                            + SidePanelBrowserosIsOpenFunction
│                              ("sidePanel.browserosIsOpen")
├── side_panel_api.cc       ← RunFunction() bodies
├── side_panel_service.h    ← BrowserosToggleSidePanelForTab(),
│                              BrowserosIsSidePanelOpenForTab()
└── side_panel_service.cc   ← implementation (tab lookup + panel registration)
```

## Rules

**SPN1 — These are tab-scoped, unlike the stock sidePanel API.** Both service
methods take `int tab_id` and `bool include_incognito_information`, and both
resolve the tab through `../browser_os/browser_os_api_utils.h:GetTabFromOptionalId`.
Reusing that helper is mandatory — it is where incognito handling is decided.

**SPN2 — `desired_state == nullopt` means toggle, not "leave alone".** That is
the documented contract in the header. A caller that wants "ensure open" must
pass `true` explicitly.

**SPN3 — Toggling auto-registers contextual panel options.**
`BrowserosToggleSidePanelForTab` creates the per-tab `SidePanelEntry` when none
exists, because the BrowserOS side panels are contextual. Skipping the
registration makes the toggle a silent no-op on a fresh tab.

**SPN4 — Return `base::expected<bool, std::string>`, not a bare bool.**
The error string is the API's only diagnostic channel; an empty error makes a
failed toggle indistinguishable from "panel was already closed".

**SPN5 — The two function classes are non-copyable and use
`DECLARE_EXTENSION_FUNCTION`.** Follow the existing pattern exactly
(`= default` ctor, deleted copy/assign, private `~…() override = default`,
protected `RunFunction()`), or the API table generation breaks.

**SPN6 — Schema changes live under `chrome/common/extensions/api/`.** The
`DECLARE_EXTENSION_FUNCTION` strings must match the JSON there; the C++ header
is only half the contract.

## Workflows

**Adding a tab-scoped side-panel function**
1. Add the schema in `chrome/common/extensions/api/`.
2. Declare the function class in `side_panel_api.h` with a
   `DECLARE_EXTENSION_FUNCTION` name and the `SidePanelApiFunction` base.
3. Add the matching `SidePanelService` method returning
   `base::expected<…, std::string>`, resolving the tab via
   `GetTabFromOptionalId()`.
4. Extract both diffs and keep them in one `features.yaml` block.

**Changing what "open" means for a BrowserOS panel**
1. Edit `BrowserosToggleSidePanelForTab` / `BrowserosIsSidePanelOpenForTab` in
   `side_panel_service.cc`.
2. Keep the auto-registration of contextual options in the toggle path.
3. Re-check `../browseros/core/browseros_constants.h`:
   `UsesContextualSidePanelToggle()` decides which extension IDs this path
   applies to.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `extensions/api/` group.
- [`../browser_os/AGENTS.md`](../browser_os/AGENTS.md) — the shared
  `GetTabFromOptionalId()` helper.
- [`../../../../../../build/features.yaml`](../../../../../../build/features.yaml) —
  patch manifest.

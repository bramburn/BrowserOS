# `devtools/` — DevTools session wiring and the BrowserOS CDP surface

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/`) in
> [`packages/browseros/`](../../../../AGENTS.md).

## What's here

The browser side of BrowserOS's CDP extension. `inspector_protocol_config.json`
is the important file: it declares which commands each domain exposes, and the
diff adds 25 commands to the `Browser` domain plus two brand-new domains,
`Bookmarks` and `History`. `chrome_devtools_session.{h,cc}` constructs the new
handlers, `chrome_devtools_manager_delegate.{h,cc}` adds
`GetTargetTabId()` so a target can be mapped back to a tab/window ID, and
`BUILD.gn` pulls in the handler sources and the `//components/bookmarks`,
`//components/history` and `//components/tab_groups` deps. The handler
implementations live in `protocol/`.

## Contents

```
devtools/
├── BUILD.gn                            ← adds protocol/bookmarks.{cc,h},
│                                         protocol/history.{cc,h},
│                                         protocol/{bookmarks,history}_handler.{cc,h};
│                                         mac-only browser_handler_mac.{h,mm};
│                                         new deps: components/bookmarks/browser,
│                                         components/history/core/browser,
│                                         components/tab_groups
├── inspector_protocol_config.json       ← Browser domain += getTabForTarget,
│                                         getTargetForTab, get/create/close/activateWindow,
│                                         setWindowVisibility, get/create/close/activate/
│                                         move/duplicate/pin/unpin/showTab, and the
│                                         whole tab-group command set; new Bookmarks
│                                         and History domains
├── chrome_devtools_session.h / .cc      ← owns BookmarksHandler + HistoryHandler
├── chrome_devtools_manager_delegate.h / .cc ← GetTargetTabId() via
│                                         sessions::SessionTabHelper (tab + window id)
└── protocol/                            ← the handlers (see protocol/AGENTS.md)
```

## Rules

**DV1 — `inspector_protocol_config.json` is the command allowlist.** Anything
not in an `include` array is invisible over CDP regardless of whether the C++
`Backend` implements it. A new command needs three edits: the JSON entry here,
the `Browser::Backend` override in `protocol/browser_handler.h`, and the
implementation in `protocol/browser_handler.cc`.

**DV2 — `History` is the one domain declared `async`.**
`inspector_protocol_config.json` marks `search`, `getRecent`, `deleteUrl` and
`deleteRange` under an `"async"` key; the other domains have `"include_events":
[]` only. Adding a synchronous History command means deciding which list it
belongs to, and the handler must then use a `…Callback`-style signature
rather than `protocol::Response`.

**DV3 — Tab and window identity comes from `SessionTabHelper`, not from
tab-strip indices.** `GetTargetTabId()` in
`chrome_devtools_manager_delegate.cc` uses
`sessions::SessionTabHelper::IdForTab()` and `IdForWindowContainingTab()`, and
returns `-1` for the window when the session ID is invalid. A new tab command
must use these IDs too; raw strip indices are not stable across windows.

**DV4 — `GetTabs()` takes an explicit `include_hidden` parameter.**
The hidden-Browser rule from `../AGENTS.md` (BR4) applies to the CDP surface
too: hidden agent workspaces are excluded unless the caller asks for them. Any
new tab-enumeration command needs the same opt-in.

**DV5 — The mac-only sources are guarded by `if (is_mac)` in `BUILD.gn`.**
`protocol/browser_handler_mac.{h,mm}` provide `SetWindowHeadless()`, which
swizzles AppKit to fake `NSWindow` visibility while keeping the compositor
live, bypassing `constrainFrameRect:toScreen:` for off-screen agent windows.
They are added inside the existing `is_mac` `sources +=` block, not a new one.

**DV6 — `chrome_devtools_session.h` must own every handler it constructs.**
Adding a handler means a forward declaration, a `std::unique_ptr` member, and
construction in `chrome_devtools_session.cc`. A handler constructed anywhere
else will not receive CDP dispatches.

**DV7 — This directory depends on `content/browser/devtools/protocol/` for
target identity plumbing**, which is patched separately under
`../../../content/browser/devtools/`. Both must ship together; a partial
update leaves targets without stable tab IDs.

## Workflows

**Adding a `Browser.*` CDP command**
1. Append the name to the `Browser` domain `include` array in
   `inspector_protocol_config.json`.
2. Declare the `protocol::Response` override in `protocol/browser_handler.h`
   under the right section (window management / tab management / tab groups).
3. Implement it in `protocol/browser_handler.cc`.
4. Add a case to `devtools_protocol_browsertest.cc` if it is a mutating
   command.
5. Extract the three diffs and keep them in one `features.yaml` block.

**Adding a whole CDP domain**
1. Create the `.pdl` under `../../../third_party/blink/public/devtools_protocol/domains/`.
2. Add the domain block to `inspector_protocol_config.json`.
3. Create the handler in `protocol/`, register it in
   `chrome_devtools_session.{h,cc}`, and list it in `BUILD.gn`.
4. Add the component `deps` the handler needs.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/` overlay; hidden Browsers.
- [`protocol/AGENTS.md`](protocol/AGENTS.md) — the handler implementations.
- [`../../../content/browser/devtools/AGENTS.md`](../../../content/browser/devtools/AGENTS.md)
  — target/tab identity plumbing these commands depend on.
- [`../../../third_party/blink/public/devtools_protocol/AGENTS.md`](../../../third_party/blink/public/devtools_protocol/AGENTS.md)
  — the `.pdl` domain definitions.
- [`../../../../build/features.yaml`](../../../../build/features.yaml) — patch manifest.

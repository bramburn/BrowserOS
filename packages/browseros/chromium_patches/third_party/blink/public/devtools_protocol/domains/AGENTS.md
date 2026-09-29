# `third_party/blink/public/devtools_protocol/domains/` — BrowserOS CDP domains

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

Four CDP domain schemas. Two are new BrowserOS-authored files; two are
upstream files with BrowserOS additions. Together they define the protocol
surface the BrowserOS agent drives on port 9200 — tab/window/tab-group
management, bookmarks, and browsing history.

| File | Status | Content |
|---|---|---|
| `Bookmarks.pdl` | **new file** (79 lines) | `BookmarkNode`, `getBookmarks`, `searchBookmarks`, `createBookmark`, `updateBookmark`, `moveBookmark`, `removeBookmark` |
| `History.pdl` | **new file** (53 lines) | `HistoryEntry`, `search`, `getRecent`, `deleteUrl`, `deleteRange` |
| `Browser.pdl` | upstream + 250 lines | `TabID`, `WindowType`, `WindowInfo`, `TabInfo`, `TabGroupID`, `TabGroupInfo` + 26 commands |
| `Target.pdl` | upstream + 6 lines | optional `tabId` / `windowId` on `TargetInfo` |

Feature block: **`cdp-api`** for all four.

## Contents

```
domains/
├── Bookmarks.pdl   domain Bookmarks  (depends on Browser)
├── Browser.pdl     domain Browser    — window/tab/tab-group management
├── History.pdl     domain History
└── Target.pdl      domain Target     — TargetInfo gains tabId/windowId
```

### BrowserOS commands added to `Browser.pdl`

Windows: `getWindows`, `getActiveWindow`, `createWindow`, `closeWindow`,
`activateWindow`, `setWindowVisibility`.

Tabs: `getTabs`, `getActiveTab`, `getTabInfo`, `createTab`, `closeTab`,
`activateTab`, `moveTab`, `duplicateTab`, `pinTab`, `unpinTab`, `showTab`.

Tab groups: `getTabGroups`, `createTabGroup`, `updateTabGroup`,
`closeTabGroup`, `addTabsToGroup`, `removeTabsFromGroup`, `moveTabGroup`.

Target↔tab mapping: `getTabForTarget`, `getTargetForTab`.

## Rules

**DOM1 — New members in upstream domains are `experimental` and `optional`.**
`Browser.pdl` and `Target.pdl` ship in stock Chromium. A required or
non-experimental addition breaks upstream DevTools consumers and the protocol
compatibility checks.

**DOM2 — Each domain's `depends on` list is exactly as written.** `Bookmarks.pdl`
declares `depends on Browser`; `History.pdl` deliberately declares none.
Adding `depends on Browser` to `History.pdl` pulls in browser-level types it
does not use, and dropping it from `Bookmarks.pdl` changes the generated
header set.

**DOM3 — Registration is not optional.** Each of these four files also needs
its entry in `../BUILD.gn` and, for `Bookmarks.pdl` / `History.pdl`, its
`include` in `../browser_protocol.pdl`. See
[`../AGENTS.md`](../AGENTS.md).

**DOM4 — The two mapping commands take one selector each.** `getTabForTarget`
takes `optional Target.TargetID targetId` (absent means the session's own
target); `getTargetForTab` takes a required `TabID tabId`. Adding a second
selector means changing the schema comment, the handler, and the client.

**DOM5 — A `.pdl` without a handler is a compile-and-runtime trap.** The
generated code compiles, the client can call the method, and the browser answers
method-not-found. Verify the handler exists in
`chrome/browser/devtools/protocol/{bookmarks,browser,history}_handler.{h,cc}`.

**DOM6 — `Browser.pdl` additions are plain, comment-free command blocks.** They
sit in the same regions upstream CLs touch and carry no section banners, so
anchor an extract on the nearest real upstream context line.

**DOM7 — `getTabs` has a subtle `includeHidden` rule.** The comment states
`includeHidden` only affects all-window enumeration; a specific `windowId`
returns that window's tabs even when hidden. Preserve that semantic.

## Workflows

**Adding a new BrowserOS CDP command**
1. Add the command to the appropriate domain `.pdl` as `experimental`.
2. Add the return/parameter types it needs (usually to `Browser.pdl`'s type
   block).
3. Implement it in the matching handler under
   `chrome/browser/devtools/protocol/`.
4. Re-extract the `.pdl`; keep it under `cdp-api`.
5. Expose it through the agent's MCP surface in
   `packages/browseros-agent/apps/server/src/tools/registry.ts`.

**Adding a whole new domain**
1. Create `domains/<Name>.pdl`.
2. Register in `../BUILD.gn` and `../browser_protocol.pdl`.
3. Create the handler pair under `chrome/browser/devtools/protocol/`.
4. Extract all three; keep under `cdp-api`.

**Debugging "the method exists in the JSON but returns nothing"**
1. Is the handler registered with `DevToolsAgentHostClient`?
2. For `getTabs`/`getWindows`, is the session attached with the right target
   filter?
3. For `getTargetForTab`, is the tab's target actually discoverable?

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — registration rules for these domains.
- [`../../AGENTS.md`](../../AGENTS.md) — `blink/public/devtools_protocol/` rules.
- [`../../../../../../AGENTS.md`](../../../../../../AGENTS.md) — package overview.
- [`../../../../../../build/features.yaml`](../../../../../../build/features.yaml) — the `cdp-api` block.

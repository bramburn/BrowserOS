# `devtools/protocol/` — CDP domain handlers

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

The C++ behind BrowserOS's CDP commands. `browser_handler.{h,cc}` is the large
one: it adds ~25 window, tab and tab-group commands to the existing `Browser`
domain. `bookmarks_handler.{h,cc}` and `history_handler.{h,cc}` are new
handlers for the two new domains — `bookmarks_handler.cc` is synchronous
(`protocol::Response` returns, tree navigation via
`bookmarks::BookmarkModel`), while `history_handler.cc` is fully asynchronous
(callback-style returns, `base::CancelableTaskTracker` plus a `WeakPtrFactory`,
all four commands going through `history::HistoryService`).
`browser_handler_android.{h,cc}` is the Android/headless variant of the same
command set, `browser_handler_mac.{h,mm}` supplies the macOS headless-window
support, and `devtools_protocol_browsertest.cc` is the browsertest coverage.

## Contents

```
protocol/
├── browser_handler.h / .cc          ← Browser domain: 25 new commands
│                                      (GetWindows, CreateWindow, SetWindowVisibility,
│                                      GetTabs, CreateTab, MoveTab, PinTab, ShowTab,
│                                      and the full tab-group set)
├── browser_handler_android.h / .cc  ← same command set for the Android/headless path
├── browser_handler_mac.h / .mm      ← SetWindowHeadless(BrowserWindow*, bool)
├── bookmarks_handler.h / .cc        ← getBookmarks, searchBookmarks, createBookmark,
│                                      updateBookmark, moveBookmark, removeBookmark
├── history_handler.h / .cc          ← search, getRecent, deleteUrl, deleteRange (all async)
└── devtools_protocol_browsertest.cc ← CreateTabGroupAcceptsUnsortedTabIds,
                                       History.search timestamp-boundary test
```

## Rules

**DVP1 — Every command here must be listed in
`../inspector_protocol_config.json`.** A `Backend` override that is not in the
config `include` array is unreachable over CDP. The JSON is the contract; this
directory is the implementation.

**DVP2 — Bookmarks is synchronous, History is asynchronous. Do not mix.**
`BookmarksHandler` overrides return `protocol::Response` directly and hold
nothing but `target_id_` plus a `Profile*` accessor. `HistoryHandler`
overrides take `std::unique_ptr<…Callback>` and must keep their
`base::CancelableTaskTracker task_tracker_` and `base::WeakPtrFactory`
members, because the history backend calls back on another thread. Removing
either is a use-after-free.

**DVP3 — `browser_handler_android.cc` mirrors `browser_handler.cc`.** The two
files declare the same command set. Adding a command to one without the other
makes the CDP surface differ by platform, which is exactly what the agent
cannot tolerate. Change them together.

**DVP4 — Tab and window IDs are session IDs, not strip indices.** Both
handlers go through the `SessionID` values that `GetTargetTabId()` in
`../chrome_devtools_manager_delegate.cc` produces. A new command that resolves
a tab by index breaks as soon as a tab moves between windows.

**DVP5 — Handlers are deleted-copy, as all Chromium protocol backends are.**
`BookmarksHandler` and `HistoryHandler` both `= delete` copy and assignment.
The dispatcher owns them by `unique_ptr`.

**DVP6 — `browser_handler_mac.{h,mm}` is compiled only on macOS.** `../BUILD.gn`
adds them inside the `if (is_mac) { sources += … }` block. Any call site must
be guarded to match, or the Linux/Windows builds break.

**DVP7 — `devtools_protocol_browsertest.cc` is the only test surface here.**
It uses `SendCommandSync()`, checks `error()` after every command, and waits
on the history backend explicitly (`ui_test_utils::WaitForHistoryToLoad` and
`history::BlockUntilHistoryProcessesPendingRequests`) before asserting
timestamps. A new async command's test must do the same.

## Workflows

**Adding a tab-group command**
1. Add the name to the `Browser` `include` array in
   `../inspector_protocol_config.json`.
2. Declare the override in `browser_handler.h` **and**
   `browser_handler_android.h`.
3. Implement it in both `.cc` files.
4. Add a case to `devtools_protocol_browsertest.cc`; the existing
   `CreateTabGroupAcceptsUnsortedTabIds` test shows the expected shape
   (`base::DictValue` params, `FindList("tabIds")`, ordering assertions).
5. Extract all four diffs into the same `features.yaml` block.

**Adding a History command**
1. Add it to both `include` and `async` arrays in
   `../inspector_protocol_config.json`.
2. Add a callback-style override to `history_handler.h` and a
   `base::OnceCallback` handler in the `.cc`.
3. Route it through `GetHistoryService()` and guard with `task_tracker_`.
4. Add a browsertest that waits for the history backend before asserting.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the config file and BUILD.gn wiring.
- [`../../../../components/bookmarks/AGENTS.md`](../../../../components/bookmarks/AGENTS.md)
  — the `BookmarkModel` the bookmarks handler walks.
- [`../../../../third_party/blink/public/devtools_protocol/domains/AGENTS.md`](../../../../third_party/blink/public/devtools_protocol/domains/AGENTS.md)
  — the `.pdl` declarations these handlers implement.
- [`../../../../../build/features.yaml`](../../../../../build/features.yaml) —
  patch manifest.

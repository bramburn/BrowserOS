# `third_party/blink/` — DevTools Protocol definitions and one Blink fix

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

Chromium's Blink subtree, patched for two unrelated reasons:

- **`public/devtools_protocol/`** — the BrowserOS CDP surface. Two brand-new
  domains (`Bookmarks.pdl`, `History.pdl`) authored by BrowserOS, plus
  substantial additions to `Browser.pdl` (tab/window/tab-group management) and
  two optional fields on `Target.pdl` (`tabId`, `windowId`).
- **`renderer/core/frame/navigator.cc`** — `navigator.webdriver` is hard-coded
  to `false`, removing Chromium's automation detection.

The two are in different features: `cdp-api` for the protocol tree, `misc` for
`navigator.cc`.

## Contents

```
blink/
├── public/devtools_protocol/
│   ├── BUILD.gn                     ← registers domains/Bookmarks.pdl and domains/History.pdl
│   ├── browser_protocol.pdl         ← + include domains/Bookmarks.pdl, + include domains/History.pdl
│   └── domains/
│       ├── Bookmarks.pdl   new file  (79 lines) — getBookmarks / searchBookmarks / create / update / move / remove
│       ├── Browser.pdl     +250 lines — WindowInfo/TabInfo/TabGroupInfo + 26 commands
│       ├── History.pdl     new file  (53 lines) — search / getRecent / deleteUrl / deleteRange
│       └── Target.pdl      +6 lines   — optional tabId + windowId on TargetInfo
└── renderer/core/frame/navigator.cc  ← Navigator::webdriver() → false
```

## Rules

**BLK1 — A `.pdl` change requires a handler and a build registration.** Three
files must move together: the `.pdl`, the `include` in `browser_protocol.pdl`,
and the entry in that directory's `BUILD.gn` `domains` list. Handlers live
under `chrome/browser/devtools/protocol/` and
`content/browser/devtools/protocol/`.

**BLK2 — New protocol members must be `experimental` and `optional`.**
BrowserOS's CDP clients are the only consumers, but `Browser.pdl` and
`Target.pdl` are upstream files — required or non-experimental members break
stock Chrome DevTools and other embedders.

**BLK3 — `Target.pdl` gains `tabId`/`windowId`; it does not replace
`targetId`.** Existing consumers key on `targetId`. Commands in `Browser.pdl`
that accept both document "Exactly one must be provided" — preserve that
contract.

**BLK4 — `Navigator::webdriver()` returning `false` is a deliberate
anti-detection change, not a stub.** It removes the
`AutomationControlledEnabled()` check *and* the `probe::ApplyAutomationOverride`
call. Restoring either reinstates the `navigator.webdriver === true` signal
that sites use to detect CDP automation — which is the single behaviour the
BrowserOS agent depends on. The unused-include consequence is acceptable; do not
"clean it up" by re-adding a call.

**BLK5 — `Bookmarks.pdl` and `History.pdl` are BrowserOS-authored.** No
upstream counterpart exists, so there is no drift to reconcile — but also no
upstream review. They are ours to keep correct and documented.

**BLK6 — Do not add Blink runtime patches without weighing the rebase cost.**
`renderer/` is the most churned code in Chromium. This folder has exactly one
renderer patch; that restraint is deliberate.

## Workflows

**Adding a CDP command to an existing domain**
1. Declare the command in the domain `.pdl` as `experimental`, with
   `optional` parameters where backward compatibility matters.
2. Implement the handler in `chrome/browser/devtools/protocol/<domain>_handler.{h,cc}`.
3. If it's a new domain, also add the `include` line in `browser_protocol.pdl`
   and the entry in `BUILD.gn`.
4. Re-extract all touched `.pdl` files; keep them under `cdp-api`.

**Adding a brand-new domain**
1. Create `domains/<Name>.pdl` with a `domain <Name>` block and a
   `depends on Browser` clause if it needs `WindowID`/`TargetID`.
2. `include domains/<Name>.pdl` in `browser_protocol.pdl`, alphabetically.
3. Add `"domains/<Name>.pdl"` to the `domains` list in `BUILD.gn`, alphabetically.
4. Add the handler under `chrome/browser/devtools/protocol/`.
5. Add every path to the `cdp-api` feature in `features.yaml`.

**A site still detects CDP via `navigator.webdriver`**
1. Confirm the `navigator.cc` patch applied (`webdriver()` returns a bare
   `false`).
2. The site may be using CDP detection via other channels (e.g. missing
   `window.chrome`, permission-timing). Those are client-side in
   `packages/browseros-agent/apps/server/src/browser/backends/cdp.ts`, not in
   this patch.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `third_party/` subtree rules.
- [`public/devtools_protocol/AGENTS.md`](public/devtools_protocol/AGENTS.md) — the protocol tree.
- [`renderer/AGENTS.md`](renderer/AGENTS.md) — the `navigator.webdriver` patch.
- [`../../../AGENTS.md`](../../../AGENTS.md) — package overview.
- [`../../../build/features.yaml`](../../../build/features.yaml) — `cdp-api` and `misc` blocks.

# Plan — Page context-menu screenshot

> **Status:** proposal, not implemented. Nothing in this plan has been built or
> tested. The Chromium tree is not yet on disk, so every Chromium-side API
> signature below is **unverified at `148.0.7778.97`** and is marked as such.
> See [Verification gaps](#verification-gaps).
>
> **Scope:** Windows. Other OSes are out of scope until the Windows build is
> green (see [`docs/WINDOWS_BUILD.md`](../docs/WINDOWS_BUILD.md)).
>
> **Decision (2026-09-28): implement this — but only after the Windows build
> succeeds.** Do not begin Step 1 until `docs/WINDOWS_BUILD.md` reports a
> completed `autoninja` run that produced `chrome.exe`. Two things are blocked
> on that, not just one: every Chromium-side signature below needs the tree to
> verify against, and the **Step 0 gate needs a running browser** to answer
> whether Chrome already ships this item. Implementing before then would
> produce a diff written against remembered APIs rather than real ones, which
> is the failure mode this whole plan is structured to avoid.
>
> **Repo-local paths** are relative to this repo. **Chromium-tree paths** are
> paths inside `<chromium_src>/` after `gclient sync`, and are *not* files you
> edit directly — see [File changes](#file-changes).

## Goal

Let a user right-click a page and capture a screenshot, without leaving the
mouse, and without the screenshot UI belonging to a different product.

## What already exists (verified in this repo)

| Finding | Evidence |
|---|---|
| A working viewport-capture implementation already exists | `chromium_patches/chrome/browser/ui/views/side_panel/third_party_llm/third_party_llm_panel_coordinator.cc:623-698` — `OnScreenshotContent()` / `OnScreenshotCaptured()` |
| It uses `RenderWidgetHostView::CopyFromSurface` | same file, line 636-657 |
| It writes to clipboard and logs a metric | same file, line 680-683 |
| **No patch touches Chrome's stock screenshot menu item** | zero matches for `IDS_SHOW_SCREENSHOT`, `kShowScreenshot`, `ShowScreenshot`, `"save and share"` across all of `chromium_patches/` |
| `renderer_context_menu.cc` is **not** currently patched | absent from every `files:` list in `build/features.yaml` |
| BrowserOS owns command IDs in the 403xx range | `chromium_patches/chrome/app/chrome_command_ids.h` — 40304, 40305, 40306, 40307 are taken |
| A metrics API exists for exactly this | `browseros_metrics::BrowserOSMetrics::Log("llmchat.screenshot.captured")` |
| The MCP server has an *independent* CDP screenshot path | `packages/browseros-agent/apps/server/src/tools/page-actions.ts` + `packages/cdp-protocol/src/generated/domains/page.ts` |

The headline: **the capture half of this feature is already written and
proven.** This plan is about extracting it into a shared helper and giving it a
context-menu entry point.

## Step 0 — resolve the "is it already there?" question

Before writing any C++, answer this, because it changes the work by an order of
magnitude.

Chrome ships a page context-menu item at *Cast, save, and share → Screenshot…*.
BrowserOS does not patch it away (verified above), so it is plausibly **already
present and simply unbranded or undiscoverable**.

**Do this first:**

1. Build and run the browser (requires the green build from
   [`docs/WINDOWS_BUILD.md`](../docs/WINDOWS_BUILD.md)).
2. Right-click any page. Look for *Cast, save, and share → Screenshot…*.
3. Record which of these you observe:

| Observation | What it means | Next step |
|---|---|---|
| Item present, works, just unbranded | Stock Chromium already delivers it | **Stop.** Only branding/placement work remains — see [Route C](#route-c--promote-the-stock-item). |
| Item present but opens Chrome-branded UI | Stock item works, wrong product chrome | [Route C](#route-c--promote-the-stock-item) |
| Item absent | Branding/gating stripped it | Proceed to [Route A](#route-a--native-c-patch-recommended) |
| Item present but errors or does nothing | Broken or mis-wired upstream | Proceed to [Route A](#route-a--native-c-patch-recommended) |

Everything after this point assumes **Route A**. If Step 0 lands on Route C,
stop reading — that plan is a tenth of the work.

## Route A — native C++ patch (recommended)

BrowserOS-native capture, independent of the MCP server and of extension
release cycles.

### Why not reuse the MCP/CDP path

The MCP server can already screenshot via `Page.captureScreenshot`, and routing
the context menu through it is tempting because the code exists. Don't. The MCP
server is a *child process* that may be missing, crashed, upgrading, or slow to
start; a right-click menu item that silently no-ops when it isn't running is a
bad user experience. CDP capture also costs a round-trip and adds a failure mode
that a browser affordance should not have. The native path is ~30 lines and has
no runtime dependency.

Do keep the MCP path in mind for a later feature: **full-page** capture and
**element-scoped** capture are much easier over CDP, and the existing native
code explicitly notes it only grabs the visible viewport.

### Architecture

```
User right-clicks page
        │
        ▼
chrome/browser/renderer_context_menu.cc
   RenderViewContextMenu::AppendMenuItems()
        │  appends BrowserOSScreenshotHandler::BuildMenuItems()
        ▼
┌───────────────────────────────────────────────────────────┐
│ chrome/browser/browseros/screenshot/                       │
│                                                            │
│  browseros_screenshot_handler.{h,cc}                        │
│    - BuildMenuItems()   → contributes entries to the menu  │
│    - OnCommand(id)      → dispatches                        │
│    - owns the Toast / bubble feedback view                 │
│                                                            │
│  browseros_screenshot_service.{h,cc}                        │
│    - CaptureViewport(web_contents) → gfx::Image (async)    │
│    - CopyToClipboard(image)                                 │
│    - SaveAsFile(image)                                      │
│    - source of truth; no UI dependencies                    │
└───────────────────────────────────────────────────────────┘
        │
        ▼
BrowserOSMetrics::Log("contextmenu.screenshot.captured")
```

The split matters: `service` is pure capture (no Views, no menu), so the
existing side-panel button in `third_party_llm_panel_coordinator.cc` can be
migrated onto it later and both features share one code path.

### API surface

| Surface | Name | Where |
|---|---|---|
| Command ID | `IDC_BROWSEROS_SCREENSHOT` = **40308** | `chrome/app/chrome_command_ids.h` (next free in 403xx) |
| Menu item IDs | `IDC_BROWSEROS_SCREENSHOT_COPY` = **40309**, `IDC_BROWSEROS_SCREENSHOT_SAVE` = **40310** | same |
| Menu string | `IDS_BROWSEROS_SCREENSHOT`, `IDS_BROWSEROS_SCREENSHOT_COPY`, `IDS_BROWSEROS_SCREENSHOT_SAVE` | `chrome/app/generated_resources.grd` |
| Vector icon | `kBrowserOSScreenshotIcon` (new) or reuse `kPhotoChromeRefreshIcon` | `ui/vector_icons/` |
| Capture entry point | `browseros::screenshot::CaptureViewport(content::WebContents*, Callback)` | new service |
| Clipboard entry point | `browseros::screenshot::CopyToClipboard(const gfx::Image&)` | new service |
| File save entry point | `browseros::screenshot::SaveAsFile(const gfx::Image&, Profile*)` | new service |
| Menu contribution | `browseros::screenshot::BuildMenuItems(const ContextMenuParams&)` | new handler |
| Metric events | `contextmenu.screenshot.captured`, `contextmenu.screenshot.saved`, `contextmenu.screenshot.failed` | `chrome/browser/browseros/metrics/browseros_metrics.h` |
| Pref (optional) | `kBrowserOSContextMenuScreenshotEnabled` | `chrome/common/pref_names.h` + `chrome/browser/browseros/core/browseros_prefs.cc` |

The pref is optional. Add it only if the feature needs an enterprise kill
switch; otherwise skip it — every added pref is a migration surface.

### UX / UI

**Menu.** Top-level entry in the page context menu, placed immediately after
*Cast, save, and share* so it sits with the other capture affordances rather
than in a BrowserOS-branded corner. A submenu is justified because the two
destinations are genuinely different intents:

```
Right-click on page
  …
  Cast, save, and share
  Take screenshot  ▸
      Copy to clipboard
      Save as file…
  Inspect
```

**Why a submenu and not a single item.** Chrome's stock behaviour opens an
editable screenshot overlay. BrowserOS already has a one-shot
copy-to-clipboard path in the LLM side panel. Offering both as explicit
destinations is honest about the difference and avoids hijacking the user's
expectation that *Screenshot…* means "let me crop this". A single top-level
"Take screenshot" that silently copies would surprise anyone who came from
Chrome.

**Feedback.** Use Chromium's existing transient UI, not a custom view:

- Copy → `Toast` reading `Screenshot copied to clipboard`, ~2.5 s, matching
  the side panel's `feedback_timer_` timing so the two feel identical.
- Save → file picker (`SelectFileDialog`); on success a Toast with the filename.
- Failure → Toast `Couldn't capture this page` (or a more specific reason if
  the capture result carries one). Never fail silently — a right-click action
  that does nothing reads as a broken browser.

**Disabled states.** Hide the whole submenu when the target is not a real page
(`chrome://`, the NTP, the Web Store, a PDF plugin, a dragged link). Match
whatever guard stock Chromium uses for its own screenshot item rather than
inventing one — that guard is already correct for this Chromium version.

**Accessibility.** The vector icon and the menu strings must both resolve;
`generated_resources.grd` strings are automatically exposed to the Views a11y
tree for menu items, so no extra work is needed beyond not hardcoding literals.

**No new page in v1.** See [Pages](#pages).

### Pages

**v1 adds no WebUI page.** The whole flow is menu → capture → toast. Adding a
preview/annotate page would be a second feature.

If a crop-and-annotate surface is wanted later, it is a WebUI page and
belongs in the existing WebUI plumbing, not in `browseros/`:

| Artefact | Chromium-tree path | Registered in |
|---|---|---|
| Page source | `chrome/browser/ui/webui/browseros_screenshot/` | `chrome/browser/ui/webui/browseros_screenshot_ui.cc` |
| URL constant | `chrome://browseros-screenshot` | `chrome/common/webui_url_constants.cc` (already patched by `first-run`) |
| Page registration | `WebUIConfig` map | `chrome/browser/ui/webui/chrome_web_ui_configs.cc` (already patched by `first-run`) |
| Mojo glue | `browseros_screenshot.mojom` | `chrome/browser/ui/webui/browseros_screenshot/` |

This reuses exactly the registration path `first-run` already established, so
it is a known-good template.

### File changes

**New — BrowserOS code** (patched in as `new file` diffs):

```
chrome/browser/browseros/screenshot/BUILD.gn
chrome/browser/browseros/screenshot/browseros_screenshot_service.h
chrome/browser/browseros/screenshot/browseros_screenshot_service.cc
chrome/browser/browseros/screenshot/browseros_screenshot_handler.h
chrome/browser/browseros/screenshot/browseros_screenshot_handler.cc
```

**New — repo artefacts:**

| Path | What |
|---|---|
| `packages/browseros/chromium_patches/chrome/browser/browseros/screenshot/*` | the five diffs above, plus `BUILD.gn` |
| `packages/browseros/build/features.yaml` | new `context-menu-screenshot` block (see below) |

**Modified — Chromium files** (new diffs, or amend an existing one):

| Chromium-tree path | Change |
|---|---|
| `chrome/app/chrome_command_ids.h` | add 40308 / 40309 / 40310 |
| `chrome/app/generated_resources.grd` | add the three `IDS_` strings |
| `chrome/browser/renderer_context_menu.cc` | call `BuildMenuItems()`; dispatch the command IDs |
| `chrome/browser/renderer_context_menu.h` | declare the hook if not free-standing |
| `chrome/browser/browseros/BUILD.gn` | add `//chrome/browser/browseros/screenshot` to the `browseros` group |

**Shared files that already carry a patch.** `chrome_command_ids.h` and
`generated_resources.grd` are already patched by `browseros-core` and
`llm-chat` respectively. A feature block can only be applied once per file, so
these two edits must be folded into the **existing** diffs, not added as
competing ones. This is the single most common way a feature block fails to
apply — see [Pitfalls](#pitfalls).

### `features.yaml` registration

```yaml
  context-menu-screenshot:
    description: "feat: page context menu screenshot"
    files:
      # New BrowserOS capture + menu handler
      - chrome/browser/browseros/screenshot/
      # Menu contribution. Not currently patched by any block — safe to own.
      - chrome/browser/renderer_context_menu.cc
      - chrome/browser/renderer_context_menu.h
      # Already owned by browseros-core — append the new IDs to that diff
      # instead of listing the file here.
      # - chrome/app/chrome_command_ids.h
      # Already owned by llm-chat — append the new IDS_ strings to that diff.
      # - chrome/app/generated_resources.grd
```

Place the block in the layer above `browseros-core` — it depends on core
infrastructure but nothing depends on it. The commented-out lines are
deliberate: they document *why* the files are absent, so the next person
doesn't "fix" it by adding them.

### Implementation order

1. Run **Step 0** and record the result. Stop if it resolves to Route C.
2. Create `chrome/browser/browseros/screenshot/` with `service.{h,cc}` only —
   capture to clipboard, no menu, no UI. Port the logic from
   `third_party_llm_panel_coordinator.cc:623-698`.
3. Add the `BUILD.gn` for `screenshot/` and register it in
   `chrome/browser/browseros/BUILD.gn`.
4. Build and verify the service compiles and the existing side-panel screenshot
   still works. **This is the first real checkpoint** — a compile failure here
   means the `CopyFromSurface` signature moved, and everything downstream is
   speculative until it's fixed.
5. Migrate `third_party_llm_panel_coordinator.cc` to call the service. This
   proves the service is genuinely reusable and removes the duplicate.
6. Add the three command IDs to `chrome/app/chrome_command_ids.h`.
7. Add the three strings to `chrome/app/generated_resources.grd`.
8. Add `handler.{h,cc}` — `BuildMenuItems()` + `OnCommand()` + toasts.
9. Patch `renderer_context_menu.cc` to contribute and dispatch.
10. Add the `context-menu-screenshot` block to `build/features.yaml`.
11. Build phases 2–3, verify, then wire metrics events.

Steps 2–4 before 6–10 is deliberate: it isolates the "does capture work at
this Chromium version" question from the "does the menu wire up" question. A
combined change makes both fail at once and tells you nothing.

### Testing

| Level | How |
|---|---|
| Service unit | A `browseros_screenshot_service_unittest.cc` with a mock `WebContents` returning a known bitmap; assert a non-empty image and correct size. Mirrors `browseros_server_manager_unittest.cc` in the same tree. |
| Menu wiring | Build, right-click a page, confirm the submenu renders with both items and correct strings. |
| Copy path | Invoke, confirm a real paste in Paint produces the viewport. |
| Save path | Invoke, confirm the file picker opens and writes a valid PNG. |
| Guard states | Repeat on `chrome://settings`, the NTP, and a link-drag — submenu must be hidden. |
| Regression | Confirm the side-panel screenshot button still works after step 5. |
| Patch landed | `cd src && git log --oneline -- chrome/browser/browseros/screenshot/` |

Windows is the only target. Do not spend time on Linux/macOS until the Windows
build is green.

## Route C — promote the stock item

Only if Step 0 finds the item already working. Much smaller:

1. Patch `chrome/browser/renderer_context_menu.cc` to reposition the existing
   screenshot entry out of the *Cast, save, and share* submenu to the top level,
   under a BrowserOS string.
2. Add the `IDS_` string to `generated_resources.grd`.
3. No new command ID, no new service, no new BUILD.gn — the stock handler
   already owns the capture.

Roughly a fifth of Route A. Everything else in this document is unnecessary
work if Step 0 lands here.

## Pitfalls

| Pitfall | Consequence | Avoid by |
|---|---|---|
| Listing an already-patched file in a second block | `git apply` fails; the whole prep phase aborts | Fold shared-file edits into the existing diff. Check `build/features.yaml` first. |
| Assuming the stock item is absent | Building a feature that already exists | Do Step 0. |
| Copying menu code instead of extracting the service | Two capture implementations drift apart | Service has no Views dependency; migrate the side panel onto it in step 5. |
| Guessing Chromium signatures | Patch doesn't apply, or compiles wrong | Verify each signature against the pinned tree before writing the diff. |
| Hardcoding menu strings | Not localisable, breaks a11y | Use `IDS_` in `generated_resources.grd`. |
| A silent failure path | User right-clicks, nothing happens, browser looks broken | Always Toast on failure. |
| Forgetting `deps` in `BUILD.gn` | Link error deep in an unrelated target | Wire the group into `chrome/browser/browseros/BUILD.gn` in the same change. |

## Verification gaps

Honest limits on this plan:

- **The Chromium tree is not on disk.** The build never completed
  ([`docs/WINDOWS_BUILD.md`](../docs/WINDOWS_BUILD.md) § *What is unverified*).
  Every Chromium-side signature here — the exact
  `RenderViewContextMenu` hook, the menu-append call, the `CopyFromSurface`
  overload, the `Toast` helper — is inferred from the BrowserOS code that
  already calls into similar APIs at this version, **not read from the tree.**
  Confirm each one before writing its diff.
- The `CopyFromSurface` call was copied from a working BrowserOS file, so it
  is the most trustworthy line in this plan. Everything about the *context
  menu* is the speculative part.
- Whether stock Chromium's screenshot item is present is **unverified** — the
  only evidence is that no patch removes it.
- The metrics event names are proposed, following the existing
  `llmchat.screenshot.captured` convention. Confirm against the histogram
  registration in `chrome/browser/browseros/metrics/`.
- No part of this has been built or run.

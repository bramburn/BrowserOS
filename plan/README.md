# BrowserOS — state of play

> Single place to answer "where are we and what happens next". Written
> 2026-09-28, end of the Windows build bring-up session.
>
> Everything here is on branch `windows-build-toolchain`, **not pushed**.

## The one-line status

The repo is relocated to `D:\BrowserOs`, the build tree is aimed at
`D:\browseros-build`, three real Windows blockers are fixed, and the
**Chromium build has never completed on this host.** Two feature plans are
written and gated on that build going green.

## Where things live

| What | Path | Size | Drive |
|---|---|---|---|
| Repo checkout | `D:\BrowserOs` | ~0.6 GB | D: |
| Chromium source | `D:\browseros-build\src` | ~30 GB | D: |
| CIPD package cache | `D:\browseros-build\.gclient_cache` | ~20 GB | D: |
| Build output | `D:\browseros-build\src\out\Default` | ~100 GB | D: |

C: had 187 GB free and a build needs ~150 GB. It fits, but leaves no
headroom on the system drive, so the whole tree moved to D: (576 GB free).

## Committed work

Nine commits on `windows-build-toolchain`, no upstream, nothing pushed.

| Commit | What |
|---|---|
| `97ec0d64` | plan: record Step 0 result — stock Chrome screenshot item is absent |
| `1829b404` | plan: record the build-gated implementation decision + fix 3 broken anchors |
| `5e5990a3` | plans for context-menu screenshot and native localhost server |
| `3017aa44` | `docs/WINDOWS_BUILD.md` — procedure plus honest verification status |
| `7d907e88` | 35 hardcoded C: paths rewritten to the D: layout; AGENTS-build.md drift fixes |
| `e37a2763` | `clean` no longer wipes a pristine tree before `git_setup` re-syncs it |
| `7baec47e` | build tree on D:, `ForkRoot` move-proof, `depot_tools` on PATH |
| `9168ec8a` | `tools/fetch-chromium.ps1` — Chromium source bootstrap |
| `0ef46771`… | (pre-existing, from `main`) |

Uncommitted on purpose, none of it ours: `.github/workflows/deploy-docs.yml`
(pre-existing edit), `.pi/`, `docs-site/package-lock.json`. Note `.pi/` is
**not** gitignored — a `git add -A` would commit agent harvest data. Use
explicit pathspecs.

Untracked and also not gitignored, currently only on `D:`:
`.agents-manifest.txt`, `.agents-parts/`.

## Build status: not green

Full procedure and per-item verification in
[`docs/WINDOWS_BUILD.md`](../docs/WINDOWS_BUILD.md). Summary:

**Fixed**

1. **gclient could not find `.gclient`** — it resolves from the current
   working directory and parents. The fetch wrote the file but never
   `cd`'d into the build root, so it aborted in 8 seconds with
   `client not configured`. Reads like a CIPD/auth failure; isn't one.
2. **Logs written as UTF-16** — PowerShell `*>>` under PS 5.1. The real
   error was invisible behind mojibake. Now raw byte redirects, UTF-8.
3. **`autoninja`/`gn` unresolvable** — depot_tools was not on PATH, so
   phases 2 and 3 would have failed with "command not found".
4. **`clean` deleted the freshly-synced tree** — `clean` runs before
   `git_setup`, and `git clean -fdx third_party/` wiped ~30 GB that the
   following sync re-downloaded. Now skipped when the tree is pristine.

**Unverified**

- No `chrome.exe` or `mini_installer.exe` has ever been produced here.
- All four fixes are verified only by parse check, `py_compile`, and
  validating the generated `.gclient` as Python.
- GN args, patch application, linking, signing, packaging: untested.

**Open risk**

- `flags.windows.release.gn` sets `enable_widevine=true`, but there are
  **zero** widevine references anywhere under `packages/browseros/`. The
  Widevine CDM is proprietary and absent from the public tree. Expect a
  link failure, or DRM that silently does not work. Untested.

## Next actions, in order

```powershell
# 1. Reclaim ~22.7 GB of dead partial syncs from C:
& 'C:\Users\bramburn\.minimax\bin\mavis-trash.cmd' `
    'C:\browersos-build\src' 'C:\browersos-build\_src_partial_20260919'

# 2. Fetch the pinned Chromium tree (~50 GB, 1-3 h)
& D:\BrowserOs\tools\fetch-chromium.ps1

# 3. Setup: clean + git_setup + sparkle_setup
& D:\BrowserOs\tools\bramburn-build.ps1 -StopAfterPhase 1

# 4. Prep: resources + patches + GN config
& D:\BrowserOs\tools\bramburn-build.ps1 -StopAfterPhase 2

# 5. Build: autoninja, 7-13 h
& D:\BrowserOs\tools\bramburn-build.ps1 -StopAfterPhase 3
```

**The pass/fail signal for step 2 is timing.** The last run died in 8
seconds. If it is still going after ~60 s, the CWD fix worked.

## Feature plans

Both written, neither implemented, both gated on a green build.

### [`context-menu-screenshot.md`](context-menu-screenshot.md)

Add a page context-menu screenshot. **Step 0 is resolved: the stock
*Cast, save, and share → Screenshot…* item is absent** (verified in the
built browser, 2026-09-28). Route A (native C++ patch) is confirmed;
Route C is ruled out.

The capture half already exists and works —
`third_party_llm_panel_coordinator.cc` uses
`RenderWidgetHostView::CopyFromSurface` + `ScopedClipboardWriter`. The
plan extracts it into a shared service and adds a menu entry, then
migrates the side panel onto the same code.

First action once the tree exists:

```powershell
Select-String -Path D:\browseros-build\src\chrome\browser\renderer_context_menu.cc `
    -Pattern 'Screenshot','save and share'
```

String present → gated at runtime, find the gate. Absent → the whole
share block is compiled out. This decides menu item vs submenu.

### [`feature-gating-and-native-server.md`](feature-gating-and-native-server.md)

`chrome://flags` architecture, plus a native in-process HTTP server on
`127.0.0.1:1337` as the first gated feature — the first slice of
re-homing the MCP server from an extension into browser code, because
the extension port crashes and does not auto-reload.

Feature gating **already works** here: a four-file contract
(`browser_features`, `flag_descriptions`, `about_flags`, consumer) with
two BrowserOS flags registered. So the plan supplies conventions, not
machinery. The load-bearing one: the feature name and flag key must
agree, or the toggle silently gates nothing.

The server's hard parts are already solved in-repo —
`BrowserOSServerProxy` is a working `net::HttpServer::Delegate`, and
`BrowserOSServerManager` has the lock, orphan recovery and health-check
restart.

## Traps worth remembering

| Trap | Symptom | Fix |
|---|---|---|
| gclient CWD | `client not configured` in seconds | Run it with the working directory set to the dir *containing* `.gclient` |
| `cache_dir` on the wrong volume | ~20 GB of CIPD cache lands on C: | Put `cache_dir` inside the build root |
| Sync DEPS after `checkout`, not before | Mismatched dependency revisions | bootstrap sync → fetch tags → checkout → sync again |
| A file in two `features.yaml` blocks | `git apply` fails, **whole prep phase aborts** | Check the ownership map; fold edits into the existing block's diff |
| `Get-Content -Raw` on UTF-8 under PS 5.1 | Em-dashes read as `â€”`; "fixing" it corrupts the file | Use the read/grep/edit tools, which are encoding-safe |

## Ownership map (Chromium files are single-owner)

Read this before adding any feature. Each Chromium file may be patched by
at most one `features.yaml` block.

| File | Block |
|---|---|
| `chrome/browser/browser_features.{h,cc}`, `chrome/browser/about_flags.cc` | `chromium-ui-fixes` |
| `chrome/browser/flag_descriptions.{h,cc}` | `flags` |
| `chrome/browser/ui/ui_features.{h,cc}` | `llm-chat` |
| `chrome/browser/browseros/core/**`, `chrome/browser/browseros/BUILD.gn` | `browseros-core` |
| `chrome/browser/browseros/server/**` | `server` |
| `chrome/browser/chrome_browser_main.cc` | `first-run` |
| `chrome/browser/renderer_context_menu.{h,cc}` | **unowned** — free to claim |

## Doc drift corrected (2026-09-28)

`AGENTS-build.md` said `chromium_patches/` files are complete replacements.
They are **git-style unified diffs** applied with `git apply`; new C++ files
ship as `new file mode` diffs, and `chromium_files/` only holds `BRANDING`
and `branding.gni`. The manifest is at `packages/browseros/build/features.yaml`
(24 blocks, layered), not `packages/browseros/features.yaml`.

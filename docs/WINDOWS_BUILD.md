# Building BrowserOS on Windows

> Windows-only. Linux/macOS paths exist in the pipeline but are out of scope
> until the Windows build is green.
>
> Companion to [`AGENTS-build.md`](../AGENTS-build.md) (pipeline reference) and
> [`docs/CI_AND_RELEASES.md`](CI_AND_RELEASES.md) (release surface).
>
> **Status: procedure documented, build NOT yet verified end-to-end.**
> See [What is unverified](#what-is-unverified) before trusting this.

## Disk layout

| What | Path | Size |
|---|---|---|
| Repo checkout | `D:\BrowserOs` | ~0.6 GB |
| Chromium tree | `D:\browseros-build\src` | ~30 GB |
| CIPD package cache | `D:\browseros-build\.gclient_cache` | ~20 GB |
| Build output | `D:\browseros-build\src\out\Default` | ~100 GB |

Both the checkout and the Chromium tree are on **D:**, deliberately. C: had
187 GB free, and a full build needs ~150 GB, which fits but leaves almost no
headroom on the system drive.

`.gclient` is written with `cache_dir` **inside the build root**. gclient
re-downloads the entire ~20 GB CIPD cache wherever `cache_dir` points, even
when `src/` is on another volume — leaving it on C: refills the system disk.
That is what `C:\browersos-build\.gclient_cache` was.

## Toolchain (verified on this host)

| Component | Found | Notes |
|---|---|---|
| Visual Studio | Community 2022 + Build Tools 2022 | Both present |
| MSVC toolset | 14.44.35207 | Required for `is_clang`/`clang-cl` |
| Windows SDK | 10.0.26100.0 | `Include/` and `Lib/` both present |
| depot_tools | `C:\dev\depot_tools` | **Not on PATH** — see below |
| Python | 3.12.10 (`C:\Python312`) | `browseros.exe` lives in its `Scripts\` |
| Git | 2.55.0.windows.5 | |

Both a Community and a Build Tools install exist with the same toolset
version. Either satisfies the C++ workload.

## Procedure

```powershell
# 1. Fetch the pinned Chromium tree (~50 GB, 1-3 h)
& D:\BrowserOs\tools\fetch-chromium.ps1

# 2. Setup: clean + git_setup + sparkle_setup
& D:\BrowserOs\tools\bramburn-build.ps1 -StopAfterPhase 1

# 3. Prep: resources + patches + GN config
& D:\BrowserOs\tools\bramburn-build.ps1 -StopAfterPhase 2

# 4. Build: autoninja, 7-13 h
& D:\BrowserOs\tools\bramburn-build.ps1 -StopAfterPhase 3

# 5+6. Sign + package
& D:\BrowserOs\tools\bramburn-build.ps1 -StopAfterPhase 5
```

Run it detached; the bash tool's 5-minute ceiling will kill it otherwise.
`powershell.exe -File` is required — handing a `.ps1` to `python.exe` makes
Python try to parse it as Python and die immediately:

```powershell
Start-Process -FilePath "powershell.exe" `
  -ArgumentList "-NoProfile","-ExecutionPolicy","Bypass","-File","D:\BrowserOs\tools\bramburn-build.ps1","-StopAfterPhase","3" `
  -WindowStyle Hidden `
  -RedirectStandardOutput "D:\browseros-build\logs\build-stdout.log" `
  -RedirectStandardError  "D:\browseros-build\logs\build-stderr.log"
```

`ForkRoot` defaults to the repo root derived from the script's own location,
so the orchestrator survives a move of the checkout. Override `-ChromiumSrc`
or `-ForkRoot` if the layout differs.

## Blockers found and fixed

### 1. gclient could not find `.gclient` — fetch aborted in 8 seconds

`gclient` calls `GClient.LoadCurrentConfig()`, which reads `.gclient` from the
**current working directory** and its parents. The first version of
`fetch-chromium.ps1` wrote the file but never changed into the build root, so
gclient aborted with:

```
gclient.bat : Error: client not configured; see 'gclient config'
```

This reads like a CIPD/auth failure and is not one. `tools/bramburn-build.ps1`
never hits it because the `browseros` CLI chdir's on the caller's behalf.
Fixed by running gclient with `WorkingDirectory = $BuildRoot`.

**Telltale:** a real 50 GB sync cannot finish in 8 seconds. An instant
`exit=1` is a missing `.gclient`, not a network or auth problem.

### 2. Build logs written as UTF-16

PowerShell's `*>>` redirection emits UTF-16 under PS 5.1, so captured gclient
output came back as spaced-out mojibake and was unreadable. `fetch-chromium.ps1`
now routes native commands through `Start-Process` with raw byte redirects and
appends UTF-8 explicitly.

### 3. `autoninja` / `gn` not resolvable

`standard.py` invokes `autoninja.bat` and `configure.py` invokes `gn.bat` by
bare name. depot_tools is not on `PATH` on this host, so phases 2 and 3 would
both have died with "command not found". `bramburn-build.ps1` now prepends
`-DepotTools` (default `C:\dev\depot_tools`) to `PATH`.

### 4. `clean` deleted the freshly-synced tree before `git_setup` re-fetched it

`EXECUTION_ORDER` in `cli/build.py` is:

```python
("setup", ["clean", "git_setup", "sparkle_setup"]),
```

`CleanModule._git_reset` runs `git clean -fdx chrome/ components/ third_party/`,
which deletes the entire DEPS-managed `third_party/` tree — the bulk of the
~30 GB checkout. Because `clean` runs *before* `git_setup`, the immediately
following `gclient sync` re-downloaded all of it. On a first setup run that
turns a completed fetch into hours of redundant downloading.

`CleanModule` now skips the destructive reset when the tree is pristine — no
`out/` directory and no local modifications under the patched paths. A dirty
tree still gets the full clean, so rebuild behaviour is unchanged. If
`git status` fails, it conservatively assumes dirty and cleans anyway.

### 5. `chromium.googlesource.com` returns HTTP 503 from this host

**This blocks `gclient sync`, and therefore blocks the entire build.** Observed
2026-09-28, repeatedly, on every path and via two independent HTTP clients:

```
https://chromium.googlesource.com/                                     -> 503
https://chromium.googlesource.com/chromium/src/+/refs/tags/148.0.7778.97/VERSION?format=TEXT -> 503
https://chromium.googlesource.com/chromium/src/+/6b3fa66a.../chrome/browser/about_flags.cc?format=TEXT -> 503
```

Not a proxy problem and not local: `netsh winhttp show proxy` reports direct
access, `www.google.com` returns 200, and
`raw.githubusercontent.com/chromium/chromium/148.0.7778.97/...` serves the
same release tag fine. It is googlesource specifically.

**The GitHub mirror is not a workaround for a build.** Cloning `src` from
`https://github.com/chromium/chromium.git` would work, but Chromium's `DEPS`
points every dependency at `chromium.googlesource.com`, so `gclient sync`
would still fail on the first few hundred dep clones. There is no supported
way to redirect DEPS URLs to the mirror.

Options, in order of preference:

1. **Wait it out** and re-probe before assuming it is permanent:
   ```powershell
   Invoke-WebRequest -Uri "https://chromium.googlesource.com/" -UseBasicParsing
   ```
2. **Use a build host with access** — the Linux LAN box in
   [`AGENTS-ubuntu-dev.md`](../AGENTS-ubuntu-dev.md), or a cloud VM. This is
   what the WSL teardown already pushed toward.
3. **Single-file fetches still work** via the GitHub mirror, which is enough
   to answer API and guard questions without a tree — this is what
   `tools/verify-fetch-sources.ps1` does.

### Verifying against real sources without a tree

A full 50 GB sync is not needed to answer "does this API exist at 148" or
"which `#if` guard encloses this line". `tools/verify-fetch-sources.ps1` pulls
individual files from the GitHub mirror at the pinned tag into
`D:\browseros-build\verify\`, mirroring the Chromium tree layout:

```powershell
& .\tools\verify-fetch-sources.ps1
```

It fetches 16 files in a few seconds. `chrome/VERSION` from the mirror is
byte-identical to `packages/browseros/CHROMIUM_VERSION`, so the tag is the
right one. This found two compile blockers in the native-server implementation
that header review had missed — see the Verification status section of
[`../plan/feature-gating-and-native-server.md`](../plan/feature-gating-and-native-server.md).

## Known risks

### Widevine

`flags.windows.release.gn` sets `enable_widevine=true`, but there are **zero**
references to `widevine` anywhere under `packages/browseros/` — no CDM
provisioning, no patch, no download step. The Widevine CDM is Google's
proprietary component and is not in the public Chromium tree.

Expect either a `gn gen` or link failure, or a build that produces a browser
where DRM playback silently does not work. If it blocks, the options are to
supply `widevinecdm.dll` into `third_party/widevine/cdm/`, or set
`enable_widevine=false` and lose Widevine. **Not yet observed** — untested.

### DEPS must match the tag

`gclient sync` resolves DEPS for whatever commit is checked out. Syncing at
`main` and *then* checking out `148.0.7778.97` leaves mismatched dependency
revisions. `fetch-chromium.ps1` therefore checks out the tag **before** the
second sync. If you bootstrap the tree by hand instead, preserve that order.

## What is unverified

Honest status as of 2026-09-28:

- **The build has never completed on this host.** All four fixes above are
  reasoned from source and static checks (parse, `py_compile`, generated
  `.gclient` validated as Python); none has been exercised against a real
  `gn gen` or `autoninja` run.
- Phase 1 has not been observed past the `gclient` bootstrap.
- GN args, patch application, linking, signing and packaging are all untested.
- The Widevine risk above is a prediction, not an observed failure.
- No phase has produced a `chrome.exe` or `mini_installer.exe` on this host.

When the build does go green, update this section and record the outcome in
`AGENTS.md` alongside the existing 2026-09-20 incident log.

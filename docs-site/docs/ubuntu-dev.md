---
title: Local Ubuntu dev build (SSH)
description: Build BrowserOS on the local Ubuntu LAN box (192.168.0.45) via SSH for Linux-shaped iteration. Companion to the Windows build path.
sidebar_position: 1
---

# Local Ubuntu dev build (SSH)

> Companion to [Build](build). The Windows build path (`bramburn-build.ps1`)
> produces a signed Windows installer for release to R2 + GitHub Releases.
> This page covers the **development-only** build path on the local Ubuntu
> LAN box (`192.168.0.45`, hostname `macmini2024`) reached via SSH from
> this Windows dev machine.

## When to use this path

Use this path when you're iterating on patches that primarily affect Linux
shaped code (Chromium's primary dev target is Linux; the Windows path adds
noise), or you need a `.deb` / AppImage artifact.

Use the Windows path for producing a signed `.exe` for the release pipeline.

Both paths share the same fork (`packages/browseros/`); edit + commit + push
once, only the build host changes.

## Connection

### `ssh` in cmd/PowerShell PATH

Git Bash ships its own `ssh.exe`. For `cmd.exe` and `PowerShell`, the
Windows OpenSSH client must be on PATH. Check:

```cmd
where ssh
```

If empty:

```powershell
[Environment]::SetEnvironmentVariable("Path",
  [Environment]::GetEnvironmentVariable("Path","User") + ";C:\Windows\System32\OpenSSH",
  "User")
```

Open a new cmd window after.

### Connect

```powershell
ssh -i "$env:USERPROFILE/.ssh/id_ed25519_qalos" bramburn@192.168.0.45
```

### Recommended `~/.ssh/config` (Windows side)

```sshconfig
Host 192.168.0.45
  HostName 192.168.0.45
  User bramburn
  IdentityFile ~/.ssh/id_ed25519_qalos
```

## Box inventory (verified 2026-09-21)

| | |
|---|---|
| Hostname | `macmini2024` (Ubuntu-on-Mac-Mini) |
| OS | Ubuntu 24.04.5 LTS (Noble Numbat) |
| Kernel | 6.8.0-139-generic |
| CPU | Intel i7-3615QM @ 2.30 GHz — **Ivy Bridge, 2012, 4C/8T** |
| RAM | 15 GiB total, ~14 GiB available |
| Disk (`/`) | 591 GiB total, ~180 GiB free (post-Checkout: ~175 GiB) |
| User | `bramburn` (sudoer) |
| LAN latency | 4 ms |
| Outbound HTTPS to chromium.googlesource.com | HTTP 200, ~0.8 s |
| Existing workloads | AOSP volumes (~34 GiB), qdrant, postgres — see [AOSP coexistence](#aosp-coexistence) |

The Ivy Bridge CPU is the build-rate bottleneck. A full Chromium build is
projected at 16–24 h here (vs. 7–13 h on a modern Windows desktop).

## Workspace layout on the box

```
~/browersos-src/                   # the fork clone (matches C:\dev\BrowserOs)
├── packages/browseros/            # pipx-installed as editable
├── tools/
│ ├── ubuntu-build.sh              # orchestrator
│ └── ubuntu-launch.sh             # detached-launch wrapper
└── ...

~/browseros-build/                  # build root (matches C:\browersos-build\)
├── src/                            # Chromium source (post-setup)
├── .gclient                        # depot_tools config (sibling of src/)
├── build.log                       # orchestrator log
├── build-state.json                # {phase, phase_name, status, ...}
├── build.pid                       # PID of running orchestrator
├── build.log.phase{N}.stdout       # per-phase browseros stdout
└── build.log.phase{N}.stderr       # per-phase browseros stderr
```

After a full build:

```
~/browseros-build/src/out/Default/
├── chrome                        # runnable binary (~500 MB)
└── ... (ninja artifacts; .deb + AppImage produced by phase 5)
```

## First-time toolchain setup

The host prep (apt install + depot_tools + fork clone + `pipx install` of
`packages/browseros`) lives in the dedicated **[Toolchain](toolchain)**
page. Run those steps first (~5 min on a fresh box), then come back here
for the *build*-specific steps below (chromium bootstrap, scripts, verify).

Before continuing, confirm the toolchain is ready:

```bash
export PATH="$HOME/.local/bin:$HOME/depot_tools:$PATH"
command -v ninja g++ git gh browseros gclient
ninja --version
g++ --version | head -1
browseros build --help | head -3
```

If those print without errors, proceed.

### Bootstrap `chromium_src` (the step the README skips)

`browseros build --setup` requires `chromium_src` to **already contain a
cloned Chromium** with the pinned tag. There is no `browseros init`
subcommand — bootstrap is manual.

```bash
CHROMIUM_SRC=~/browseros-build/src
PIN_COMMIT="$(cat ~/browersos-src/packages/browseros/BASE_COMMIT)"
PIN_TAG="$(grep -E '^(MAJOR|MINOR|BUILD|PATCH)=' \
  ~/browersos-src/packages/browseros/CHROMIUM_VERSION \
  | tr -d '\n' | sed 's/MAJOR=//; s/MINOR=././; s/BUILD=././; s/PATCH=//')"

mkdir -p "$(dirname "$CHROMIUM_SRC")"
rm -rf "$CHROMIUM_SRC"
mkdir -p "$CHROMIUM_SRC"
cd "$CHROMIUM_SRC"
git init -q
git remote add origin https://chromium.googlesource.com/chromium/src.git

# Fetch only the pinned commit (depth-1 keeps it small; avoids 429 risk)
git fetch --depth=1 origin "$PIN_COMMIT"
git checkout FETCH_HEAD
git tag -f "$PIN_TAG" FETCH_HEAD

# Write sibling .gclient
cat > ~/browseros-build/.gclient <<'EOF'
solutions = [
  { "name" : "src",
    "url" : "https://chromium.googlesource.com/chromium/src.git",
    "deps_file" : "DEPS",
    "managed" : True,
    "custom_deps" : {},
  },
]
target_os = ["chromeos"]
target_os_only = False
EOF
```

**Rate-limit workaround (verified 2026-09-21, end-to-end):**

The earlier workarounds (manual chromium bootstrap + 15-min cooldowns +
git fetch --depth=1) were insufficient by themselves. Even `gclient sync`
hung at `src` for 13+ min (PCPU 0.2%, `STALL DETECTED` every 5 min) on
chromium.googlesource.com.

**The actual fix** is to tell gclient NOT to manage chromium/src — we
already cloned it manually; gclient just needs DEPS. Edit
`~/browseros-build/.gclient` and set the src solution to `managed: False`:

```python
solutions = [
  { "name" : "src",
    "url" : "https://github.com/chromium/chromium.git",  # or googs
    "deps_file" : "DEPS",
    "managed" : False,                                    # ← key fix
    "custom_deps" : {},
  },
]
target_os = ["chromeos"]
target_os_only = False
```

With `managed=False`, gclient skips the chromium/src fetch entirely and
goes straight to DEPS. Verified 2026-09-21: `gclient sync` completed in
~12 min, populated 164 DEPS repos + ~22 GB of prebuilds, zero errors.

```bash
export PATH="$HOME/.local/bin:$HOME/depot_tools:$PATH"
export GCLIENT_PARALLEL_FETCH=4
cd ~/browseros-build/src
gclient sync -D --no-history --shallow --verbose
```

Trade-off: chromium/src stays at the manually-pinned commit (no
auto-update to upstream HEAD) — fine for dev builds. Release builds
usually want `managed=True` + authenticated access.

### Pull in the orchestrator + launch wrapper

```powershell
scp -i "$env:USERPROFILE/.ssh/id_ed25519_qalos" `
  "$env:USERPROFILE\..\dev\BrowserOs\tools\ubuntu-build.sh" `
  bramburn@192.168.0.45:~/browseros-build/ubuntu-build.sh

scp -i "$env:USERPROFILE/.ssh/id_ed25519_qalos" `
  "$env:USERPROFILE\..\dev\BrowserOs\tools\ubuntu-launch.sh" `
  bramburn@192.168.0.45:~/browseros-build/ubuntu-launch.sh
```

Then on the box:

```bash
chmod +x ~/browseros-build/ubuntu-build.sh ~/browseros-build/ubuntu-launch.sh
```

### Verify (after pull)

```bash
~/browseros-build/ubuntu-build.sh --help        # orchestrator help
bash -n ~/browseros-build/ubuntu-build.sh && echo "syntax OK"
~/browseros-build/ubuntu-launch.sh --help
```

Base-toolchain checks (ninja/g++/browseros CLI) live in [Toolchain](toolchain) §Step 5.

## The orchestrator: `tools/ubuntu-build.sh`

Mirrors `tools/bramburn-build.ps1` phase-for-phase (verified against
`browseros build --list`):

| Phase | CLI | What it does | Wall time (projected) |
|---|---|---|---|
| 1 setup | `browseros build --setup` | `clean` + `git_setup` + `sparkle_setup` | 30–90 min |
| 2 prep | `browseros build --prep` (or `--modules=resources,…,configure` if no R2 creds) | `resources` + `chromium_replace` + `string_replaces` + `patches` + `configure` | 5–15 min |
| 3 build | `browseros build --build -t release -a x64` | `compile` (autoninja) | **12–24 h** |
| 4 sign | `browseros build --sign` | `sign_linux` — **no-op** per `--list` | seconds |
| 5 package | `browseros build --package` | `package_linux` → AppImage + `.deb` | 1–3 min |

### CLI

```bash
./tools/ubuntu-build.sh                        # all 5 phases
./tools/ubuntu-build.sh --stop-after-phase 3   # stop after phase 3
./tools/ubuntu-build.sh --phase 1 2            # only phases 1 and 2
./tools/ubuntu-build.sh --dry-run              # print, do not execute
./tools/ubuntu-build.sh --help
```

## Build workflows

### Detached background build (default for phase 3)

`tmux` does not survive SSH disconnects on this box (verified 2026-09-21).
Use `tools/ubuntu-launch.sh`, which wraps the orchestrator in
`setsid nohup` and writes a PID file:

```powershell
# Kick off detached:
ssh -i "$env:USERPROFILE/.ssh/id_ed25519_qalos" bramburn@192.168.0.45 `
  '~/browersos-src/tools/ubuntu-launch.sh --stop-after-phase 3'

# Tail from any shell:
ssh -i "$env:USERPROFILE/.ssh/id_ed25519_qalos" bramburn@192.168.0.45 `
  'tail -f ~/browseros-build/build.log'

# Snapshot of current state:
ssh -i "$env:USERPROFILE/.ssh/id_ed25519_qalos" bramburn@192.168.0.45 `
  'cat ~/browseros-build/build-state.json | python3 -m json.tool'

# Stop:
ssh -i "$env:USERPROFILE/.ssh/id_ed25519_qalos" bramburn@192.168.0.45 `
  'kill $(cat ~/browseros-build/build.pid)'
```

### Tail logs

```powershell
# Live tail:
ssh -i "$env:USERPROFILE/.ssh/id_ed25519_qalos" bramburn@192.168.0.45 `
  'tail -f ~/browseros-build/build.log'

# Where are we right now:
ssh -i "$env:USERPROFILE/.ssh/id_ed25519_qalos" bramburn@192.168.0.45 `
  'cat ~/browseros-build/build-state.json'
```

### AOSP coexistence

The box also hosts an AOSP sync workflow (~34 GiB AOSP volumes, Aliyun CLI,
occasional qdrant / postgres load). **Do not run the BrowserOS build and a
fresh AOSP sync simultaneously.**

- Watch load: `uptime` — back off if > 2.0.
- Watch disk: `df -h /` — free space if > 85% used.
- Pick off-peak windows; AOSP syncs tend to happen weekday evenings,
 BrowserOS builds go best overnight.

### Pulling artifacts back to Windows

```powershell
# Find what package_linux emitted:
ssh -i "$env:USERPROFILE/.ssh/id_ed25519_qalos" bramburn@192.168.0.45 `
  'find ~/browseros-build/src/out/Default -maxdepth 3 `
   \( -name "*.deb" -o -name "*.AppImage" \) -printf "%p\n"'

# Or grab the chrome binary directly for a quick test:
scp -i "$env:USERPROFILE/.ssh/id_ed25519_qalos" `
  bramburn@192.168.0.45:~/browseros-build/src/out/Default/chrome `
  C:\Users\bramburn\AppData\Local\Temp\browseros-chrome
```

### Incremental rebuild

After a successful full build, the source tree at `~/browseros-build/src`
holds the applied BrowserOS patches. To rebuild after editing patches:

1. Edit patches in `~/browersos-src/packages/browseros/chromium_patches/`.
2. Commit + push from Windows.
3. On the box: `cd ~/browersos-src && git pull`.
4. Rerun `~/browersos-src/tools/ubuntu-launch.sh --phase 2 3` — phase 2
 reapplies patches, phase 3 incrementally recompiles only what changed.

## Troubleshooting

| Symptom | Fix |
|---|---|
| `browseros: command not found` | Prepend `export PATH="$HOME/.local/bin:$HOME/depot_tools:$PATH"` |
| `DIRECT MODE: chromium_src does not exist` | Run the manual bootstrap (§First-time toolchain setup / Bootstrap) |
| `E: Unable to locate package libpango-1.0-dev` | Use `libpango1.0-dev` (1.0, not 1.0-0) on Noble |
| `ninja-build: command not found` | Binary is `ninja`, not `ninja-build` |
| tmux session vanishes after SSH ends | Use `tools/ubuntu-launch.sh` (`setsid + nohup`) |
| `gclient sync` hits `git_cache.ClobberNeeded()` + `RESOURCE_EXHAUSTED` | chromium.googlesource.com rate-limited; re-run with `GCLIENT_PARALLEL_FETCH=1` (serial) and 15-min cool-downs. See AGENTS.md §"Chromium sync" |
| `gclient sync` hangs on `src` step forever (PCPU ~0.2%, no log output, STALL DETECTED every 5 min) | chromium.googlesource.com anonymous rate limit on the bulk src fetch | Cool down 15+ min, retry with `GCLIENT_PARALLEL_FETCH=1`. As a last resort, switch the chromium remote to the GitHub mirror (`https://github.com/chromium/chromium.git`). See "Bootstrap chromium_src". |
| `browseros build --prep` fails with `R2 configuration not set. Required env vars: R2_ACCOUNT_ID, R2_ACCESS_KEY_ID, R2_SECRET_ACCESS_KEY` | Phase 2's first sub-module `download_resources` fetches the bundled MCP server binaries from Cloudflare R2 | For dev builds, skip just that module. Run with `--modules=resources,bundled_extensions,chromium_replace,string_replaces,patches,configure` instead of `--prep` — the bundled MCP server binaries are optional. The orchestrator (`tools/ubuntu-build.sh`) does this automatically when R2 env vars are unset. Verified 2026-09-21: 341/341 patches applied, GN configure succeeded in ~60 s. |
| `git fetch --tags` runs for hours | Use the depth-1 SHA-fetch pattern instead |
| Disk full during phase 1 | DEPS pulls many GB; `df -h /`; if > 90%, free space before continuing |
| OOM during phase 3 | Linux Chromium link step is RAM-hungry; watch for `c++: internal compiler error: Killed`; may need `-j4` |

## Cross-references

- [Build](build) — Windows-host build path.
- [AGENTS.md](https://github.com/bramburn/BrowserOS/blob/main/AGENTS.md) — strategic roadmap, Windows build, Chromium sync lessons learned.
- [packages/browseros/README.md](https://github.com/bramburn/BrowserOS/blob/main/packages/browseros/README.md) — the `browseros` CLI reference (note: README mentions `browseros setup` as a standalone subcommand; this is outdated — `setup` is now folded into `browseros build --setup`, and a manual chromium bootstrap is required first).
- [packages/browseros/CHROMIUM_VERSION](https://github.com/bramburn/BrowserOS/blob/main/packages/browseros/CHROMIUM_VERSION) — pinned Chromium version.
- [packages/browseros/BASE_COMMIT](https://github.com/bramburn/BrowserOS/blob/main/packages/browseros/BASE_COMMIT) — pinned Chromium commit SHA.

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

### apt-install Chromium build deps

Ubuntu 24.04 (Noble) package names. **Note** the package is
`libpango1.0-dev` — NOT `libpango-1.0-dev` (that name fails on Noble).

```bash
sudo apt-get update -qq
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
  git curl wget ca-certificates gnupg \
  build-essential ccache g++-13 gcc-13 g++-multilib \
  python3 python3-dev python3-pip pipx \
  ninja-build pkg-config \
  libnss3-dev libatk1.0-dev libatk-bridge2.0-dev libcups2-dev \
  libxkbcommon-dev libxcomposite-dev libxdamage-dev libxfixes-dev \
  libxrandr-dev libgbm-dev libpango1.0-dev libcairo2-dev libasound2-dev \
  libdbus-1-dev libdrm-dev libxshmfence-dev libglib2.0-dev libxkbfile-dev \
  libavif-dev libwoff-dev libopus-dev libwebp-dev libharfbuzz-dev \
  libxslt1-dev libevent-dev libvpx-dev libgstreamer1.0-dev \
  libgstreamer-plugins-base1.0-dev mesa-common-dev \
  tmux rsync unzip
```

### Install depot_tools + fork + browseros CLI

```bash
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git ~/depot_tools
git clone --depth=1 https://github.com/bramburn/BrowserOS.git ~/browersos-src
pipx install -e ~/browersos-src/packages/browseros

# PATH for this session (also add to ~/.bashrc for interactive shells)
export PATH="$HOME/.local/bin:$HOME/depot_tools:$PATH"
```

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

**Rate-limit workaround (verified 2026-09-21):** `browseros build --setup`
internally runs `git fetch --tags --force` which walks all ~5000 Chromium
tags and gets stuck on anonymous-rate-limited chromium.googlesource.com
(PCPU 0.3% indefinitely, no progress, no 429 in the log). Bypass it by
running `gclient sync` directly:

```bash
export PATH="$HOME/.local/bin:$HOME/depot_tools:$PATH"
export GCLIENT_PARALLEL_FETCH=1   # serial fetches; avoid 429
cd ~/browseros-build/src
gclient sync -D --no-history --shallow --verbose
```

This is what `git_setup` would have done after the tag fetch — it pulls
DEPS-specified third-party repos (v8, skia, angle, etc.) and is the
long-running part of phase 1.

**Hard wall (same day):** `gclient sync` itself can also hit a
chromium.googlesource.com rate-limit wall — PCPU ~0.2%, no log output,
`STALL DETECTED` every 5 min. A direct `curl
.../info/refs?service=git-upload-pack` timed out at 10 s with only 11 MB
downloaded (full ref advertisement is ~50 MB), and a direct `git fetch
--depth=1 origin HEAD` succeeds in 5 s sometimes and stalls other times.
The rate-limit window is unpredictable. Resume path: cool down 15+ min,
retry, and as a last resort switch the chromium remote to the GitHub
mirror (`https://github.com/chromium/chromium.git`) for reliability over
freshness.

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

### Verify

```bash
export PATH="$HOME/.local/bin:$HOME/depot_tools:$PATH"
ninja --version            # 1.11.x
python3 --version          # 3.12.x
gclient --version          # prints usage
browseros build --help     # rich Typer-formatted help
bash -n ~/browseros-build/ubuntu-build.sh && echo "syntax OK"
```

## The orchestrator: `tools/ubuntu-build.sh`

Mirrors `tools/bramburn-build.ps1` phase-for-phase (verified against
`browseros build --list`):

| Phase | CLI | What it does | Wall time (projected) |
|---|---|---|---|
| 1 setup | `browseros build --setup` | `clean` + `git_setup` + `sparkle_setup` | 30–90 min |
| 2 prep | `browseros build --prep` | `resources` + `chromium_replace` + `string_replaces` + `patches` + `configure` | 5–15 min |
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
| `browseros build --setup` stuck on `git fetch --tags --force` for 10+ min, PCPU ~0%, no log output | Tag fetch on anonymous-rate-limited chromium.googlesource.com | Skip `git_setup` and run `gclient sync -D --no-history --shallow --verbose` directly from `~/browseros-build/src` with `GCLIENT_PARALLEL_FETCH=1`. See "Bootstrap chromium_src". |
| `git fetch --tags` runs for hours | Use the depth-1 SHA-fetch pattern instead |
| Disk full during phase 1 | DEPS pulls many GB; `df -h /`; if > 90%, free space before continuing |
| OOM during phase 3 | Linux Chromium link step is RAM-hungry; watch for `c++: internal compiler error: Killed`; may need `-j4` |

## Cross-references

- [Build](build) — Windows-host build path.
- [AGENTS.md](https://github.com/bramburn/BrowserOS/blob/main/AGENTS.md) — strategic roadmap, Windows build, Chromium sync lessons learned.
- [packages/browseros/README.md](https://github.com/bramburn/BrowserOS/blob/main/packages/browseros/README.md) — the `browseros` CLI reference (note: README mentions `browseros setup` as a standalone subcommand; this is outdated — `setup` is now folded into `browseros build --setup`, and a manual chromium bootstrap is required first).
- [packages/browseros/CHROMIUM_VERSION](https://github.com/bramburn/BrowserOS/blob/main/packages/browseros/CHROMIUM_VERSION) — pinned Chromium version.
- [packages/browseros/BASE_COMMIT](https://github.com/bramburn/BrowserOS/blob/main/packages/browseros/BASE_COMMIT) — pinned Chromium commit SHA.

# BrowserOS — Local Ubuntu build (development via SSH)

> Companion to `AGENTS.md`. The strategic roadmap and the Windows-hosted build
> live there. This file covers the **development-only** build path on the
> local Ubuntu LAN box (`192.168.0.45`, hostname `macmini2024`) reached via
> SSH from this Windows dev machine.
>
> **Status: 2026-09-21 (later):** phases 1 + 2 complete.
> - Phase 1 (gclient sync) succeeded after setting `managed: False` for
>  the `src` solution in `.gclient` (works around the
>  chromium.googlesource.com anonymous rate-limit). 164 DEPS repos +
>  ~22 GB of CIPD prebuilds populated. Zero errors.
> - Phase 2 (prep) succeeded in ~60 s, applying **341/341 patches** clean
>  and running `gn gen out/Default_x64` ("Made 28979 targets from 4615
>  files in 49154 ms"). Final `out/Default_x64/{build.ninja,args.gn}`
>  exist. Built-in workaround in `tools/ubuntu-build.sh` for the R2-creds
>  issue (skips `download_resources` automatically when R2 env vars are
>  unset).
> - Phase 3 (autoninja, 12-24 h) is ready to fire on your schedule
>  (`tools/ubuntu-launch.sh --phase 3`).

## When to use this path vs. the Windows path

| | Windows path (`bramburn-build.ps1`) | Ubuntu path (`tools/ubuntu-build.sh`) |
|---|---|---|
| **Goal** | Producing a signed Windows `.exe` for release to R2 + GitHub Releases | Producing a Linux `.deb` / AppImage + dev tree for local iteration on Linux-shaped patches |
| **CPU** | Modern Windows desktop | **i7-3615QM (Ivy Bridge, 2012), 4C/8T @ 2.3 GHz** — the build-rate bottleneck |
| **RAM** | 32+ GiB typical | 15 GiB (14 GiB available — sufficient for Chromium) |
| **Disk** | `C:\browersos-build\src` (~150 GB) | `~/browseros-build/src` (~150 GB at ~180 GB free today) |
| **Network** | Same home LAN egress | Same home LAN egress |
| **Detach** | `Start-Process -WindowStyle Hidden` + redirected stdout (see AGENTS.md §3) | `setsid + nohup` with PID file (this file §6.3) |
| **Runtime** | 7–13 h per AGENTS.md table | **Projected 16–24 h** (slower CPU; not yet measured end-to-end) |
| **Output** | Signed Windows installer + R2 upload | Unsigned `.deb` + AppImage + dev tree at `out/Default` |

**Rules of thumb:**

- A patch that needs a Chromium-side C++ test or runs on Linux → use this path
 (Chromium's primary dev target is Linux; the Windows path adds noise).
- A patch that produces a Windows installer for QA / the signing pipeline → use
 the Windows path or the self-hosted Windows runner.
- Both paths share the same fork (`packages/browseros/`), so edit + commit +
 push once; only the *build host* changes.

## 1. Connection

### 1.1 One-time Windows prereq: `ssh` in cmd/PowerShell PATH

Git Bash ships its own `ssh.exe`, but `cmd.exe` / `PowerShell` need the
Windows OpenSSH client on PATH. Check:

```cmd
where ssh
```

If empty, add `C:\Windows\System32\OpenSSH` to your user PATH (no admin):

```powershell
[Environment]::SetEnvironmentVariable("Path",
  [Environment]::GetEnvironmentVariable("Path","User") + ";C:\Windows\System32\OpenSSH",
  "User")
```

Open a new cmd window after.

### 1.2 Connection command

Works from `cmd`, `PowerShell`, `Git Bash`, and `WSL`:

```powershell
ssh -i "$env:USERPROFILE/.ssh/id_ed25519_qalos" bramburn@192.168.0.45
```

```cmd
ssh -i "%USERPROFILE%\.ssh\id_ed25519_qalos" bramburn@192.168.0.45
```

```bash
# Git Bash / WSL
ssh -i "$USERPROFILE/.ssh/id_ed25519_qalos" bramburn@192.168.0.45
```

### 1.3 Recommended `~/.ssh/config` (Windows side)

So the bare `ssh <ip>` Just Works without the `-i` flag:

```sshconfig
Host 192.168.0.45
  HostName 192.168.0.45
  User bramburn
  IdentityFile ~/.ssh/id_ed25519_qalos

Host 192.168.0.46
  HostName 192.168.0.46
  User bramburn
  IdentityFile ~/.ssh/id_ed25519_qalos
```

### 1.4 Connection gotchas

1. **`'ssh' is not recognized`** in cmd/PowerShell — PATH missing
 `C:\Windows\System32\OpenSSH`. See §1.1.
2. **`/tmp` in Git Bash ≠ Windows temp.** Files written there are invisible to
 PowerShell/cmd. Use `$env:TEMP` or `C:\Users\bramburn\AppData\Local\Temp\`.
3. **`-i` paths must be quoted** in cmd/PowerShell (backslashes + spaces).
4. **Connection timed out** = host off-LAN or firewall blocks port 22; not an
 SSH/key problem. Check the LAN first.
5. **First-time host key prompts.** `ssh` will ask to fingerprint the host.
 Accept once (`yes`); `known_hosts` is updated for future runs.
6. **Non-interactive SSH sessions don't source `.bashrc`** — `PATH` updates
 you make interactively don't persist to scripted SSH. The scripts in this
 repo (`tools/ubuntu-launch.sh`, `tools/ubuntu-build.sh`) set PATH inline
 so this doesn't bite you, but if you copy-paste snippets from here, remember
 to prepend `export PATH="$HOME/.local/bin:$HOME/depot_tools:$PATH"`.

## 2. Box inventory (verified 2026-09-21)

| | |
|---|---|
| Hostname | `macmini2024` (yes, it's an Ubuntu-on-Mac-Mini) |
| OS | Ubuntu 24.04.5 LTS (Noble Numbat) |
| Kernel | 6.8.0-139-generic |
| Arch | x86_64 |
| CPU | Intel(R) Core(TM) i7-3615QM @ 2.30 GHz — **Ivy Bridge, 2012** |
| Cores / threads | 4C / 8T |
| RAM | 15 GiB total, ~14 GiB available |
| Disk (`/`) | 591 GiB total, ~180 GiB free (after Chromium checkout: ~175 GiB) |
| User | `bramburn` (uid 1000, in `sudo`, `adm`, `lxd` groups) |
| LAN latency from this Windows box | 4 ms ping (verified 2026-09-21) |
| Outbound HTTPS to chromium.googlesource.com | HTTP 200, ~0.8 s (verified 2026-09-21) |
| Existing workloads | AOSP volumes (~34 GiB), `qdrant`, `postgres` — **see §6.5 for coexistence** |

## 3. Workspace layout on the box

```
~/browersos-src/                   # the fork clone (matches C:\dev\BrowserOs)
├── .git/
├── packages/browseros/            # pipx-installed as editable
├── tools/
│ ├── ubuntu-build.sh              # orchestrator (5 phases)
│ └── ubuntu-launch.sh             # detached-launch wrapper
└── ... (rest of fork)

~/browseros-build/                  # build root (matches C:\browersos-build\)
├── src/                            # Chromium source (post-setup)
├── .gclient                        # depot_tools config (sibling of src/)
├── build.log                       # orchestrator log
├── build-state.json                # {phase, phase_name, status, last_error, updated_at, ...}
├── build.pid                       # PID of the running orchestrator
├── build.log.phase{N}.stdout       # per-phase browseros stdout
├── build.log.phase{N}.stderr       # per-phase browseros stderr
└── release.out, launch.out         # launcher wrapper output
```

After a successful full build, also:

```
~/browseros-build/src/out/Default/  # autoninja output (~100 GB)
~/browseros-build/src/out/Default/
 ├── chrome                        # runnable binary (~500 MB)
 ├── mini_installer                # Linux installer (if produced)
 └── ... (ninja build artifacts)
```

The Linux package phase produces a `.deb` and an AppImage — where exactly
they land is determined by `tools/release/package_linux.py`; check
`build.log.phase5.stdout` after that phase finishes.

## 4. First-time toolchain setup

> **Run the [Toolchain](docs-site/docs/toolchain.md) doc first** — apt install
> + `depot_tools` + fork clone + `pipx install` of `packages/browseros` take
> ~5 min. This section only covers the steps *after* the toolchain is ready
> and that are specific to the build (chromium bootstrap, scripts, verify).

Verify the toolchain is ready before continuing:

```bash
export PATH="$HOME/.local/bin:$HOME/depot_tools:$PATH"
command -v ninja g++ git gh browseros gclient
ninja --version
g++ --version | head -1
browseros build --help | head -3
```

If those print without errors, proceed to §4.5.

### 4.5 Bootstrap `chromium_src` (the step the README skips)

`browseros build --setup` requires `chromium_src` to **already contain a
cloned Chromium** with the pinned tag. There is no `browseros init`
subcommand — bootstrap is manual.

```bash
CHROMIUM_SRC=~/browseros-build/src
PIN_COMMIT="$(cat ~/browersos-src/packages/browseros/BASE_COMMIT)"
PIN_TAG="$(grep -E '^(MAJOR|MINOR|BUILD|PATCH)=' ~/browersos-src/packages/browseros/CHROMIUM_VERSION | tr -d '\n' | sed 's/MAJOR=//; s/MINOR=././; s/BUILD=././; s/PATCH=//')"

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
```

Then write the sibling `.gclient`:

```bash
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

**Important rate-limit workaround (verified 2026-09-21, second pass):**

The earlier workarounds (manual chromium bootstrap + 15-min cooldowns) were
insufficient. Even `gclient sync` directly, on chromium.googlesource.com,
hung at `src` for 13+ min (PCPU 0.2%, `STALL DETECTED` every 5 min).
A 1.5-hour cooldown + a github.com mirror swap still hung in the same spot.

**The actual fix** is to tell gclient NOT to manage chromium/src at all.
We already cloned it manually; gclient just needs to fetch DEPS. Edit
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
goes straight to DEPS. Verified 2026-09-21 14:58–15:10: `gclient sync`
completed in ~12 min, populated 164 DEPS repos + ~22 GB of prebuilds,
zero errors.

Then continue with `git_setup`'s actual work (which is now safe because
DEPS are in place):

```bash
export PATH="$HOME/.local/bin:$HOME/depot_tools:$PATH"
export GCLIENT_PARALLEL_FETCH=4   # github tolerates parallel; googs tolerates serial
cd ~/browseros-build/src
gclient sync -D --no-history --shallow --verbose
```

The trade-off: chromium/src stays at the manually-pinned commit (no
auto-update to upstream HEAD) — fine for dev builds where we control the
patch set. For release builds you'd typically want `managed=True` +
authenticated access (see the GitHub Actions runner section in
[`docs/CI_AND_RELEASES.md`](docs/CI_AND_RELEASES.md)).

### 4.6 Pull in the launch + orchestrator scripts

```bash
scp -i "$env:USERPROFILE/.ssh/id_ed25519_qalos" \
  "$env:USERPROFILE\..\dev\BrowserOs\tools\ubuntu-build.sh" \
  bramburn@192.168.0.45:~/browseros-build/ubuntu-build.sh

scp -i "$env:USERPROFILE/.ssh/id_ed25519_qalos" \
  "$env:USERPROFILE\..\dev\BrowserOs\tools\ubuntu-launch.sh" \
  bramburn@192.168.0.45:~/browseros-build/ubuntu-launch.sh
```

Then on the box:

```bash
chmod +x ~/browseros-build/ubuntu-build.sh ~/browseros-build/ubuntu-launch.sh
```

(For long-term, prefer committing these two scripts to the fork and pulling
on the box — see §7.5.)

### 4.7 Verify toolchain (after pull)

```bash
cd ~/browseros-build
export PATH="$HOME/.local/bin:$HOME/depot_tools:$PATH"

~/browseros-build/ubuntu-build.sh --help        # expect the orchestrator help
bash -n ~/browseros-build/ubuntu-build.sh && echo "syntax OK"
~/browseros-build/ubuntu-launch.sh --help
```

The base-toolchain checks (ninja/g++/browseros CLI) live in
[`AGENTS-toolchain.md`](AGENTS-toolchain.md) / [Toolchain](docs-site/docs/toolchain.md) §Step 5.

## 5. The orchestrator: `tools/ubuntu-build.sh`

Mirrors `tools/bramburn-build.ps1` phase-for-phase. Same 5 phases, same
`--stop-after-phase` semantics, same log + state JSON pattern.

### 5.1 Phases (verified against `browseros build --list`)

| Phase | CLI | Internally runs | Wall time (projected) |
|---|---|---|---|
| 1 setup | `browseros build --setup` | `clean` + `git_setup` + `sparkle_setup` | 30–90 min (DEPS sync is the rate-limited step) |
| 2 prep | `browseros build --prep` | `resources` + `chromium_replace` + `string_replaces` + `patches` + `configure` | 5–15 min |
| 3 build | `browseros build --build -t release -a x64` | `compile` (autoninja) | **12–24 h** on this CPU |
| 4 sign | `browseros build --sign` | `sign_linux` (**officially a no-op** per `--list`) | seconds |
| 5 package | `browseros build --package` | `package_linux` → AppImage + `.deb` | 1–3 min |

### 5.2 CLI

```bash
./tools/ubuntu-build.sh                        # all 5 phases
./tools/ubuntu-build.sh --stop-after-phase 3   # stop after phase 3 (build)
./tools/ubuntu-build.sh --phase 1 2            # only phases 1 and 2
./tools/ubuntu-build.sh --chromium-src PATH    # override chromium src dir
./tools/ubuntu-build.sh --dry-run              # print commands, do nothing
./tools/ubuntu-build.sh --help
```

Env overrides (see `tools/ubuntu-build.sh` head):

- `BROWSEROS_CHROMIUM_SRC` — default `~/browseros-build/src`
- `BROWSEROS_BUILD_ROOT` — default `~/browseros-build`
- `BROWSEROS_LOG_DIR` — default `$BROWSEROS_BUILD_ROOT`
- `BROWSEROS_BROWSEROS_EXE` — default `~/.local/bin/browseros`
- `BROWSEROS_FORK_ROOT` — default `~/browersos-src`
- `BROWSEROS_JOBS` — default `$(nproc)` (= 8 on this box)

## 6. Build workflows

### 6.1 First build end-to-end (interactive — only for the first run)

```bash
ssh -i "$USERPROFILE/.ssh/id_ed25519_qalos" bramburn@192.168.0.45

# On the box:
export PATH="$HOME/.local/bin:$HOME/depot_tools:$PATH"
cd ~/browersos-src

./tools/ubuntu-build.sh
```

For the 12–24 h phase 3, you'll get disconnected by the SSH timeout. Use
the detached pattern instead (§6.3).

### 6.2 Running from Windows over SSH (also interactive)

```bash
ssh -i "$USERPROFILE/.ssh/id_ed25519_qalos" bramburn@192.168.0.45 \
  '~/browersos-src/tools/ubuntu-build.sh --stop-after-phase 2'
```

### 6.3 Detached background build (the default for phase 3)

Use `tools/ubuntu-launch.sh` — it wraps the orchestrator in `setsid nohup`
and writes a PID file. **tmux does not survive SSH disconnects on this
box** (verified 2026-09-21); `setsid + nohup` is the only reliable detach
pattern we have.

```bash
# Kick off detached:
ssh -i "$USERPROFILE/.ssh/id_ed25519_qalos" bramburn@192.168.0.45 \
  '~/browersos-src/tools/ubuntu-launch.sh --stop-after-phase 3'

# Tail from any shell:
ssh -i "$USERPROFILE/.ssh/id_ed25519_qalos" bramburn@192.168.0.45 \
  'tail -f ~/browseros-build/build.log'

# Or just the latest state:
ssh -i "$USERPROFILE/.ssh/id_ed25519_qalos" bramburn@192.168.0.45 \
  'cat ~/browseros-build/build-state.json | python3 -m json.tool'

# Stop:
ssh -i "$USERPROFILE/.ssh/id_ed25519_qalos" bramburn@192.168.0.45 \
  'kill $(cat ~/browseros-build/build.pid)'
```

The launcher refuses to start a second build if the PID file shows a live
process. Either wait for it or kill it explicitly.

### 6.4 Tail logs from Windows

```powershell
# Live tail (single SSH connection):
ssh -i "$env:USERPROFILE/.ssh/id_ed25519_qalos" bramburn@192.168.0.45 `
  'tail -f ~/browseros-build/build.log'
```

```powershell
# One-shot "where are we?" check:
ssh -i "$env:USERPROFILE/.ssh/id_ed25519_qalos" bramburn@192.168.0.45 `
  'cat ~/browseros-build/build-state.json'
```

### 6.5 AOSP coexistence

The box already hosts an AOSP sync workflow (~34 GiB AOSP volumes on disk,
Aliyun CLI for the sync gateway, occasional `qdrant` / `postgres` load).
**Do not run the BrowserOS build and a fresh AOSP sync simultaneously.**

- Watch load before starting: `uptime` — anything > 2.0 means back off.
- Watch disk before starting: `df -h /` — if `/` is > 85% used, free space
 first (`mavis-trash` on the box if available, or `rm -rf` carefully).
- Pick off-peak windows: AOSP syncs tend to happen on weekday evenings;
 BrowserOS builds go best overnight.

### 6.6 Pulling artifacts back to Windows

```bash
# rsync the produced .deb + AppImage (path depends on what package_linux emits):
ssh -i "$USERPROFILE/.ssh/id_ed25519_qalos" bramburn@192.168.0.45 \
  'find ~/browseros-build/src/out/Default -maxdepth 3 \( -name "*.deb" -o -name "*.AppImage" \) -printf "%p\n"'
# then rsync those paths back via scp.

# Or just scp the full chrome binary for a quick test:
scp -i "$USERPROFILE/.ssh/id_ed25519_qalos" \
  bramburn@192.168.0.45:~/browseros-build/src/out/Default/chrome \
  C:\Users\bramburn\AppData\Local\Temp\browseros-chrome
```

### 6.7 Incremental rebuild

After a successful full build, the source tree at `~/browseros-build/src`
holds the applied BrowserOS patches. To rebuild after editing patches:

1. Edit patches in `~/browersos-src/packages/browseros/chromium_patches/`.
2. Commit + push from Windows.
3. On the box: `cd ~/browersos-src && git pull`.
4. Rerun `~/browersos-src/tools/ubuntu-launch.sh --phase 2 3` — phase 2
 reapplies patches, phase 3 incrementally recompiles only what changed.

## 7. Watchdog / cron (optional)

For overnight builds, a thin cron loop on the box (analogous to the
Windows `browseros-build-watchdog` mentioned in AGENTS.md) can restart a
build that died mid-phase, write a heartbeat, and clean up old log files.

Sketch (not yet installed on the box — file a follow-up if you want this):

```bash
# ~/browseros-build/watchdog.sh
#!/usr/bin/env bash
STATE="$HOME/browseros-build/build-state.json"
LOG="$HOME/browseros-build/watchdog.log"
[ -f "$STATE" ] || exit 0
status=$(python3 -c "import json; print(json.load(open('$STATE'))['status'])")
phase=$(python3 -c "import json; print(json.load(open('$STATE'))['phase'])")
if [ "$status" = "failed" ]; then
  echo "[$(date -Iseconds)] phase $phase failed — restarting from there" >> "$LOG"
  exec ~/browersos-src/tools/ubuntu-launch.sh --phase "$phase"
fi
```

Add to the box's `crontab -e`:

```cron
*/15 * * * * /home/bramburn/browseros-build/watchdog.sh
```

## 8. Troubleshooting

| Symptom | Cause | Fix |
|---|---|---|
| `browseros: command not found` | `~/.local/bin` not on PATH in this SSH session | Prepend `export PATH="$HOME/.local/bin:$HOME/depot_tools:$PATH"` to the command, or update `.bashrc` (§4.4) |
| `DIRECT MODE: chromium_src does not exist: .../src` | Phase 1 ran before manual bootstrap | Run the §4.5 bootstrap steps |
| `E: Unable to locate package libpango-1.0-dev` | Wrong name for Noble | Use `libpango1.0-dev` (1.0, not 1.0-0) |
| `ninja-build: command not found` | Binary is `ninja`, not `ninja-build` | Run `ninja --version` (the package `ninja-build` ships `/usr/bin/ninja`) |
| `apt install` exits with errors for one pkg but installs nothing | `set -e` in wrapper aborted mid-line | Re-run with explicit per-package installs (the script in §4.1 uses one line, but if it bails, split into chunks) |
| tmux session vanishes after SSH ends | tmux master doesn't survive SSH disconnects on this box | Use `tools/ubuntu-launch.sh` instead (`setsid + nohup`) |
| `gclient sync` hits `git_cache.ClobberNeeded()` + `RESOURCE_EXHAUSTED` | chromium.googlesource.com anonymous rate limit | Re-run with `GCLIENT_PARALLEL_FETCH=1` (serial), 15-min cool-downs. Mirror the Windows lessons in AGENTS.md §"Chromium sync" |
| `browseros build --setup` stuck on `git fetch --tags --force` for 10+ min, PCPU ~0%, no log output, no 429 | chromium.googlesource.com anonymous rate limit on the full tag fetch (thousands of refs) | Skip the tag fetch: run `gclient sync -D --no-history --shallow --verbose` directly from `~/browseros-build/src` with `GCLIENT_PARALLEL_FETCH=1`. See §4.5 for the full workaround. |
| `git fetch --tags` runs for hours | Tag count is large; rate-limited per fetch | Use the depth-1 SHA-fetch pattern in §4.5 instead of `--tags` |
| `gclient sync` hangs on `src` step forever (PCPU ~0.2%, no log output, STALL DETECTED every 5 min) | chromium.googlesource.com anonymous rate limit on the bulk src fetch. Verified 2026-09-21: a direct `curl ... /info/refs?service=git-upload-pack` timed out at 10 s with only 11 MB downloaded (full ref advertisement is ~50 MB), and even a direct `git fetch --depth=1 origin HEAD` succeeds in 5 s sometimes and stalls other times | (1) Cool down 15+ min between attempts. (2) Use `GCLIENT_PARALLEL_FETCH=1` for serial fetches. (3) As a last resort, switch the chromium remote to the GitHub mirror (`https://github.com/chromium/chromium.git`) — loses googs freshness, gains reliability. |
| Disk full during phase 1 | DEPS pulls many GB | `df -h /`; if `/` > 90%, free space before continuing (the box has no separate build volume) |
| OOM during phase 3 | Linux Chromium link step is RAM-hungry | Phase 3 may need swap tweaks or `-j4` (we have 15 GiB RAM — usually OK, watch for `c++: internal compiler error: Killed`) |

## 9. Future: GitHub Actions self-hosted runner

**Status: not on the roadmap right now.** See the strategic discussion in
the original session that landed this doc — the home NAT blocks inbound
connections from GitHub's hosted service, and adding a public-IP /
Tailscale-Funnel relay for one overnight build is premature.

Reconsider only when **all three** are true:

1. Browser-feature PRs want a Linux `.deb` smoke-test on every push
 (not just the Windows release exe).
2. A `tailscale-funnel` (or equivalent) stands up so GitHub can reach a
 self-hosted runner on `192.168.0.45`.
3. AOSP use has moved off this box (so the runner gets uncontested CPU).

Then add `.github/workflows/test-linux.yml` + a third self-hosted runner
label `[self-hosted, Linux, browseros-builder-linux]`.

## 10. Cross-references

- [`AGENTS.md`](AGENTS.md) — strategic roadmap, Windows build path, lessons
 learned from Chromium sync failures (HTTP 429 / cache mislabel).
- [`packages/browseros/README.md`](packages/browseros/README.md) — the
 `browseros` CLI reference (note: README mentions `browseros setup` as a
 standalone subcommand; this is outdated — `setup` is now folded into
 `browseros build --setup`, and a manual chromium bootstrap is required
 first; see §4.5 above).
- [`packages/browseros/CHROMIUM_VERSION`](packages/browseros/CHROMIUM_VERSION) — pinned Chromium version (148.0.7778.97 at this writing).
- [`packages/browseros/BASE_COMMIT`](packages/browseros/BASE_COMMIT) — pinned Chromium commit SHA (6b3fa66a923a9442c8ab0bc71b4b41ff24528d3b).
- `tools/bramburn-build.ps1` — the Windows orchestrator this file mirrors.
- `tools/ubuntu-build.sh` — the Linux orchestrator documented here.
- `tools/ubuntu-launch.sh` — the detached-launch wrapper.

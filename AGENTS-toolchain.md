---
title: Toolchain (Ubuntu 24.04 dev host)
description: Toolchain bootstrap for a Linux dev host that builds BrowserOS. Captured 2026-09-21 from the macmini2024 (192.168.0.45) box.
sidebar_position: 2
---

# Toolchain (Ubuntu 24.04 dev host)

> One-time host bootstrap for building BrowserOS on a Linux box. Companion to
> [Local Ubuntu dev build (SSH)](ubuntu-dev) — that doc assumes this toolchain
> is in place; this doc is purely about getting the box ready.
>
> **Captured 2026-09-21** from a fresh install on `macmini2024`
> (`192.168.0.45`, Ubuntu 24.04.5 LTS, kernel 6.8.0-139-generic,
> i7-3615QM, 15 GiB RAM). The steps below are generic for any
> Ubuntu 24.04 (Noble) host.

## When to use this

Run this once on a fresh Ubuntu 24.04 box you intend to build BrowserOS
on. Re-runs are safe — `apt` is idempotent, the git clones are shallow,
the `pipx` install is editable and won't clobber your changes.

## Prerequisites

| Requirement | Verify with |
|---|---|
| Ubuntu 24.04 LTS (Noble Numbat) | `cat /etc/os-release` |
| sudo access for the build user | `sudo -n true && echo OK` |
| Internet egress to chromium.googlesource.com + R2 + GitHub | `curl -sSf -o /dev/null -w '%{http_code}\n' https://chromium.googlesource.com` |
| ~25 GB free disk | `df -h /` |
| 8+ GiB RAM | `free -h` |

## Step 1 — apt-install Chromium build deps

**Time:** ~3 min on a fresh box (depends on apt cache).

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

### Gotchas (verified 2026-09-21)

- **`libpango1.0-dev`** — note the **1.0** (decimal). `libpango-1.0-dev`
 (hyphen) does **not exist** in Noble's apt repo and will abort the
 entire line under `set -e`.
- **`ninja-build`** ships `/usr/bin/ninja`, not `ninja-build`.
 Use `ninja --version`, not `ninja-build --version`.
- **One bad package aborts the whole line.** If you split the install into
 chunks (e.g. core vs chromium-deps) for diagnostics, use
 `apt-get install -y ... ; echo done` (no `set -e`) so a single
 failure shows up but doesn't kill the script.
- **Build deps for Chromium move with the version.** The list above is
 what Chromium 148 needs on Noble. If you bump to Chromium 150+, check
 <https://chromium.googlesource.com/chromium/src/+/main/docs/linux/build_instructions.md>.

## Step 2 — Clone `depot_tools`

**Time:** ~30 sec for the first time (one shallow clone).

```bash
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git ~/depot_tools
```

## Step 3 — Clone the fork

Shallow clone matches the Windows checkout at `C:\dev\BrowserOs`:

```bash
git clone --depth=1 https://github.com/bramburn/BrowserOS.git ~/browersos-src
```

If you want to commit changes that flow back to GitHub, set the `origin`
remote to your fork:

```bash
cd ~/browersos-src
git remote set-url origin https://github.com/bramburn/BrowserOS.git
gh auth setup-git  # if you intend to push
```

## Step 4 — Install the `browseros` CLI

**Time:** ~30 sec.

```bash
pipx install -e ~/browersos-src/packages/browseros
```

`pipx` warns that `~/.local/bin` isn't on PATH. Fix:

```bash
echo 'export PATH="$HOME/.local/bin:$HOME/depot_tools:$PATH"' >> ~/.bashrc
export PATH="$HOME/.local/bin:$HOME/depot_tools:$PATH"
```

(For scripted SSH sessions — which the
[Local Ubuntu dev build (SSH)](ubuntu-dev) workflow uses — the
orchestrator script sets PATH inline so the `.bashrc` line is only
needed for interactive shells.)

## Step 5 — Verify

```bash
export PATH="$HOME/.local/bin:$HOME/depot_tools:$PATH"

ninja --version          # expect 1.11.x
python3 --version        # 3.12.x
g++ --version | head -1  # 13.3.x on Noble
pkg-config --modversion glib-2.0  # 2.80.x
gclient --version        # prints usage (no error)
browseros build --help   # Typer-formatted help
```

All six should print without errors. If any fails, re-run the
corresponding step.

## Idempotency

Each step is idempotent:

- `apt-get install -y` with `--no-install-recommends` is safe to re-run.
- `git clone` of the same URL into a non-empty dir will refuse; use
 `git fetch` instead, or `rm -rf` first if you want a clean state.
- `pipx install -e` is a no-op if the venv already exists.

## PATH management (non-interactive sessions)

SSH runs from `ssh ...` are non-interactive, so bash does **not** source
`~/.bashrc`. PATH updates you make interactively do not propagate. The
[Local Ubuntu dev build (SSH)](ubuntu-dev) workflow handles this by
setting PATH inline in its scripts; if you copy-paste snippets, prepend:

```bash
export PATH="$HOME/.local/bin:$HOME/depot_tools:$PATH"
```

## Time budget (one-time, fresh box)

| Step | Wall |
|---|---|
| 1 apt-install | ~3 min |
| 2 depot_tools clone | ~30 sec |
| 3 fork clone | ~30 sec |
| 4 browseros CLI install | ~30 sec |
| 5 verify | ~5 sec |
| **Total** | **~5 min** |

After this, the first `gclient sync` (phase 1) takes 30–90 min; the first
full autoninja build (phase 3) takes 12–24 h on an i7-3615QM-equivalent
host.

## Cross-references

- [Local Ubuntu dev build (SSH)](ubuntu-dev) — uses this toolchain.
- [AGENTS.md](https://github.com/bramburn/BrowserOS/blob/main/AGENTS.md) — strategic context, Windows build path.
- [`AGENTS-ubuntu-dev.md`](https://github.com/bramburn/BrowserOS/blob/main/AGENTS-ubuntu-dev.md) — full in-repo reference for the Ubuntu-via-SSH workflow.
- [Chromium Linux build instructions](https://chromium.googlesource.com/chromium/src/+/main/docs/linux/build_instructions.md) — canonical upstream guide.

<div align="center">

# `bramburn/BrowserOS`

### A self-hostable Chromium fork with an Edge/Chrome-grade auto-update workflow.

[![Upstream](https://img.shields.io/badge/upstream-browseros--ai%2FBrowserOS-blue)](https://github.com/browseros-ai/BrowserOS)
[![License: AGPL v3](https://img.shields.io/badge/License-AGPL%20v3-blue.svg)](LICENSE)
[![Build status](https://img.shields.io/badge/build-pending-lightgrey)](docs/MCP_TOOL_SPEC.md)

</div>

This is a public fork of [browseros-ai/BrowserOS](https://github.com/browseros-ai/BrowserOS)
maintained for **self-hosted use** and **release-cadence resilience**. We use
BrowserOS as the runtime for our own browser-automation CLI
([`C:\dev\browser-cli`](../browser-cli)), and every upstream release has
historically broken our wrappers within a week. This fork exists so we can
ship backwards-compat shims, ship our own updates on our own cadence, and
contribute the patches back to upstream.

See [`WHY_FORK.md`](WHY_FORK.md) for the full motivation and
[`AGENTS.md`](AGENTS.md) for the runbook (architecture, build pipeline,
toolchain, MCP tool spec).

---

## Strategic roadmap — what we're working on, in order

We build bottom-up. The goal is a fork that ships like Edge or Chrome:
**boring, automatic, never blocks you**. Everything else comes after that
foundation is solid.

### #1 — A proper auto-update workflow that works like Edge / Chrome *(highest priority)*

> "We can ship fast only once the update story works end-to-end."

Right now BrowserOS ships updates out-of-band (manual download from
`files.browseros.com`) — that's not shippable. We want the same UX you get
from Edge or Chrome:

- **Background download** while the browser is in use.
- **"Update available — restart to update"** toast on a quiet moment.
- **Restart-and-keep-tabs** so the user never loses context.
- **Signed delta updates** — small payloads, fast apply.
- **Staged rollout** — internal ring → beta ring → stable ring, with
  kill-switch via a server config we control.
- **Update self-host** — point our fork's update URL at our own server so we
  are not pinned to upstream's release schedule.

The Chromium update machinery (`components/update_client`, Omaha 4 protocol,
`GoogleUpdate` setup on Windows) is already wired into the fork — it just
points at upstream's URL and signing keys. The work is:

1. Replace the update URL with our own server (nginx + signed payloads).
2. Sign our own payloads with a fork-owned key.
3. Patch the toast + restart flow so it matches Edge/Chrome UX.
4. End-to-end test: ship v1 → ship v2 to update server → click "restart to
   update" → restart into v2 with all tabs intact.

This is the **gating dependency** for everything below. Until updates Just
Work, we cannot ship weekly.

### #2 — Make the browser robust

A browser that crashes weekly is worse than no browser. Priority work:

- Crash reporter wired to our own backend so we see field crashes ourselves.
- Session restore proven across hard kills (`kill -9` / BSOD / power loss).
- Bookmark / password / extension data survives update + rollback.
- Memory: no leaks past 24 h of normal use; aggressive tab discard policy.
- Renderer sandbox escapes caught and reported.
- Reproducible builds (same source → same binary hash) so we trust the
  signed delta.

### #3 — Installation UX / UI

The current installer is a fairly thin wrapper around the Chromium
mini_installer. We want:

- First-run experience that is actually a first-run experience: import from
  Chrome / Edge / Firefox, ask about default browser, walk through the AI
  agent onboarding without overwhelming.
- A real "About" / "What's new" page driven by the update server's
  release notes — no more out-of-band blog posts.
- Uninstall that is honest about what stays behind (user data) and what
  is removed (the binary).
- Telemetry opt-in / opt-out that is clear and one click.

### #4 — Custom Docusaurus site for the fork

`docs.browseros.com` is upstream's docs site. We need our own:

- Lives in [`docs-site/`](docs-site/) in this repo (Docusaurus 3 classic).
- Documents only the **fork-specific** behaviour: the update server config,
  the build pipeline, the MCP tool spec (fork-specific additions only),
  the release process.
- Deployed automatically by CI on every merge to `main`.
- Versioned alongside the fork's releases.

### #5 (later) — MCP and agent improvements

The bundled MCP server (`browseros_server.exe`) and the Bun MCP server
(`packages/browseros-agent/apps/server`) both work today, but we have a
known breakage history on every upstream release (see `WHY_FORK.md`). Once
the foundation above is solid, we tackle:

- Pin the JSON-RPC schema with a versioned, backwards-compat shim so the
  next upstream release doesn't break our CLI.
- Add the missing tool docs and tool discovery UX.
- Hardening the agent loop (timeouts, retries, observable traces).

> **Do not start MCP/agent work until #1–#4 are shippable.** Otherwise we
> will be patching agent behaviour against a moving browser target.

---

## What this fork contains

```
bramburn/BrowserOS/
├── packages/browseros/              # Chromium fork + build system (Python CLI)
│   ├── chromium_patches/            # ~342 patches layered on Chromium 146
│   ├── build/                       # Build orchestrator (setup / prep / build / sign / package)
│   └── resources/                   # Icons, entitlements, signing assets
│
├── packages/browseros-agent/        # Agent platform (TypeScript / Go)
│   ├── apps/
│   │   ├── server/                  # Bun MCP server + AI agent loop (port 9100)
│   │   ├── agent/                   # Browser extension UI (WXT + React)
│   │   ├── cli/                     # CLI tool (browseros-cli, Go)
│   │   ├── eval/                    # Benchmark framework
│   │   └── controller-ext/          # Chrome API bridge extension (source of bundled MCP server)
│   │
│   └── packages/
│       ├── agent-sdk/               # Node.js SDK
│       ├── cdp-protocol/            # CDP type bindings
│       └── shared/                  # Shared constants
│
├── docs/                            # Markdown docs (in-repo)
│   └── MCP_TOOL_SPEC.md             # Canonical 24-tool spec, captured live from port 9200
│
├── docs-site/                       # (planned) Docusaurus site for fork-specific docs
│
├── tools/
│   └── bramburn-build.ps1           # 5-phase Chromium-from-source orchestrator
│
├── AGENTS.md                        # Runbook for AI coding agents working on the fork
├── WHY_FORK.md                      # Motivation, scope, and what comes next
├── CONTRIBUTING.md                  # (upstream) — how to contribute
└── LICENSE                          # AGPL-3.0
```

The bundled MCP server (`browseros_server.exe`, 92 MB, version 0.0.165,
listens on `http://127.0.0.1:9200/mcp`) is **precompiled** into the shipped
BrowserOS app and its source is **not** in this public fork. The C++ side
that launches and supervises the subprocess
(`chrome/browser/browseros/server/`) **is** in this fork. See
[`AGENTS.md`](AGENTS.md) § "Bundled MCP server source" for the full
picture.

---

## Building this fork

| What | Where | Time | Notes |
|---|---|---|---|
| Bun MCP server | `packages/browseros-agent/apps/server` | 2-5 min | `bun install && bun run build` |
| Chromium browser + bundled MCP | `packages/browseros/` via `tools/bramburn-build.ps1` | 6-13 h wall, 150 GB disk | 5 phases: setup → prep → build → sign → package |
| Same Chromium build via CI | `.github/workflows/release-windows.yml` on the self-hosted runner | 6-13 h wall, ~5 min review | Produces the `.exe`, uploads to R2 + GitHub Release, tags `browseros-windows-v<version>` |

**Prerequisites** (verified on this host 2026-09-19): VS2022 Community,
Windows 10 SDK 10.0.26100, `depot_tools` (with ninja), Rust + cargo, Bun
1.4+, Python 3.12.

Full build instructions, the two execution paths (interactive vs detached),
and the per-phase wall-time estimates are in
[`AGENTS.md`](AGENTS.md) § "Build the Chromium browser".

## Releases and auto-update

Every release of the fork publishes two artifacts:

1. A signed `BrowserOS_v<version>_win-x64.exe` installer on R2 and a
   matching GitHub Release (tag `browseros-windows-v<version>`).
2. Two XML manifests on R2 that drive auto-update:
   - An **Omaha-4 update_check response** for Chromium's in-browser
     updater (`components/update_client`) — drives the
     "restart to update" toast.
   - A **Sparkle-style appcast RSS** for `browseros-cli` and
     external tools.
   - Plus a tiny JSON pointer for lightweight CLI checks.

The release pipeline:

```
release-windows.yml (self-hosted Windows runner)
  → bumps BROWSEROS_VERSION
  → runs bramburn-build.ps1 (5 phases, 6-13 h)
  → optional AIP wrap (wrap_with_aip: true)
  → uploads .exe to R2 + GitHub Release + tag
  → triggers update-manifest.yml

update-manifest.yml (ubuntu-latest)
  → downloads .exe from GitHub Releases
  → computes SHA-256
  → runs tools/release/generate_update_manifests.py
  → uploads update_check.xml, appcast.xml, latest.json to R2
```

See [`docs/CI_AND_RELEASES.md`](docs/CI_AND_RELEASES.md) for the full
pipeline, R2 layout, tag conventions, and manual fallback.
Rendered version at <https://bramburn.github.io/BrowserOS/>.

---

## Contributing

We contribute **upwards**: anything we land here that is genuinely an
improvement goes back to `browseros-ai/BrowserOS` as a PR. AGPL-3.0 applies
in both directions.

Before opening a PR upstream, please:

1. Open an issue here in `bramburn/BrowserOS` describing the change.
2. Land it in our fork first so we can run it against our internal users.
3. Then open the upstream PR with a note "this has been running in our
   fork for N weeks".

This protects upstream from being a guinea pig for our changes.

For **fork-only** work (build pipeline, update server config, Docusaurus
site, release tooling) just open a PR directly here.

---

## Why we forked (TL;DR)

| Upstream release | MCP server | Broke |
|---|---|---|
| 0.49.x | pre-165 | (baseline) |
| 0.50.5 | 0.0.165 | `evaluate` semantics; `window` → `windows`; `tabs.select` removed; new required `session` arg |

Every upstream release has cost us 30+ minutes of wrapper debugging. This
fork lets us ship backwards-compat shims locally, on our own cadence. See
[`WHY_FORK.md`](WHY_FORK.md) for the full story.

---

## License

AGPL-3.0, same as upstream. All patches contributed upstream must remain
AGPL-3.0.

```
Copyright © 2026 bramburn (and the BrowserOS contributors)
```
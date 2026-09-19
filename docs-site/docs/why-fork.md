---
title: Why fork
description: Motivation, scope, and what comes next for the bramburn/BrowserOS fork.
---

# Why we forked

Every BrowserOS release has broken our wrappers in `C:\dev\browser-cli`:

| BrowserOS | MCP server | Broke |
|---|---|---|
| 0.49.x | pre-165 | (baseline) |
| 0.50.5 | 0.0.165 | `evaluate` semantics; `window` → `windows`; `tabs.select` removed; new required `session` arg |

The 0.50.5 breakage took 30+ minutes of debugging before the wrappers
worked again. We expect the next release to break more.

## Why fork instead of waiting for fixes

1. **Self-host resilience** — having our own fork lets us ship
   backwards-compat shims locally instead of waiting for upstream.
2. **Build our own** — once we can build Chromium + the bundled MCP
   server ourselves, we control the release cadence.
3. **Upstream contribution** — any shims we add become PRs upstream.
   The fork is the staging ground.

## Scope

This fork is a **mirror of the public upstream**. We haven't yet:

- ❌ Built any artifacts from this source (no Chromium binary yet).
- ❌ Patched the bundled `browseros_server.exe` (its source isn't in
  this repo — it's precompiled into `BrowserOSServer/default/`).
- ❌ Forked the `browseros_server.exe` source into a public repo.

What we HAVE done:

- ✅ Mirror `bramburn/BrowserOS` from `browseros-ai/BrowserOS` (public).
- ✅ Documented the architecture, the breakage pattern, and the build
  path in `AGENTS.md`.
- ✅ Audited the host toolchain — VS2022 + Win10 SDK + depot_tools +
  Rust + Bun + Python are all present; Chromium build is viable.
- ✅ Captured the live MCP tool spec to `docs/MCP_TOOL_SPEC.md`.

See the raw [`WHY_FORK.md`](https://github.com/bramburn/BrowserOS/blob/main/WHY_FORK.md)
for the full version.
# Why we forked BrowserOS

**Date**: 2026-09-19
**Authors**: bramburn (via Mavis orchestrator)
**Scope**: public fork for self-hosted use + upstream-patch contribution

## Background

BrowserOS is a Chromium-based browser with built-in AI agent capabilities and a
JSON-RPC-over-HTTP MCP server on `127.0.0.1:9200/mcp` (the bundled MCP server).
We use it from `C:\dev\browser-cli` (Python Click CLI) to drive Perplexity,
Gemini, and Semrush automations.

Every BrowserOS release has broken the wrappers:

| BrowserOS | MCP server | Broke |
|---|---|---|
| 0.49.x | pre-165 | (baseline) |
| 0.50.5 | 0.0.165 | `evaluate` → `run` (Bun sandbox, no page-DOM); `window` → `windows`; `tabs.select` removed; new required `session` arg on every tool call |

The 0.50.5 breakage took 30+ minutes of debugging and a full source audit
before the wrappers worked again. We expect the next release to break more.

## Why fork instead of waiting for fixes

Three reasons:

1. **Self-host resilience** — having our own fork lets us ship
   backwards-compat shims locally instead of waiting for upstream.
2. **Build our own** — once we can build Chromium + the bundled MCP server
   ourselves, we control the release cadence and aren't pinned to
   browseros-ai's weekly-cadence breakage.
3. **Upstream contribution** — any shims we add become PRs upstream.
   The fork is the staging ground.

## Scope (what's in the fork vs not)

This fork is a **mirror of the public upstream**. We haven't yet:

- ❌ Built any artifacts from this source (no Chromium browser binary yet).
- ❌ Patched the bundled `browseros_server.exe` (its source isn't in this repo;
  see `AGENTS.md` § "Bundled MCP server source").
- ❌ Forked the `browseros_server.exe` source into a public repo.

What we HAVE done:

- ✅ Mirror `bramburn/BrowserOS` from `browseros-ai/BrowserOS` (public).
- ✅ Documented the architecture, the breakage pattern, and the build path
  in `AGENTS.md`.
- ✅ Audited the host toolchain — VS2022 + Win10 SDK + depot_tools + Rust + Bun
  + Python are all present; Chromium build is viable (but slow).

## What comes next

In order of priority:

1. **Decide on the bundled MCP server source**. Either:
   - (a) get browseros-ai to publish the bundled server source (open issue
     + PR against their public repo), OR
   - (b) reverse-engineer the controller CRX
     (`nlnihljpboknmfagkikhkdblbedophja.crx`) to extract the protocol,
   - (c) build a substitute `apps/server`-based MCP server that speaks the
     old protocol and translates to the new one.

2. **Start the Chromium build** (6-12 hours on this box).
   Use the `Start-Process -WindowStyle Hidden` pattern in `AGENTS.md`
   § "Recommended path" to keep it alive past the bash tool's 5-min timeout.
   Monitor via mavis cron every 30 min; report completion via the LLM.

3. **Land backwards-compat shims upstream**. Once we have patches, file
   PRs against `browseros-ai/BrowserOS` so the next BrowserOS release
   doesn't break wrappers again.

## License

This fork is AGPL-3.0 (same as upstream). All patches contributed upstream
must remain AGPL-3.0.

## Contacts

- bramburn (GitHub: @bramburn)
- Upstream: browseros-ai

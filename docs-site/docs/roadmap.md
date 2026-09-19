---
title: Roadmap
description: Strategic priorities for the bramburn/BrowserOS fork.
---

# Strategic roadmap

We build **bottom-up**. The goal is a fork that ships like Edge or Chrome:
**boring, automatic, never blocks you**. Everything else comes after that
foundation is solid.

## #1 — Auto-update like Edge / Chrome *(highest priority)*

> "We can ship fast only once the update story works end-to-end."

Right now BrowserOS ships updates out-of-band (manual download from
`files.browseros.com`) — that's not shippable. We want the same UX you
get from Edge or Chrome:

- Background download while the browser is in use.
- "Update available — restart to update" toast on a quiet moment.
- Restart-and-keep-tabs so the user never loses context.
- Signed delta updates — small payloads, fast apply.
- Staged rollout — internal → beta → stable, with kill-switch via a
  server config we control.
- Update self-host — point our fork's update URL at our own R2 bucket.

The Chromium update machinery (`components/update_client`, Omaha 4
protocol) is already wired into the fork — it just points at upstream's
URL and signing keys. The work is:

1. Replace the update URL with our own R2 bucket.
2. Sign our own payloads with a fork-owned key.
3. Patch the toast + restart flow to match Edge/Chrome UX.
4. End-to-end test: ship v1 → ship v2 to update server → click
   "restart to update" → restart into v2 with tabs intact.

This is the **gating dependency** for everything below.

## #2 — Browser robustness

- Crash reporter wired to our own backend.
- 24h soak test on a clean profile passes without a crash.
- Session restore proven across hard kills.
- Memory: no leaks past 24h of normal use.
- Renderer sandbox escapes caught and reported.
- Reproducible builds.

## #3 — Installation UX / UI

- First-run wizard with Chrome/Edge/Firefox data import.
- Real "About" / "What's new" page driven by release notes.
- Honest uninstall.
- Telemetry opt-in / opt-out that is clear and one click.
- Optional [Advance Installer Pro](advance-installer) layer for
  polished branding and prerequisites (see its dedicated page).

## #4 — This Docusaurus site *(you are here)*

Docusaurus 3 classic. Lives in `docs-site/` in this repo. Mirrors the
existing `AGENTS.md`, `WHY_FORK.md`, and `docs/MCP_TOOL_SPEC.md` into
the content tree. Deployed automatically by CI on every merge to `main`.

## #5 — MCP + agent hardening *(deferred)*

The bundled MCP server (`browseros_server.exe`) and the Bun MCP server
both work today, but we have a known breakage history on every upstream
release. Once #1–#4 are shippable, we tackle:

- Pin the JSON-RPC schema with a versioned, backwards-compat shim.
- Add the missing tool docs and tool discovery UX.
- Harden the agent loop (timeouts, retries, observable traces).

> **Do not start MCP/agent work until #1–#4 are shippable.** Otherwise
> we will be patching agent behaviour against a moving browser target.
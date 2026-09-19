---
slug: /
title: bramburn/BrowserOS
sidebar_position: 1
description: A self-hostable Chromium fork with an Edge/Chrome-grade auto-update workflow.
---

# bramburn/BrowserOS

This is a public fork of [`browseros-ai/BrowserOS`](https://github.com/browseros-ai/BrowserOS)
maintained for **self-hosted use** and **release-cadence resilience**. We use
BrowserOS as the runtime for our own browser-automation CLI, and every
upstream release has historically broken our wrappers within a week. This
fork exists so we can ship backwards-compat shims, ship our own updates on
our own cadence, and contribute the patches back to upstream.

## Strategic roadmap (in order)

| # | Priority | Status |
|---|---|---|
| **1** | **Auto-update like Edge/Chrome** | Gating — we cannot ship fast until this works. |
| 2 | Browser robustness / stability | Crash reporter + 24h soak + session restore. |
| 3 | Installation UX/UI | First-run wizard + data import + About page. |
| 4 | This Docusaurus site | **You are here.** |
| 5 | MCP + agent hardening | Deferred until #1–#4 shippable. |

See the [Roadmap](roadmap) page for the full version with concrete
deliverables per priority.

## Where to start

- **I'm new to the fork** — read [WHY_FORK](why-fork).
- **I want to build from source** — see [Build](build).
- **I want to publish a release** — see [Release](release).
- **I want to understand the auto-update flow** — see [Update server](update-server).
- **I want to extend the agent / MCP layer** — see [MCP tool spec](mcp-tool-spec).

## Quick links

- [GitHub](https://github.com/bramburn/BrowserOS)
- [Releases](https://github.com/bramburn/BrowserOS/releases)
- [WHY_FORK.md (raw)](https://github.com/bramburn/BrowserOS/blob/main/WHY_FORK.md)
- [AGENTS.md (raw)](https://github.com/bramburn/BrowserOS/blob/main/AGENTS.md)
---
title: Agent layer (overview)
description: Pointer to the canonical architecture map. The detailed agent-layer docs live in AGENTS.md and AGENTS-architecture.md.
sidebar_position: 4
---

# Agent layer (overview)

The canonical reference for the BrowserOS agent architecture is
[`AGENTS-architecture.md`](https://github.com/bramburn/BrowserOS/blob/main/AGENTS-architecture.md)
(in-repo) — start there for the layered map (L0–L6), data flow,
opinionated rules (R1–R14), and key integration seams.

For the Chromium-fork build, see
[`AGENTS-build.md`](https://github.com/bramburn/BrowserOS/blob/main/AGENTS-build.md).

Sub-package views:

- [`packages/browseros/AGENTS.md`](https://github.com/bramburn/BrowserOS/blob/main/packages/browseros/AGENTS.md) — Chromium fork.
- [`packages/browseros-agent/AGENTS.md`](https://github.com/bramburn/BrowserOS/blob/main/packages/browseros-agent/AGENTS.md) — Bun monorepo.
- [`packages/browseros-agent/apps/server/AGENTS.md`](https://github.com/bramburn/BrowserOS/blob/main/packages/browseros-agent/apps/server/AGENTS.md) — MCP server.
- [`packages/browseros-agent/apps/agent/AGENTS.md`](https://github.com/bramburn/BrowserOS/blob/main/packages/browseros-agent/apps/agent/AGENTS.md) — extension.

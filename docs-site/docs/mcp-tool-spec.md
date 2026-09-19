---
title: MCP tool spec
description: Canonical tool spec captured live from the bundled MCP server on port 9200.
---

# MCP tool spec (fork-specific)

This page covers **fork-specific additions and changes** to the
bundled MCP server's tool surface. For the canonical full schema
(24 tools, JSON-RPC), see
[`docs/MCP_TOOL_SPEC.md`](https://github.com/bramburn/BrowserOS/blob/main/docs/MCP_TOOL_SPEC.md)
in the repo root.

## Tool inventory (24 tools)

Captured live from `http://127.0.0.1:9200/mcp` on 2026-09-19:

### Browser control (15)

- `tabs` — `list | active | new | close | update`
- `tab_groups`
- `history`
- `navigate`
- `windows` — `list | create | close | activate`
- `snapshot` — full accessibility tree
- `diff` — page diff
- `act` — `click | fill | press | type | hover | scroll | focus | check | uncheck | select | click_at | hover_at | drag_at | type_at`
- `read` — Markdown / text / links extraction (CSS selector)
- `grep` — lightweight snapshot subset
- `screenshot`
- `wait`
- `pdf`
- `download`
- `upload`

### JS execution (2 — TWO tools, NOT renames)

- **`evaluate(page, code)`** — page-context JS. Has `document`,
  `window`. Use for DOM queries.
- **`run(code)`** — Bun sandbox JS. No DOM. Use for server-side
  computation.

The two are **separate tools**, not renames. Earlier confusion in
the fork's `browser-cli` wrappers treated them as renames; this was
corrected on 2026-09-19 and the wrappers now route by context.

### Klavis Strata (7)

- `connector_mcp_servers`
- `discover_server_categories_or_actions`
- `get_category_actions`
- `get_action_details`
- `execute_action`
- `search_documentation`
- `handle_auth_failure`

## Required `session` argument

Every tool call returns `_meta.com.browseros/session` (opaque UUID).
Pass it back as the `session` argument on every subsequent call to
keep CDP context / tab ownership stable.

The schema marks `session` as optional, but it's effectively
required for any call beyond the first. The `browser-cli` MCP client
captures the session automatically and re-injects it on every
`call_tool`.

## Fork-specific additions (planned)

When MCP work is unblocked (after priorities #1–#4 ship):

- A versioned, backwards-compat shim for the JSON-RPC surface.
- A `windows_check_update` tool that polls the
  [Update server](update-server) and returns `{ update_available,
  version, url }`.
- A `cli_status` tool that exposes the MCP server's own health
  metrics for `browseros-cli --check-health`.

These will be documented here as they're added.
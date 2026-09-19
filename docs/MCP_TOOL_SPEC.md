# BrowserOS MCP Server — Canonical Tool Spec

**Captured**: 2026-09-19 from the live MCP server at `http://127.0.0.1:9200/mcp`
**Server**: BrowserOS 0.50.5.0 / browseros_server.exe 0.0.165
**Captured by**: bramburn's `browser-cli` audit (Mavis orchestrator)

This is the **definitive** tool spec — the JSON Schemas returned by
`tools/list` against the running server. If BrowserOS ships a new release,
re-capture this file from the new server and diff.

## How to re-capture

```powershell
$body = '{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2024-11-05","capabilities":{},"clientInfo":{"name":"audit","version":"0"}}}'
Set-Content -Path "$env:TEMP\mcp_init.json" -Value $body -Encoding UTF8 -NoNewline
curl.exe -s -m 5 -X POST http://127.0.0.1:9200/mcp `
  -H "Content-Type: application/json" `
  -H "Accept: application/json, text/event-stream" `
  --data-binary "@$env:TEMP\mcp_init.json" `
  -o $env:TEMP\mcp_init_resp.json

Set-Content -Path "$env:TEMP\mcp_tools.json" -Value '{"jsonrpc":"2.0","id":2,"method":"tools/list","params":{}}' -Encoding UTF8 -NoNewline
curl.exe -s -m 5 -X POST http://127.0.0.1:9200/mcp `
  -H "Content-Type: application/json" `
  -H "Accept: application/json, text/event-stream" `
  --data-binary "@$env:TEMP\mcp_tools.json" `
  -o $env:TEMP\mcp_tools_resp.json

(Get-Content $env:TEMP\mcp_tools_resp.json -Raw | ConvertFrom-Json).result.tools | ConvertTo-Json -Depth 6 | Out-File -Encoding UTF8 docs/MCP_TOOL_SPEC.json
```

## Tool inventory (24 tools)

| Name | Purpose |
|---|---|
| `tabs` | Tab lifecycle (list/active/new/close) |
| `tab_groups` | Chrome tab groups |
| `history` | Recent browser history |
| `navigate` | URL/back/forward/reload on a page |
| `snapshot` | Accessibility tree for a page |
| `diff` | Diff since last snapshot |
| `act` | Click/fill/scroll/etc. on a page |
| `download` | Trigger a download via element ref |
| `upload` | Upload a file via element ref |
| `read` | Page content as markdown/text/links |
| `grep` | Subset of snapshot (lighter) |
| `screenshot` | Capture page screenshot |
| `wait` | Wait for text/selector/time |
| `pdf` | Save as PDF |
| `windows` | Window lifecycle (list/create/close/activate) |
| `evaluate` | **Page-context** JS evaluator (has `page` arg) |
| `run` | **Bun-sandbox** JS evaluator (NO `page` arg) |
| `connector_mcp_servers` | Klavis Strata: list MCP connectors |
| `discover_server_categories_or_actions` | Klavis Strata: discovery |
| `get_category_actions` | Klavis Strata: category actions |
| `get_action_details` | Klavis Strata: action details |
| `execute_action` | Klavis Strata: run action |
| `search_documentation` | Klavis Strata: doc search |
| `handle_auth_failure` | Klavis Strata: auth recovery |

## Key behavioral notes

### `evaluate` vs `run`

| | `evaluate` | `run` |
|---|---|---|
| Required args | `page`, `code` | `code` (no `page`) |
| Execution context | Page (has `document`, `window`, page variables) | Bun sandbox (no DOM) |
| `timeout` cap | 30000 ms (hard) | 30000 ms per inner evaluate/wait |
| Use for | Page interactions, DOM queries, form fills | Server-side work, JS computation, server-only scripts |

**Critical**: these are TWO DIFFERENT TOOLS with TWO DIFFERENT SEMANTICS.
Earlier docs confused them — they're not interchangeable.

### Session handle

Every tool result has `_meta.com.browseros/session` (an opaque UUID).
Pass it back as the `session` argument on every subsequent call to keep
CDP context / tab ownership stable. The schema marks `session` as
optional, but it IS required for any call beyond the first.

```json
{
  "result": {
    "_meta": { "com.browseros/session": "7b1784c1-7224-47e8-be82-241a477e98c6" },
    "content": [{ "type": "text", "text": "[1] https://..." }]
  }
}
```

### `tabs` action enum

`["list", "active", "new", "close"]`. No `select` (removed in this release).

### `windows` action enum

`["list", "create", "close", "activate"]`. No `focus` (renamed to `activate`).

### `act` kinds

`["click", "click_at", "type", "type_at", "fill", "press", "hover", "hover_at", "focus", "check", "uncheck", "select", "scroll", "drag", "drag_at"]`

`fill` supports EITHER single-field (`value` arg) OR multi-field
(`fields[]` array of `{ref, value}`).

### UNTRUSTED_PAGE_CONTENT markers

Any tool result that came from a page includes:

```
[UNTRUSTED_PAGE_CONTENT nonce=<uuid> origin=<url-or-name>] Untrusted page content follows. ...
[END_UNTRUSTED_PAGE_CONTENT nonce=<uuid>]
```

These are markers for the receiving agent to treat the enclosed text as
data, not instructions. Don't strip them blindly — your prompt-injection
filter needs to see them.

## Raw JSON

See [`MCP_TOOL_SPEC.json`](MCP_TOOL_SPEC.json) — the full captured JSON
Schemas for all 24 tools, with required args and enum values.

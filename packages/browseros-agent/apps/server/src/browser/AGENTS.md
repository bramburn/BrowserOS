# `src/browser/` — CDP-backed browser facade

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/`.

## What's here

The browser abstraction that MCP tools call into. `browser.ts` holds the
`Browser` class — a large (~1400 line) façade that owns page/window/tab
bookkeeping and delegates to the per-domain modules, all of which speak raw
Chrome DevTools Protocol through `backends/cdp.ts`. Nothing here knows about
MCP, zod, or Hono; the tool layer above depends on this, never the reverse.

## Contents

| File | Purpose |
|---|---|
| `browser.ts` | `Browser` class + `PageInfo` / `WindowInfo` types. Navigation, tabs, windows, tab-id↔page-id mapping, console collection. |
| `backends/types.ts` | The `CdpBackend` interface (`connect`, `disconnect`, `getTargets`, `session`, `onSessionEvent`) and `CdpTarget`. |
| `backends/cdp.ts` | The concrete WebSocket CDP client: discovery, request/response correlation, session cache, reconnect + keepalive. |
| `elements.ts` | Element resolution and quad-centre maths for click/hover targeting. |
| `snapshot.ts` | Accessibility-tree snapshot builder (`AXNode`). |
| `dom.ts` | CDP DOM node → attribute parsing, `DomSearchResult`. |
| `content-markdown.ts` | `buildContentMarkdownExpression` — in-page JS that turns a page into markdown. |
| `console-collector.ts` | Buffers `Runtime.consoleAPICalled` / `Log.entryAdded` events for `get_console_logs`. |
| `keyboard.ts` | Key event synthesis. |
| `mouse.ts` | Pointer/mouse event synthesis. |
| `history.ts` | `HistoryEntry` + Chrome history queries. |
| `bookmarks.ts` | `BookmarkNode` + bookmark tree queries. |
| `tab-groups.ts` | `TabGroup` + tab-group CDP operations. |

## Rules

**B1 — `backends/types.ts` is the interface, `backends/cdp.ts` is the only
implementation.** `browser.ts` imports the *type* from `types.ts`. A second
backend must satisfy that interface rather than reach into the WebSocket
implementation.

**B2 — Go through `Browser`, never through CDP from a tool.** Tools receive a
`ToolContext` whose `browser` field is a `Browser`. Page-id ↔ tab-id mapping,
window resolution, and the console collector are `Browser`'s job; a tool that
pokes `Page.*` directly will break the mapping.

**B3 — CDP types come from `@browseros/cdp-protocol`.** Import
`ProtocolApi`, `CdpNode`, `ConsoleAPICalledEvent`, etc. from that workspace
package. Never hand-roll protocol shapes or reach for `any` on a CDP payload.

**B4 — Connect to a live Chromium.** `main.ts` calls `new CdpBackend({ port:
config.cdpPort }).connect()` and exits if `cdpPort` is null. There is no
in-process browser; `config.ts` treats `--cdp-port` as mandatory at startup.

**B5 — Ports and limits come from `@browseros/shared`.** `cdp.ts` pulls
`TIMEOUTS` and `CDP_LIMITS` from `constants/timeouts` and `constants/limits`.
No literals.

**B6 — Page-context JS lives in `content-markdown.ts`, not scattered.** When
you need to run code in the page, add it to the expression builders here so it
is reviewable as one place rather than inlined into a tool handler.

## Workflows

**Adding a new browser capability:** 1. Add the CDP calls to the matching
domain module (`elements.ts`, `history.ts`, …). 2. Expose a method on the
`Browser` class in `browser.ts`. 3. Call it from a tool in `../tools/`. 4. Test
via `tests/tools/` with `withBrowser()` (real Chromium required).

**Changing CDP reconnection behaviour:** edit `backends/cdp.ts` — it owns the
pending-request map, `sessionCache`, keepalive interval, and the
`exitOnReconnectFailure` policy — then re-run `tests/browser/backends/cdp.test.ts`,
which drives it with a `MockWebSocket` and needs no real browser.

**Running the backend tests:** `bun run test:browser` (alias `bun run test:cdp`)
from `apps/server/`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `apps/server/` MCP server internals.
- [`../tools/AGENTS.md`](../tools/AGENTS.md) — the tool layer that calls this.
- [`../../tests/browser/AGENTS.md`](../../tests/browser/AGENTS.md) — CDP backend tests.
- [`../../../../docs/MCP_TOOL_SPEC.md`](../../../../../../docs/MCP_TOOL_SPEC.md) — surface comparison against the port 9200 server.

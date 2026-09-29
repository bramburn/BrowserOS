# `src/tools/` — MCP tool definitions

> Part of [`../../AGENTS.md`](../../AGENTS.md) in `apps/server/src/`.

## What's here

Every browser-automation tool exposed over the MCP protocol, plus the
framework that binds them. `framework.ts` defines the `ToolDefinition`
contract and `executeTool()`; `registry.ts` is the canonical list of the
**61 registered tools**; `tool-registry.ts` holds the de-duplicating
`ToolRegistry` class; `response.ts` provides the `ToolResponse` builder
(text / image / structured data / post-actions) that every handler writes
into. Individual tool files group as: navigation, snapshot, dom, input,
page-actions, windows, bookmarks, history, tab-groups, console,
browseros-info, nudges. Two subfolders: `acl/` (reference-only scoring, not
wired into the runtime) and `filesystem/` (AI-SDK toolset for the agent
loop, **not** part of the MCP registry).

## Contents

```
tools/
├── framework.ts             ← ToolDefinition, ToolContext, defineTool(), executeTool()
├── registry.ts              ← CANONICAL list: `export const registry = createRegistry([...])`
├── tool-registry.ts         ← ToolRegistry class: throws on duplicate names
├── tool-label-registry.ts   ← tool name → human activity label for the chat UI
├── response.ts              ← ToolResponse / ToolResult / ContentItem
├── navigation.ts            ← get_active_page, list_pages, navigate_page, new_page,
│                              new_hidden_page, show_page, move_page, close_page, wait_for*
├── snapshot.ts              ← take_snapshot, take_enhanced_snapshot, get_page_content,
│                              get_page_links, take_screenshot, evaluate_script
├── dom.ts                   ← get_dom, search_dom
├── input.ts                 ← 17 tools: click/hover/type/drag/focus/fill/check/uncheck/
│                              upload_file/press_key/scroll/handle_dialog/select_option/clear
├── page-actions.ts          ← save_pdf, save_screenshot, download_file
├── windows.ts               ← list/create/create_hidden/close/activate/set_visibility
├── bookmarks.ts             ← get/create/remove/update/move/search_bookmarks
├── history.ts               ← search/get_recent/delete_history_url/delete_history_range
├── tab-groups.ts            ← list/group/update/ungroup/close_tab_group
├── console.ts               ← get_console_logs
├── browseros-info.ts        ← browseros_info
├── nudges.ts                ← suggest_schedule, suggest_app_connection
├── ElementProperties        ← EMPTY FILE (0 bytes). Dead placeholder.
├── acl/                     ← reference-only ACL scorer (see acl/README.md)
└── filesystem/              ← AI-SDK ToolSet for the agent loop, not MCP-registered
```

`*` `wait_for` is imported and commented out of the registry array
("temporarily disabled") — it exists in code but is not served.

## Rules

**T1 — `registry.ts` is the canonical tool list.** A tool not listed in the
`createRegistry([...])` array is unreachable over MCP. When adding one, add
the import *and* the array entry. `ToolRegistry` throws
`Duplicate tool name` on a collision, so names are globally unique.

**T2 — Every tool is built with `defineTool()` from `framework.ts`.** The
contract is: `name`, `description`, zod `input` (optional zod `output`), and
an `async handler(args, ctx, response)`. Never return a value from the
handler — write into the `ToolResponse` it is handed.

**T3 — Report failures through `response.error(msg)`, not by throwing.**
`executeTool()` catches throws and wraps them as
`Internal error in <tool>: ...`; using `response.error()` gives the agent a
domain-level message. An aborted signal is checked before the handler runs.

**T4 — `tool-label-registry.ts` must cover every new tool.** It is the
editorial layer that turns snake_case tool names into user-facing activity
strings ("Opened tab", "Captured page snapshot"). A missing entry means a
blank activity row in the chat UI.

**T5 — Filesystem tools are a separate, non-MCP surface.** `filesystem/` builds
a Vercel AI SDK `ToolSet` consumed only by `agent/ai-sdk-agent.ts`. Adding a
filesystem tool does **not** require a `registry.ts` entry, and adding an MCP
tool never touches `build-toolset.ts`.

**T6 — `ElementProperties` is a 0-byte file.** It is a leftover placeholder,
not a module. Don't import it; if you need element-property helpers they
belong in `../browser/elements.ts`.

**T7 — `acl/` is reference-only.** Nothing in the server runtime may import
it. Its own `README.md` states enforcement was removed.

## Workflows

**Adding a browser tool:** 1. Add/extend a file in this directory with
`defineTool({ name, description, input: z.object({...}), handler })`.
2. Import it in `registry.ts` and add it to the correctly-commented category
array (T1). 3. Add a verb to `VERB_OVERRIDES` in `tool-label-registry.ts` (T4).
4. Add `tests/tools/<tool>.test.ts` using `withBrowser()` from
`tests/__helpers__`. 5. `bun run test:tools`.

**Disabling a tool without deleting it:** keep the export in its own file,
keep the import in `registry.ts` marked with a `biome-ignore
lint/correctness/noUnusedImports` comment, and comment the array entry out —
exactly how `wait_for` is handled today.

**Reading what the agent actually sees:** `response.ts` post-actions
(`snapshot` / `screenshot` / `pages`) are what refresh context after a
mutation. When adding a tool that changes the page, queue the matching
post-action instead of telling the model to re-snapshot.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `apps/server/` MCP server internals.
- [`../browser/AGENTS.md`](../browser/AGENTS.md) — the CDP layer these tools drive.
- [`./acl/AGENTS.md`](./acl/AGENTS.md) — reference-only ACL scorer.
- [`./filesystem/AGENTS.md`](./filesystem/AGENTS.md) — agent-loop filesystem tools.
- [`../../../../docs/MCP_TOOL_SPEC.md`](../../../../../../docs/MCP_TOOL_SPEC.md) — tool schemas captured from the *port 9200* server (different protocol, similar surface). Use it to reason about naming drift, not as a schema source of truth.

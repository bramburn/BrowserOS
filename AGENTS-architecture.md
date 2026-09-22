# BrowserOS — Opinionated Architecture

> Repository-wide map of the BrowserOS monorepo. Read this first.
> Companion to [`AGENTS.md`](AGENTS.md) (strategic context), [`AGENTS-build.md`](AGENTS-build.md)
> (build pipeline), and the package-local `AGENTS.md`s in
> [`packages/browseros/`](packages/browseros/AGENTS.md) and
> [`packages/browseros-agent/`](packages/browseros-agent/AGENTS.md).
>
> **Captured 2026-09-22** from the `bramburn/BrowserOS` fork at Chromium
> `148.0.7778.97` (`6b3fa66a92`) + Bun MCP server `0.0.165` + WXT extension `~0.0.99`.

## TL;DR

BrowserOS is a **Chromium 148 fork** with a **Bun MCP server** baked into
the binary (via a precompiled `browseros_server.exe` step) and a **WXT
browser extension** that gives the user a sidebar / new-tab agent UI.
User intent (natural-language prompt) → extension chat panel → MCP tool
calls → Chromium CDP → real browser actions.

```
┌───────────────────────────────────────────────────────────────────────┐
│ BrowserOS.exe (Chromium 148 + BrowserOS C++ patches) │
│ ┌──────────────────────────────────────────────────────────────────┐ │
│ │ C++ wrapper: chrome/browser/browseros/server/ │ │
│ │ • browseros_server_manager.cc — spawns the bundled MCP server │ │
│ │ • browseros_server_proxy.cc — IPC to the subprocess │ │
│ │ • process_controller_impl.cc — health checks, restart │ │
│ └──────────────────────────────────────────────────────────────────┘ │
│ │ spawns (at browser startup) │
│ ▼ │
│ browseros_server.exe ◄── precompiled, NOT in this fork's source │
│ listens on http://127.0.0.1:9200/mcp (JSON-RPC) │
└───────────────────────────────────────────────────────────────────────┘
 ▲ ▲
 │ │ MCP via http://127.0.0.1:9100/mcp
 │ │ (different transport — Hono + SSE)
 │ │
┌─────────────────────────────────────────────────────────────────────┐
│ packages/browseros-agent/ (Bun + TypeScript monorepo) │
│ ┌────────────────────────────────────┐ ┌────────────────────────┐ │
│ │ apps/server (Bun MCP server) │ │ apps/agent (WXT ext) │ │
│ │ • 24+ tools: snapshot, dom, │ │ • entrypoints: sidepanel│ │
│ │ navigation, page-actions, input, │ │ newtab, app/*, bg, │ │
│ │ history, bookmarks, console, etc. │ │ onboarding, glow │ │
│ │ • AI agent loop (Gemini/Anthropic │ │ • UI: chat, sidebar, │ │
│ │ adapter, prompt builder, │ │ scheduled-tasks, │ │
│ │ session store, tool adapter) │ │ llm-hub, mcp-settings │ │
│ │ • Hono HTTP server (9200/9100/ │ │ • graphql schema │ │
│ │ agentPort/extensionPort) │ │ • lib: 30+ features │ │
│ │ • Drizzle + SQLite (browseros- │ │ • Storage: chrome. │ │
│ │ dir state, sessions, prompts) │ │ storage.local │ │
│ └────────────────────────────────────┘ └────────────────────────┘ │
│ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌──────────┐ ┌──────────┐ │
│ │cli (Go) │ │eval (TS)│ │shared/ │ │build- │ │cdp- │ │
│ │browseros│ │benchmark│ │constants│ │tools/ │ │protocol/ │ │
│ │CLI v2+ │ │harness │ │types │ │helpers│ │CDP types │ │
│ └─────────┘ └─────────┘ └─────────┘ └──────────┘ └──────────┘ │
└───────────────────────────────────────────────────────────────────────┘
```

## Layered architecture

| Layer | Lives at | Language | What it does |
|---|---|---|---|
| **L0 Browser binary** | `packages/browseros/` Chromium checkout + patches | C++ | Vanilla Chromium 148 with BrowserOS-specific source patches applied |
| **L1 Bundled MCP server** | `browseros_server.exe` (precompiled, downloaded) | C++ / Bun-compiled single-binary (NEW) / old | JSON-RPC over HTTP/9200 — exposed to wrappers like `browser-cli` |
| **L2 Open-source MCP server** | `packages/browseros-agent/apps/server` | TypeScript / Bun | Modern implementation: 24+ tools, AI agent loop, Hono HTTP, Drizzle/SQLite. Listens on 9100. |
| **L3 Browser extension** | `packages/browseros-agent/apps/agent` | TypeScript / React / WXT | Sidepanel, newtab, ai-settings, mcp-settings, scheduled-tasks, agent-command. Sends prompts to L2. |
| **L4 LLM provider** | (external) | HTTP | Gemini, Anthropic, OpenAI, OpenRouter, LMStudio, Ollama — configured per user |
| **L5 CLI** | `packages/browseros-agent/apps/cli` | Go | `browseros` CLI for power users; uses L2's MCP via stdio |
| **L6 Eval / benchmarks** | `packages/browseros-agent/apps/eval` | TypeScript | LLM-judged ACL scoring, monitoring |

L1 + L2 both serve the same MCP protocol but are different products.
L1 is what ships inside BrowserOS.exe for end users. L2 is what runs in
development against the latest tools (and what `browseros-cli`
typically talks to in 2026+).

## Repo map

```
C:/dev/BrowserOs/
├── AGENTS.md ← strategic context, top-level file map
├── AGENTS-architecture.md ← THIS FILE (opinionated architecture)
├── AGENTS-build.md ← build pipeline (gclient, browseros CLI, release)
├── AGENTS-toolchain.md ← Ubuntu 24.04 toolchain reference
├── AGENTS-ubuntu-dev.md ← dev workflow via SSH
├── README.md, CONTRIBUTING.md, CLAUDE.md ← entrypoints
├── WHY_FORK.md ← fork motivation + scope
├── docs/ ← in-repo developer references (CI_AND_RELEASES, MCP_TOOL_SPEC)
├── docs-site/ ← Docusaurus render (renders docs/, plus docs-site/docs/)
├── installer/ ← AIP advanced installer template (Caphyon)
├── packages/
│ ├── browseros/ ← Chromium fork (Python build CLI + chromium_patches/)
│ │ ├── AGENTS.md ← sub-package view
│ │ ├── build/ ← Python CLI: browseros build / dev / release / ota / upload
│ │ ├── chromium_patches/ ← 342 patches layered onto vanilla Chromium
│ │ ├── chromium_files/ ← NEW files dropped on top of Chromium
│ │ ├── series_patches/ ← GNU Quilt-style ordered patches
│ │ ├── resources/ ← icons, branding, entitlements, BROWSEROS_VERSION
│ │ └── tools/patch ← BrowserOS patch CLI
│ └── browseros-agent/ ← Bun monorepo (TypeScript server, WXT extension)
│ ├── AGENTS.md ← sub-package view
│ ├── apps/
│ │ ├── server/ ← Bun MCP server (the modern implementation)
│ │ │ ├── AGENTS.md ← sub-package view
│ │ │ ├── src/tools/ ← 24+ MCP tools (registry.ts is the catalog)
│ │ │ ├── src/agent/ ← AI agent loop (Gemini adapter, prompts, sessions)
│ │ │ ├── src/api/ ← Hono HTTP routes
│ │ │ ├── src/browser/ ← CDP client (Browser/Backends)
│ │ │ ├── src/lib/db/ ← Drizzle + SQLite
│ │ │ └── tests/ ← fixtures, helpers, integration
│ │ ├── agent/ ← WXT extension
│ │ │ ├── AGENTS.md ← sub-package view
│ │ │ ├── entrypoints/ ← background, content, app/*, newtab, sidepanel, onboarding
│ │ │ ├── components/ ← React UI (ai-elements, chat, sidebar, theme…)
│ │ │ ├── lib/ ← 30+ feature modules (auth, chat, mcp, llm-hub…)
│ │ │ └── schema/schema.graphql ← GraphQL schema
│ │ ├── cli/ ← Go CLI (browseros v2+)
│ │ └── eval/ ← eval/bench harness
│ ├── packages/
│ │ ├── shared/ ← TS constants & types
│ │ ├── build-tools/ ← build helpers
│ │ └── cdp-protocol/ ← Chrome DevTools Protocol type defs
│ └── docs/ ← additional package docs
├── signatures/ ← code-signing artifacts for releases
├── tools/
│ ├── bramburn-build.ps1 ← Windows host orchestrator
│ ├── ubuntu-build.sh ← Ubuntu LAN host orchestrator
│ ├── ubuntu-launch.sh ← Ubuntu detached-launch wrapper
│ └── release/ ← Omaha-4 + appcast + JSON pointer generators
├── .github/workflows/ ← CI (Ubuntu hosted + self-hosted Win/Mac)
└── lefthook.yml, biome.json, .gitignore, …
```

## Opinionated rules — read these before editing

These apply across the whole repo. They live here, not in sub-package
docs, because they're cross-cutting.

### Rule R1 — One runtime per app
- **MCP server** runs on Bun (`apps/server`). Never Node. `apps/server/src/index.ts` exits early if `Bun === undefined`.
- **Extension** is WXT + React + TypeScript. HMR via `bun run dev:watch`.
- **Browser fork** builds with Chromium's own toolchain (gn/ninja).

### Rule R2 — No magic numbers anywhere outside `@browseros/shared`
All shared constants go in `packages/browseros-agent/packages/shared/`:
- `constants/ports.ts` → `DEFAULT_PORTS`, `TEST_PORTS`
- `constants/timeouts.ts` → `TIMEOUTS`
- `constants/limits.ts` → `RATE_LIMITS`, `AGENT_LIMITS`
- `constants/urls.ts` → `EXTERNAL_URLS`
- `constants/paths.ts` → `PATHS`
- `constants/exit-codes.ts` → `EXIT_CODES`
- `types/logger.ts` → `LoggerInterface`, `LogLevel`

If you find yourself typing `2000`, `30000`, or `localhost` in the
server/extension, import from `@browseros/shared` instead.

### Rule R3 — No `index.ts` in apps/ or packages/
Don't create or export from `index.ts` — it inflates the bundle with
all transitive exports. Export from named files (e.g. `logger.ts`,
`ports.ts`) and import directly: `import { X } from '@browseros/shared/logger'`.

### Rule R4 — File naming: kebab-case
| Type | Convention | Example |
|---|---|---|
| Multi-word files | kebab-case | `gemini-agent.ts`, `mcp-context.ts` |
| Single-word files | lowercase | `types.ts`, `browser.ts`, `index.ts` (where unavoidable) |
| Test files | `.test.ts` suffix | `mcp-context.test.ts` |
| Folders | kebab-case | `rate-limiter/`, `browser-tools/` |

Classes stay PascalCase but live in kebab-case files:
```typescript
// file: gemini-agent.ts
export class GeminiAgent { ... }
```

### Rule R5 — Logging
No `[prefix]` tags in log messages (e.g. `[Config]`, `[HTTP]`). Sentry/dev-mode source tracking adds `file:line:function` automatically.

### Rule R6 — Comments: minimal
Only comment non-obvious logic, complex algorithms, or critical
warnings. Skip for self-explanatory function names.

### Rule R7 — Extensionless imports (Bun TS)
Bun resolves `.ts` automatically. **Never** use `.js` extensions in imports.

### Rule R8 — Bun test, not Jest/Vitest
`bun test` everywhere. Test files live next to `apps/server/tests/`.

### Rule R9 — Chromium fork uses Python, not Bun
`packages/browseros/build/` is a Python CLI (Typer-based). Don't
reimplement it in TS. Edit `features.yaml`, `chromium_patches/`, and
Python modules. Run `bun run dev` is the TS monorepo — `browseros dev`
is the Chromium side.

### Rule R10 — MCP tools are catalogued
Every tool is registered in exactly one place:
`apps/server/src/tools/registry.ts`. That file is the canonical list.
**Before opening a PR for a new tool, that file is updated.** Don't
auto-discover tools via glob; keep the registry explicit.

### Rule R11 — Patch edits land in `chromium_patches/`, mirrored to Chromium tree path
If you fix or feature in `chrome/browser/foo/bar.cc`, the patch lives at
`packages/browseros/chromium_patches/chrome/browser/foo/bar.cc` and is
listed in `packages/browseros/build/features.yaml` under the relevant
feature block. `browseros dev annotate` converts in-tree edits back
into patch commits.

### Rule R12 — Drizzle migrations folder is checked in
All DB schema changes go through
`apps/server/src/lib/db/migrations/*.sql` (Drizzle-generated), with
`apps/server/src/lib/db/schema/<table>.ts` as the source of truth.

### Rule R13 — Hono, not Express
HTTP server is Hono (`apps/server/src/api/server.ts`). All routes
under `apps/server/src/api/routes/*.ts`. Adding a route = add a file
+ register it in `server.ts`. Always write an integration test under
`tests/integration/`.

### Rule R14 — WXT entrypoints under `entrypoints/`
Browser-extension entry points live in `apps/agent/entrypoints/`. Sub-routes
under `entrypoints/app/<route>/`. Page components and small components per
route. WXT picks them up automatically.

## Data flow — one full agent turn

1. **User opens side panel** → WXT hydrates `entrypoints/sidepanel/` (React app).
2. **User types prompt** → `lib/chat/` writes to local message store
 (chrome.storage.local) and `lib/rpc/` POSTs to the MCP server's
 `http://127.0.0.1:9100/chat`.
3. **MCP server receives the prompt** → `api/routes/chat.ts` → handler in
 `api/services/chat-service.ts`.
4. **Service builds AI request** → `agent/provider-factory.ts` picks the
 active provider (Gemini / Anthropic / etc., from `server.json` config).
 Uses `agent/prompt.ts` + `agent/soul-prompt.ts` + `agent/tool-adapter.ts`
 to assemble the messages + tool schemas.
5. **Provider returns tool calls** → `agent/ai-sdk-agent.ts` translates
 them into `ToolDefinition` calls.
6. **MCP server executes tools** → `tools/registry.ts` looks up
 `ToolDefinition`, calls its handler (e.g. `tools/snapshot.ts`).
7. **Tool handler drives Chromium** → `browser/CdpBackend` sends CDP
 commands. The MCP server reaches Chromium either via:
 - `--cdp-port` (dev: connect to running Chrome), or
 - Bundled spawn (BrowserOS.exe / L1): the precompiled `browseros_server.exe` orchestrates this.
8. **Results return to the agent loop** → tool outputs re-attach as
 messages → LLM is called again until it returns a final response.
9. **Response streamed back to side panel** via SSE (`sse.ts`) →
 `components/chat/` renders incrementally.
10. **History persisted** → `lib/db` writes to SQLite
 (`./.browseros/sessions.db`); `lib/agent-conversations` mirrors to
 Chrome storage for offline view.

## Key integration seams (where to look first)

| Need | File |
|---|---|
| Add a Chromium feature (C++) | `packages/browseros/chromium_patches/...` + `features.yaml` |
| Add a tool the agent can call | `packages/browseros-agent/apps/server/src/tools/<tool>.ts` + register in `tools/registry.ts` + fixture in `apps/server/tests/tools/__fixtures__/<tool>/` |
| Add an HTTP route | `apps/server/src/api/routes/<route>.ts` + register in `api/server.ts` + integration test in `tests/integration/` |
| Add an LLM provider | `apps/server/src/agent/provider-factory.ts` + adapter + `packages/browseros-agent/config.sample.json` |
| Add an extension entry point | `apps/agent/entrypoints/<name>/` (WXT auto-discovers) |
| Add a new feature flag | `packages/browseros/build/features.yaml` (Chromium side) and `apps/agent/lib/llm-providers/` + `apps/agent/components/ai-settings/` (extension side) |
| Add a DB schema change | `apps/server/src/lib/db/schema/<table>.ts` + run `drizzle-kit generate` → migration lands in `src/lib/db/migrations/` |
| Change update URL or R2 layout | `tools/release/generate_update_manifests.py` + `.github/workflows/update-manifest.yml` + chromium patch that bakes the URL into `components/update_client/` |
| Trigger a build on Linux LAN box | `tools/ubuntu-launch.sh --phase N` |
| Trigger a build on Windows box | `tools/bramburn-build.ps1 -StopAfterPhase N` |

## Build, test, lint from the repo root

```bash
# BrowserOS Chromium-fork side (Python)
cd packages/browseros
uv sync                       # install
browseros build --list        # show pipeline
browseros dev --help          # patch management

# Bun monorepo (MCP server + extension)
cd packages/browseros-agent
bun install
bun run start                 # MCP server (dev)
bun run test                  # tool tests
bun run test:tools
bun run test:integration
bun run lint                  # Biome
bun run typecheck             # tsc
bun run dev:server            # build server for development
bun run dev:ext               # build extension for development
bun run dist:server           # production (all targets)
bun run dist:ext              # production extension build
bun run generate:models       # refresh models.dev data
```

## Sub-package docs (deeper)

- [`packages/browseros/AGENTS.md`](packages/browseros/AGENTS.md) — Chromium fork view (patches, modules, release).
- [`packages/browseros-agent/AGENTS.md`](packages/browseros-agent/AGENTS.md) — Bun monorepo entry (apps, packages, scripts).
- [`packages/browseros-agent/apps/server/AGENTS.md`](packages/browseros-agent/apps/server/AGENTS.md) — MCP server internals.
- [`packages/browseros-agent/apps/agent/AGENTS.md`](packages/browseros-agent/apps/agent/AGENTS.md) — extension internals.
- [`packages/browseros-agent/apps/cli/AGENTS.md`](packages/browseros-agent/apps/cli/AGENTS.md) — Go CLI.
- [`packages/browseros-agent/apps/eval/AGENTS.md`](packages/browseros-agent/apps/eval/AGENTS.md) — Eval / benchmark harness.
- [`packages/browseros-agent/packages/shared/AGENTS.md`](packages/browseros-agent/packages/shared/AGENTS.md) — Shared constants & types.

## External references

- [`AGENTS.md`](AGENTS.md) — strategic roadmap, fork context, current work.
- [`AGENTS-build.md`](AGENTS-build.md) — full build pipeline (gclient → autoninja → release).
- [`AGENTS-toolchain.md`](AGENTS-toolchain.md) — Ubuntu 24.04 toolchain.
- [`AGENTS-ubuntu-dev.md`](AGENTS-ubuntu-dev.md) — Ubuntu-via-SSH dev workflow.
- [`docs/CI_AND_RELEASES.md`](docs/CI_AND_RELEASES.md) — CI workflows + release pipeline.
- [`docs/MCP_TOOL_SPEC.md`](docs/MCP_TOOL_SPEC.md) — Captured JSON-RPC schemas for the bundled MCP server (port 9200).
- [`packages/browseros-agent/CLAUDE.md`](packages/browseros-agent/CLAUDE.md) — coding guidelines for the Bun monorepo.
- [`packages/browseros-agent/process-compose.yaml`](packages/browseros-agent/process-compose.yaml) — dev process orchestration.

# `src/agent/` — AI agent loop

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/`.

## What's here

The in-process AI agent: a Vercel AI SDK tool loop that turns natural language
into browser actions. `ai-sdk-agent.ts` is the loop and the only consumer of
the `../tools/filesystem/` toolset; `provider-factory.ts` maps an LLM config
onto a Vercel `LanguageModel`; `tool-adapter.ts` bridges AI-SDK tools and MCP
tool results; `mcp-builder.ts` attaches external MCP clients; `prompt.ts` and
`soul-prompt.ts` hold the system prompts; `compaction.ts` keeps long
histories inside the context window, with its helpers split into
`compaction/`.

## Contents

| File | Purpose |
|---|---|
| `ai-sdk-agent.ts` | `AiSdkAgent` — the streamText tool loop; wires `buildFilesystemToolSet`, MCP tools, and Klavis tools. |
| `provider-factory.ts` | Provider dispatch: Anthropic, OpenAI, Google, Azure, Amazon Bedrock, OpenRouter, openai-compatible, … |
| `tool-adapter.ts` | Converts AI-SDK `tool()` results to/from `LanguageModelV2ToolResultOutput`; builds a `ToolSet`. |
| `mcp-builder.ts` | `createMCPClient` wiring for user-supplied MCP servers, with `TIMEOUTS` and a `BrowserContext`. |
| `prompt.ts` | "BrowserOS Agent System Prompt v6". |
| `soul-prompt.ts` | The persona/"soul" preamble layered on the system prompt. |
| `session-store.ts` | Per-session state keyed by conversation id, carrying the `BrowserContext`. |
| `message-normalization.ts` | Normalises inbound UI messages into model messages. |
| `message-validation.ts` | Rejects UI messages with no meaningful content before they reach the model. |
| `format-message.ts` | Renders model messages back to the UI stream. |
| `chat-mode.ts` | `CHAT_MODE_ALLOWED_TOOLS` — the reduced tool set for read-only chat. |
| `errors.ts` | `HttpAgentError` and siblings; `api/server.ts` maps these to status codes. |
| `types.ts` | Shared agent types (re-exported as `@browseros/server/agent/types`). |
| `compaction.ts` | Compaction orchestrator + the public re-exports of `compaction/`. |
| `compaction/` | `content.ts`, `prompt.ts`, `utils.ts` — see [`compaction/AGENTS.md`](compaction/AGENTS.md). |

## Rules

**A1 — Providers are registered in `provider-factory.ts` only.** Adding an
`@ai-sdk/<name>` provider means importing its factory there and adding a
branch; nothing else in the agent layer switches on the provider string.

**A2 — Chat mode is a tool allowlist, not a flag on every tool.**
`CHAT_MODE_ALLOWED_TOOLS` in `chat-mode.ts` is the single definition of what a
read-only chat turn may call. A new mutating tool must be *absent* from that
set for the allowlist to keep working.

**A3 — Long histories go through `compaction.ts`, never a manual trim.** It
estimates tokens, reduces tool outputs, finds a safe split point, and
summarises. Bypass only if you are changing compaction itself.

**A4 — The filesystem toolset is injected, not imported by the tool layer.**
`ai-sdk-agent.ts` calls `buildFilesystemToolSet(cwd)`; no other module in
`src/tools/` may import it.

**A5 — Reject empty turns at `message-validation.ts`.** Sending an empty
`UIMessage` to the provider wastes a round trip and corrupts the summary
chain.

**A6 — Logging via `lib/logger`.** No `console.log`, no `[prefix]` tags.

## Workflows

**Adding an LLM provider:** 1. Import the AI-SDK factory in
`provider-factory.ts`. 2. Add the branch mapping the config's provider string
to it. 3. Add the config key to `ServerConfigSchema` in `../config.ts` **and**
`config.sample.json` at the monorepo root. 4. Surface it in
`apps/agent/components/ai-settings/`. 5. Refresh the model catalogue with
`bun run generate:models` from `packages/browseros-agent/`.

**Adding a tool only for chat mode:** add it to `CHAT_MODE_ALLOWED_TOOLS` in
`chat-mode.ts` and confirm it is *not* a mutating action.

**Testing:** `bun run test:agent` runs `tests/agent/` — `prompt.test.ts`,
`message-validation.test.ts`, `compaction.test.ts`, `compaction-e2e.test.ts`,
`copilot-fetch.test.ts`. These are offline (no CDP port required).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `apps/server/` MCP server internals.
- [`./compaction/AGENTS.md`](compaction/AGENTS.md) — context compaction internals.
- [`../tools/filesystem/AGENTS.md`](../tools/filesystem/AGENTS.md) — the toolset this loop mounts.
- [`../../../../tests/agent/AGENTS.md`](../../tests/agent/AGENTS.md) — agent tests.

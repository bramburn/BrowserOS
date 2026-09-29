# `src/lib/clients/llm/` — LLM provider creation

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/lib/clients/`.

## What's here

Turning an `LLMConfig` into a Vercel AI SDK `LanguageModel`, plus the
lightweight text client used by the provider-verify endpoint. This is the
*low-level* creation path; the agent's own provider dispatch lives one level
up in [`../../../agent/provider-factory.ts`](../../../agent/provider-factory.ts)
and is what the chat loop uses.

## Contents

| File | Purpose |
|---|---|
| `config.ts` | Resolves an `LLMConfig`, including the `BROWSEROS` provider lookup (the gateway-backed provider). |
| `provider.ts` | Creates the Vercel AI SDK language model for a resolved config. |
| `client.ts` | `LlmClient`-style lightweight text generation client, used by the SDK verify endpoint. |
| `refine-prompt.ts` | `streamText` call that rewrites a user prompt via `TIMEOUTS` + `LLMConfig`. Backs `POST /refine-prompt`. |
| `types.ts` | Internal client types. |
| `mock-language-model.ts` | A `LanguageModelV3`-shaped fake implementing `generate` and `stream`. |
| `test-provider.ts` | A test provider with a bounded `TIMEOUTS`-based delay, for the verify endpoint. |

## Rules

**LL1 — `config.ts` resolves; `provider.ts` constructs.** Don't merge them.
Resolution (which credentials, which base URL) and construction (which
`@ai-sdk/*` factory) fail differently and are tested differently.

**LL2 — Two provider paths, deliberately.** `provider-factory.ts` in
`../agent/` serves the interactive agent loop; this folder serves one-shot
calls (verify, refine-prompt). Adding a provider to the agent loop does not
automatically make it available to `POST /test-provider` — wire both.

**LL3 — Never log or persist API keys.** Config objects flow through
`config.ts`; nothing here may `logger.info` the config or write it to disk.
`logger.debug` on the *resolved provider name and model id only*.

**LL4 — Use the mock/test providers for tests.** `mock-language-model.ts` and
`test-provider.ts` exist so tests don't hit a real provider. Adding a
hand-rolled `vi.mock`-style fake elsewhere duplicates them.

**LL5 — Timeouts from `TIMEOUTS`.** `refine-prompt.ts` and `test-provider.ts`
already do; keep it that way for any new one-shot call.

## Workflows

**Adding a provider usable by `/test-provider`:** 1. Add the branch in
`provider.ts`. 2. Handle its config shape in `config.ts`. 3. Add the key to
`ServerConfigSchema` in `../../../config.ts` and to
`config.sample.json` at the monorepo root. 4. Confirm the UI list in
`apps/agent/components/ai-settings/` matches.

**Refreshing the model catalogue:** run `bun run generate:models` from
`packages/browseros-agent/` (fetches `models.dev/api.json`).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `src/lib/clients/` conventions.
- [`../../../agent/AGENTS.md`](../../../agent/AGENTS.md) — the agent loop's provider dispatch.
- [`../../../api/routes/provider.ts`](../../../api/routes/provider.ts) — `POST /test-provider`.
- [`../../../api/routes/refine-prompt.ts`](../../../api/routes/refine-prompt.ts) — `POST /refine-prompt`.

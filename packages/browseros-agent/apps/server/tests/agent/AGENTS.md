# `tests/agent/` — agent loop and compaction tests (offline)

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/`.

## What's here

Tests for `../../src/agent/` — the Vercel AI SDK tool loop and its context
compaction. All five run offline: no browser, no CDP port, no server boot.
Compaction is the bulk of the coverage because it is the most intricate pure
logic in the server.

## Contents

| File | Covers |
|---|---|
| `prompt.test.ts` | `../../src/agent/prompt.ts` — the "BrowserOS Agent System Prompt v6". |
| `message-validation.test.ts` | `message-validation.ts` — rejection of UI messages with no meaningful content. |
| `compaction.test.ts` | `compaction.ts` + `compaction/utils.ts` — token estimation, `computeConfig`, `findSafeSplitPoint`, `reduceToolOutputs`, `slidingWindow`. |
| `compaction-e2e.test.ts` | The full compaction path: a long history in, a summarised history out. |
| `copilot-fetch.test.ts` | `../../src/lib/clients/oauth/copilot-fetch.ts` — the GitHub Copilot fetch wrapper. |

## Rules

**AT1 — Use the sanctioned fakes.** `../../src/lib/clients/llm/mock-language-model.ts`
and `test-provider.ts` exist so these tests don't call a real provider. A
compaction test that needs a summariser uses the mock model, not an API key.

**AT2 — Compaction thresholds are asserted, not re-derived.** Assert against
the values `computeConfig()` produces from `AGENT_LIMITS`; if you need to pin
a number, import the constant from `@browseros/shared/constants/limits`.

**AT3 — `copilot-fetch.test.ts` lives here by history, not by subject.** It
tests an OAuth transport, but it is grouped with the agent because it is
agent-adjacent. If you add sibling OAuth fetch tests, `../lib/clients/oauth/`
is the better home — don't grow a second convention.

**AT4 — No browser in this folder.** If a test here starts needing a real
page, it belongs under `../tools/`.

## Workflows

**Running:** `bun run test:agent` from `apps/server/` (also part of
`bun run test:core`).

**Testing a compaction change:** 1. Run `compaction.test.ts` for the unit
behaviour. 2. Run `compaction-e2e.test.ts` for the end-to-end path. 3. If you
changed the summary format, confirm both the summariser prompt and the
turn-prefix prompt still agree (see `../../src/agent/compaction/prompt.ts`).

**Adding a test for a new agent module:** put it here if it's agent-loop
logic with no I/O; otherwise under `../lib/`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the test suite and group rules.
- [`../../src/agent/AGENTS.md`](../../src/agent/AGENTS.md) — the agent loop.
- [`../../src/agent/compaction/AGENTS.md`](../../src/agent/compaction/AGENTS.md) — compaction internals.
- [`../__helpers__/AGENTS.md`](../__helpers__/AGENTS.md) — the harness (not required here).

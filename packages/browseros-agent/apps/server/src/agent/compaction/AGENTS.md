# `src/agent/compaction/` — context compaction internals

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/agent/`.

## What's here

The three pure helpers behind `../compaction.ts`, which keeps a long agent
conversation inside the model's context window. `utils.ts` holds the token
accounting and the sliding window; `prompt.ts` builds the summarisation and
turn-prefix prompts; `content.ts` normalises AI-SDK message content into
plain text. All three are dependency-free apart from `ai` types and
`lib/logger`, which is what makes them cheap to unit test.

## Contents

| File | Purpose |
|---|---|
| `utils.ts` | `ComputedConfig` (contextWindow, reserveTokens, triggerRatio, triggerThreshold, keepRecentTokens, minSummarizableTokens, maxSummarizationInput, summarizerMaxOutputTokens, summarizationTimeoutMs, fixedOverhead, safetyMultiplier, imageTokenEstimate, toolOutputMaxChars), `computeConfig`, `estimateTokens`, `estimateTokensForThreshold`, `findSafeSplitPoint`, `getCurrentTokenCount`, `reduceToolOutputs`, `slidingWindow`, `isCompactionState`, `StepWithUsage`. |
| `prompt.ts` | `buildSummarizationPrompt`, `buildSummarizationSystemPrompt`, `buildTurnPrefixPrompt`, `messagesToTranscript`. Module-private `const`s (not exported): `SUMMARIZATION_SYSTEM_PROMPT`, `SUMMARY_FORMAT`, `INITIAL_PROMPT`, `UPDATE_PROMPT`, `TURN_PREFIX_PROMPT`. The summary has a fixed markdown format: `## Goal`, `## Constraints & Preferences`, `## Progress` (Done / In Progress), … |
| `content.ts` | `stripBinaryContent`, `stripToolResultOutput`, `toolResultOutputToText`, `estimateToolResultOutput`; formatting for denied executions and file ids. |

## Rules

**C1 — Thresholds come from `AGENT_LIMITS`.** `utils.ts` and `prompt.ts` both
import `AGENT_LIMITS` from `@browseros/shared/constants/limits`. Don't
introduce numeric literals; add to `AGENT_LIMITS` instead.

**C2 — The summariser treats the transcript as data.** The system prompt
explicitly forbids continuing the conversation and instructs the model to
ignore instructions embedded in tool outputs, because they may be prompt
injection. If you change the prompt, keep both clauses.

**C3 — Never summarise a turn boundary.** `findSafeSplitPoint()` is what
keeps a tool-call/tool-result pair intact; skipping it produces orphaned tool
messages that most providers reject.

**C4 — Strip binary before estimating.** `content.ts` must run before
`estimateTokens` — image parts are charged at `imageTokenEstimate`, not
counted as text.

**C5 — Summarisation is time-boxed and output-bounded.**
`summarizationTimeoutMs` and `summarizerMaxOutputTokens` come from
`computeConfig`. A summarisation call that ignores the timeout will stall the
chat stream.

**C6 — Keep this folder pure.** No db, no browser, no config, no I/O. If you
need state, put it in `../session-store.ts`.

## Workflows

**Changing the compaction threshold:** edit `computeConfig()` in `utils.ts`
(it derives from `AGENT_LIMITS` plus a `safetyMultiplier`), then re-run
`tests/agent/compaction.test.ts` and `compaction-e2e.test.ts`.

**Changing the summary shape:** edit `SUMMARY_FORMAT` in `prompt.ts` and
`buildTurnPrefixPrompt` together — the prefix prompt is what carries forward
post-summary context, and the two must agree.

**Adding a new message content kind:** handle it in `content.ts`
(`toolResultOutputToText` / `stripBinaryContent`) so both the estimator and
the summariser see it consistently.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `src/agent/` conventions.
- [`../compaction.ts`](../compaction.ts) — the orchestrator that composes these helpers.
- [`../../../../tests/agent/AGENTS.md`](../../../tests/agent/AGENTS.md) — compaction unit + e2e tests.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `apps/server/` MCP server internals.

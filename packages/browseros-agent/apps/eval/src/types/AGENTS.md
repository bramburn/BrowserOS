# `src/types/` — Zod schemas and shared types

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/`; the shapes everything else agrees on.

## What's here

Five schema modules and a barrel. `config.ts` defines `EvalConfigSchema` and
the three agent config variants; `task.ts` defines the dataset line format;
`message.ts` defines the agent stream event union plus the type guards and
helpers used to read it back; `result.ts` defines `TaskMetadata` and
`GraderResult` (the persisted task record); `errors.ts` defines `ErrorSource`.
`index.ts` re-exports all of them, and it is the only import path most code
should use (`from '../types'`).

```
types/
├── config.ts  ← SingleAgentConfigSchema, OrchestratorExecutorConfigSchema,
│                ClaudeCodeAgentConfigSchema, AgentConfigSchema, EvalConfigSchema
├── task.ts    ← TaskInputMetadataSchema, TaskSchema (dataset JSONL line)
├── message.ts ← MessageSchema, UIMessageStreamEvent, guards, extractLastAssistantText
├── result.ts  ← TaskMetadataSchema, GraderResultSchema, TokenUsageSchema, …
├── errors.ts  ← ErrorSourceSchema, TaskErrorSchema, EvalWarningSchema
└── index.ts   ← barrel
```

## Rules

### TY1 — zod first, types derived
Every exported type is `z.infer` of a schema in the same file. Don't add an
interface that isn't backed by a schema — the parse sites (task loader, config
validator, suite loader, message reader) rely on the schema being the
definition.

### TY2 — The dataset line format is fixed
`TaskSchema` requires `query_id`, `dataset`, `query` and `metadata`
(`original_task_id` plus optional `website`/`category`/`additional`).
`graders` defaults to `[]`; `start_url` and `setup_script` are optional. A file
with a line that fails this schema fails to load entirely
(`../runner/task-loader.ts`).

### TY3 — `query_id` is a path segment
It becomes the task output directory name and the viewer path key. Keep it
filesystem-safe: lowercase, no slashes, stable across re-grades.

### TY4 — `ErrorSource` is a closed set
`window_creation | navigation | agent_execution | mcp_tool | screenshot |
grader | message_logging | cleanup | unknown`. The run summary aggregates errors
by source; a new source requires a schema change, and callers in
`../capture/context.ts` / `../runs/task-run-pipeline.ts` must use it.

### TY5 — Agent config is a discriminated union on `type`
`AgentConfigSchema` is a `z.discriminatedUnion`, so adding a variant is a
compile-time break in `../agents/index.ts` `createAgent` (deliberately — there
is no fallback branch) and in `../suites/schema.ts` `SuiteAgentSchema`.

### TY6 — The message stream is one union, not per-agent shapes
`UIMessageStreamEvent` is what every agent must emit, whether it drives the
server's tool loop, the orchestrator, or the `claude` CLI
(`../agents/claude-code/stream-parser.ts` translates into it). Consumers —
grader prompts, metrics, the viewer, the dashboard — assume this shape.

### TY7 — Import from `./types`, not from the individual files
`../types` (the barrel) is the established import path; it keeps the module
graph stable when a schema moves between files.

## Workflows

### Adding a field to the persisted task record
1. Extend `TaskMetadataSchema` in `result.ts`.
2. Update `TrajectorySaver.createInitialMetadata` in `../capture/trajectory-saver.ts`
   so new runs write it, and any evaluator that builds metadata
   (`../agents/single-agent.ts`, `../agents/claude-code/index.ts`,
   `../agents/orchestrator-executor/index.ts`).
3. Old `metadata.json` files must still parse — make the field optional or give
   `TaskMetadataSchema` a default, or `grade` and `publish` will fail on
   existing runs.

### Adding a stream event type
1. Add it to `UIMessageStreamEvent` in `message.ts` and the matching guard.
2. Teach `../capture/context.ts` `createStreamWriter` whether it carries a
   screenshot stamp, and `../reporting/task-metrics.ts` whether it counts as a
   tool call.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — harness-wide validation rule.
- [`../capture/AGENTS.md`](../capture/AGENTS.md) — writes `TaskMetadata` and the message log.
- [`../graders/AGENTS.md`](../graders/AGENTS.md) — consumes `GraderResult` / `GraderInput`.
- [`../agents/AGENTS.md`](../agents/AGENTS.md) — the exhaustive `createAgent` switch.
- [`../../../tests/AGENTS.md`](../../tests/AGENTS.md) — test tree that exercises these schemas.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.

# `.claude/skills/dev2-design/` — stage 2: design options

> Part of [`../../../AGENTS.md`](../../../AGENTS.md) in `packages/browseros-agent`.

## What's here

Stage 2 of the `/dev` pipeline: it turns the exploration notes into 2-4
high-level design alternatives, each with an overview, pros, cons, and
Low/Medium/High complexity and risk ratings. It reads
`.llm/<slug>/tmp_context.md` and `tmp_exploration.md`, pauses for the user to
choose, writes the decision to `design.md`, and invokes `/dev3-prd`.

Prompt text only — no code, no files outside `.llm/`.

## Contents

```
dev2-design/
└── SKILL.md    ← 2.3 KB: name, description ("Sub-skill of the /dev workflow."), argument-hint
```

## Rules

**D2R — No code snippets in design options.** Explicit rule. Descriptions are
2-3 paragraphs of architecture, data flow and decisions. The only document
that carries code is `prd.md` at stage 3.

**D2R2 — 2-4 options, each honestly costed.** Each needs Name, Overview,
Advantages, Disadvantages, Complexity, Risk. "Don't present a straw man option
just to fill the count." One option is acceptable only when genuinely the
only reasonable approach — and then it must say why alternatives fail.

**D2R3 — Options must be grounded in the real codebase.** Reference actual
modules, patterns and conventions found in stage 1, not hypothetical ones.

**D2R4 — Design for the immediate need.** "Design options need not be very
futureproof." Reject speculative extensibility in the review of this stage.

**D2R5 — This is a hard user-pause.** The skill must present the options and
wait for a choice. `design.md` records the chosen option *plus* any user
modifications, so a hybrid of two options is a valid outcome.

**D2R6 — Read the two inputs before proposing anything.** If
`.llm/<slug>/design.md` is being written, `tmp_exploration.md` must already
exist — that ordering is what makes the options concrete.

## Workflows

**Presenting design options**
1. `/dev2-design add_user_auth` (run by the pipeline, or standalone).
2. Read `tmp_context.md` + `tmp_exploration.md`.
3. Present 2-4 options in the six-field shape, with no code.
4. Wait for the user's pick, then write `.llm/<slug>/design.md` and invoke
   `/dev3-prd <slug>`.

**Comparing against the code before proposing**
1. Re-read `design.md` later (stage 4 uses it) to confirm the design maps onto
   modules that actually exist — e.g. `apps/server/src/agent/provider-factory.ts`
   rather than an invented path.

## Cross-references

- [`SKILL.md`](SKILL.md) — the stage definition.
- [`../../../AGENTS.md`](../../../AGENTS.md) — Bun monorepo parent.
- [`../dev1-start/AGENTS.md`](../dev1-start/AGENTS.md) — produces the exploration this stage reads.
- [`../dev3-prd/AGENTS.md`](../dev3-prd/AGENTS.md) — the stage it hands off to.
- [`../AGENTS.md`](../AGENTS.md) — the `.llm/` artifact map.
- [`../../../CLAUDE.md`](../../../CLAUDE.md) — existing patterns the options should follow.

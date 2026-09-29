# `.claude/skills/dev3-prd/` — stage 3: questions & PRD

> Part of [`../../../AGENTS.md`](../../../AGENTS.md) in `packages/browseros-agent`.

## What's here

Stage 3 of the `/dev` pipeline, and the only stage that produces a formal
document: the PRD. It first writes clarifying questions to
`.llm/<slug>/tmp_questions.md`, marking each `[RESOLVED]` (answered from the
code) or `[NEEDS INPUT]` (needs a human), asks the user only the latter, and
then writes `prd.md` in the pyramid principle: Level 1 executive summary
(requirements, background, design overview), Level 2 component details (one
paragraph each), Level 3 implementation details (snippets, file paths,
migrations). Then it invokes `/dev4-implement`.

Prompt text only.

## Contents

```
dev3-prd/
└── SKILL.md    ← 2.5 KB: name, description ("Sub-skill of the /dev workflow."), argument-hint
```

## Rules

**D3R — Self-answer before asking.** Every question is first attempted against
the codebase and tagged `[RESOLVED]` or `[NEEDS INPUT]`. Only the
`[NEEDS INPUT]` set reaches the user — batching resolved trivia back at them
is the failure mode this step exists to prevent. If nothing needs input, say
so and proceed.

**D3R2 — Keep the pyramid.** Requirements first, then background, then design
overview. Code snippets and file paths belong **only** in Level 3; putting
implementation detail in the executive summary breaks the format the level-1
summary is designed to support (it is reused as the PR body in
[`dev7-pr/`](../dev7-pr/AGENTS.md)).

**D3R3 — Level 1 must stand alone.** Stage 7 pastes it into `gh pr create
--body`. Write it as a reader-facing summary: bullet requirements, 2-3
paragraphs of context, one paragraph of design.

**D3R4 — This is a hard user-pause** (alongside `dev2-design` and `dev7-pr`).
Do not proceed to implementation with `[NEEDS INPUT]` questions outstanding.

**D3R5 — Level 3 names real paths.** The PRD's implementation level lists the
actual files that will change, so it must match the tree — e.g.
`apps/server/src/tools/registry.ts`, not a plausible-looking invention.

## Workflows

**Writing the PRD**
1. `/dev3-prd add_user_auth` (or let the pipeline invoke it).
2. Read `tmp_context.md`, `tmp_exploration.md`, `design.md`.
3. Write `tmp_questions.md`; resolve what the code answers.
4. Present only `[NEEDS INPUT]` questions and wait.
5. Write `prd.md` (3 levels), show the user the Level 1 summary, ask whether
   to review the full document.
6. Invoke `/dev4-implement <slug>`.

**Checking a PRD before implementing**
1. Confirm every `[NEEDS INPUT]` question in `tmp_questions.md` has an answer.
2. Confirm Level 3 file paths exist or are clearly marked as new.
3. Confirm the Level 1 text would make sense pasted into a PR.

## Cross-references

- [`SKILL.md`](SKILL.md) — the stage definition.
- [`../../../AGENTS.md`](../../../AGENTS.md) — Bun monorepo parent.
- [`../dev2-design/AGENTS.md`](../dev2-design/AGENTS.md) — produces `design.md`, an input here.
- [`../dev4-implement/AGENTS.md`](../dev4-implement/AGENTS.md) — the stage it hands off to.
- [`../dev5-review/AGENTS.md`](../dev5-review/AGENTS.md) — reviews against this PRD.
- [`../AGENTS.md`](../AGENTS.md) — the `.llm/` artifact map.

# `.claude/skills/dev/` — `/dev` pipeline orchestrator

> Part of [`../../../AGENTS.md`](../../../AGENTS.md) in `packages/browseros-agent`.

## What's here

The entrypoint of the seven-stage feature development pipeline. It holds no
logic of its own: it names the ordered list of stages, then tells the agent to
invoke `/dev1-start $ARGUMENTS` and let the stages chain themselves. Two
frontmatter flags matter here — `disable-model-invocation: true` (the model
must not start a whole feature pipeline on its own) and
`argument-hint: [feature description]`.

Prompt text only. Nothing here is executed by Bun, Biome, or the test suite.

## Contents

```
dev/
└── SKILL.md    ← 1.2 KB: name, description, disable-model-invocation, argument-hint
```

## Rules

**D1 — Keep the stage list and the delegation line in sync.** The file both
lists the 7 stages and opens with "Invoke `/dev1-start $ARGUMENTS` now". Change
one, change the other.

**D2 — `disable-model-invocation: true` must stay.** Without it a coding agent
can kick off a full PRD-and-PR pipeline unprompted.

**D3 — Pass `$ARGUMENTS` through verbatim.** The raw feature description is
handed to `dev1-start`, which derives the snake_case slug. Do not pre-parse or
normalise it here.

**D4 — `dev6-review-fix` is conditional.** The stage list annotates it
"skipped if review is clean"; `dev5-review` implements that branch. If you
reorder stages, preserve that skip.

**D5 — Stage docs stay in their own directories.** This file is a manifest,
not documentation — keep it to the list plus the handoff sentence, and let
each `devN-*/SKILL.md` own its procedure.

## Workflows

**Starting a feature**
1. `/dev add credits badge to the chat header`
2. The agent writes the description to `.llm/<snake_case_slug>/tmp_context.md`
   and the remaining stages run unattended until the next user-pause point.

**Reordering or inserting a stage**
1. Edit the numbered list in `SKILL.md`.
2. Edit the "Hand off" section of the preceding stage to invoke the new name.
3. Edit the "Input" list of the following stage to read the new artifact.

## Cross-references

- [`SKILL.md`](SKILL.md) — the orchestrator itself.
- [`../../../AGENTS.md`](../../../AGENTS.md) — Bun monorepo parent.
- [`../AGENTS.md`](../AGENTS.md) — the full 11-skill map and frontmatter contract.
- [`dev1-start/AGENTS.md`](../dev1-start/AGENTS.md) — the stage this file always invokes.
- [`../../../.config/AGENTS.md`](../../../.config/AGENTS.md) — Worktrunk,
  which `dev4-implement` uses to create the feature worktree.

# `.claude/skills/dev4-implement/` — stage 4: implement

> Part of [`../../../AGENTS.md`](../../../AGENTS.md) in `packages/browseros-agent`.

## What's here

Stage 4 of the `/dev` pipeline and the only stage that writes product code.
It reads `prd.md` and `design.md`, creates a feature worktree if it is running
in the main tree, breaks the PRD into small testable steps
(`tmp_impl_plan.md`), implements them one at a time, and runs the test suite
plus a manual check before handing off to `/dev5-review`. It also carries the
pipeline's longest style section — a code-style guide that is stricter and
more specific than [`../../../CLAUDE.md`](../../../CLAUDE.md) in places.

## Contents

```
dev4-implement/
└── SKILL.md    ← 2.9 KB: name, description ("Sub-skill of the /dev workflow."), argument-hint
```

## Rules

**D4R — Work in a worktree.** Step 1 checks `git worktree list`; if the agent
is in the main tree it runs `wt switch -c feat/<slug>`. `wt` is
[Worktrunk](../../../.config/AGENTS.md) — its hooks install deps and copy
`.env.*` and `.llm/` into the new worktree. If already in a feature worktree,
continue in place.

**D4R2 — Steps must be independently verifiable.** Each entry in
`tmp_impl_plan.md` is small enough to test on its own, dependency-ordered, and
runnable. Implement → test → fix before moving to the next step.

**D4R3 — The style guide in this file is binding for the stage.** Functions
20-30 lines max; related logic grouped with no blank lines between related
lines; comments explain *why*, plus short one-line signposts on roughly half
the major blocks; no debug `console.log`; no premature abstraction; match
existing conventions. Note this is *more prescriptive* than both
[`../../../CLAUDE.md`](../../../CLAUDE.md) (minimal comments) and
[`../ts-style-review/`](../ts-style-review/AGENTS.md) (Google style) — where
they conflict inside the pipeline, this stage's rules apply.

**D4R4 — Verify before handing off.** Run the full test suite, manually confirm
the feature matches the PRD, loop until it passes. Do not hand a broken build
to the review stage.

**D4R5 — The implementation must match `prd.md` Level 3.** Deviating from the
named files or interfaces is a scope change; stop and say so rather than
silently improvising.

## Workflows

**Implementing a planned feature**
1. `/dev4-implement add_user_auth` (or let the pipeline invoke it).
2. Read `prd.md`, `design.md`, `tmp_exploration.md`.
3. `wt switch -c feat/add_user_auth` if not already in a worktree.
4. Write `tmp_impl_plan.md`, then implement and test step by step.
5. Full suite + manual verification, then invoke `/dev5-review <slug>`.

**Reviewing what a previous stage produced**
1. `git diff` in the feature worktree to see the change set.
2. `cat .llm/<slug>/tmp_impl_plan.md` to see the intended step order.
3. Compare against `.llm/<slug>/prd.md` Level 3 for scope drift.

## Cross-references

- [`SKILL.md`](SKILL.md) — the stage definition.
- [`../../../AGENTS.md`](../../../AGENTS.md) — Bun monorepo parent.
- [`../dev3-prd/AGENTS.md`](../dev3-prd/AGENTS.md) — produces the `prd.md` this stage implements.
- [`../dev5-review/AGENTS.md`](../dev5-review/AGENTS.md) — the stage it hands off to.
- [`../ts-style-review/AGENTS.md`](../ts-style-review/AGENTS.md) — the style gate run during review.
- [`../../../.config/AGENTS.md`](../../../.config/AGENTS.md) — Worktrunk config for `wt switch`.
- [`../../../CLAUDE.md`](../../../CLAUDE.md) — the repo's authoritative coding rules.

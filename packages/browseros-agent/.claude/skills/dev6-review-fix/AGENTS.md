# `.claude/skills/dev6-review-fix/` — stage 6: fix review comments

> Part of [`../../../AGENTS.md`](../../../AGENTS.md) in `packages/browseros-agent`.

## What's here

Stage 6 of the `/dev` pipeline, and the smallest one. It reads the review
comments written by stage 5 into `.llm/<slug>/tmp_review.md`, triages them into
critical / suggestions / nits, applies the fixes in that order, re-runs the
test suite, and commits as `fix: address review comments for <slug>`. Then it
invokes `/dev7-pr`. It is skipped entirely when stage 5 finds nothing
actionable.

## Contents

```
dev6-review-fix/
└── SKILL.md    ← 1.3 KB: name, description ("Sub-skill of the /dev workflow."), argument-hint
```

## Rules

**D6R — Input is only `tmp_review.md` and `prd.md`.** It needs the review
comments and the feature intent. It must not re-review from scratch — that
duplicates stage 5.

**D6R2 — Fix in severity order, not file order.** Critical → suggestions →
nits. The severity labels come from
[`../dev5-review/`](../dev5-review/AGENTS.md) and this stage's triage is
exactly that three-way split.

**D6R3 — Nits are optional.** "Fix if quick." They are not a blocker and must
not block the handoff to stage 7.

**D6R4 — Each fix is verified.** After applying a fix, confirm it did not
break anything (run tests where applicable) before moving to the next comment.
A batch of unverified edits is how a review-fix pass introduces a regression.

**D6R5 — This stage commits as `fix:`, distinct from stage 5's `feat:`.** The
two commits are the audit trail of "what was built" vs "what was corrected";
don't squash them or reuse the prefix.

**D6R6 — No scope creep.** The skill is a fixer, not a refactorer. If a comment
implies a larger design change, surface it rather than expanding the diff.

## Workflows

**Working the review queue**
1. `/dev6-review-fix add_user_auth` (or let the pipeline invoke it).
2. Read `.llm/add_user_auth/tmp_review.md`, grouped by severity.
3. Apply criticals → suggestions → nits, testing as you go.
4. Run the full test suite; loop on failure.
5. `git commit -m "fix: address review comments for add_user_auth"`, then
   invoke `/dev7-pr add_user_auth`.

**Skipping this stage**
1. If stage 5 produced only nits or nothing, stage 5 goes directly to
   `/dev7-pr` — do not invoke this skill to produce an empty commit.

## Cross-references

- [`SKILL.md`](SKILL.md) — the stage definition.
- [`../../../AGENTS.md`](../../../AGENTS.md) — Bun monorepo parent.
- [`../dev5-review/AGENTS.md`](../dev5-review/AGENTS.md) — produces `tmp_review.md` and the severities.
- [`../dev7-pr/AGENTS.md`](../dev7-pr/AGENTS.md) — the stage it hands off to.
- [`../AGENTS.md`](../AGENTS.md) — the `.llm/` artifact map.
- [`../../commands/AGENTS.md`](../../commands/AGENTS.md) — `/browseros-review`, the
  separate multi-agent review path.

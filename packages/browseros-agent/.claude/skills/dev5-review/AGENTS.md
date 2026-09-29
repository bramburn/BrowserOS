# `.claude/skills/dev5-review/` — stage 5: code review

> Part of [`../../../AGENTS.md`](../../../AGENTS.md) in `packages/browseros-agent`.

## What's here

Stage 5 of the `/dev` pipeline. It reviews the implementation against the PRD
on four axes — correctness, code quality, architecture, safety — optionally
delegating TypeScript style to `/ts-style-review`, writes structured comments
to `.llm/<slug>/tmp_review.md`, presents a summary, and **commits** the work as
`feat: <description>`. It then branches: actionable comments go to
`/dev6-review-fix`, a clean review goes straight to `/dev7-pr`.

This is a single-reviewer stage. The heavyweight multi-agent review lives in
the separate `/browseros-review` command
([`../../commands/browseros-review.md`](../../commands/browseros-review.md)) and is
not part of the `/dev` chain.

## Contents

```
dev5-review/
└── SKILL.md    ← 2.5 KB: name, description ("Sub-skill of the /dev workflow."), argument-hint
```

## Rules

**D5R — Comment format is fixed.** Each entry in `tmp_review.md` is
`### [file_path:line_number] — severity (critical/suggestion/nit)` followed by
the description and suggested fix. Stage 6 triages on exactly those three
severity names.

**D5R2 — Severity names are the contract.** `critical` / `suggestion` / `nit`
and nothing else; stage 6 groups on them and stage 5's own branch decision
depends on "critical or suggestion" existing.

**D5R3 — This stage commits.** `git add -A && git commit -m "feat: <…>"`.
That is the only commit in the pipeline before `dev6`; a feature that arrives
uncommitted at this point will produce an empty diff for the reviewer.

**D5R4 — Invoke `/ts-style-review` for TypeScript diffs.** Step 1 is
conditional on the project being TypeScript — this monorepo always is. Fold
its findings into the review comments rather than reporting separately.

**D5R5 — The branch decision is not optional.** Actionable comments
(critical or suggestion) → `/dev6-review-fix`; zero actionable comments (nits
or clean) → skip stage 6, go to `/dev7-pr`. Do not run `dev6` on a clean
review.

**D5R6 — Present counts, not prose.** Total files reviewed, count per
severity, and the top 3 issues. The stage's output is a triage input, not a
report to the user.

## Workflows

**Reviewing a feature implementation**
1. `/dev5-review add_user_auth` — reads `prd.md`, `design.md`, `git diff`.
2. Runs `/ts-style-review` over the changed TypeScript files.
3. Checks each file on correctness, quality, architecture, safety.
4. Writes `tmp_review.md` in the fixed format, summarises counts, commits.
5. Routes to `dev6` or straight to `dev7`.

**Re-running review on a fix**
1. Invoke the stage again on the same slug after the fixes land.
2. It re-reads `tmp_review.md` context via the same PRD and diff.

## Cross-references

- [`SKILL.md`](SKILL.md) — the stage definition.
- [`../../../AGENTS.md`](../../../AGENTS.md) — Bun monorepo parent.
- [`../dev4-implement/AGENTS.md`](../dev4-implement/AGENTS.md) — the stage whose output is reviewed.
- [`../dev6-review-fix/AGENTS.md`](../dev6-review-fix/AGENTS.md) — consumes `tmp_review.md`.
- [`../ts-style-review/AGENTS.md`](../ts-style-review/AGENTS.md) — the style sub-review invoked here.
- [`../AGENTS.md`](../AGENTS.md) — the `.llm/` artifact map.
- [`../../../CLAUDE.md`](../../../CLAUDE.md) — the rules the "architecture" check is measured against.

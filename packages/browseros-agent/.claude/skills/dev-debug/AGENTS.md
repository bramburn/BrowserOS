# `.claude/skills/dev-debug/` — `/dev-debug` root-cause loop

> Part of [`../../../AGENTS.md`](../../../AGENTS.md) in `packages/browseros-agent`.

## What's here

A standalone debugging workflow, separate from the `/dev` feature pipeline. It
derives a slug from the issue text, creates `.llm/debug_<slug>/`, traces the
relevant code paths, and writes **2-5 possible root causes ranked by
probability** to `tmp_root_causes.md` — each with Why, Evidence, and Files. The
user confirms or overrides the ranking, then the skill fixes only cause #1,
tests, commits, and re-ranks the remainder if the issue persists.

Its two frontmatter flags: `disable-model-invocation: true` (it must not fire
on its own) and `argument-hint: [issue description]` (it takes free text, not
a slug).

## Contents

```
dev-debug/
└── SKILL.md    ← 2.3 KB: name, description, disable-model-invocation, argument-hint
```

## Rules

**DB1 — Diagnose before fixing.** The skill's first substantive output is a
ranked hypothesis list, presented to the user *before* any code changes. A fix
applied without a stated cause is a violation of this skill.

**DB2 — Rank 2-5 causes, and say why.** The fixed entry shape is
`### N. [short title]` with **Why** / **Evidence** / **Files**. "Evidence"
means something in the code, not intuition.

**DB3 — The user can override the ranking.** Step 1 ends by asking. Fixing the
agent's #1 choice by default is allowed, but only when the user doesn't
object; never silently skip the question.

**DB4 — Minimal, no drive-by refactors.** "Fix the bug, nothing else." The same
discipline as [`dev6-review-fix/`](../dev6-review-fix/AGENTS.md), applied to
production code.

**DB5 — One commit per root cause.** `fix: <what was fixed and why>`, committed
after each successful fix. That gives a revert unit per hypothesis — if the
diagnosis was wrong, revert one commit rather than untangle a combined diff.

**DB6 — Re-rank on failure.** If cause #1 didn't resolve the issue, mark it
addressed in `tmp_root_causes.md`, re-rank what remains, and loop. The file is
the running log, not a one-shot artifact.

**DB7 — The namespace differs from `/dev`.** Debug runs live in
`.llm/debug_<slug>/`, not `.llm/<feature_name>/`. Don't mix the two — a
feature run and a debug run on the same slug must not collide.

## Workflows

**Debugging a live issue**
1. `/dev-debug login fails after redirect` → `.llm/debug_login_redirect_fail/`.
2. Trace the code path, write `tmp_root_causes.md` with 2-5 ranked causes.
3. Present the ranking; wait for confirmation or an override.
4. Fix cause #1 only, run or identify the relevant tests, commit.
5. Repeat with the next cause if unresolved; report the commit count and the
   analysis file at the end.

**Distinguishing this from `/dev`**
1. Bug with no agreed design → `/dev-debug` (diagnosis first).
2. New feature needing a PRD → [`/dev`](../dev/AGENTS.md) (design first).

## Cross-references

- [`SKILL.md`](SKILL.md) — the skill definition.
- [`../../../AGENTS.md`](../../../AGENTS.md) — Bun monorepo parent.
- [`../AGENTS.md`](../AGENTS.md) — the `.llm/` artifact map and the `dev*` chain.
- [`../dev/AGENTS.md`](../dev/AGENTS.md) — the feature pipeline this is an alternative to.
- [`../../../CLAUDE.md`](../../../CLAUDE.md) — coding rules the fix must follow.
- [`../../../docs/tool-reference.md`](../../../docs/tool-reference.md) — useful when the bug is in MCP tool behaviour.

# `.claude/commands/` — slash commands

> Part of [`../../AGENTS.md`](../../AGENTS.md) in `packages/browseros-agent`.

## What's here

One slash command: [`browseros-review.md`](browseros-review.md), invoked as
`/browseros-review`. It is a full code-review *procedure* written as a
prompt — scope determination, a screening step, a documentation-discovery
step, a 7-agent parallel review, per-issue validation, and a fixed output
template. It is the only file in this directory.

The command is agent-facing prompt text. It is not run by Bun, not linted by
Biome, and not covered by tests.

## Contents

```
commands/
└── browseros-review.md    ← 9.5 KB, the PR / branch-diff review workflow
```

## Rules

**CM1 — The filename *is* the command name.** `browseros-review.md` →
`/browseros-review`. Adding frontmatter with a different `name` would shadow
it; the file currently has **no frontmatter at all** and starts at
`# BrowserOS Code Review`.

**CM2 — Arguments arrive via `$ARGUMENTS`.** The file treats `$ARGUMENTS` as
an optional PR number. With no argument it must ask one question offering
exactly two options (PR review vs branch diff) — that two-option contract is
deliberate, don't collapse it into a free-text prompt.

**CM3 — Preserve the high-signal filter.** The file has explicit
DO-NOT-FLAG sections (pre-existing issues, linter-caught problems, nitpicks,
subjective suggestions) and a "validate before reporting" step. Removing the
validation step is the fastest way to make this command useless.

**CM4 — Keep the output template.** The command ends with a fixed
`## Code Review Summary` shape (Scope / Files reviewed / Bugs / Type Safety /
Readability / Design Patterns / Action Items, with 🔴🟡🟢 buckets) and an
explicit "No issues found. Code looks good." variant. Downstream consumers
parse this shape.

**CM5 — Suggestion blocks must be complete.** Committable suggestion blocks
are only allowed for fixes under 5 lines that need no other edits; anything
larger gets a high-level description plus a copyable
`Fix [file:line]: …` prompt instead. Do not relax this to save tokens.

**CM6 — Commands live flat here; skills go in `../skills/`.** If the command
needs reusable, invokable logic of its own, promote it to a skill rather than
growing a multi-step prompt tree in `commands/`.

## Workflows

**Reviewing a pull request**
1. `/browseros-review 1234` → it runs `gh pr view` + `gh pr diff`.
2. Screening: it stops silently for closed/draft PRs, trivial changes, or
   PRs it has already commented on (Claude-generated PRs are still reviewed).
3. It discovers applicable `CLAUDE.md` files, summarises the change, then runs
   7 parallel reviewers (2× CLAUDE.md compliance, 2× bug hunting, 1× design
   principles, 1× design patterns, 1× readability/type safety).
4. Every bug finding is re-validated by a further subagent; unvalidated
   findings are dropped.
5. Read the `## Code Review Summary` at the end.

**Reviewing an un-pushed branch instead**
1. `/browseros-review` with no argument, then choose "Branch Diff".
2. It computes `git merge-base main HEAD` and diffs from the fork point, not
   from current `main`.

**Adding a second command**
1. Create `commands/<command-name>.md` in lowercase kebab-case.
2. Start with a `#` title; add frontmatter only if the loader needs it.
3. Use `$ARGUMENTS` for input, and end with an explicit output template.

## Cross-references

- [`../../AGENTS.md`](../../AGENTS.md) — Bun monorepo parent.
- [`../AGENTS.md`](../AGENTS.md) — `.claude/` overview and frontmatter rules.
- [`../skills/AGENTS.md`](../skills/AGENTS.md) — the `/dev`, `/dev-debug`,
  `/test-ui`, `/ts-style-review` skills (this command is not part of `/dev`).
- [`../../CLAUDE.md`](../../CLAUDE.md) — the rulebook two of its review
  agents audit against.
- [`../../.config/AGENTS.md`](../../.config/AGENTS.md) — Worktrunk, the
  parallel-branch workflow this command is usually run inside.

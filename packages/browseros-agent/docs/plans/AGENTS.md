# `docs/plans/` — dated design and implementation plans

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`.

## What's here

Feature plans kept alongside the code they describe, one file per document
type, date-prefixed so they sort chronologically. There is exactly one
feature documented today — the credits-tracking UI — in the standard two-file
shape:

| File | Size | What it is |
|---|---|---|
| `2026-03-19-credits-tracking-ui-design.md` | 2.3 KB | Design options and the chosen approach: side-panel credit badge + `/settings/usage` page, threshold colours, `useCredits()` refresh strategy, zero-credit error handling |
| `2026-03-19-credits-tracking-ui.md` | 11.0 KB | Task-by-task implementation plan: numbered tasks, `Create:`/`Modify:` file lists, code snippets, and a commit step per task |

Both are historical records of a feature, not living specifications.

## Contents

```
docs/plans/
├── 2026-03-19-credits-tracking-ui-design.md   ← why + what
└── 2026-03-19-credits-tracking-ui.md          ← how, task by task
```

## Rules

**PL1 — Filename is `<YYYY-MM-DD>-<feature-slug>[-design].md`.** kebab-case
slug, `-design` suffix for the design doc. The two files share a date and
slug; the plan reads the design.

**PL2 — A plan is superseded by the code, not by a newer plan.** Every file
the credits plan names — `apps/agent/lib/credits/useCredits.ts`,
`apps/agent/components/credits/CreditBadge.tsx`,
`apps/agent/entrypoints/sidepanel/index/{ChatHeader,useChatSession,ChatError}.tsx`,
`apps/agent/entrypoints/app/usage/UsagePage.tsx`,
`apps/agent/entrypoints/app/App.tsx`,
`apps/agent/components/sidebar/SettingsSidebar.tsx` — now exists on disk. The
plan was executed. **Read the code, not the plan, when you need current
behaviour.**

**PL3 — The plan format is task-scoped and commit-per-task.** Each task has
`Files:` (`- Create:` / `- Modify:`), numbered steps, code, and an explicit
`git commit`. Preserve that shape when adding a plan; it is what makes a plan
reviewable against the resulting diff.

**PL4 — This plan targets the superpowers workflow, not `/dev`.** Its header
reads "REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this
plan task-by-task." The `/dev` pipeline
([`../../.claude/skills/dev/`](../../.claude/skills/dev/AGENTS.md)) writes its
own artifacts to `.llm/<feature>/`, not here. Don't mix the two.

**PL5 — Plans describe intent, not enforced structure.** The credits design
names a `/settings/usage` route; verify against
`apps/agent/entrypoints/app/App.tsx` before trusting it. Nothing in the build
or test suite reads this directory.

**PL6 — Design doc and plan must be added together.** A plan without its
design loses the rationale; a design without a plan has no executable
sequence. Add both in one change, same date prefix.

## Workflows

**Adding a plan for new work**
1. Write `<date>-<slug>-design.md` — overview, per-component behaviour, data
   flow, error handling, future hooks.
2. Write `<date>-<slug>.md` — goal, architecture, tech stack, then numbered
   tasks with `Files:`, steps, and a commit per task.
3. Cross-link the two by filename.

**Answering "why is it built this way?"**
1. Find the pair in this directory.
2. Read the design doc for the decision and its trade-offs.
3. Confirm against the current code — per PL2 the plan is a record, not a spec.

**Checking whether a plan was implemented**
1. Extract the `- Create:` / `- Modify:` paths from the plan.
2. Check each path exists; for edits, `git log -- <path>` for the matching
   commit.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the parent docs folder, including the
  generated `tool-reference.md`.
- [`2026-03-19-credits-tracking-ui-design.md`](2026-03-19-credits-tracking-ui-design.md) — the design doc.
- [`2026-03-19-credits-tracking-ui.md`](2026-03-19-credits-tracking-ui.md) — the implementation plan.
- [`../../AGENTS.md`](../../AGENTS.md) — Bun monorepo parent.
- [`../../apps/agent/AGENTS.md`](../../apps/agent/AGENTS.md) — where the plan's target files live.
- [`../../.claude/skills/AGENTS.md`](../../.claude/skills/AGENTS.md) — the
  `/dev` pipeline, the other planning mechanism in this package.

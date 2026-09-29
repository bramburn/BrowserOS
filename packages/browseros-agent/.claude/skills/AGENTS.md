# `.claude/skills/` — hand-written project skills

> Part of [`../../AGENTS.md`](../../AGENTS.md) in `packages/browseros-agent`.

## What's here

Eleven hand-written skills, one directory each, each holding a `SKILL.md` with
YAML frontmatter. They split into three families: the `/dev` feature pipeline
(`dev` + `dev1-start` … `dev7-pr`, 8 skills), `/dev-debug`, and two
standalone helpers — `/test-ui` (drive the extension UI over CDP) and
`/ts-style-review` (Google TypeScript style rules, and the only skill with a
bundled reference file).

The `dev` stages are not independent tools: each one ends with a "Hand off"
step that invokes the next stage by name, passing a single snake_case
`$ARGUMENTS` slug. `dev1-start` derives the slug; `dev7-pr` is the last.

All prompt text — no Bun, Biome, or test reads it.

## Contents

```
skills/
├── dev/                    ← orchestrator: 7-stage list, invokes dev1-start
├── dev1-start/             ← derive slug, .llm/<slug>/, explore, hand off
├── dev2-design/            ← 2-4 design options, user picks, design.md
├── dev3-prd/               ← questions ([RESOLVED]/[NEEDS INPUT]) + PRD
├── dev4-implement/         ← worktree, impl plan, code, verify
├── dev5-review/            ← style + code review, tmp_review.md, commit
├── dev6-review-fix/        ← triage + apply + commit
├── dev7-pr/                ← push, gh pr create, Greptile wait, address
├── dev-debug/              ← 2-5 ranked root causes, fix #1, commit, repeat
├── test-ui/                ← dev:watch + inspect-ui.ts CDP loop (10.9 KB)
└── ts-style-review/        ← 9 rule groups + google-ts-styleguide.md (28 KB)
```

| Skill | `disable-model-invocation` | `argument-hint` |
|---|---|---|
| `dev`, `dev-debug` | `true` | yes |
| `dev1`–`dev7` | no | `[feature_name]` |
| `test-ui` | no | `[what to test]` |
| `ts-style-review` | no | none (no argument) |

## Rules

**SK1 — `<dir-name> == name:` in frontmatter.** Checked in all 11 files. This
is how the agent resolves the skill.

**SK2 — `description` is the trigger contract.** Write it as the sentence that
tells the agent *when* to reach for the skill, and for the pipeline stages
include the literal phrase "Sub-skill of the /dev workflow." so they read as
part of one system.

**SK3 — `disable-model-invocation: true` on anything that starts a long
autonomous run.** Only `dev` and `dev-debug` set it. Adding it to a mid-pipeline
stage would break the chain; adding a new autonomous entrypoint without it
would let it fire unprompted.

**SK4 — One artifact directory per run, under `.llm/`.** Feature runs use
`.llm/<snake_case_feature>/`; debug runs use `.llm/debug_<slug>/`. Stage files
are `tmp_context.md`, `tmp_exploration.md`, `design.md`, `tmp_questions.md`,
`prd.md`, `tmp_impl_plan.md`, `tmp_review.md`, `tmp_done.md`. Never write
scratch output into `skills/` itself.

**SK5 — The `dev` chain is order-dependent.** `dev` must invoke `dev1-start`,
and each stage's hand-off must name the correct successor. Change the stage
list in `dev/SKILL.md` and the hand-off lines together, never one alone.
`dev5-review` branches: actionable comments → `dev6-review-fix`, clean →
`dev7-pr` directly.

**SK6 — The pipeline pauses for the user at three points.** `dev2-design`
(option choice), `dev3-prd` (only the `[NEEDS INPUT]` questions), and `dev7-pr`
(PR approval). Do not collapse these into autonomous steps.

**SK7 — `test-ui` documents a macOS-oriented dev loop.** Its prerequisites
are `brew install go` and `/Applications/BrowserOS.app/`, and the loop assumes
`export BROWSEROS_CDP_PORT=…`. On this Windows host use the same
`bun scripts/dev/inspect-ui.ts` commands but export the env var via
`$env:BROWSEROS_CDP_PORT`. The transferable rules are: re-snapshot after every
navigation (element IDs churn), prefer `snapshot` over `screenshot`, and
re-read the CDP port from `bun run dev:watch` output because it is randomised.

**SK8 — `ts-style-review` delegates detail to its sibling file.** The
`SKILL.md` carries nine condensed rule groups; full rationale and examples live
in `ts-style-review/google-ts-styleguide.md`. Extend the reference, not the
body, and note that rule group 8 (Zod schema + `z.infer`, schema at the top of
the file that uses it) is a **team convention** layered on the Google guide —
keep the two visibly distinct.

## Workflows

**Running a full feature**
1. `/dev add user auth` → `dev` invokes `/dev1-start add_user_auth`.
2. The slug becomes the `.llm/add_user_auth/` directory; stages write
   `tmp_context.md` → `tmp_exploration.md` → `design.md` → `tmp_questions.md`
   → `prd.md` → `tmp_impl_plan.md` → `tmp_review.md` → `tmp_done.md`.
3. `dev4-implement` runs `wt switch -c feat/<slug>` if you are in the main
   worktree (see [`../../.config/AGENTS.md`](../../.config/AGENTS.md)).
4. `dev5-review` invokes `/ts-style-review` automatically for TypeScript diffs.

**Jumping into a single stage**
1. Run `/dev5-review my_feature` on its own — it only needs
   `.llm/my_feature/{prd.md,design.md}` and a clean `git diff`.

**Debugging without the full pipeline**
1. `/dev-debug login fails after redirect` — it writes
   `.llm/debug_login_redirect_fail/tmp_root_causes.md` with 2-5 ranked causes.
2. Confirm or override the ranking, then it fixes only cause #1, tests, commits.
3. If unresolved, it re-ranks and loops — no drive-by refactors.

**Adding a skill here**
1. `mkdir .claude/skills/<name>` and write `SKILL.md` with `name` +
   `description`, adding `argument-hint` if it takes input.
2. Put long-form material in a sibling `.md` and link it relatively.
3. If it starts an autonomous run, set `disable-model-invocation: true`.
4. Do not add it to `.agents/skills/` as well — the two trees are disjoint.

## Cross-references

- [`../../AGENTS.md`](../../AGENTS.md) — Bun monorepo parent.
- [`../AGENTS.md`](../AGENTS.md) — `.claude/` overview and the `.llm/` contract.
- [`../commands/AGENTS.md`](../commands/AGENTS.md) — `/browseros-review`, the
  other (non-pipeline) command.
- [`../../CLAUDE.md`](../../CLAUDE.md) — the rulebook `/ts-style-review` and
  `/browseros-review` audit against.
- [`../../.agents/AGENTS.md`](../../.agents/AGENTS.md) — the parallel
  vendored skills tree (no overlap).
- [`../../.config/AGENTS.md`](../../.config/AGENTS.md) — Worktrunk
  (`.llm/` is copied into and synced back out of worktrees).

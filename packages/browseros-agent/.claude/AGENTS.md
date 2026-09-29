# `.claude/` — agent commands and skills

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`.

## What's here

The hand-written half of the repo's agent tooling: two prompt trees, one per
convention. `commands/` holds a single slash command
(`browseros-review.md` → `/browseros-review`). `skills/` holds 11 skills in
11 directories — the `/dev` feature pipeline (`dev`, `dev1-start` …
`dev7-pr`), `/dev-debug`, `/test-ui`, and `/ts-style-review`. Everything here
is prompt text: no Bun script, Biome rule, or test reads any of it.

There is **no** `settings.json`, `CLAUDE.md`, or config file in this
directory — only `commands/` and `skills/`. A parallel `.agents/skills/` tree
exists alongside it and does not overlap (see
[`../.agents/AGENTS.md`](../.agents/AGENTS.md)).

## Contents

```
.claude/
├── commands/
│   └── browseros-review.md    ← /browseros-review: multi-agent PR review
└── skills/
    ├── dev/                   ← /dev orchestrator for the 7-step pipeline
    ├── dev1-start/ … dev7-pr/ ← the 7 pipeline stages, chained by name
    ├── dev-debug/             ← /dev-debug: root-cause-then-fix loop
    ├── test-ui/               ← /test-ui: CDP-driven extension UI testing
    └── ts-style-review/       ← /ts-style-review: Google TS style rules
```

## Rules

**CL1 — Skills live in `<name>/SKILL.md`; the directory name is the skill
name.** All 11 match their directory exactly (`dev1-start/SKILL.md` has
`name: dev1-start`). Commands are a flat `<name>.md` in `commands/`.

**CL2 — Required frontmatter on a skill: `name` and `description`.** Ten of
the 11 add `argument-hint: [<what the user types>]`. `ts-style-review` is the
exception — it takes no argument and has no `argument-hint`.

**CL3 — `disable-model-invocation: true` marks a user-invoked entrypoint.**
Only `dev` and `dev-debug` set it, so the model will not auto-fire a whole
feature pipeline. The `dev1`–`dev7` stages and `ts-style-review` omit it and
are meant to be chained.

**CL4 — Use `$ARGUMENTS`, not a parsed arg.** Every skill reads the raw
argument string via `$ARGUMENTS` (feature slug on `/dev`, `[feature_name]` on
the stages, issue text on `/dev-debug`). The pipeline passes one snake_case
slug through every stage.

**CL5 — Command files have no frontmatter.** `commands/browseros-review.md`
starts at `# BrowserOS Code Review` and uses `$ARGUMENTS` for an optional PR
number. Don't add YAML frontmatter to a command unless the loader requires it.

**CL6 — Skills write scratch artifacts to `.llm/`, never into `.claude/`.**
Stage outputs go to `.llm/<feature_name>/` (`tmp_context.md`,
`tmp_exploration.md`, `design.md`, `tmp_questions.md`, `prd.md`,
`tmp_impl_plan.md`, `tmp_review.md`, `tmp_done.md`) and debug output to
`.llm/debug_<slug>/`. `.llm/` does not exist in the package today and is
transient — it is git-shared through the Worktrunk hooks in
[`../.config/wt.toml`](../.config/wt.toml).

**CL7 — Don't duplicate into `.agents/skills/`.** The two trees are disjoint by
design: `.claude/` is BrowserOS-authored workflow, `.agents/` is vendored
upstream content pinned in [`../skills-lock.json`](../skills-lock.json).

**CL8 — `ts-style-review/` is the only skill with a bundled reference doc**
(`google-ts-styleguide.md`, 28 KB). The pattern for detail: one file next to
`SKILL.md`, linked relatively, never pasted into the body.

## Workflows

**Starting a feature**
1. Run `/dev <feature description>` — the `dev` skill immediately invokes
   `/dev1-start $ARGUMENTS` and the stages chain themselves through `/dev7-pr`.
2. Watch the artifact trail under `.llm/<snake_case_feature>/`.
3. Three stages pause for you: `/dev2-design` (pick an option), `/dev3-prd`
   (answer `[NEEDS INPUT]` questions), and `/dev7-pr` (approve the PR).

**Adding a new stage to the pipeline**
1. Create `.claude/skills/devN-<name>/SKILL.md` with `name`, `description`
   ("Sub-skill of the /dev workflow."), and `argument-hint: [feature_name]`.
2. List it in the ordered pipeline inside [`skills/dev/SKILL.md`](skills/dev/SKILL.md)
   and re-point that file's opening instruction at the new first stage.
3. End the file with a "Hand off" step that invokes the next stage by name.

**Reviewing a PR**
1. Run `/browseros-review <PR number>`, or `/browseros-review` with no
   argument and answer the one question it asks (PR vs branch diff).
2. It screens, then fans out 7 review subagents, then validates each finding
   before reporting — expect "No issues found" as a valid outcome.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — Bun monorepo parent.
- [`../CLAUDE.md`](../CLAUDE.md) — the coding rules several skills enforce
  (the review command and `/ts-style-review` both audit against it).
- [`commands/AGENTS.md`](commands/AGENTS.md) — slash-command format.
- [`skills/AGENTS.md`](skills/AGENTS.md) — the 11 skills and their frontmatter contract.
- [`../.agents/AGENTS.md`](../.agents/AGENTS.md) — the parallel vendored skills tree.
- [`../.config/AGENTS.md`](../.config/AGENTS.md) — Worktrunk hooks that share `.llm/` across worktrees.

# `.claude/skills/dev1-start/` — stage 1: start & explore

> Part of [`../../../AGENTS.md`](../../../AGENTS.md) in `packages/browseros-agent`.

## What's here

Stage 1 of the `/dev` pipeline. It does three things: derive a snake_case
feature slug from `$ARGUMENTS`, create the run's scratch directory
`.llm/<feature_name>/`, and do a deliberately *shallow* orientation pass
(stack, top-level layout, where the feature likely fits). Its output is
`tmp_context.md` and `tmp_exploration.md`; it then invokes `/dev2-design`.

Prompt text only.

## Contents

```
dev1-start/
└── SKILL.md    ← 2.0 KB: name, description ("Sub-skill of the /dev workflow."), argument-hint
```

## Rules

**D1S — `argument-hint: [feature description]`, but the body treats
`$ARGUMENTS` as the description.** Unlike stages 2-7, which receive an
already-derived slug, this stage is the only one that sees free text. It owns
the slug derivation (`"add user auth"` → `add_user_auth`).

**D2S — The slug is the run's identity.** It names `.llm/<feature_name>/` and
is what every later stage receives. Once created, it is never renamed
mid-pipeline.

**D3S — `.llm/` is scratch, not source.** The directory is not in the repo
today; Worktrunk copies it into and out of worktrees via the `llm` /
`llm-sync` hooks in
[`../../../.config/wt.toml`](../../../.config/wt.toml). Don't commit it.

**D4S — `tmp_context.md` is verbatim.** It records the original request
unchanged, plus a timestamp and the working directory — it is the audit trail
for what the user actually asked for.

**D5S — Keep the exploration shallow.** The skill explicitly says "This is not
a deep dive". Deep analysis belongs in `dev4-implement`; expanding stage 1
re-introduces the cost the pipeline was designed to avoid.

**D6S — `dev1-start` must read `CLAUDE.md` first.** Step 2.1 instructs the
agent to read the project-root `CLAUDE.md` (or `.claude/CLAUDE.md`) before
exploring — that is where the authoritative coding rules live.

## Workflows

**Pipeline entry**
1. Invoked by [`dev/`](../dev/SKILL.md) as `/dev1-start $ARGUMENTS`.
2. Writes `.llm/<slug>/tmp_context.md` and `tmp_exploration.md`.
3. Summarises stack + structure + likely fit to the user, then invokes
   `/dev2-design <slug>`.

**Running only this stage**
1. `/dev1-start add credits badge`
2. Read `.llm/add_credits_badge/tmp_exploration.md` for the orientation notes
   that stages 2-7 build on.

## Cross-references

- [`SKILL.md`](SKILL.md) — the stage definition.
- [`../../../AGENTS.md`](../../../AGENTS.md) — Bun monorepo parent.
- [`../dev/AGENTS.md`](../dev/AGENTS.md) — the orchestrator that invokes it.
- [`../dev2-design/AGENTS.md`](../dev2-design/AGENTS.md) — the stage it hands off to.
- [`../AGENTS.md`](../AGENTS.md) — the `.llm/` artifact map for all stages.
- [`../../../CLAUDE.md`](../../../CLAUDE.md) — the file this stage reads first.

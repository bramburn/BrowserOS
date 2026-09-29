# `.config/` — Worktrunk configuration

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`.

## What's here

Worktrunk (`wt`) configuration for running parallel agent branches in git
worktrees. Two files: [`README.md`](README.md) is the human-facing setup guide
(install via `brew install max-sixty/worktrunk/wt`, `wt config shell install`,
then `wt switch -c -x claude feat-name`), and [`wt.toml`](wt.toml) holds the
two hook tables the `wt` CLI actually executes. Nothing here is read by Bun,
Biome, WXT, or any runtime code.

This directory is distinct from the repo-root `D:\BrowserOs\.config/`, which
carries its own `README.md` and `wt.toml` for the fork-level worktrees.

## Contents

```
.config/
├── README.md    ← 880 B: install steps, quick-command table, hook summary
└── wt.toml      ← 495 B: [post-create] and [pre-remove] hook definitions
```

`wt.toml` in full, by hook:

| Hook | Key | Command | Effect |
|---|---|---|---|
| `post-create` | `install` | `bun install` | install deps in the new worktree |
| `post-create` | `env` | `for f in {{ repo_root }}/apps/*/.env.*; do … cp …` | copy every `apps/*/.env.*` from the main tree |
| `post-create` | `llm` | `cp -r {{ repo_root }}/.llm . 2>/dev/null \|\| true` | copy the agent-pipeline scratch dir in |
| `pre-remove` | `llm-sync` | `rsync -au .llm/ {{ repo_root }}/.llm/ …` | sync scratch artifacts back to the main tree |

Two per-app `env-*` hooks are present but **commented out**, superseded by the
generic `env` loop.

## Rules

**CF1 — `{{ repo_root }}` is Worktrunk's placeholder** for the main worktree.
Don't hardcode absolute paths; worktrees live outside the repo root and the
placeholder is what makes the copy portable.

**CF2 — The `llm` / `llm-sync` pair is why `/dev` survives worktree
switches.** `.llm/` holds every artifact of the
[`/dev` pipeline](../.claude/skills/AGENTS.md) (`prd.md`, `design.md`,
`tmp_review.md`, …). It is copied in on create and rsynced back on remove, so
a run started in one worktree can finish in another. Preserve both hooks
together — dropping one loses pipeline state.

**CF3 — These hooks are POSIX shell.** `for … cp`, `rsync`, and `/dev/null`
all require bash. They work in the repo's dev flow (which drives
`tools/dev/run.sh`), not in Windows PowerShell. Do not "fix" them with
PowerShell syntax — the shell is chosen by `wt`, not by this file.

**CF4 — `.llm/` is scratch and stays uncommitted.** Everything it holds is
transient pipeline state. Nothing here should be added to `.gitignore` config
in this folder; the copy hooks assume the directory may not exist yet, which
is why they end in `2>/dev/null || true`.

**CF5 — The `env` hook copies secrets deliberately.** It duplicates
`apps/*/.env.*` (API keys, PostHog tokens) into every worktree. Keep that
behaviour; don't add new `cp` targets that widen the blast radius.

**CF6 — Keep `README.md` and `wt.toml` in step.** The README documents the
hook list; a hook added to `wt.toml` without a README line is undocumented
behaviour.

## Workflows

**Starting a parallel feature branch**
1. `wt switch -c -x claude feat-name` — creates `../browseros-agent.feat-name/`
   (or the sibling path Worktrunk derives), launches Claude Code.
2. Hooks fire automatically: `bun install`, `.env.*` copy, `.llm/` copy.
3. Work normally; the pipeline writes into the copied `.llm/`.

**Cleaning up**
1. `wt remove feat-name` — fires `pre-remove` `llm-sync`, rsyncing `.llm/`
   back into the main worktree before deletion.
2. Verify `ls .llm/` in the main tree if you were mid-pipeline.

**Adding a hook**
1. Add it under `[post-create]` or `[pre-remove]` in `wt.toml`.
2. Make it idempotent and non-fatal (`2>/dev/null || true`).
3. Document it in `README.md` under "Hooks".

**Using it from the `/dev` pipeline**
1. `dev4-implement` runs `wt switch -c feat/<slug>` when it detects it is in
   the main worktree (see
   [`../.claude/skills/dev4-implement/AGENTS.md`](../.claude/skills/dev4-implement/AGENTS.md)).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — Bun monorepo parent.
- [`README.md`](README.md) — Worktrunk setup guide.
- [`wt.toml`](wt.toml) — the hook definitions.
- [`../.claude/skills/AGENTS.md`](../.claude/skills/AGENTS.md) — the `/dev`
  pipeline whose `.llm/` artifacts these hooks carry between worktrees.
- [`../.vscode/AGENTS.md`](../.vscode/AGENTS.md) — the other dev-tooling
  directory in this package.

# `.agents/` — vendored agent skills tree

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`.

## What's here

A second, parallel agent-skills tree living next to `.claude/skills/`. It
contains exactly one skill — `skills/ai-sdk/` — which is **vendored from
upstream Vercel**, not hand-written for this repo. The pin is recorded in
[`../skills-lock.json`](../skills-lock.json), which lists `ai-sdk` with
`"source": "vercel/ai"`, `"sourceType": "github"` and a `computedHash`. There is
no `commands/`, no settings file, and no other skill here.

This is developer tooling for coding agents. Nothing in this directory is
compiled, bundled, or shipped to users; Bun, Biome, and the test suite never
read it.

## Contents

```
.agents/
├── skills/
│   └── ai-sdk/          ← SKILL.md + references/ (Vercel AI SDK docs helper)
└── skills-lock.json is at the package root, not in this directory
```

| Entry | What it is |
|---|---|
| `skills/ai-sdk/SKILL.md` | Skill entry point: trigger list, "don't trust memory" rule, doc-lookup procedure |
| `skills/ai-sdk/references/` | 4 reference docs loaded on demand via relative links |

## Rules

**AG1 — Vendored, not authored here.** `skills/ai-sdk/` is a copy of an
upstream skill. Fix the upstream, or document the deviation — don't quietly
fork BrowserOS-specific guidance into it.

**AG2 — `skills-lock.json` is the pin.** If you re-vendor or update the skill,
update the `computedHash` / `source` entry in
[`../skills-lock.json`](../skills-lock.json) in the same change, or the lock
becomes a lie.

**AG3 — Do not mirror skills from `.claude/skills/` into this tree** (or vice
versa) to "fix" the difference. They serve different owners: `.claude/` is
hand-written project workflow, `.agents/` is upstream content. Adding the same
skill twice guarantees drift.

**AG4 — Frontmatter here uses `name` + `description`.** That matches the
`.claude/skills/` convention for entry points. Note the *reference* files under
`references/` use a different two-key form (`title` + `description`) — see
[`skills/ai-sdk/references/AGENTS.md`](skills/ai-sdk/references/AGENTS.md).

## Workflows

**Using the ai-sdk skill when writing agent code**
1. Open [`skills/ai-sdk/SKILL.md`](skills/ai-sdk/SKILL.md) and check the
   `Prerequisites` section — it requires `node_modules/ai/docs/` to exist.
2. Follow its "Do Not Trust Internal Knowledge" rule: grep
   `node_modules/ai/src/` and `node_modules/ai/docs/` before writing AI SDK code.
3. On a type error, grep
   [`skills/ai-sdk/references/common-errors.md`](skills/ai-sdk/references/common-errors.md)
   *first* — it documents the renamed/removed parameters.
4. Run `bun run typecheck` from the package root afterwards.

**Updating the vendored copy**
1. Fetch the newer upstream skill.
2. Diff it against `skills/ai-sdk/` and keep only intended changes.
3. Recompute and update `computedHash` in [`../skills-lock.json`](../skills-lock.json).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — Bun monorepo parent (authoritative conventions).
- [`../CLAUDE.md`](../CLAUDE.md) — coding guidelines (authoritative for this sub-package).
- [`../skills-lock.json`](../skills-lock.json) — the skill pin this tree is governed by.
- [`../.claude/AGENTS.md`](../.claude/AGENTS.md) — the hand-written sibling tree.
- [`../.claude/skills/AGENTS.md`](../.claude/skills/AGENTS.md) — hand-written project workflows.

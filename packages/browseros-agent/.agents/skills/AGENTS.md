# `.agents/skills/` — skill registry for this tree

> Part of [`../../AGENTS.md`](../../AGENTS.md) in `packages/browseros-agent`.

## What's here

The `skills/` level of the `.agents/` tree. Convention: one directory per
skill, named after the skill, each containing a `SKILL.md` entry point plus an
optional `references/` directory of detail docs. Exactly one skill is present
today: `ai-sdk/`, vendored from Vercel's AI SDK. There is no shared index,
manifest, or registry file in this directory — discovery is by directory scan
of `SKILL.md` files.

## Contents

```
.agents/skills/
└── ai-sdk/                  ← Vercel AI SDK helper (vendored from vercel/ai)
    ├── SKILL.md             ← entry point: frontmatter + trigger + procedure
    └── references/          ← 4 on-demand reference docs
```

## Rules

**AS1 — One directory per skill, `SKILL.md` inside it.** No loose `.md` files
at this level; nothing else belongs directly in `skills/`.

**AS2 — The directory name is the skill's public name.** `ai-sdk/` → the
`name: ai-sdk` in its frontmatter. A mismatch means the agent resolves the
wrong skill.

**AS3 — Keep detail out of `SKILL.md`.** Long-form material goes in
`references/*.md` and is pulled in with a relative markdown link from
`SKILL.md`. Today's skill does exactly this in four places.

**AS4 — New skills here must also be added to
[`../../skills-lock.json`](../../skills-lock.json)** with `source`,
`sourceType`, and `computedHash`. That lock file is the only record of
provenance for this tree.

**AS5 — Skills are prompts, not code.** There is no test, lint, or typecheck
coverage for anything in this directory. A malformed frontmatter key fails
silently at agent-load time, so re-read the file after editing it.

## Workflows

**Adding a skill to this tree**
1. Create `<skill-name>/SKILL.md` with `name` and `description` frontmatter.
2. Put long content in `<skill-name>/references/*.md` and link it relatively.
3. Add the entry to [`../../skills-lock.json`](../../skills-lock.json).
4. Confirm the directory name matches the `name:` field exactly.

**Reading a skill before acting on it**
1. Read `<skill-name>/SKILL.md` first — the description tells you when it
   applies and the body gives the procedure.
2. Follow only the `references/` links the skill actually points you at.
   `ai-sdk` links all four; don't read them as a batch.

## Cross-references

- [`../../AGENTS.md`](../../AGENTS.md) — Bun monorepo parent.
- [`../AGENTS.md`](../AGENTS.md) — this tree's root: provenance and vendoring rules.
- [`../../.claude/skills/AGENTS.md`](../../.claude/skills/AGENTS.md) — the
  separate, hand-written skills tree (11 skills, no overlap).
- [`ai-sdk/AGENTS.md`](ai-sdk/AGENTS.md) — the only skill present.
- [`../../skills-lock.json`](../../skills-lock.json) — the provenance lock.

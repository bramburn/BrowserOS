# `.agents/skills/ai-sdk/` — AI SDK reference skill

> Part of [`../../../AGENTS.md`](../../../AGENTS.md) in `packages/browseros-agent`.

## What's here

A single vendored skill that teaches a coding agent how to write AI SDK code
without hallucinating APIs. `SKILL.md` is the entry point: it declares the
`ai-sdk` name, a long trigger list, and a hard rule — *"Everything you know
about the AI SDK is outdated or wrong"* — followed by a lookup procedure
(`node_modules/ai/docs/` → `node_modules/ai/src/` → `ai-sdk.dev`). Four
on-demand reference docs live in `references/`. Nothing here is executed;
this is prompt text for agents.

The upstream origin is recorded in
[`../../../skills-lock.json`](../../../skills-lock.json) as `vercel/ai` (GitHub
source type) with a `computedHash`.

## Contents

```
.agents/skills/ai-sdk/
├── SKILL.md                      ← entry point (4.7 KB)
└── references/
    ├── ai-gateway.md             ← Vercel AI Gateway auth + model lookup
    ├── common-errors.md          ← renamed/removed API parameters (11 KB)
    ├── devtools.md               ← @ai-sdk/devtools capture + web UI
    └── type-safe-agents.md       ← InferAgentUIMessage end-to-end typing
```

`SKILL.md` frontmatter is `name: ai-sdk` plus a single-quoted, multi-sentence
`description` ending in explicit `Triggers on: "…"` keywords. The four
reference files use a *different* two-key frontmatter (`title` +
`description`) — see [`references/AGENTS.md`](references/AGENTS.md).

## Rules

**AI1 — This is vendored content.** `ai-sdk/` is a copy of an upstream skill
(`vercel/ai`). Do not add BrowserOS-specific instructions to it; put those in
a `.claude/skills/` skill instead, or upstream the change.

**AI2 — Keep the "do not trust internal knowledge" rule intact.** It is the
reason the skill exists. If you edit `SKILL.md`, keep the Prerequisites block
(`node_modules/ai/docs/` must exist) and the verification ordering: local docs
first, ai-sdk.dev only as a fallback.

**AI3 — Reference links are relative and load-bearing.** `SKILL.md` links all
four `references/*.md` files by filename. Renaming or moving a reference file
breaks the skill silently.

**AI4 — Facts in `common-errors.md` are version-sensitive.** Its renames
(`maxTokens` → `maxOutputTokens`, `maxSteps` → `stopWhen: stepCountIs(n)`, and
others) describe the current AI SDK 6 surface. Before trusting an entry, check
it against the installed `ai` package; don't extend the list from memory.

**AI5 — Don't treat the skill as executable docs for the repo.** It documents
the *upstream* AI SDK. The actual agent code in this monorepo lives in
`apps/server/src/agent/` — see [`../../../apps/server/AGENTS.md`](../../../apps/server/AGENTS.md).

## Workflows

**Writing or changing AI SDK code in this monorepo**
1. Read [`SKILL.md`](SKILL.md) and follow its Prerequisites (confirm
   `node_modules/ai/docs/` exists).
2. Grep `node_modules/ai/src/` and `node_modules/ai/docs/` for the API you
   intend to use — do not answer from memory.
3. On a type error, grep
   [`references/common-errors.md`](references/common-errors.md) for the failing
   property before searching source.
4. Pick the provider via the gateway and fetch current model IDs
   ([`references/ai-gateway.md`](references/ai-gateway.md)).
5. Run `bun run typecheck` from `packages/browseros-agent`.

**Adding a new reference doc**
1. Write it into `references/` with `title` + `description` frontmatter.
2. Link it from the `## References` list in `SKILL.md` and reference it inline
   at the point of use.
3. Bump `computedHash` in
   [`../../../skills-lock.json`](../../../skills-lock.json).

## Cross-references

- [`../../../AGENTS.md`](../../../AGENTS.md) — Bun monorepo parent.
- [`../AGENTS.md`](../AGENTS.md) — skills-tree conventions.
- [`references/AGENTS.md`](references/AGENTS.md) — the reference-doc format.
- [`../../../CLAUDE.md`](../../../CLAUDE.md) — coding guidelines (authoritative).
- [`../../../apps/server/AGENTS.md`](../../../apps/server/AGENTS.md) — where
  AI SDK code actually lives (`src/agent/`, `provider-factory.ts`).

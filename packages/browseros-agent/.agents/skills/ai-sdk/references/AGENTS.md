# `.agents/skills/ai-sdk/references/` — on-demand skill references

> Part of [`../../../../AGENTS.md`](../../../../AGENTS.md) in `packages/browseros-agent`.

## What's here

Four reference documents for the `ai-sdk` skill. They are **not read on skill
load** — [`../SKILL.md`](../SKILL.md) links them by relative path and the
agent is told to open only the one a given situation calls for. Each is a
plain, self-contained explainer with runnable snippets.

| File | Size | What it answers |
|---|---|---|
| `ai-gateway.md` | 2.2 KB | How to authenticate to and call the Vercel AI Gateway; how to list current model IDs per provider with `curl` + `jq` |
| `common-errors.md` | 11 KB | The big one: AI SDK parameters that were renamed or removed (`maxTokens` → `maxOutputTokens`, `maxSteps` → `stopWhen: stepCountIs(n)`, …), each as a ❌/✅ pair |
| `devtools.md` | 1.2 KB | Capturing every `generateText` / `streamText` / `ToolLoopAgent` call to `.devtools/generations.json` via `@ai-sdk/devtools` middleware, and the `npx @ai-sdk/devtools` UI on port 4983 |
| `type-safe-agents.md` | 5.3 KB | File layout for agents/tools and `InferAgentUIMessage<typeof agent>` for type-safe `useChat` |

## Contents

```
references/
├── ai-gateway.md        ← gateway auth, `gateway('provider/model')`, model-ID lookup
├── common-errors.md     ← deprecated/renamed AI SDK parameters
├── devtools.md          ← local call capture + web UI
└── type-safe-agents.md  ← `lib/agents/` + `lib/tools/` layout, end-to-end typing
```

## Rules

**RF1 — Frontmatter here is `title` + `description`, not `name`.** These files
are not skills and are not discovered by skill name. `SKILL.md` uses
`name` + `description`; do not "normalise" the two.

**RF2 — Keep `common-errors.md` grep-shaped.** `SKILL.md` tells the agent to
grep it *by the failing property name* before searching source. Entries must
lead with the exact identifier in a heading (e.g. `## \`maxTokens\` →
\`maxOutputTokens\``), not bury it in prose.

**RF3 — Commands here are POSIX shell.** `ai-gateway.md` and `devtools.md`
use `curl`, `jq`, `cat`, `npx`. This monorepo's dev scripts are `.sh`-driven
(`tools/dev/run.sh`) but the dev host here is Windows PowerShell — translate
before pasting, and never pipe file content through PowerShell 5.1 (it
corrupts non-ASCII).

**RF4 — Never add repo-specific facts here.** These docs describe upstream AI
SDK behaviour. BrowserOS specifics belong in
`../../../../CLAUDE.md` or `../../../../apps/server/AGENTS.md`.

**RF5 — `devtools.md` output lands in `.devtools/generations.json`.** That is
captured LLM traffic; treat it as sensitive and never commit it.

## Workflows

**A typecheck fails with an AI SDK property name**
1. `grep` the property in [`common-errors.md`](common-errors.md).
2. If present, apply the documented replacement.
3. If absent, search `node_modules/ai/src/` and `node_modules/ai/docs/`.
4. Re-run `bun run typecheck` from `packages/browseros-agent`.

**Choosing a model ID**
1. Read [`ai-gateway.md`](ai-gateway.md).
2. Fetch the live list — never a model ID from memory:
   `curl -s https://ai-gateway.vercel.sh/v1/models | jq -r '[.data[] | select(.id | startswith("anthropic/")) | .id] | reverse | .[]'`
3. Take the highest version number, and do **not** truncate with `head`.

**Inspecting a live agent run**
1. Install `@ai-sdk/devtools` and wrap the model with `devToolsMiddleware()`
   per [`devtools.md`](devtools.md).
2. Read `.devtools/generations.json` directly, or run `npx @ai-sdk/devtools`
   and open `http://localhost:4983`.

## Cross-references

- [`../SKILL.md`](../SKILL.md) — the skill that links these four docs.
- [`../AGENTS.md`](../AGENTS.md) — the `ai-sdk` skill itself.
- [`../references` sibling dir index: `../../AGENTS.md`](../../AGENTS.md) — the skills tree.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.
- [`../../../../CLAUDE.md`](../../../../CLAUDE.md) — coding guidelines.

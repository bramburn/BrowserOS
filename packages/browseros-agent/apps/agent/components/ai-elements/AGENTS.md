# `components/ai-elements/` — Vendored chat-message primitives

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/components`.

## What's here

30 vendored primitives for rendering AI conversation content: message and
reasoning parts, tool calls, task/plan chains, sources and inline
citations, the prompt composer, the model picker, and a small
React-Flow-based graph surface. They are typed against the Vercel AI SDK
(`UIMessage`, `ToolUIPart`, `DynamicToolUIPart` from the `ai` package) and
compose `components/ui/` underneath. Every export carries a `@public`
JSDoc tag, which is the signature of an upstream/ported file.

## Contents

```
ai-elements/
├── message, reasoning, loader, shimmer          ← assistant turn pieces
├── tool, task, checkpoint, chain-of-thought, plan
│                                                 ← tool-call / plan rendering
│                                                 (ExecutionStepItem.tsx
│                                                 consumes ./tool)
├── source*, inline-citation, suggestion, artifact, canvas, image, code-block
├── prompt-input, model-selector, open-in-chat   ← composer
├── conversation, controls, toolbar, panel, run-result-dialog
├── context                                      ← token-usage / cost hovercard
│                                                 (tokenlens-backed)
└── node, edge, connection, web-preview          ← @xyflow/react graph pieces
```

## Rules

- **AIE1 — Treat as vendored: do not hand-edit.** These are ported from the
  ai-elements registry. Re-pull or wrap instead; local behaviour goes in a
  consumer under `components/` or `entrypoints/`.
- **AIE2 — Preserve the `@public` JSDoc tag and the `XProps = ComponentProps<...>`
  pattern.** These are what make the primitives mergeable by consumers.
- **AIE3 — Props are AI-SDK shaped, not app shaped.** Accept `UIMessage` parts
  (`ToolUIPart`, `DynamicToolUIPart`) directly; do not introduce a parallel
  "BrowserOS tool" type here. App-level records live in
  `lib/execution-history/types.ts` and are mapped at the call site.
- **AIE4 — Compose `components/ui/`, don't inline raw Radix.** `tool.tsx` and
  `context.tsx` both build on `Collapsible`, `Badge`, `HoverCard`, `Progress`.
- **AIE5 — `context.tsx` computes cost via `tokenlens`.** Any change to token
  accounting must keep `getUsage({ modelId, usage })` inputs intact.

## Workflows

**Rendering a new tool-call state**
1. Check whether `tool.tsx` already covers the `ToolUIPart['state']` union.
2. If not, add the branch in a wrapper component outside this directory and
   pass the result into `ToolInput` / `ToolOutput`.

**Wiring the token-usage hovercard**
1. Wrap the trigger in `Context` with `usedTokens`, `maxTokens`, and
   optionally `usage` + `modelId`.
2. Use `ContextContent*` children for a custom layout; the defaults render
   percentage, progress bar, and USD totals.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — component-tree rules (CMP2).
- [`ui/AGENTS.md`](../ui/AGENTS.md) — the Radix base these build on.
- [`../execution-history/AGENTS.md`](../execution-history/AGENTS.md) — the main consumer of `tool.tsx`.
- [`../../lib/execution-history/AGENTS.md`](../../lib/execution-history/AGENTS.md) — `ExecutionStepRecord` types.
- [`../../package.json`](../../package.json) — `ai`, `@ai-sdk/react`, `@xyflow/react`, `tokenlens`, `shiki`, `streamdown`.

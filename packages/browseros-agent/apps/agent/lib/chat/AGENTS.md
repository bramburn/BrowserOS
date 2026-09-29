# `lib/chat/` — Harness adapter visibility

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

Two files and one job: deciding which agent-harness adapters the user is
allowed to see. `HIDDEN_ADAPTERS` currently contains only `hermes` — the
adapter still exists in the backend and in the TypeScript union, but is
withheld from every picker, settings list, and create dialog. Keeping that
switch in one 20-line module means re-enabling it is a one-line change
rather than a hunt.

## Contents

```
chat/
├── adapter-visibility.ts       ← HIDDEN_ADAPTERS: ReadonlySet<'hermes'>;
│                                 isAdapterHidden(adapter);
│                                 visibleAdapters(descriptors) — filters a
│                                 HarnessAdapterDescriptor[] by id
└── adapter-visibility.test.ts  ← asserts hermes is hidden and the rest pass
```

## Rules

- **CHV1 — Filtering happens at the call site, via `visibleAdapters()`.** The
  components do not know about `HIDDEN_ADAPTERS`; anything that builds a
  provider list (`components/chat/ChatProviderSelector`) must run the
  descriptors through this filter first.
- **CHV2 — Hiding is presentation-only.** The adapter stays in
  `HarnessAgentAdapter` (from
  `entrypoints/app/agents/agent-harness-types`) and remains valid to the
  server. Don't delete the union member to "remove" one.
- **CHV3 — Add to `HIDDEN_ADAPTERS`; never branch on the name elsewhere.**
  A second `adapter === 'hermes'` check outside this file will drift.
- **CHV4 — Feature gating is a different mechanism.** Whole-feature gating
  (browser/server version) belongs in `../browseros/capabilities.ts`. This
  folder is per-adapter visibility with no version logic.
- **CHV5 — `visibleAdapters` takes descriptors, not ids.** It returns the
  same objects it was given, so callers keep their `agentId` / `adapterName`
  metadata.

## Workflows

**Hiding an adapter**
1. Add its id to `HIDDEN_ADAPTERS` in `adapter-visibility.ts`.
2. Add the assertion to `adapter-visibility.test.ts`.
3. Confirm every picker still renders — no call site needs editing.

**Re-enabling `hermes`**
1. Remove `'hermes'` from `HIDDEN_ADAPTERS` (delete the set entirely if it
   becomes empty).
2. Update `adapter-visibility.test.ts` to expect it visible.
3. Check the version gate in `../browseros/capabilities.ts`
   (`Feature.AGENT_HARNESS_SUPPORT`) still matches the target browsers.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB7).
- [`../../components/chat/AGENTS.md`](../../components/chat/AGENTS.md) — the picker that consumes `visibleAdapters()`.
- [`../browseros/AGENTS.md`](../browseros/AGENTS.md) — `Feature.AGENT_HARNESS_SUPPORT`.
- [`../../entrypoints/app/agents/AGENTS.md`](../../entrypoints/app/agents/AGENTS.md) — `HarnessAdapterDescriptor` / `HarnessAgentAdapter`.
- [`adapter-visibility.test.ts`](adapter-visibility.test.ts) — run with `bun test`.

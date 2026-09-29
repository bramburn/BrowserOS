# `entrypoints/app/agents/` — agent-harness data layer (adapters, agents, CRUD)

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/app`.

## What's here

The client for the local agent-harness HTTP API (`<baseUrl>/agents/*`, base URL from
`@/lib/browseros/useBrowserOSProviders`). This folder is a **data layer**, not a
route: it exports `useAgentAdapters`, `useHarnessAgents` and the create/update/delete
mutations, plus the TypeScript types for the harness wire format. `ai-settings` and
`agent-command` both consume it; there is no `/agents` page of its own (that route
redirects — see `app/App.tsx`).

Three adapters exist: `claude`, `codex`, `hermes`. `hermes` is hidden from the UI by
`visibleAdapters` / `isAdapterHidden` in `@/lib/chat/adapter-visibility`.

## Contents

```
agents/
├── useAgents.ts             ← all TanStack Query hooks + agentsFetch + AGENT_QUERY_KEYS
├── agent-api-url.ts         ← buildAgentApiUrl(baseUrl, path)
├── agent-harness-types.ts   ← HarnessAgent, HarnessAdapterDescriptor, stream-event union
├── agents-page-types.ts     ← AgentListItem (page-facing shape) + ProviderOption
├── agents-page-utils.ts     ← toHarnessListItem(), formatHarnessAdapter()
├── agents-page-hooks.ts     ← useDefaultAgentName()
├── agents-list-order.ts     ← orderAgentsByPinThenRecency / compareAgentsByPinThenRecency
├── agent-display.helpers.ts ← displayName, formatRelativeTime, livenessDetail
├── AgentList.tsx            ← the ordered rows + empty state
├── AgentRowCard.tsx         ← composition shell for one row
├── AgentsEmptyState.tsx     ← no-agents panel
├── NewAgentDialog.tsx       ← create form (adapter-locked by the caller)
├── ProviderSelector.tsx     ← model / reasoning controls
├── LivenessDot.tsx          ← working | idle | asleep | error | unknown dot
├── AdapterIcon.tsx          ← per-adapter icon + adapterLabel()
├── PageAlerts.tsx           ← InlineErrorAlert etc.
├── agents-list-order.test.ts
├── useAgents.test.ts
└── agent-row/               ← row sub-components (see its own AGENTS.md)
```

## Rules

- **AG1 — All harness I/O goes through `useAgents.ts`.** No component calls
  `fetch(.../agents)` directly. `agentsFetch` already handles the `baseUrl` join and
  unwraps `{ error }` bodies into real `Error`s.
- **AG2 — Every query is gated on capability + resolved base URL.** `useAgentAdapters`
  is `enabled: Boolean(baseUrl) && !urlLoading && enabled && agentsSupported`. Copy
  that predicate when adding a query; an ungated query fires against an empty URL.
- **AG3 — Query keys come from `AGENT_QUERY_KEYS`** (`adapters`, `agents`) and always
  include the resolved `baseUrl` — the port changes when the server rebinds, and a
  stale cache from a dead port renders as an empty page.
- **AG4 — Two orderings exist and are not interchangeable.**
  `agents-list-order.ts` (pin → recency → id) serves list/rail surfaces;
  `home-agent-card.helpers.ts` in `../agent-command` floats the active turn first.
  The file header says so — do not unify them.
- **AG5 — Page shapes come from `agents-page-utils.ts`.** Convert a `HarnessAgent` to
  an `AgentListItem` with `toHarnessListItem`; do not hand-build the object in a
  component, or `key`/`runtimeLabel` will drift.
- **AG6 — Keep `agent-display.helpers.ts` React-free.** It is pure formatting
  (`displayName`, `formatRelativeTime`, `livenessDetail`) precisely so the row card
  stays a layout component.
- **AG7 — Hidden adapters stay hidden.** Filtering is centralised in
  `@/lib/chat/adapter-visibility`. Do not add a second `adapter !== 'hermes'` guard.

## Workflows

**Adding a field to `HarnessAgent`**
1. Add it to `HarnessAgent` in `agent-harness-types.ts`.
2. If the row or card should show it, extend `AgentRowData` in `agent-row/agent-row.types.ts`.
3. Thread it through `toHarnessListItem` (`agents-page-utils.ts`) or the pane that
   assembles `AgentRowData`.
4. Add a formatter to `agent-display.helpers.ts` if it needs one.

**Adding a harness mutation**
1. Add the mutation hook to `useAgents.ts` next to the existing CRUD hooks.
2. Reuse `agentsFetch<T>` so the error body is surfaced.
3. Invalidate the affected `AGENT_QUERY_KEYS.*` query in `onSuccess`.
4. Add a case to `useAgents.test.ts` if the hook does real work beyond the call.

**Adding a new harness adapter**
1. Extend `HarnessAgentAdapter` in `agent-harness-types.ts`.
2. Add the icon + label to `AdapterIcon.tsx` and the display name to
   `agents-page-utils.ts#formatHarnessAdapter`.
3. Decide visibility in `@/lib/chat/adapter-visibility` (hidden adapters still work,
   they are just not offered).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — app route table.
- [`./agent-row/AGENTS.md`](./agent-row/AGENTS.md) — row sub-components and their data contract.
- [`../ai-settings/AGENTS.md`](../ai-settings/AGENTS.md) — the only UI that mounts `AgentList`.
- [`../agent-command/AGENTS.md`](../agent-command/AGENTS.md) — the chat surface built on these hooks.
- [`../../../../lib/chat/adapter-visibility.ts`](../../../lib/chat/adapter-visibility.ts) — Hermes filtering.

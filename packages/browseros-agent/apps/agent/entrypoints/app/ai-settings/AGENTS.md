# `entrypoints/app/ai-settings/` — AI & Agents settings (`/settings/ai`)

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/app`.

## What's here

The `/settings/ai` page. It is a tabbed master–detail shell: the active pane is
driven by the `?section=` search param, so a deep link like
`/settings/ai?section=claude` opens straight onto one harness adapter. The default
section (`browseros`) shows LLM provider configuration; every other section shows
that adapter's agent list.

This is the largest settings folder and the only place `NewProviderDialog.tsx`
(≈39 KB) lives — provider templates, model pickers, base URLs and API keys are all
configured here.

## Contents

```
ai-settings/
├── AISettingsPage.tsx        ← tab shell; reads ?section=, renders BrowserOsAiPane or AdapterAgentsPane
├── ai-settings-sections.ts   ← BROWSEROS_SECTION + resolveAiSettingsSection()
├── ai-settings-sections.test.ts
├── BrowserOsAiPane.tsx       ← LLM-provider pane: templates, configured list, OAuth, delete
├── AdapterAgentsPane.tsx     ← per-adapter agent pane (delegates to ../agents)
├── NewProviderDialog.tsx     ← add/edit an LLM provider (largest file here)
├── ProviderCard.tsx, ProviderTemplateCard.tsx, ProviderTemplatesSection.tsx
├── ConfiguredProvidersList.tsx
├── IncompleteProviderCard.tsx, IncompleteProvidersList.tsx
├── LlmProvidersHeader.tsx, McpPromoBanner.tsx
├── DeviceCodeDialog.tsx      ← device-code OAuth flow UI
├── models.ts                 ← getModelsForProvider / getModelContextLength (models.dev + overrides)
└── graphql/
    └── aiSettingsDocument.ts ← GetRemoteLlmProviders + DeleteRemoteLlmProvider
```

## Rules

- **AIS1 — Unknown `?section=` values fall back to `browseros`, silently and on
  purpose.** `resolveAiSettingsSection` must return `BROWSEROS_SECTION` for a missing
  param, a hidden adapter, a stale link, and the window before adapters load. Never
  make it throw or render an empty pane.
- **AIS2 — Adding a section is a two-file change:** an entry in the `items` array in
  `AISettingsPage.tsx`, and a branch in the pane switch. Adapters come from
  `useAgentAdapters()` + `visibleAdapters` — do not hardcode the tab list.
- **AIS3 — Adapter panes are `key`-ed by adapter id** so local create-form state
  (model, reasoning effort) cannot leak between adapters when you switch tabs.
  Preserve the `key={activeAdapter.id}`.
- **AIS4 — Model metadata comes from `models.ts`.** Hard-coded overrides live in
  `CUSTOM_PROVIDER_MODELS`; everything else resolves through
  `@/lib/llm-providers/models-dev`. Refresh the non-overridden data with
  `bun run generate:models`, not by editing this file.
- **AIS5 — Remote provider mutations use the documents in `graphql/`.** Put new
  `graphql(...)` strings in `graphql/aiSettingsDocument.ts` and invalidate with
  `getQueryKeyFromDocument(...)` — never a hand-written query key string.
- **AIS6 — Analytics events come from `@/lib/constants/analyticsEvents`.** OAuth
  start/complete/disconnect events are already named per provider; reuse the existing
  constants rather than inlining new names.
- **AIS7 — Local secrets stay in the storage layer.** This page renders and edits
  provider configs; persistence and backup-to-BrowserOS are handled by
  `@/lib/llm-providers/storage` (wired up in the background service worker).

## Workflows

**Adding a new LLM provider type**
1. Add the type to `ProviderType` in `@/lib/llm-providers/types` and the storage/config
   layer under `apps/agent/lib/llm-providers/<name>/`.
2. Add an OAuth entry to `OAUTH_PROVIDERS_CONFIG` in `BrowserOsAiPane.tsx` if it uses
   `useOAuthProviderFlow`.
3. Add a `ProviderTemplate` in `@/lib/llm-providers/providerTemplates` and, if the
   model list is not from models.dev, an entry in `models.ts#CUSTOM_PROVIDER_MODELS`.
4. Update `packages/browseros-agent/config.sample.json` if a server-side key is needed.
5. Nothing else here changes — the template grid picks it up.

**Adding a settings section for a new feature**
1. Put a `{ id, label, icon }` entry in the `items` array in `AISettingsPage.tsx`.
2. If the id is a harness adapter, it appears automatically once `useAgentAdapters`
   returns it and `visibleAdapters` keeps it.
3. Otherwise add a `resolveAiSettingsSection` guard and a pane component.

**Wiring a new remote query**
1. Add the document to `graphql/aiSettingsDocument.ts`.
2. Ensure the field exists in `apps/agent/schema/schema.graphql`.
3. Run `bun run codegen`.
4. Consume with `useGraphqlQuery` / `useGraphqlMutation` and invalidate by
   `getQueryKeyFromDocument`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — route table (`settings/ai`).
- [`./graphql/AGENTS.md`](./graphql/AGENTS.md) — documents in this folder.
- [`../agents/AGENTS.md`](../agents/AGENTS.md) — `AdapterAgentsPane` dependencies.
- [`../../../../lib/llm-providers/`](../../../lib/llm-providers/) — provider config, storage, OAuth flow.
- [`../../../../components/ui/`](../../../components/ui/) — shadcn primitives used throughout.

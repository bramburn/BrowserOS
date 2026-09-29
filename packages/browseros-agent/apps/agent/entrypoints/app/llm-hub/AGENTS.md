# `entrypoints/app/llm-hub/` — LLM Hub providers (`/settings/chat`)

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/app`.

## What's here

The `/settings/chat` page: a CRUD list of "LLM Hub" providers — user-managed chat
provider entries (name, base URL, key) that are separate from the Agent tab in
`ai-settings`. Add, edit (index-addressed) and delete, with a confirm dialog.

`models.ts` in this folder is a deliberate two-line re-export of
`@/lib/llm-hub/storage`; it exists so this route can import `LlmHubProvider` and
`getFaviconUrl` through a local path.

## Contents

```
llm-hub/
├── LlmHubPage.tsx        ← list + add/edit index state + delete confirmation
├── HubProvidersList.tsx  ← the list
├── HubProviderRow.tsx     ← one row
├── AddHubProviderDialog.tsx← create/edit form
├── LlmHubHeader.tsx      ← page header
└── models.ts             ← re-export of LlmHubProvider + getFaviconUrl
```

## Rules

- **LH1 — Rows are addressed by array index, not id.** `editingIndex` /
  `deleteIndex` are indices into the `providers` array from
  `useLlmHubProviders()`. If the storage layer ever returns stable ids, change all
  three call sites together.
- **LH2 — `useLlmHubProviders` is the only data source.** Persistence lives in
  `@/lib/llm-hub/storage`; this folder holds no storage calls.
- **LH3 — Destructive action is confirmed.** Deleting opens an `AlertDialog` naming
  the provider. Keep it.
- **LH4 — `models.ts` stays a re-export.** Do not move the types here; the
  `../../../../lib/llm-hub/` layer is the owner and other routes import from it
  directly.
- **LH5 — Do not confuse this with `../ai-settings`.** `/settings/chat` is the LLM
  Hub list; `/settings/ai` is provider configuration plus harness agents. Shared
  UI yes, different data and different storage key.

## Workflows

**Adding a field to a hub provider**
1. Extend `LlmHubProvider` in `@/lib/llm-hub/storage.ts` and bump the persisted
   shape (there is a storage version/migration concern — check the file).
2. Add the input to `AddHubProviderDialog.tsx`, wired to `initialValues`.
3. Render it in `HubProviderRow.tsx`.
4. `models.ts` needs no change unless the type name changes.

**Adding a per-row action**
1. Add the control in `HubProviderRow.tsx` and lift the handler to `LlmHubPage.tsx`.
2. Add the mutation hook next to `useLlmHubProviders` in `@/lib/llm-hub/`.
3. Track with a constant from `@/lib/constants/analyticsEvents`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — route table.
- [`../ai-settings/AGENTS.md`](../ai-settings/AGENTS.md) — the other, distinct LLM settings page.
- [`../../../../lib/llm-hub/`](../../../lib/llm-hub/) — `LlmHubProvider`, storage, `useLlmHubProviders`.
- [`../../../../components/ui/`](../../../components/ui/) — shadcn primitives (`AlertDialog`, etc.).

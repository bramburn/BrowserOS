# `components/chat/` — Unified provider/agent picker

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/components`.

## What's here

The single picker that lets a user choose **what** answers them: a
configured LLM provider or an agent harness (ACP). `ChatProviderSelector`
is a `ui/popover` + `ui/command` combobox; the grouping and search-text
logic live in a separate `.helpers.ts` module so they can be unit-tested
without a DOM, and the shared shapes live in `chatComponentTypes.ts`.

## Contents

```
chat/
├── ChatProviderSelector.tsx        ← the combobox; renders two groups,
│                                     check-marks the selection, and
│                                     exposes a "+" action to add a provider
├── ChatProviderSelector.helpers.ts ← groupProviderOptions() splits on
│                                     `kind === 'acp'`; getProviderSearchValue()
│                                     and getProviderSubtitle() build the
│                                     searchable string
├── ChatProviderSelector.test.tsx   ← happy-dom render tests for the selector
└── chatComponentTypes.ts           ← Provider / ChatProviderType:
                                      id, name, type, kind ('llm' | 'acp'),
                                      agentId, adapterName, modelLabel,
                                      modelControl
```

## Rules

- **CHT1 — Keep the logic in `.helpers.ts`.** Anything testable without React
  belongs there; `ChatProviderSelector.test.tsx` should not need to render to
  assert grouping or search behaviour.
- **CHT2 — The `Provider` type in `chatComponentTypes.ts` is the picker
  contract.** `ChatProviderType = ProviderType | 'acp'`; extend that union
  rather than passing `LlmProviderConfig` straight into the component.
  Callers map `lib/llm-providers/types.ts` configs into `Provider`.
- **CHT3 — Two groups, always `llm` first then `acp`.** `groupProviderOptions`
  omits an empty group; don't add a third group without changing it there.
- **CHT4 — Filtered adapters live in `lib/chat/adapter-visibility.ts`.** Hidden
  harnesses (e.g. `hermes`) must be filtered at the call site using
  `visibleAdapters()` before constructing the `Provider[]`, not inside this
  component.
- **CHT5 — Icons come from `lib/llm-providers/providerIcons.tsx`.** Don't add a
  second provider-icon switch in this folder.

## Workflows

**Adding a new picker entry kind**
1. Extend `ChatProviderType` / `Provider` in `chatComponentTypes.ts`.
2. Add the grouping branch in `ChatProviderSelector.helpers.ts`.
3. Extend `getProviderSearchValue` so the new field is searchable.
4. Add a case in `ChatProviderSelector.test.tsx`.
5. Build the `Provider[]` at the call site (sidepanel / newtab composers).

**Changing what the subtitle shows**
1. Edit `getProviderSubtitle()` in `.helpers.ts` only.
2. Leave the component untouched; it renders whatever the helper returns.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — component-tree rules (CMP1, CMP3).
- [`../ui/AGENTS.md`](../ui/AGENTS.md) — `command.tsx` and `popover.tsx` bases.
- [`../../lib/chat/AGENTS.md`](../../lib/chat/AGENTS.md) — `visibleAdapters()`.
- [`../../lib/llm-providers/AGENTS.md`](../../lib/llm-providers/AGENTS.md) — `ProviderType`, `providerIcons.tsx`.
- [`../../lib/chat-actions/AGENTS.md`](../../lib/chat-actions/AGENTS.md) — the composer hook that mounts this.

# `lib/chat-actions/` — Shared composer behaviour

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

The composer's behaviour, extracted so the sidepanel and the newtab chat
can behave identically. `useChatActions` owns composer input state,
attached tabs, the mode toggle, stop, tab toggling, and voice input; it
takes its analytics event names as **props** so each surface reports under
its own namespace. `types.ts` defines the action payloads that travel
between the newtab and the sidepanel.

## Contents

```
chat-actions/
├── useChatActions.ts  ← useChatActions({ events: { modeChanged, stopClicked,
│                        suggestionClicked, tabToggled, tabRemoved,
│                        aiTriggered, voiceRecordingStarted,
│                        voiceRecordingStopped, voiceTranscriptionCompleted,
│                        voiceError }, autoAttachActiveTab? })
│                        → { input, setInput, attachedTabs, mode, send, stop,
│                          toggleTab, removeTab, runSuggestion, voice }
│                        side-effects: auto-attaches the active http tab on
│                        mount, appends the voice transcript to the input,
│                        fires every event through ../metrics/track
└── types.ts           ← BaseChatAction, AITabAction, BrowserOSAction,
                         ChatAction union, SearchActionData,
                         createAITabAction(), createBrowserOSAction()
```

## Rules

- **CA1 — Event names are always injected, never hard-coded.** The `events`
  config is what lets the same hook serve both surfaces; a literal
  `'ui.message.sent'` inside this file breaks that contract and the
  constants rule in `../constants/`.
- **CA2 — This hook reads the session from context, not props.** It calls
  `useChatSessionContext()` from the sidepanel's `ChatSessionContext`, so
  it is not usable outside a provider — check the wiring before reusing it.
- **CA3 — The voice transcript is appended, not assigned.** The effect
  joins `transcript` onto the existing input with a single space and then
  calls `voice.clearTranscript()`; replacing the input would drop what the
  user already typed.
- **CA4 — `autoAttachActiveTab` filters to `http` URLs.** Internal pages
  (`chrome://`, extension pages) are never attached to a prompt. Keep the
  filter if you touch the mount effect.
- **CA5 — `createBrowserOSAction` / `createAITabAction` mint ids with
  `crypto.randomUUID()` and stamp `Date.now()`.** Use these factories rather
  than constructing action objects by hand; the newtab and sidepanel must
  agree on the shape.

## Workflows

**Adding a composer action**
1. Add the event name to `../constants/analyticsEvents.ts`.
2. Add it to the `events` config type in `useChatActions.ts`.
3. Fire it with `track(config.events.<name>)` in the handler.
4. Pass the constant from each surface's call site.

**Extending the newtab → sidepanel payload**
1. Extend the relevant action interface in `types.ts` (or `SearchActionData`).
2. Keep the `ChatAction` discriminated union closed on `type`.
3. Update `../search-actions/searchActionsStorage.ts` if the shape changed.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB1, LIB7).
- [`../voice/AGENTS.md`](../voice/AGENTS.md) — `useVoiceInput`, wired in here.
- [`../metrics/AGENTS.md`](../metrics/AGENTS.md) — the `track()` used for every event.
- [`../search-actions/AGENTS.md`](../search-actions/AGENTS.md) — persists `SearchActionData`.
- [`../../components/chat/AGENTS.md`](../../components/chat/AGENTS.md) — the provider picker mounted by the composer.
- [`../../entrypoints/AGENTS.md`](../../entrypoints/AGENTS.md) — `ChatSessionContext` and the composers.

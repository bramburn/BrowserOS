# `lib/metrics/` — First-party telemetry

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

One function, `track()`, that reports an event to BrowserOS's own metrics
pipeline via the `chrome.browserOS.logMetric` browser API. Every call
automically carries the extension, Chromium and BrowserOS versions, so a
raw event name is all a call site supplies. This is the **host** telemetry
channel and is separate from the PostHog product analytics in
`../analytics/`.

## Contents

```
metrics/
└── track.ts   ← module-scope version cache (extension / chromium / browseros,
                 the latter two resolved once via getBrowserOSAdapter());
                 track(eventName, properties?) →
                 adapter.logMetric(eventName, { extension_version,
                 chromium_version?, browseros_version?, ...properties })
                 with the promise error swallowed
```

## Rules

- **MET1 — Event names are constants from `../constants/analyticsEvents.ts`.**
  Never pass a template-literal name; the registry is what makes the
  funnel analysable.
- **MET2 — `track` is fire-and-forget.** It returns `void` and swallows
  errors; a metrics failure must never surface to the user. Don't `await` it
  or wrap it in a try/catch for UI purposes.
- **MET3 — The three version fields are injected automatically and cannot be
  overridden** with the same keys — caller properties are spread last, so
  passing `extension_version` does win. Don't; it breaks the funnel.
- **MET4 — No user content in properties.** Message text, prompts, provider
  API keys and conversation ids must not be sent here.
- **MET5 — Use `track()` for host metrics, `posthog.capture()` for product
  analytics.** Some call sites deliberately take an `events` config so a
  single handler can report to either channel (see
  `../chat-actions/useChatActions.ts`).
- **MET6 — The adapter call is unconditional at module load.** Importing this
  module in a context without `chrome.browserOS` resolves `null` versions
  and still reports; that degradation is intended.

## Workflows

**Adding a tracked event**
1. Add the name constant in `../constants/analyticsEvents.ts`.
2. `import { track } from '@/lib/metrics/track'`
3. `track(NAME_EVENT, { from, to })` — keep properties flat and non-identifying.

**Enriching an existing event**
1. Add the property at the call site.
2. Re-check MET4: nothing that identifies a user or leaks page content.

**Debugging missing events**
1. Confirm the name matches the constant exactly (a typo compiles).
2. Confirm the call site runs inside a context where `chrome.runtime`
   exists (extension page, background, content script).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB1, LIB3).
- [`../browseros/AGENTS.md`](../browseros/AGENTS.md) — `logMetric` on the adapter.
- [`../analytics/AGENTS.md`](../analytics/AGENTS.md) — the PostHog channel (ANL6 explains the split).
- [`../constants/AGENTS.md`](../constants/AGENTS.md) — the event-name registry.

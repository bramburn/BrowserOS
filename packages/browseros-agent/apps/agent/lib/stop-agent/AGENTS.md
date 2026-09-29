# `lib/stop-agent/` — Stop signal for the active run

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

A single storage item that lets a surface that is not the one running the
agent ask it to stop: it records which conversation the user hit "stop" on
and when. The background worker (and the sidepanel) read the entry to
abort the in-flight request, then clear it. It is a signal, not a queue.

## Contents

```
stop-agent/
└── stop-agent-storage.ts   ← StopAgentStorage { conversationId, timestamp };
                              stopAgentStorage =
                              storage.defineItem<StopAgentStorage | null>
                              ('local:stop-agent', { fallback: null })
```

## Rules

- **STOP1 — Store a conversation id, not a boolean.** Stopping is per-run;
  a global flag would let a stop click in one conversation abort another.
- **STOP2 — `fallback: null` is the "no stop pending" state.** Branch on
  `null` before aborting; `undefined` is not a valid stored value.
- **STOP3 — Compare `timestamp` when consuming.** A stale entry from a
  previously stopped run would abort a fresh one on mount. Only honour a
  signal newer than the run's start.
- **STOP4 — Clear the item once handled.** It is a one-shot latch; leaving
  it set means the next conversation with the same id aborts immediately.
- **STOP5 — Don't add a second stop channel.** Aborting the underlying
  `AbortController` is the mechanism; this item only carries the intent
  across contexts that can't reach that controller.

## Workflows

**Requesting a stop from another surface**
1. `await stopAgentStorage.setValue({ conversationId, timestamp: Date.now() })`
2. The runner watching that conversation aborts and clears the item.

**Handling a stop in the runner**
1. Read the item; ignore it unless `conversationId` matches and
   `timestamp` is newer than the run start.
2. Abort the request.
3. `await stopAgentStorage.removeValue()`.

**Resetting between tests/dev reloads**
1. `await stopAgentStorage.removeValue()` — the `null` fallback covers
   first-run.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB2).
- [`../chat-actions/AGENTS.md`](../chat-actions/AGENTS.md) — the composer's `stop` handler.
- [`../execution-history/AGENTS.md`](../execution-history/AGENTS.md) — a stopped run is recorded as `stopped`.
- [`../../entrypoints/AGENTS.md`](../../entrypoints/AGENTS.md) — the sidepanel session and background worker.

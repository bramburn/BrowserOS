# `entrypoints/glow.content/` — Glow overlay content script

> Part of the WXT entry points in `apps/agent/entrypoints`. Parent guide:
> [`../../AGENTS.md`](../../AGENTS.md).

## What's here

A content script that paints a pulsing orange border around the viewport while an
agent turn is running, and fires `canvas-confetti` when a turn finishes. It is the
"the agent is working" ambient feedback, deliberately outside the extension UI.

`index.ts` injects its own `<style>` and a fixed-position overlay div into the host
page, listens for a start/stop message, and keeps a module-scope
`activeConversationId` so repeated messages for the same turn do not re-trigger.
`GlowMessage.ts` holds the (tiny) message type for that channel.

## Contents

```
glow.content/
├── index.ts        ← style injection, overlay/stop-button DOM, message listener, confetti
└── GlowMessage.ts  ← the message payload type
```

## Rules

- **GC1 — Every injected node has a stable id** (`browseros-glow-overlay`,
  `browseros-glow-styles`, `browseros-glow-stop-btn`) and injection is guarded by
  `getElementById`. Re-injecting duplicates the overlay and doubles the animation.
- **GC2 — Inject only what the feature needs and remove it on stop.** The script runs
  in the host page's DOM; leaked nodes are visible to the site and survive navigation
  expectations badly.
- **GC3 — Stop is user-reachable.** The overlay renders a stop button, and the stop
  path routes through the same storage contract as the side panel's
  (`stopAgentStorage`). Do not remove the button without adding another route.
- **GC4 — Guard on `activeConversationId`.** A second "started" message for the turn
  already glowing must be a no-op; confetti likewise fires once per turn.
- **GC5 — Styles are a single injected string,** not a second bundled stylesheet.
  The page is untrusted; keep the CSS scoped to the injected ids.
- **GC6 — Keep `GlowMessage.ts` a type-only module.** It should stay free of runtime
  imports so both ends of the channel can share it.

## Workflows

**Changing the glow visuals**
1. Edit the keyframes/values in `injectStyles()` in `index.ts`
   (`GLOW_THICKNESS`, `GLOW_OPACITY` are the tuning knobs).
2. Do not add class names that could collide with the host page — the injected
   selector list is `#browseros-glow-*` only.
3. Reload the extension; content-script changes are not hot-reloaded.

**Wiring glow to a new trigger**
1. Extend `GlowMessage.ts` with the new payload field.
2. Handle it in the `main()` listener in `index.ts`, guarding on conversation id.
3. Update the sender (side panel or agent-command chat) to post it.

**Debugging a missing glow**
1. Check the content script is injected on that origin (matches list in `defineContentScript`).
2. Check the sender actually posts the message — the channel is `window`-level and
   page code can be noisy.
3. Check `activeConversationId` was not left set by a previous turn.

## Cross-references

- [`../../AGENTS.md`](../../AGENTS.md) — extension internals and content-script conventions.
- [`../sidepanel/index/AGENTS.md`](../sidepanel/index/AGENTS.md) — the main sender (stop/finish events).
- [`../../../lib/stop-agent/stop-agent-storage.ts`](../../lib/stop-agent/stop-agent-storage.ts) — stop signal contract.
- [`../selection.content.ts`](../selection.content.ts) — the other thin content script.

# `lib/jtbd-popup/` — Jobs-to-be-done survey popup

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

A product-research nudge: after the user has sent enough messages, a
small popup offers a survey, subject to sampling. `useJtbdPopup` owns the
eligibility maths and the tracking; `storage.ts` persists the counters
across sessions; `constants.ts` holds the tunables. The survey itself
lives outside this folder (it is rendered by the sidepanel's
`JtbdPopup.tsx`).

## Contents

```
jtbd-popup/
├── storage.ts        ← JtbdPopupState { messageCount, surveyTaken, samplingId,
│                        dontShowAgain, shownCount };
│                        jtbdPopupStorage (local:jtbdPopupState)
│                        with samplingId: -1 meaning "not yet assigned"
├── constants.ts      ← JTBD_POPUP_CONSTANTS: MESSAGE_THRESHOLD 10,
│                        SAMPLING_DIVISOR 1 (1 = everyone),
│                        DONT_SHOW_AGAIN_AFTER 2
└── useJtbdPopup.ts   ← useJtbdPopup() → { popupVisible, showDontShowAgain,
                         recordMessageSent, … }; assigns a random samplingId
                         0–99 on first run and fires JTBD_POPUP_SHOWN /
                         _CLICKED / _DISMISSED events
```

## Rules

- **JTB1 — Eligibility is a single predicate** (`isEligible` in
  `useJtbdPopup.ts`): not `dontShowAgain`, not `surveyTaken`, message count
  an exact multiple of `MESSAGE_THRESHOLD`, and `samplingId % SAMPLING_DIVISOR === 0`.
  Change the rule there, not in a component.
- **JTB2 — `samplingId === -1` means unassigned.** The mount effect draws
  `Math.floor(Math.random() * 100)` once and writes it back. Reading
  `samplingId` before that write yields `-1`, which fails the modulo — that
  is intended, not a bug.
- **JTB3 — `MESSAGE_THRESHOLD` is a multiple, not a floor.** The `%` check
  means the popup can appear on message 10, 20, 30… not 10, 11, 12….
- **JTB4 — `SAMPLING_DIVISOR: 1` ships to everyone.** Changing it is a
  deliberate experiments decision; it is a constant for that reason.
- **JTB5 — The "don't show again" checkbox only appears after
  `DONT_SHOW_AGAIN_AFTER` impressions**, via `showDontShowAgain`. Keep the
  two coupled.
- **JTB6 — Event names are the `JTBD_POPUP_*_EVENT` constants** from
  `../constants/analyticsEvents.ts`.

## Workflows

**Wiring the popup into a surface**
1. Mount the hook once in the surface that sends messages.
2. Call `recordMessageSent()` on every send.
3. Render the popup when `popupVisible`; show the opt-out checkbox only when
   `showDontShowAgain`.

**Changing the trigger frequency**
1. Edit `JTBD_POPUP_CONSTANTS` in `constants.ts` only.
2. Sanity-check the interaction with `DONT_SHOW_AGAIN_AFTER`: a small
   threshold plus a low message count can spam.

**Resetting for a manual test**
1. `await jtbdPopupStorage.removeValue()` — the fallback re-initialises
   counters and draws a new `samplingId`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB2, LIB7).
- [`../constants/analyticsEvents.ts`](../constants/analyticsEvents.ts) — the `JTBD_POPUP_*` events.
- [`../metrics/AGENTS.md`](../metrics/AGENTS.md) — `track()`.
- [`../../entrypoints/AGENTS.md`](../../entrypoints/AGENTS.md) — `JtbdPopup.tsx` in the sidepanel.

# `entrypoints/app/jtbd-agent/` — JTBD survey (`/settings/survey`)

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/app`.

## What's here

A short, self-contained survey chat. The user gets a welcome screen, then a
streaming interview against an external JTBD service, then a thank-you card. The
page is a four-state machine driven by `useChat` — `idle → active → completed | error`.

The survey service is **not** the local agent server: `useSurveyChat.ts` streams
SSE from `https://jtbd-agent.fly.dev`, passing an install id read from the browser
pref `BROWSEROS_PREFS.INSTALL_ID`.

## Contents

```
jtbd-agent/
├── SurveyPage.tsx      ← phase switch; also marks the survey taken in jtbdPopupStorage
├── SurveyChat.tsx      ← streaming message list + input
├── SurveyHeader.tsx    ← page header
├── SurveyWelcome.tsx   ← pre-start screen
├── VoiceInputButton.tsx← mic affordance
└── useSurveyChat.ts    ← SSE stream, phase state, maxTurns/experimentId, stop/reset
```

## Rules

- **JT1 — This folder is temporary.** `useSurveyChat.ts` carries a
  `biome-ignore lint/complexity/noExcessiveCognitiveComplexity: JTBD agent is
  temporary` suppression. Do not grow it; do not copy its structure into new code.
- **JT2 — `maxTurns` and `experimentId` arrive as props from `app/App.tsx`,** which
  reads them from `window.location.search`. Keep the hook's defaults
  (`7` from the route helper, `20` in the hook) in mind when changing either.
- **JT3 — Completion is signalled by the `__INTERVIEW_COMPLETE__` marker** in the
  stream, not by turn counting. Both exist; the marker is authoritative.
- **JT4 — Starting the survey writes `jtbdPopupStorage.surveyTaken = true`** before
  the first send. That flag is what stops the popup resurfacing elsewhere
  (`sidepanel/index/Chat.tsx` also uses `useJtbdPopup`).
- **JT5 — Keep install-id failure non-fatal.** `getInstallId` returns `''` when the
  BrowserOS adapter is unavailable; the survey still runs.
- **JT6 — No state survives a reset.** `reset()` clears messages and phase in one
  place in the hook — do not clear pieces in the page.

## Workflows

**Changing the interview flow**
1. Edit the phase transitions in `useSurveyChat.ts` (single source of truth).
2. Add the matching branch in `SurveyPage.tsx`'s phase switch.
3. Keep the marker constant and the turn cap consistent.

**Testing the survey locally**
1. Uncomment `LOCAL_JTBD_API_URL` in `useSurveyChat.ts` and point `JTBD_API_URL` at it.
2. `bun scripts/dev/inspect-ui.ts open-app` then navigate to `app.html#/settings/survey`.
3. Pass overrides via the hash: `app.html#/settings/survey?maxTurns=3&experimentId=local`.

**Wiring voice input here**
1. Reuse `@/lib/voice/useVoiceInput` (already wired in `sidepanel/index/Chat.tsx` and
   `newtab/index/NewTab.tsx`).
2. Track with a new constant in `@/lib/constants/analyticsEvents` — do not reuse the
   sidepanel event names for a different surface.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — route table and `getSurveyParams()`.
- [`../../../../lib/jtbd-popup/`](../../../lib/jtbd-popup/) — `jtbdPopupStorage`, `useJtbdPopup`.
- [`../../../../lib/voice/useVoiceInput.ts`](../../../lib/voice/useVoiceInput.ts) — voice input hook.
- [`../../../sidepanel/index/AGENTS.md`](../../sidepanel/index/AGENTS.md) — the other place the JTBD popup fires.

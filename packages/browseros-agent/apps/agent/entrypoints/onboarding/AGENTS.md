# `entrypoints/onboarding/` — first-run flow (routes inside `app.html`)

> Part of the WXT entry points in `apps/agent/entrypoints`. Parent guide:
> [`../../AGENTS.md`](../../AGENTS.md).

## What's here

**This folder is not a WXT entry point.** There is no `index.html` here. The four
onboarding screens are React routes rendered by `../app/App.tsx` inside `app.html`:
`/onboarding` (welcome), `/onboarding/steps/:stepId` (three-step setup),
`/onboarding/demo` (try it now), and `/onboarding/features` (feature tour).

The service worker opens `app.html#/onboarding` in a new tab on **install only**
(`chrome.runtime.OnInstalledReason.INSTALL`), which is how a first-run user lands
here. Onboarding routes sit outside `SidebarLayout`, so they render without auth or
navigation chrome.

## Contents

```
onboarding/
├── index/     ← welcome screen (Onboarding, header, feature cards, focus grid)
├── steps/     ← StepsLayout + StepOne / StepConnectApps / StepTwo + transitions
├── demo/      ← OnboardingDemo: pick an app + a prompt, opens the side panel
└── features/  ← Features: the marketing bento grid with videos
```

## Rules

- **ON1 — No `index.html` here.** Onboarding is a route tree inside `app.html`. A new
  screen is registered in `../app/App.tsx` under the `onboarding` branch.
- **ON2 — Step identity is a numeric id in `steps/steps.ts`.** `StepsLayout` matches
  `:stepId` against that array and falls back to a null component when it does not
  match. Add a step by extending the array — not by special-casing the layout.
- **ON3 — Progress is derived, not stored.** Completed = `step.id < currentStep`;
  the last step navigates to `/onboarding/demo`. Don't persist step numbers.
- **ON4 — Steps must call `onContinue`; the layout owns navigation.** A step that
  navigates itself will fight the direction-based transition.
- **ON5 — `RpcClientProvider` is mounted by `StepsLayout`,** not by individual steps.
  Steps that need the local server inherit it.
- **ON6 — Onboarding completion is recorded in storage**
  (`onboardingCompletedStorage`, `onboardingProfileStorage` in
  `@/lib/onboarding/onboardingStorage`). The worker also calls `syncOnboardingProfile`
  on session change. Keep both paths.
- **ON7 — Track entry and step views.** `ONBOARDING_STARTED_EVENT` fires in `Onboarding`
  on mount; `ONBOARDING_STEP_VIEWED_EVENT` fires in `StepsLayout` on step change. Both
  are biome-ignored for exhaustive deps on purpose.

## Workflows

**Adding an onboarding step**
1. Create `steps/Step<Name>.tsx` accepting `{ direction, onContinue }`.
2. Append `{ id, name, component }` to `steps.ts` (ids must be contiguous from 1 —
   `isLastStep` is `currentStep >= steps.length`).
3. Check the transition height: `StepsLayout` fixes the animated area at `h-[550px]`
   and `StepTransition` matches it.
4. Nothing in `../app/App.tsx` changes — `:stepId` already matches by parameter.

**Changing the entry point**
1. First run is triggered from `../background/index.ts` on install
   (`app.html#/onboarding`).
2. Deep links into a step are `/onboarding/steps/<id>`.
3. Completion routes to `/onboarding/demo`; update both `StepsLayout.onContinue` and
   the demo's "done" path if you change the ending.

**Debugging a step that renders blank**
1. `stepEntry?.component ?? (() => null)` swallows unknown ids — check `:stepId` is
   numeric and present in `steps.ts`.
2. Confirm the step uses `StepTransition` (or is inside the `AnimatePresence` block);
   a step without the transition wrapper will not animate but will render.
3. `bun scripts/dev/inspect-ui.ts open-app` then load `app.html#/onboarding/steps/1`.

## Cross-references

- [`../../AGENTS.md`](../../AGENTS.md) — extension internals.
- [`../app/AGENTS.md`](../app/AGENTS.md) — the route table that mounts this folder.
- [`../background/AGENTS.md`](../background/AGENTS.md) — the install hook that opens onboarding.
- [`./steps/AGENTS.md`](./steps/AGENTS.md) — the step registry and transition mechanics.
- [`../../../../lib/onboarding/`](../../lib/onboarding/) — storage and profile sync.

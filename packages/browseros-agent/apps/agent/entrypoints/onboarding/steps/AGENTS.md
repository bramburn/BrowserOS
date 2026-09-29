# `entrypoints/onboarding/steps/` — the three-step setup flow

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/onboarding`.

## What's here

The wizard at `/onboarding/steps/:stepId`. `StepsLayout` renders a progress
indicator, resolves the step component from the `steps` registry, and animates
between steps; the three steps themselves are "About You", "Connect Apps" and
"Sign In". Finishing the last step navigates to `/onboarding/demo`.

Steps receive exactly two props — `direction` (`-1` forward/back, `1`) and
`onContinue` — and are expected to render inside `StepTransition` so the layout's
`AnimatePresence` can animate them.

## Contents

```
steps/
├── StepsLayout.tsx      ← progress bar, step resolution, forward/back navigation, RpcClientProvider
├── steps.ts             ← the registry: [{ id, name, component }]
├── StepTransition.tsx   ← motion.div slide wrapper + StepDirection type
├── StepOne.tsx          ← id 1 "About You"
├── StepConnectApps.tsx  ← id 2 "Connect Apps"
└── StepTwo.tsx          ← id 3 "Sign In"
```

## Rules

- **ST1 — The registry in `steps.ts` is the only source of step truth.** Layout,
  progress indicator, and `isLastStep` all read it. Adding a step means adding an
  entry, nothing else.
- **ST2 — Step ids must be contiguous starting at 1.** `isLastStep` is
  `currentStep >= steps.length` and `canGoPrevious` is `currentStep > 1`; a gap makes
  the wrong step "last".
- **ST3 — Steps never navigate.** They call `onContinue()`. Navigation, direction, and
  the demo hand-off are the layout's job.
- **ST4 — The animated area is `h-[550px]` in both files.** `StepsLayout` sets the
  container height and `StepTransition` sets the same height on the motion div. Change
  one and the other must follow, or content is clipped.
- **ST5 — An unknown `:stepId` renders nothing** (`stepEntry?.component ?? (() => null)`).
  Debug an empty step by checking the id before suspecting the component.
- **ST6 — Back navigation is a `NavLink` with `onClick={() => setDirection(-1)}`** so
  the transition knows which way to slide. Use the same pattern for any new backward
  path.
- **ST7 — Analytics uses `ONBOARDING_STEP_VIEWED_EVENT` with `{ step, step_name }`,**
  fired on step change. The biome-ignore for exhaustive deps is intentional — the
  effect keys on `currentStep` only.

## Workflows

**Adding a fourth step**
1. Create `Step<Name>.tsx` with the `{ direction, onContinue }` props and
   `StepTransition` as the root.
2. Append `{ id: 4, name: '…', component: Step<Name> }` to `steps.ts`.
3. Confirm the last-step hand-off still points at `/onboarding/demo` — it does, and
   it will now fire from step 4 instead of 3.
4. Check the progress indicator still fits three or four nodes at `md`.

**Changing the progress indicator**
1. Edit the `steps.map(...)` block in `StepsLayout.tsx`.
2. Completed = `step.id < currentStep`; active = `step.id === currentStep`. Preserve
   both comparisons.
3. Step names are hidden below `md` (`hidden … md:block`); keep that if you widen.

**Debugging a step that will not advance**
1. Confirm the step calls `onContinue` (and not `navigate`).
2. Confirm the id in the URL matches `steps.ts` — a stale `/onboarding/steps/9` after
   removing a step renders blank.
3. `bun scripts/dev/inspect-ui.ts open-app`, load `app.html#/onboarding/steps/1`.

**Debugging a clipped step**
1. Compare the `h-[550px]` in `StepsLayout.tsx` and `StepTransition.tsx`.
2. Check the step's own root is not taller and not scrolling.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the onboarding route rules.
- [`../demo/AGENTS.md`](../demo/AGENTS.md) — where the last step hands off.
- [`../AGENTS.md`](../AGENTS.md) — `app/App.tsx` mounts `StepsLayout` at `steps/:stepId`.
- [`../../../../lib/onboarding/`](../../../lib/onboarding/) — storage used by the steps.
- [`../../../../lib/rpc/RpcClientProvider.tsx`](../../../lib/rpc/RpcClientProvider.tsx) — provider mounted by the layout.

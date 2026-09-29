# `entrypoints/onboarding/index/` — onboarding welcome screen

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/onboarding`.

## What's here

The landing screen at `/onboarding`: a full-height hero with the product wordmark
(glowing accent on "BrowserOS"), a "Get Started" button linking to
`/onboarding/steps/1`, a "View on GitHub" button, and a footer with the current
year. `FeatureCards` is a generic linked-card component and `FocusGrid` is the
decorative background — both are local, near-copies of their new-tab counterparts.

The service worker opens this route in a new tab on extension **install**, so this is
the first screen a new user sees.

## Contents

```
index/
├── Onboarding.tsx        ← hero; fires ONBOARDING_STARTED_EVENT on mount
├── OnboardingHeader.tsx  ← top bar (mount-gated reveal)
├── FeatureCards.tsx      ← generic external-link card (href, title, description, icon)
└── FocusGrid.tsx         ← decorative grid + radial gradient, pointer-events-none
```

## Rules

- **OI1 — Track the start, not the view.** `ONBOARDING_STARTED_EVENT` fires once in a
  mount effect. Do not move it into a render path.
- **OI2 — The primary CTA targets `/onboarding/steps/1` with `NavLink`,** not a
  `navigate()` call. It must stay a router link so middle-click and back-button work.
- **OI3 — Reveal animations are gated on `mounted`**, set in a mount effect, with
  staggered `delay-*` classes. Keep the effect; without it the hero renders in its
  final state and the animation never runs.
- **OI4 — External links use `target="_blank"` with `rel="noopener noreferrer"`,**
  and the URL comes from `productRepositoryShortUrl` in
  `@/lib/constants/productUrls`.
- **OI5 — The year comes from `getCurrentYear()`,** not `new Date().getFullYear()` in
  JSX.
- **OI6 — `FocusGrid` is decorative and `pointer-events-none`.** Never place
  interactive content in it.
- **OI7 — `FeatureCards` is generic and self-contained** (props only, no imports from
  `lib/`). Keep it that way if you reuse it; note it is currently unused by this page
  itself.

## Workflows

**Changing the welcome copy**
1. Edit the heading, sub-copy and buttons in `Onboarding.tsx`.
2. Keep the `<span class="animate-glow-once text-accent-orange">` around the
   wordmark — the animation is part of the brand treatment.
3. Adjust the staggered `delay-*` values if the copy block grows.

**Adding a CTA**
1. Use `Button asChild` wrapping a `NavLink` for internal routes.
2. Use `Button asChild variant="outline"` wrapping an `<a>` for external links.
3. Keep the group-hover translate micro-interaction used by the existing buttons.

**Debugging a missing animation**
1. Confirm the `mounted` effect still runs (React StrictMode double-invokes are fine).
2. Check the delay classes are on the same elements as the opacity/transform classes.
3. `bun scripts/dev/inspect-ui.ts open-app`, load `app.html#/onboarding`, then
   `screenshot`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the onboarding route rules.
- [`../steps/AGENTS.md`](../steps/AGENTS.md) — where "Get Started" goes.
- [`../features/AGENTS.md`](../features/AGENTS.md) — the feature tour.
- [`../../background/AGENTS.md`](../../background/AGENTS.md) — the install hook that opens this page.
- [`../../../../lib/constants/productUrls.ts`](../../../lib/constants/productUrls.ts) — external links.

# `lib/analytics/` — PostHog wiring and identity

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

PostHog product analytics for the extension. `posthog.ts` initialises the
`posthog-js` client at import time (session recording included) and
exports the singleton; `AnalyticsProvider.tsx` puts it on the React tree
via `@posthog/react`; `identify.ts` is the one place that reconciles user
identity across **both** PostHog and Sentry.

## Contents

```
analytics/
├── posthog.ts        ← posthog.init() guarded on VITE_PUBLIC_POSTHOG_KEY +
│                       VITE_PUBLIC_POSTHOG_HOST; person_profiles
│                       'identified_only'; session_recording with
│                       maskAllInputs; on load registers extension_version
│                       (chrome.runtime.getManifest().version) and ui_context;
│                       imports 'posthog-js/dist/posthog-recorder'
├── AnalyticsProvider.tsx ← <PostHogProvider client={posthog}>
└── identify.ts        ← identify({ id, email, name }) → sentry.setUser +
                           posthog.identify; resetIdentity() → sentry.setUser(null)
                           + posthog.reset()
```

## Rules

- **ANL1 — `posthog.ts` is a side-effect module; import it once, at the top
  of an entry point.** It initialises on import and is not idempotent-safe to
  import from many components.
- **ANL2 — Analytics is opt-in by env.** With no `VITE_PUBLIC_POSTHOG_KEY` /
  `_HOST` the client never initialises; never add a hard-coded key.
- **ANL3 — `identify()` is called from `../auth/AuthProvider.tsx` only.**
  That keeps login and logout symmetric across PostHog and Sentry. A second
  caller risks identifying twice or leaving a stale user on logout.
- **ANL4 — Event names come from `../constants/analyticsEvents.ts`.** Never
  inline a string literal like `'ui.message.sent'` at a call site.
- **ANL5 — `maskAllInputs` is on; don't add properties that carry message
  text, prompts, or API keys.** Session recording would capture them.
- **ANL6 — Two metric channels exist and are not interchangeable.** PostHog
  (`lib/analytics/`) is product analytics; `../metrics/track.ts` forwards to
  `chrome.browserOS.logMetric` for first-party telemetry. Pick deliberately.

## Workflows

**Adding an analytics event**
1. Add the constant to `../constants/analyticsEvents.ts` with a
   namespaced name (`ui.*`, `settings.*`, `onboarding.*`).
2. Fire it with `posthog.capture(...)` at the interaction site, or pass the
   name into a hook's `events` config (the pattern in
   `../chat-actions/useChatActions.ts`).

**Wiring PostHog into a surface**
1. Import `AnalyticsProvider` in the surface's provider stack.
2. Ensure `lib/analytics/posthog.ts` has been imported so the client exists.
3. Verify the `loaded` hook registered `extension_version`.

**Removing user data on sign-out**
1. `resetIdentity()` — it clears PostHog *and* Sentry in one call.
2. Called automatically by `AuthProvider`; do not add a manual call.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB3, LIB5).
- [`../constants/AGENTS.md`](../constants/AGENTS.md) — the event-name registry.
- [`../auth/AGENTS.md`](../auth/AGENTS.md) — `AuthProvider` calls `identify`.
- [`../metrics/AGENTS.md`](../metrics/AGENTS.md) — the separate first-party channel.
- [`../env.ts`](../env.ts) — `VITE_PUBLIC_POSTHOG_KEY` / `VITE_PUBLIC_POSTHOG_HOST`.

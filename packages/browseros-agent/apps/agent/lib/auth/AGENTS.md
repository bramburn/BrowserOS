# `lib/auth/` — Session, auth client, identity

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

The extension's authentication layer, built on
[better-auth](https://better-auth.com) against the BrowserOS cloud API.
`auth-client.ts` creates the client (magic-link plugin, base URL from
env); `sessionStorage.ts` mirrors the resolved session into
`chrome.storage.local` and exposes a watched hook so non-React contexts
(background, content) can read it; `AuthProvider` keeps the mirror in sync
and, as a side effect, identifies the user to PostHog and Sentry.

## Contents

```
auth/
├── auth-client.ts    ← createAuthClient({ baseURL: env.VITE_PUBLIC_BROWSEROS_API,
│                        plugins: [magicLinkClient()] });
│                        exports signIn, signUp, signOut, useSession
├── sessionStorage.ts ← storage.defineItem<SessionInfo>('local:sessionInfo');
│                        useSessionInfo() → { sessionInfo, isLoading,
│                        updateSessionInfo }, synced via storage.watch()
└── AuthProvider.tsx  ← useSession() → write through to sessionStorage, then
                         identify({ id, email, name }) or resetIdentity()
```

## Rules

- **AUTH1 — `auth-client.ts` owns the client configuration.** `baseURL` and
  plugins are set once there; don't re-create a client at a call site.
- **AUTH2 — `sessionStorage` is the cross-context session cache.** Non-React
  code (background service worker, messaging handlers) reads
  `await sessionStorage.getValue()`; it cannot call `useSession()`.
  UI code should prefer the hook.
- **AUTH3 — `AuthProvider` owns identity propagation.** It is the single place
  that calls `identify()` / `resetIdentity()`. Don't identify from a
  component; the effect will double-fire and leave stale identity on logout.
- **AUTH4 — Persist a minimal `SessionInfo`.** `{ session, user }` only —
  never tokens copied out of the session, and never anything from
  `llm-providers` (API keys live in `providersStorage`).
- **AUTH5 — Route protection belongs in `components/auth/AuthGuard.tsx`.**
  This folder supplies data, not navigation.
- **AUTH6 — The API base URL is `env.VITE_PUBLIC_BROWSEROS_API`** read through
  `../env.ts`; never hard-code `api.browseros.com` here.

## Workflows

**Checking whether the user is signed in**
1. In React: `const { data } = useSession()` (from `auth-client.ts`).
2. In a lib/background context: `await sessionStorage.getValue()`.
3. For route protection, wrap in `components/auth/AuthGuard.tsx`.

**Handling sign-in / sign-out**
1. `signIn.emailOtp(...)` / `signOut()` from `auth-client.ts`.
2. `AuthProvider` observes the change, updates `sessionStorage`, and calls
   `identify` / `resetIdentity` — no manual cleanup needed at the call site.

**Adding a field to the cached session**
1. Extend `SessionInfo` in `sessionStorage.ts`.
2. Ensure it comes from better-auth's `Session`/`User` types, not a custom
   fetch.
3. Old cached values will simply lack the field — code must tolerate undefined.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB2, LIB3).
- [`../../components/auth/AGENTS.md`](../../components/auth/AGENTS.md) — the route guard.
- [`../analytics/identify.ts`](../analytics/identify.ts) — `identify()` / `resetIdentity()`.
- [`../conversations/AGENTS.md`](../conversations/AGENTS.md) — uploads gated on `sessionInfo.user.id`.
- [`../env.ts`](../env.ts) — `VITE_PUBLIC_BROWSEROS_API`.
- [`../../package.json`](../../package.json) — the `better-auth` dependency.

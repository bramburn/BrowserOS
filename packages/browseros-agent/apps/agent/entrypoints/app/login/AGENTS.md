# `entrypoints/app/login/` — auth pages

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/app`.

## What's here

Three auth screens, all rendered inside `AuthLayout` (centred logo, no sidebar):
the magic-link sign-in, an explicit sign-out confirmation, and the magic-link
callback landing page. Authentication itself is owned by `@/lib/auth/auth-client`
(better-auth); these files are the UI around it.

`MagicLinkCallback` matters more than it looks: the BrowserOS web app finishes the
OAuth/magic-link handshake in a normal web page and then hands control back to the
extension. The `auth.content/` content script relays `AUTH_SUCCESS` to the service
worker, which redirects the tab into `app.html#<saved redirect path>`.

## Contents

```
login/
├── LoginPage.tsx        ← email + magic-link form; redirects to /home when a session exists
├── LogoutPage.tsx       ← sign-out confirmation
└── MagicLinkCallback.tsx← reads ?error=, waits on useSession(), then navigates to /home
```

## Rules

- **LO1 — Session state comes from `useSession()` (`@/lib/auth/auth-client`),** never
  from `chrome.storage` or a local copy. Both pages already branch on
  `isPending` before redirecting; a redirect during pending logs the user out of a
  valid session.
- **LO2 — Redirects use `{ replace: true }`.** Back-button into `/login` after a
  successful sign-in is a bug, not a feature.
- **LO3 — Redirect destinations go through `authRedirectPathStorage`.** The service
  worker reads it after `AUTH_SUCCESS`; the page must not invent a target. The stored
  path defaults to `/home`.
- **LO4 — `MagicLinkCallback` must surface `?error=`.** It decodes the param and shows
  a "Verification failed" card. Silent fallback to `/home` on a failed link hides real
  problems from users.
- **LO5 — These routes are public.** They are outside `SidebarLayout` and
  `SettingsSidebarLayout`, so no capability or login guard wraps them; do not move
  them into a guarded branch.

## Workflows

**Adding an auth route**
1. Create the component here.
2. Mount it in `app/App.tsx` inside the `AuthLayout` block.
3. Use `useSession()` for state and `navigate(..., { replace: true })` for exits.
4. If it can be an OAuth return target, ensure the incoming tab lands on
   `app.html#auth/<route>`.

**Debugging a broken magic link**
1. `bun scripts/dev/inspect-ui.ts open-app` and load
   `app.html#/auth/magic-link?error=<msg>` to check the error card rendering.
2. Check the web-side handshake: `auth.content/index.ts` only injects on
   `${VITE_PUBLIC_BROWSEROS_API}/home`, `runAt: document_start`.
3. Check the worker relay in `../../../background/index.ts` (`AUTH_SUCCESS` handler).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — route table and the `AuthLayout` block.
- [`../layout/AGENTS.md`](../layout/AGENTS.md) — `AuthLayout`.
- [`../../../../lib/auth/auth-client.ts`](../../../lib/auth/auth-client.ts) — `useSession`, `signIn`.
- [`../../../../lib/onboarding/onboardingStorage.ts`](../../../lib/onboarding/onboardingStorage.ts) — `authRedirectPathStorage`.
- [`../../../auth.content/AGENTS.md`](../../auth.content/AGENTS.md) — the content script that relays `AUTH_SUCCESS`.

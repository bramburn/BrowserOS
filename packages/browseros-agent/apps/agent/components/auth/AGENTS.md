# `components/auth/` — Route auth guard

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/components`.

## What's here

`AuthGuard.tsx` — a single wrapper component that gates a React subtree
behind a live better-auth session. It reads `useSession()` from
`lib/auth/auth-client.ts` and either renders a centred spinner (while the
session query is pending), redirects to `/login` with the current location
in router state, or renders its children.

## Contents

```
auth/
└── AuthGuard.tsx   ← useSession() + useLocation() + <Navigate to="/login"
                      state={{ from: location }} replace />
```

## Rules

- **AU1 — `useSession` is imported from `@/lib/auth/auth-client`, never
  re-declared here.** `auth-client.ts` owns the better-auth client and its
  `baseURL` / `magicLinkClient` plugin config.
- **AU2 — Always distinguish `isPending` from "no session."** Rendering the
  redirect while the query is pending causes a login flash on every mount.
- **AU3 — Preserve `state={{ from: location }}`.** The login route reads it
  to return the user to where they were headed; dropping it breaks
  post-login redirect.
- **AU4 — `replace` on the `<Navigate>`**, so the guarded URL does not sit in
  history and trap the back button.
- **AU5 — This is presentation, not session management.** `AuthProvider`
  (in `lib/auth/`) is what mirrors the session into `chrome.storage.local`
  and identifies the user to PostHog/Sentry; `AuthGuard` only reads.

## Workflows

**Protecting a new route**
1. Wrap the route element in `<AuthGuard>…</AuthGuard>` in
   `entrypoints/app/`.
2. Confirm the route's login redirect target exists in the app router.
3. Test both states: signed out (redirect) and signed in (renders).

**Changing the pending state**
1. Edit the `isPending` branch in `AuthGuard.tsx` only.
2. Keep it a full-height centred spinner using `bg-background` and
   `text-muted-foreground` so it matches the app shell.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — component-tree rules (CMP1).
- [`../../lib/auth/AGENTS.md`](../../lib/auth/AGENTS.md) — `auth-client.ts`, `AuthProvider`, `sessionStorage`.
- [`../ui/AGENTS.md`](../ui/AGENTS.md) — `Loader2` comes from `lucide-react`, not a `ui/` primitive.
- [`../../entrypoints/AGENTS.md`](../../entrypoints/AGENTS.md) — where routes are registered.

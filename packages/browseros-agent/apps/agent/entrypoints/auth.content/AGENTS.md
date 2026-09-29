# `entrypoints/auth.content/` — auth relay content script

> Part of the WXT entry points in `apps/agent/entrypoints`. Parent guide:
> [`../../AGENTS.md`](../../AGENTS.md).

## What's here

A 12-line content script — the entire folder. It runs at `document_start` on the
BrowserOS web app's `/home` page only, listens for `window` `message` events, and
forwards any `AUTH_SUCCESS` payload to the extension service worker via
`chrome.runtime.sendMessage`.

This is the bridge that lets a magic-link/OAuth handshake completed in a normal web
tab hand the user back to the extension. The tab is then navigated to
`app.html#<saved redirect path>` by the worker.

## Contents

```
auth.content/
└── index.ts   ← defineContentScript({ matches, runAt: 'document_start', main })
```

## Rules

- **AC1 — The match pattern is built from `env.VITE_PUBLIC_BROWSEROS_API`.** It is
  `${env.VITE_PUBLIC_BROWSEROS_API}/home`; do not hardcode a host. A stale host here
  silently breaks the return-to-extension flow.
- **AC2 — `runAt: 'document_start'` is required.** The listener must be installed
  before the page posts its message, or the event is missed and the user is stranded
  on the web page.
- **AC3 — Forward the message verbatim.** The worker reacts to
  `message.type === 'AUTH_SUCCESS'`; wrapping or renaming the payload breaks it.
- **AC4 — Do not add UI or logic here.** Content scripts run in the page's world on a
  third-party origin. Anything beyond a relay belongs in `lib/`.
- **AC5 — The counterpart lives in `../background/index.ts`.** Change both sides
  together; there is no typed message channel between a content script and the worker
  in this codebase.

## Workflows

**Changing the trigger condition**
1. Edit `matches` in `index.ts` using `env.VITE_PUBLIC_BROWSEROS_API`.
2. Confirm the host in `.env.development` matches what the web app actually serves.
3. Reload the unpacked extension — content-script match changes need a reload, not
   just a page refresh.

**Debugging a user stuck on the web page after sign-in**
1. Check the content script is injected: DevTools → page → Sources → Content Scripts.
2. Check `window.postMessage({ type: 'AUTH_SUCCESS' })` is actually fired by the web app.
3. Check the worker's `AUTH_SUCCESS` handler and `authRedirectPathStorage`.

## Cross-references

- [`../../AGENTS.md`](../../AGENTS.md) — extension internals and entry-point conventions.
- [`../background/AGENTS.md`](../background/AGENTS.md) — the `AUTH_SUCCESS` receiver.
- [`../app/login/AGENTS.md`](../app/login/AGENTS.md) — where the user lands afterwards.
- [`../../../lib/env.ts`](../../lib/env.ts) — `env.VITE_PUBLIC_BROWSEROS_API`.

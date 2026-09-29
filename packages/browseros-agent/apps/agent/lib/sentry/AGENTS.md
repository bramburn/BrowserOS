# `lib/sentry/` — Error reporting and PII scrubbing

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

Sentry for the extension. `sentry.ts` initialises `@sentry/react` at
import time when a DSN is configured, tags every event with the extension
page, and drops errors that are expected during browser shutdown.
`sanitize.ts` is the redaction pass that runs on every outgoing event.
`sentryRootErrorHandler.ts` exports the `ReactDOM.RootOptions` bundle
(uncaught / caught / recoverable) that entry points spread into
`createRoot`.

## Contents

```
sentry/
├── sentry.ts                 ← Sentry.init({ dsn, sendDefaultPii: true,
│                                 release: chrome.runtime.getManifest().version,
│                                 environment: PROD ? 'production' : 'development' })
│                                 + breadcrumbs (console, dom, fetch, xhr);
│                                 beforeSend: SUPPRESSED_ERRORS filter,
│                                 extensionPage tag, then sanitizeEvent();
│                                 exports the `sentry` client
├── sanitize.ts               ← recursive key-name redaction against
│                                 SENSITIVE_KEY_PATTERNS (apikey, api_key,
│                                 accesskeyid, secretaccesskey, sessiontoken,
│                                 authorization, token, password, secret,
│                                 credential) → '[REDACTED]'
└── sentryRootErrorHandler.ts ← sentryRootErrorHandler: onUncaughtError /
                                 onCaughtError / onRecoverableError, all via
                                 sentry.reactErrorHandler()
```

## Rules

- **SNT1 — Never add a property containing credentials.** `sanitize.ts`
  matches on **key name**, so a payload shaped `{ config: { apiKey } }` is
  caught, but `{ blob: '<json string with a token>' }` is not. Prefer
  structured fields over embedded strings.
- **SNT2 — `SUPPRESSED_ERRORS` is the allowlist of noise.** "The browser is
  shutting down" and "No current window" are filtered in `beforeSend`. Add a
  new message there rather than wrapping calls in try/catch at the call site.
- **SNT3 — `sentry.ts` initialises on import.** Import it as early as
  possible in an entry point so early errors are captured; it is a no-op
  without `VITE_PUBLIC_SENTRY_DSN`.
- **SNT4 — `extensionPage` is derived from the URL path** (`sidepanel.html` →
  `sidepanel`). Don't pass a raw URL as a tag; it would leak the path.
- **SNT5 — Use `sentryRootErrorHandler` at the root.** Spreading the exported
  object into `createRoot(el, sentryRootErrorHandler)` is the established
  pattern; wiring the three callbacks by hand is a regression.
- **SNT6 — The release tag is the manifest version.** Bumping
  `package.json` `version` is what makes Sentry group correctly.

## Workflows

**Reporting a caught error**
1. `import { sentry } from '@/lib/sentry/sentry'`
2. `sentry.captureException(error, { extra: { structured, nonSecret } })`
3. Do not attach user text, prompts, or provider config blobs.

**Suppressing a noisy error class**
1. Add the message fragment to `SUPPRESSED_ERRORS` in `sentry.ts`.
2. Re-run the affected flow to confirm the noise is gone.

**Adding a new sensitive key shape**
1. Add the lowercased fragment to `SENSITIVE_KEY_PATTERNS` in `sanitize.ts`.
2. Prefer a substring that cannot collide with harmless keys.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB3, LIB5).
- [`../analytics/AGENTS.md`](../analytics/AGENTS.md) — `identify()` also sets the Sentry user.
- [`../env.ts`](../env.ts) — `VITE_PUBLIC_SENTRY_DSN` and `PROD`.
- [`../../wxt.config.ts`](../../wxt.config.ts) — the Sentry Vite plugin / sourcemap upload.
- [`../../entrypoints/AGENTS.md`](../../entrypoints/AGENTS.md) — where `sentryRootErrorHandler` is applied.

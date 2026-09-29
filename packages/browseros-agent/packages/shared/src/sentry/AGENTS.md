# `packages/shared/src/sentry/` — Sentry event scrubbing

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/packages/shared`.

## What's here

One module, `sanitize.ts`, with two exports. `sanitize(obj)` walks any
structure and replaces the value of any key matching a sensitive pattern with
`[REDACTED]`. `sanitizeEvent(event)` applies that walk to the four places
secrets leak in a Sentry event — breadcrumb `data`, `contexts`, `extra`, and
per-frame `vars` — and is wired into the `beforeSend` hooks on both the
server and the agent extension.

## Contents

```
shared/src/sentry/
└── sanitize.ts   ← SENSITIVE_KEY_PATTERNS, isSensitiveKey, sanitize<T>, sanitizeEvent<E>
```

## Rules

**SY1 — `sanitize` never mutates its input.** It returns a new object/array
tree; `sanitizeEvent` is the deliberate exception because it edits the event
in place before send. Keep that distinction.

**SY2 — Matching is substring-based on the lowercased key.** The patterns are
`apikey`, `api_key`, `accesskeyid`, `secretaccesskey`, `sessiontoken`,
`authorization`, `token`, `password`, `secret`, `credential`. A key like
`myApiKeyHeader` is redacted; a *value* that looks secret but sits under a
benign key is not. Don't switch to exact-match.

**SY3 — Extending the list is cheap and safe.** Adding a pattern only ever
redacts more. Removing one can leak credentials, so treat that as a security
regression, not a cleanup.

**SY4 — Primitives and `null`/`undefined` pass through unchanged**; arrays
are mapped element-wise; only plain objects are rebuilt key-by-key.

**SY5 — The `any` casts are deliberate and commented.** `sanitizeEvent` uses
`Record<string, any>` with an `eslint-disable-next-line` because the Sentry
event type varies by SDK version. Don't "clean that up" without a replacement
type.

**SY6 — Import as `@browseros/shared/sentry/sanitize`.** Both `sanitize` and
`sanitizeEvent` come from this one subpath; there is no separate
`sentry/index.ts`.

## Workflows

**Wiring a new Sentry integration:** 1. Import `sanitizeEvent` from
`@browseros/shared/sentry/sanitize`. 2. Use it in the client's `beforeSend`.
3. Do not add a second, integration-specific redaction pass — extend
`SENSITIVE_KEY_PATTERNS` instead so all integrations benefit.

**Handling a new event field that can carry secrets:** add it to the
`sanitizeEvent` switch-along block (`breadcrumbs[].data`, `contexts`, `extra`,
`exception.values[].stacktrace.frames[].vars`) so it is scrubbed like the
existing four.

**Testing a redaction:** `sanitize` is pure and takes no I/O — exercise it
directly with a nested object containing `apiKey`, `Authorization`, and a
nested `token` array, and assert every one becomes `[REDACTED]`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `src/` layout and the exports rule (SS1).
- [`../../AGENTS.md`](../../AGENTS.md) — `@browseros/shared` package scope.
- [`../../../../../apps/server/AGENTS.md`](../../../../apps/server/AGENTS.md)
  — Sentry at the boundary (rule S5), one of the two `beforeSend` call sites.

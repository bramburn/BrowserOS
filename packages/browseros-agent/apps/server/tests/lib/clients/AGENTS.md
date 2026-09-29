# `tests/lib/clients/` — outbound client tests (offline)

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/lib/`.

## What's here

Tests for `../../../../src/lib/clients/`, mirroring that folder. Today the
only coverage is under `oauth/` — the token store and the OAuth lifecycle.
The Klavis client is covered indirectly through
[`../../api/services/klavis/`](../../api/services/klavis/AGENTS.md) (where it
is stubbed at the proxy boundary), and the LLM folder through the mock
providers it ships.

## Contents

| Path | Covers |
|---|---|
| `oauth/index.test.ts` | `initializeOAuth()` / `shutdownOAuth()` lifecycle from `../../../../src/lib/clients/oauth/index.ts`. |
| `oauth/token-store.test.ts` | Drizzle persistence over the `oauth_tokens` table. |

There are no test files directly in this directory; both tests live in
`oauth/`.

## Rules

**CL-T1 — Mirror the source tree.** A new client under
`../../../../src/lib/clients/<name>/` gets tests at
`tests/lib/clients/<name>/`.

**CL-T2 — Never hit a real provider.** Stub the transport. The OAuth tests
use a temp SQLite handle and fake responses; adding a live `chatgpt.com` or
GitHub call makes the suite non-deterministic and credential-dependent.

**CL-T3 — `token-store.test.ts` needs a temp db, not the install's.**
`openBrowserOsDatabase({ dbPath: <temp>, migrationsDir })` is the sanctioned
handle. A test pointed at the user's database is a data-loss bug.

**CL-T4 — Tokens never appear in assertions or logs.** Assert on provider
name, status, and expiry. If a test failure would print a token, the test is
wrong.

**CL-T5 — The Klavis client has no test here on purpose.** Its behaviour is
the proxy's concern (connect, retry, degrade to a null handle); duplicating
it in this folder would give two places to update.

## Workflows

**Running:** `bun run test:lib` from `apps/server/`.

**Adding a client:** 1. Implement under
`../../../../src/lib/clients/<name>/`. 2. Add `tests/lib/clients/<name>/`.
3. Stub the transport; use a temp db if it persists anything.

**Testing OAuth lifecycle changes:** `oauth/index.test.ts` pins init/shutdown;
route-level behaviour (`GET /oauth/:provider/status`, `DELETE /oauth/:provider`)
is not covered by a route test yet — `/oauth` has no file in
[`../../api/routes/`](../../api/routes/AGENTS.md).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `tests/lib/` conventions.
- [`../../../../src/lib/clients/AGENTS.md`](../../../src/lib/clients/AGENTS.md) — the clients under test.
- [`./oauth/AGENTS.md`](oauth/AGENTS.md) — the OAuth tests.
- [`../../__helpers__/AGENTS.md`](../../__helpers__/AGENTS.md) — the harness.

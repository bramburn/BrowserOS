# `tests/api/services/klavis/` — Klavis proxy and cache tests (offline)

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/api/services/`.

## What's here

Two files covering the Klavis integration layer in
[`../../../../../src/api/services/klavis/`](../../../../src/api/services/klavis/AGENTS.md).
Both run hermetically — the remote Klavis API is stubbed — because the
behaviour worth testing is precisely what happens when Klavis is slow,
unavailable, or returns the same conversation twice.

## Contents

| File | Covers |
|---|---|
| `strata-cache.test.ts` | `KlavisStrataCache` and the `klavisStrataCache` singleton — caching of Klavis `createStrata` responses. |
| `strata-proxy.test.ts` | `connectKlavisProxy`, `connectKlavisInBackground` (retry, null handle), `buildKlavisToolSet`, `registerKlavisTools`. |

## Rules

**KV-T1 — Never hit the real Klavis API.** Stub `KlavisClient`. These tests
must be deterministic and run in CI without credentials.

**KV-T2 — Pin the degradation path.** The server is designed to work with
`KlavisProxyRef.handle === null`; `connectKlavisInBackground` is explicitly
non-blocking so browser tools are available immediately. A test that only
covers the success path leaves the important invariant untested.

**KV-T3 — The cache exists because `/chat` blocked on `createStrata`.** Any
change to the cache key or TTL must keep a test that proves the second call is
served from memory. That regression is the reason this module exists.

**KV-T4 — Reset the singleton between tests.** `klavisStrataCache` is a
module-level instance; tests that share it see each other's entries.

## Workflows

**Running:** `bun run test:api` from `apps/server/`.

**Adding a Klavis endpoint:** it needs no test here unless it changes caching
or proxy lifecycle. Route-level behaviour for `/klavis` is in
[`../../routes/klavis.test.ts`](../../routes/AGENTS.md).

**Debugging a Klavis failure:** start with `strata-proxy.test.ts` for the
connect/retry behaviour, then `strata-cache.test.ts` for the memoisation. The
HTTP client itself is not covered by a unit test here — it is exercised
through the proxy.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `tests/api/services/` conventions.
- [`../../../../../src/api/services/klavis/AGENTS.md`](../../../../src/api/services/klavis/AGENTS.md) — the module under test.
- [`../../../../../src/lib/clients/klavis/AGENTS.md`](../../../../src/lib/clients/klavis/AGENTS.md) — the HTTP client it uses.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `apps/server/` MCP server internals.

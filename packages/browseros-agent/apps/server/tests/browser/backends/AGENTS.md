# `tests/browser/backends/` — CDP backend tests (offline)

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/browser/`.

## What's here

One file: `cdp.test.ts`, the sole test for
`../../../../src/browser/backends/cdp.ts`. It defines a `MockWebSocket` class
and drives `CdpBackend` against it, so request/response correlation,
timeouts, session caching, and event fan-out are all covered without spawning
a Chromium.

## Contents

| File | Covers |
|---|---|
| `cdp.test.ts` | `CdpBackend` with `MockWebSocket` — `connect()` loopback discovery, `messageId` correlation, pending-request resolve/reject, `TIMEOUTS`-based timeout, `session(id)` cache, `onSessionEvent` handlers, disconnect/reconnect. |

## Rules

**BK1 — The mock is a socket, not a backend.** `MockWebSocket` exposes
`onopen` / `onerror` / `onmessage` and a static `instances` array so a test
can emit frames by hand. Mocking `CdpBackend` itself would test nothing.

**BK2 — No browser, no port.** This is the only browser-layer test that is
hermetic. A test needing a real page belongs under
[`../../tools/`](../../tools/AGENTS.md).

**BK3 — Import timeouts from `@browseros/shared`.** The file already does;
keep it that way so a `TIMEOUTS` change surfaces here rather than as a flaky
wait.

**BK4 — `afterEach` cleanup is mandatory.** Each `CdpBackend` owns a
keepalive `setInterval`. A leaked instance keeps a handle open and the test
process won't exit.

## Workflows

**Running:** `bun run test:browser` (or `bun run test:cdp`) from `apps/server/`.

**Adding a case:** 1. Create the backend with `new CdpBackend({ port })`. 2.
`await backend.connect()`. 3. Drive `MockWebSocket.instances.at(-1)` to emit
the frame. 4. Assert the pending promise. 5. `await backend.disconnect()`.

**Testing a CDP discovery change:** the loopback host list
(`127.0.0.1`, `localhost`, `[::1]`) is a security boundary; a test that
asserts a non-loopback host is rejected is worth adding.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the browser-layer test conventions.
- [`../../../../src/browser/backends/AGENTS.md`](../../../src/browser/backends/AGENTS.md) — the module under test.
- [`../../__helpers__/AGENTS.md`](../../__helpers__/AGENTS.md) — the harness.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `apps/server/` MCP server internals.

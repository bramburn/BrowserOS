# `tests/browser/` — browser layer tests

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/`.

## What's here

One file, `backends/cdp.test.ts`, which tests
`CdpBackend` from `../../../../src/browser/backends/cdp.ts` — the WebSocket
CDP client. It substitutes a `MockWebSocket` class for the real transport, so
it exercises connect, request/response correlation, and event fan-out
**without a browser**. That is the whole reason this folder exists: the
transport layer is the one part of `src/browser/` that can be tested without
a live Chromium.

## Contents

| File | Covers |
|---|---|
| `backends/cdp.test.ts` | `CdpBackend` with a `MockWebSocket` — connect, `messageId` correlation and pending-request resolution, timeout rejection, session cache, `onSessionEvent` fan-out, and reconnect behaviour. |

## Rules

**BR1 — Mock the socket, not the backend.** The test defines its own
`MockWebSocket` with `onopen` / `onerror` / `onmessage` hooks and a static
`instances` array. Replacing it with a mocked `CdpBackend` would skip exactly
the code under test.

**BR2 — No browser required.** If you add a test here that needs a real
Chromium, it belongs under [`../tools/`](../tools/AGENTS.md) instead.

**BR3 — Timeouts come from `TIMEOUTS`.** The test imports
`@browseros/shared/constants/timeouts` rather than hard-coding a wait, so it
stays in step with the production value.

**BR4 — Reconnection tests must reset module state.** `CdpBackend` holds a
keepalive timer and a session cache per instance; a leaked instance between
tests will fire into a dead socket.

## Workflows

**Running:** `bun run test:browser` (alias `bun run test:cdp`) from
`apps/server/`.

**Adding a transport test:** 1. Add a `describe` block in
`backends/cdp.test.ts`. 2. Drive `MockWebSocket.instances[0]` to emit the CDP
frame you need. 3. Assert the promise the public API returned.

**Testing `src/browser/browser.ts` or the domain modules:** those need a
real browser. Add the test under `../tools/` using `withBrowser()`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the test suite and group rules.
- [`../../../../src/browser/AGENTS.md`](../../src/browser/AGENTS.md) — the browser façade.
- [`./backends/AGENTS.md`](./backends/AGENTS.md) — the CDP backend test.
- [`../__helpers__/AGENTS.md`](../__helpers__/AGENTS.md) — the harness.

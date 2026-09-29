# `src/browser/backends/` — CDP WebSocket backend

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/browser/`.

## What's here

The transport layer between the server and a running Chromium.
`types.ts` declares the `CdpBackend` interface and `CdpTarget`; `cdp.ts` is
the only implementation — a hand-rolled CDP client over a raw `WebSocket`
with request/response correlation, a per-target session cache, event
multiplexing, a keepalive timer, and bounded reconnect logic. It uses
`createProtocolApi` from `@browseros/cdp-protocol` to build the
`ProtocolApi` surface, which it then merges onto the class by declaration
merging.

## Contents

| File | Purpose |
|---|---|
| `types.ts` | `CdpBackend` interface: `connect`, `disconnect`, `isConnected`, `getTargets`, `session(id)`, `onSessionEvent`. Plus `CdpTarget` (`id`, `type`, `title`, `url`, `tabId`, `windowId`). |
| `cdp.ts` | The WebSocket client. Loopback discovery over `127.0.0.1` / `localhost` / `[::1]`, pending-request map keyed by incrementing `messageId`, `sessionCache`, per-session event handlers, keepalive, reconnect, and the `ProtocolApi` declaration-merge. |

## Rules

**CDP1 — Implement the interface, don't extend the class.** `browser.ts`
imports `type { CdpBackend } from './types'`. A new backend (e.g. a
non-Chromium driver) satisfies `types.ts` and plugs in without touching
`browser.ts`.

**CDP2 — Every request needs a timeout and a pending-entry cleanup.** The
`pending` map holds `{ resolve, reject, timer }` per `messageId`; timeouts
come from `TIMEOUTS` in `@browseros/shared/constants/timeouts` and size limits
from `CDP_LIMITS`. A leak here is a permanently stuck tool call.

**CDP3 — Discovery is loopback-only.** `LOOPBACK_DISCOVERY_HOSTS` is exactly
`127.0.0.1`, `localhost`, `[::1]`. Don't widen it — the browser is expected on
the same machine, and this is the security boundary for a server that also
binds `0.0.0.0`.

**CDP4 — The declaration merge is deliberate.** `interface CdpBackend extends
ProtocolApi {}` over `class CdpBackend` is filled at runtime by
`Object.assign`; both biome ignores on those lines are load-bearing. Don't
"clean up" the `noUnsafeDeclarationMerging` suppression.

**CDP5 — Reconnect policy is a constructor option.** `CdpBackendConfig` is
`{ port, exitOnReconnectFailure? }`. `main.ts` constructs it with the port
from `ServerConfig`; tests construct it directly. Failures are surfaced via
`isPortInUseError`-style typed errors, not bare `throw new Error`.

## Workflows

**Testing the backend without a browser:** `tests/browser/backends/cdp.test.ts`
substitutes a `MockWebSocket` class and asserts connect, send/receive
correlation, and event fan-out. Extend that file rather than spawning
Chromium.

**Adding a CDP domain call:** you usually don't need to — `ProtocolApi` from
`@browseros/cdp-protocol` already exposes the full typed surface, obtained via
`this.session(targetId)` or the class's own merged methods. Add a helper to
`../browser.ts` instead.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `src/browser/` façade and domain modules.
- [`../../../tests/browser/backends/AGENTS.md`](../../../tests/browser/backends/AGENTS.md) — mock-WebSocket tests.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `apps/server/` MCP server internals.

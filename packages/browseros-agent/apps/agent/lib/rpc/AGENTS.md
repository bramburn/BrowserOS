# `lib/rpc/` — Typed client for the local agent server

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

Two files that build the extension's single typed client for the local
BrowserOS agent server. `getClient.ts` uses Hono's `hc<AppType>()` to
produce a client whose routes are typed from the **server** package
(`@browseros/server`), and `RpcClientProvider.tsx` puts that client — held
as a promise — on a React context so components can `await` it. The
promise is pre-resolved at module import so the base URL lookup (a
`chrome.browserOS` pref read) starts as early as possible.

## Contents

```
rpc/
├── getClient.ts        ← type RpcClient = ReturnType<typeof hc<AppType>>;
│                         module-level `clientPromise`, getClient() memoises
│                         it, and a bare getClient() call at the bottom
│                         kicks off resolution on import
└── RpcClientProvider.tsx ← <RpcClientProvider> supplies the promise via
                            React `use()`; useRpcClient() unwraps it and
                            throws outside the provider
```

## Rules

- **RPC1 — `getClient()` is the only way to reach the agent server.** Its
  routes are typed; anything typed as `fetch(url, …)` in `lib/` is a bypass.
- **RPC2 — The client promise is created once.** Do not call `hc()` at a call
  site; import `getClient()` or `useRpcClient()`. Re-instantiating loses the
  memoised base-URL lookup.
- **RPC3 — `useRpcClient()` must be called under `<RpcClientProvider>`.** It
  throws a clear error otherwise; don't wrap the throw in a try/catch.
- **RPC4 — The server URL is never passed in.** It comes from
  `getAgentServerUrl()` in `lib/browseros/helpers.ts`, which honours
  `Feature.UNIFIED_PORT_SUPPORT`. Don't accept a `baseUrl` prop.
- **RPC5 — Adding a route means changing the server first.** `AppType` is
  imported from `@browseros/server`; a route that doesn't exist there won't
  type-check, and that's the point — don't widen the type with `as any`.

## Workflows

**Calling a server route from a component**
1. Ensure `<RpcClientProvider>` wraps the tree (both app entry points do).
2. `const client = useRpcClient()`
3. `const res = await client.<route>({ param })` — types come from the server.

**Calling it outside React**
1. `const client = await getClient()` — the same memoised instance.
2. Do not add a second `hc()` construction.

**Adding a new route**
1. Add it in `packages/browseros-agent/apps/server/src/api/`.
2. Reinstall/relink the workspace dependency and re-run typecheck; the client
   type updates automatically.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB3).
- [`../browseros/AGENTS.md`](../browseros/AGENTS.md) — `getAgentServerUrl()` and the port gates.
- [`../sse.ts`](../sse.ts) — how streamed responses are consumed.
- [`../../package.json`](../../package.json) — the `hono` dependency providing `hc`.

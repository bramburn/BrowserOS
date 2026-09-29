# `packages/cdp-protocol/` — generated Chrome DevTools Protocol bindings

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`.

## What's here

`@browseros/cdp-protocol` is the type-safe CDP surface the server uses to
talk to Chromium. Everything under `src/generated/` is emitted by
`../../scripts/codegen/cdp-protocol.ts` from Chromium's `protocol.json`:
per-domain type definitions in `domains/`, per-domain API classes in
`domain-apis/`, plus the aggregate `protocol-api.ts` and the `create-api.ts`
factory. Two extra domains — `Bookmarks` and `History` — come from the
BrowserOS protocol rather than stock Chrome.

## Contents

```
cdp-protocol/
├── package.json    ← 2 exports per domain (./domains/x, ./domain-apis/x) + protocol-api + create-api
├── tsconfig.json   ← extends ../../tsconfig.json, rootDir "src"
├── README.md       ← usage, supported-domain table, regeneration steps
└── src/generated/
    ├── domains/           ← 57 files: types only (XxxParams / XxxResult / XxxEvent)
    ├── domain-apis/       ← 57 files: XxxAPI wrapper classes
    ├── protocol-api.ts    ← the aggregated ProtocolApi type
    └── create-api.ts      ← createProtocolApi(send, on) proxy factory
```

## Rules

**CD1 — Never hand-edit `src/generated/`.** Every file starts with
`// ── AUTO-GENERATED from CDP protocol. DO NOT EDIT. ──`. A manual fix is
overwritten by the next `bun run gen:cdp`; fix the emitters in
`../../scripts/codegen/lib/` instead.

**CD2 — Two exports per domain, always together.** A new domain produces
`./domains/<kebab>` and `./domain-apis/<kebab>`; the codegen rewrites
`package.json` `exports` itself, so never hand-edit that map to add one side
only.

**CD3 — Deep subpath imports.** `@browseros/cdp-protocol/domains/page`,
`@browseros/cdp-protocol/domain-apis/page`,
`@browseros/cdp-protocol/protocol-api`,
`@browseros/cdp-protocol/create-api`. There is no root export and no barrel.

**CD4 — `create-api.ts` is a runtime Proxy, not a hand-written surface.**
`createProtocolApi(send, on)` builds a `Proxy` per domain that maps a method
call to `send('<Domain>.<method>', params)` and `on('<Domain>.<event>', …)`.
Adding a domain means adding a line to that object.

**CD5 — Types are imports; APIs are values.** Domain files export
`export interface`; domain-api files export a class. Import domain types with
`import type` so the bundler drops them.

**CD6 — Regeneration needs a Chromium build.** `CDP_PROTOCOL_JSON` must point
at `…/gen/third_party/blink/public/devtools_protocol/protocol.json`. Without
it the codegen exits 1 immediately.

## Workflows

**Adding a CDP method to a domain (e.g. a new `Page.navigate` parameter):**
1. Don't edit `src/generated/domains/page.ts`. 2. Update the upstream
`protocol.json` (Chromium source). 3. `CDP_PROTOCOL_JSON=<path> bun run
gen:cdp`. 4. Review the diff across `domains/`, `domain-apis/`,
`protocol-api.ts`, `create-api.ts`, and `package.json`.

**Consuming a domain from the server:** 1. `import type { NavigateParams }
from '@browseros/cdp-protocol/domains/page'`. 2. `import { PageAPI } from
'@browseros/cdp-protocol/domain-apis/page'`. 3. Build the API with
`createProtocolApi(send, on)` using the CDP transport's raw send/subscribe.

**Checking which domains exist:** read `../../../../packages/cdp-protocol/package.json`
`exports` keys, or the domain table in `README.md` — the two agree by
construction.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — Bun monorepo parent.
- [`README.md`](README.md) — usage and regeneration instructions.
- [`src/AGENTS.md`](src/AGENTS.md) — the `src/` layout.
- [`../../scripts/codegen/AGENTS.md`](../../scripts/codegen/AGENTS.md) — the generator.
- [`../../../../apps/server/AGENTS.md`](../../apps/server/AGENTS.md) — the main consumer.

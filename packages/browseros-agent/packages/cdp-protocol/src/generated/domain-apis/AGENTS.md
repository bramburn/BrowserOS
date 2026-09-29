# `packages/cdp-protocol/src/generated/domain-apis/` — per-domain API wrappers

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/packages/cdp-protocol`.

## What's here

57 generated runtime modules, one per CDP domain, name-aligned with
`../domains/`. Each re-exports its domain's param/result/event types and
exposes a `<Pascal>API` class that binds those calls to a raw transport
(`send(method, params)` / `on(event, handler)`). This is the layer the server
actually instantiates; `../domains/` is only its type vocabulary.

## Contents

```
generated/domain-apis/
├── page.ts, dom.ts, dom-snapshot.ts, dom-storage.ts, dom-debugger.ts, css.ts, overlay.ts
├── network.ts, fetch.ts, io.ts, service-worker.ts, cache-storage.ts
├── input.ts, emulation.ts, device-orientation.ts, device-access.ts
├── runtime.ts, debugger.ts, console.ts, profiler.ts, heap-profiler.ts
├── browser.ts, target.ts, inspector.ts, extensions.ts, pwa.ts
├── performance.ts, performance-timeline.ts, tracing.ts, memory.ts
├── media.ts, web-audio.ts, cast.ts
├── security.ts, web-authn.ts, fed-cm.ts
├── indexed-db.ts, storage.ts, file-system.ts
└── bookmarks.ts, history.ts
```

## Rules

**DA1 — Do not edit.** Same `// ── AUTO-GENERATED … DO NOT EDIT. ──` header as
`../domains/`. Fix `domain-api-emitter.ts` in the codegen instead.

**DA2 — One file per domain, one class per file.** The exported class name is
`toPascalCase(domain) + 'API'` (`PageAPI`, `DOMSnapshotAPI`,
`WebAuthnAPI`). The `package.json` exports key is `./domain-apis/<kebab>` and
must exist alongside `./domains/<kebab>`.

**DA3 — This is a value import, not `import type`.** `import { PageAPI } from
'@browseros/cdp-protocol/domain-apis/page'` pulls real code into the bundle;
the type-only sibling is `../domains/page`.

**DA4 — `create-api.ts` is the fast path.** If you only need to fire commands
by name, `createProtocolApi` builds all 57 proxies from a `(send, on)` pair
rather than you constructing 57 classes. Use the class when you want the
per-domain constructor shape the emitters produce.

**DA5 — Keep the two trees name-aligned.** Same `domainToKebab` rule drives
both filenames; a file present in one tree and missing in the other means a
broken codegen run, not an intentional exclusion.

## Workflows

**Calling one CDP domain:** 1. `import { PageAPI } from
'@browseros/cdp-protocol/domain-apis/page'`. 2. Construct it with the
transport's send/on pair. 3. Call the method with the matching
`../domains/page` params type. 4. Subscribe with the event method for the
`<Domain>.<Event>` name.

**Regenerating after a protocol bump:** run `CDP_PROTOCOL_JSON=<path> bun run
gen:cdp` from `packages/browseros-agent`, then check that every new domain
landed in **both** `domain-apis/` and `domains/` and that `package.json` gained
both exports keys.

**Debugging a method-not-found at runtime:** the method name is stringified as
`${domain}.${method}` by the transport; check `../create-api.ts` and the CDP
transport's `send` implementation rather than looking for a missing export.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — generated tree rules (CG1–CG4).
- [`../domains/AGENTS.md`](../domains/AGENTS.md) — the matching type files.
- [`../create-api.ts`](../create-api.ts) — the `createProtocolApi` proxy factory.
- [`../../../../../scripts/codegen/lib/AGENTS.md`](../../../../../scripts/codegen/lib/AGENTS.md)
  — the emitter that writes this folder.

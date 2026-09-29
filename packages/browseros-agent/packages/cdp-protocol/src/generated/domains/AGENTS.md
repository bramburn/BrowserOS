# `packages/cdp-protocol/src/generated/domains/` — per-domain CDP types

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/packages/cdp-protocol`.

## What's here

57 generated type-only modules, one per CDP domain, named by
`domainToKebab(domain)`. Each exports the params/result interfaces for that
domain's commands and the event interfaces for its events — e.g.
`domains/page.ts` exports `NavigateParams`, `NavigateResult`,
`CaptureScreenshotParams`, `FrameNavigatedEvent`, and so on. Small domains
are tiny (`domains/schema.ts` is just `Domain`, `GetDomainsResult`); `page.ts`
is one of the largest.

## Contents

```
generated/domains/
├── page.ts, dom.ts, dom-snapshot.ts, dom-storage.ts, dom-debugger.ts, css.ts, overlay.ts
├── network.ts, fetch.ts, io.ts, service-worker.ts, cache-storage.ts
├── input.ts, emulation.ts, device-orientation.ts, device-access.ts
├── runtime.ts, debugger.ts, console.ts, profiler.ts, heap-profiler.ts
├── browser.ts, target.ts, inspector.ts, extensions.ts, pwa.ts
├── performance.ts, performance-timeline.ts, tracing.ts, memory.ts
├── media.ts, web-audio.ts, cast.ts
├── security.ts, web-authn.ts, fed-cm.ts
├── indexed-db.ts, storage.ts, file-system.ts
└── bookmarks.ts, history.ts        ← BrowserOS-custom domains
```

## Rules

**DM1 — `import type` only.** These files contain only `export interface`
declarations; there is nothing to emit at runtime.

**DM2 — Do not edit.** The header is
`// ── AUTO-GENERATED from CDP protocol. DO NOT EDIT. ──`. Renaming or
"fixing" a type here is reverted by the next codegen run.

**DM3 — Import by subpath.** `import type { NavigateParams } from
'@browseros/cdp-protocol/domains/page'`. Deep-importing the file path
directly is not in the `exports` map and will fail resolution.

**DM4 — BrowserOS extras live here too.** `bookmarks.ts` and `history.ts` are
not stock Chrome domains; they come from BrowserOS's own `protocol.json` and
are regenerated like everything else.

**DM5 — Kebab names are derived, not chosen.** `DOMSnapshot` → `dom-snapshot`,
`CSS` → `css`, `WebAuthn` → `web-authn`. If a new file name looks wrong, fix
`naming.ts` in the codegen and regenerate.

## Workflows

**Looking up a command's types:** grep the domain file for
`<CommandName>Params` / `<CommandName>Result`; event payloads are
`<EventName>Event`.

**A type is missing:** 1. Confirm the command exists in the `protocol.json`
you regenerated from. 2. Re-run `bun run gen:cdp`. 3. If it still isn't
there, the upstream protocol version predates it — check
`generated/protocol-api.ts` for the version, not this folder.

**Working across domains:** cross-domain `$ref`s in the protocol JSON are
resolved into standalone interfaces here, so a `Page`-shaped field in
`dom.ts` is a duplicated type, not an import of `page.ts`. Don't try to
unify them by hand.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — generated tree rules (CG1–CG4).
- [`../domain-apis/AGENTS.md`](../domain-apis/AGENTS.md) — the matching wrappers.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `@browseros/cdp-protocol` package.
- [`../../../../../scripts/codegen/lib/AGENTS.md`](../../../../../scripts/codegen/lib/AGENTS.md)
  — the emitters that write this folder.

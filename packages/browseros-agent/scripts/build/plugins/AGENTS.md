# `scripts/build/plugins/` — Bun bundler plugins

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/scripts/build`.

## What's here

One file: `wasm-binary.ts`, a `Bun.build` plugin registered in
`../server/compile.ts` via `plugins: [wasmBinaryPlugin()]`. It exists so
esbuild-style `*.wasm?binary` imports survive `bun build --compile` into a
single-file executable — the loader inlines the bytes as a `Uint8Array`
literal instead of leaving a file reference the compiled binary can't open.

## Contents

```
scripts/build/plugins/
└── wasm-binary.ts   ← wasmBinaryPlugin(): BunPlugin with onResolve + onLoad
```

## Rules

**BP1 — The `?binary` suffix is the contract.** The `onResolve` filter is
`/\.wasm\?binary$/`; an import without the suffix is not intercepted and will
fail at runtime inside the compiled binary.

**BP2 — Bare specifiers resolve via `createRequire`.** A non-relative
specifier like `web-tree-sitter/tree-sitter.wasm?binary` is resolved with
`require.resolve(specifier, { paths: [resolveDir, cwd] })`; relative and
absolute paths are resolved with `node:path`.

**BP3 — Output is a JS module, not a binary loader.**
`onLoad` reads the file with `Bun.file(...).arrayBuffer()` and emits
`export default new Uint8Array([...]);` with `loader: 'js'`. Consumers must
treat the default export as bytes.

**BP4 — The namespace is `wasm-binary`.** `onLoad` deliberately matches
`/.*/` in that namespace only, so nothing else in the bundle is affected.

**BP5 — Add new plugins here and register them in `../server/compile.ts`.**
A plugin used by the server bundle but not listed in that `plugins` array
simply won't run.

## Workflows

**Adding a WASM dependency to the server:** 1. Add the package. 2. Import it
with the `?binary` suffix. 3. Treat the default export as a `Uint8Array` (and
`Bun` will instantiate the WASM from those bytes).

**Verifying the plugin ran:** build one target with
`bun run build:server:test` and check the bundle in
`dist/prod/server/.tmp/bundle/index.js` — an inlined `new Uint8Array([…])`
confirms interception; a leftover file path means the suffix is missing.

**Adding a second plugin (e.g. native `.node` addons):** copy this file's
shape — a factory returning `BunPlugin` with its own namespace — and append it
to the `plugins` array in `../server/compile.ts`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `scripts/build/` scope (rule SB8 covers this plugin).
- [`wasm-binary.ts`](wasm-binary.ts) — the only file here.
- [`../server/compile.ts`](../server/compile.ts) — where the plugin is registered.
- [`../../../CLAUDE.md`](../../../CLAUDE.md) — build/runtime conventions.

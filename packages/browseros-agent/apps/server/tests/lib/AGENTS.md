# `tests/lib/` — offline unit tests for `src/lib/`

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/`.

## What's here

Mock-only unit tests for `../../src/lib/` — the infrastructure layer. Nothing
here needs a browser, a CDP port, a booted server, or network access: the
subsystems under test (Lima, podman, SSH, the ACPX agent processes, the OAuth
callback) are all driven through fakes, temp directories, and injected
dependencies. This is the fastest group in the suite and the right home for
anything that *can* be tested hermetically.

## Contents

| Path | Covers | Files (direct) |
|---|---|---|
| `agents/` | ACPX runtime, agent store, catalog, turn registry, message queue, ACP UI stream | 10 |
| `agents/runtime/` | Claude / Codex / Hermes runtimes + registry + both abstract bases | 6 |
| `agents/hermes/` | Hermes path and provider mapping | 1 |
| `clients/oauth/` | Token store and OAuth lifecycle | 2 (under `oauth/`) |
| `container/` | `ContainerCli`, `image-loader` | 2 |
| `container/managed/` | `ManagedContainer` state machine | 1 |
| `db/` | `index.ts` — db handle lifecycle | 1 |
| `vm/` | `LimaCli`, `lima-config`, `paths`, `vm-runtime`, `errors` | 5 |
| `identity.test.ts` | `src/lib/identity.ts` | 1 |
| `process-lock.test.ts` | `src/lib/process-lock.ts` | 1 |

There are no test files for `logger.ts`, `sentry.ts`, `metrics.ts`,
`browseros-dir.ts` (covered at `../browseros-dir.test.ts`), `serialize-error.ts`,
`openrouter-fetch.ts`, `browseros-fetch.ts`, `port-binding.ts`, `polyfill.ts`,
`mcp-transport-detect.ts`, or `wait-for-helper.ts` — those modules are covered
indirectly or not at all.

## Rules

**L-T1 — If it can be faked, it belongs here.** The dividing line against
`../tools/` is environment need, not subject. A browser-backed test in this
folder is misplaced and will be slow in CI.

**L-T2 — Fake the binary, not the module.** `../__helpers__/fake-limactl.ts`
and `../__helpers__/fake-ssh.ts` write executable stubs into temp directories
and put them on `PATH`, so the production argument-construction code is still
exercised. Prefer them over module mocks.

**L-T3 — Use temp directories, never shared state.** Container, VM, OAuth,
and agent-state tests all need an isolated filesystem. `mkdtemp` in
`beforeEach`, `rm -r` in `afterEach`.

**L-T4 — `resetAgentRuntimeRegistry()` is a legitimate test call.**
`src/lib/agents/runtime/registry.ts` exposes it for exactly this. Production
code must not call it; tests should.

**L-T5 — The tree mirrors `src/lib/`.** A new module in `src/lib/x/` gets a
test at `tests/lib/x/`. That's what makes coverage greppable in both
directions.

## Workflows

**Running:** `bun run test:lib` from `apps/server/`.

**Testing a `src/lib` change:** 1. Find the mirrored path under `tests/lib/`.
2. Add or extend the `describe` block. 3. Stub binaries with the
`fake-*.ts` helpers where applicable. 4. Assert on the returned value or the
thrown typed error.

**Moving a test out of `../tools/`:** if you can replace the browser with a
fake, the test belongs here — it will run in `bun run test:lib` and in
`test:core`'s siblings instead of requiring a spawned BrowserOS.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the test suite and group rules.
- [`../../src/lib/AGENTS.md`](../../src/lib/AGENTS.md) — the module under test.
- [`../__helpers__/AGENTS.md`](../__helpers__/AGENTS.md) — the fakes and harness.
- [`./vm/AGENTS.md`](./vm/AGENTS.md) — Lima tests.
- [`./agents/AGENTS.md`](./agents/AGENTS.md) — ACPX agent tests.

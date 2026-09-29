# `tests/__fixtures__/` — static test fixtures

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/`.

## What's here

Data, not behaviour. `snapshot.ts` holds small HTML documents used as page
fixtures, `server.ts` provides `serverHooks` (a `node:http` server with
`before`/`after`/`afterEach` lifecycle) for tests that need a fake origin, and
`index.ts` is the barrel. `acl/` holds JSON element captures for the
reference ACL scorer. Test *utilities* live next door in
[`../__helpers__/`](../__helpers__/AGENTS.md) — the split is data here,
behaviour there.

## Contents

| File | Purpose |
|---|---|
| `index.ts` | Barrel; currently re-exports `serverHooks` from `./server`. |
| `snapshot.ts` | `screenshots: Record<string, { html: string }>` — named HTML fixtures (`basic` is `<div>Hello MCP</div>`, etc.). |
| `server.ts` | `serverHooks` — a `node:http` server with `before`/`after`/`afterEach` hooks, for tests that must fetch a real origin. |
| `acl/` | Five JSON element captures for the ACL scorer — see below. |

## Contents (acl fixtures)

| File | Case |
|---|---|
| `acl/semantic-safe.json` | Should not be blocked. |
| `acl/semantic-delete.json` | Destructive action. |
| `acl/semantic-payment.json` | Financial action. |
| `acl/semantic-send-email.json` | Outbound message. |
| `acl/submit-button.json` | A plain submit control. |

## Rules

**FX1 — Data in `__fixtures__/`, logic in `__helpers__/`.** If it has a
function in it, it belongs in `__helpers__/`. If it's a captured payload, it
belongs here.

**FX2 — There is no working barrel.** `index.ts` re-exports only
`serverHooks`; `screenshots` from `snapshot.ts` is not exported through it, and
no test file imports from `__fixtures__` today. Reference a fixture by its
concrete path (`../__fixtures__/snapshot`) or, for ACL JSON, by name through
`acl-fixture-runner.ts` — do not add a barrel import expecting it to resolve.

**FX3 — Name fixtures by scenario, not by test number.** `basic`,
`semantic-payment`. A fixture called `case-3.test.ts`-style name rots the
moment the test order changes.

**FX4 — ACL fixtures are scored by name.** `acl-fixture-runner.ts` takes a
fixture name as `argv[2]`; the JSON filename *is* the contract. Renaming a
file breaks the runner and any test referencing it.

**FX5 — Keep fixtures small and literal.** They're inlined into a page for
the browser to load; a large binary blob belongs under a dedicated test, not
here.

## Workflows

**Adding an HTML page fixture:** 1. Add a key to `screenshots` in
`snapshot.ts` with the smallest HTML that exercises the behaviour. 2. Import
it directly from the module — `import { screenshots } from '../__fixtures__/snapshot'`
— because the `index.ts` barrel does not re-export it. 3. Point the test at it
with the tool's page/content input.

**Adding an ACL scoring case:** 1. Add `acl/<scenario>.json`. 2. Reference it
by name in `tests/tools/acl-scorer.test.ts`. 3. Check it with
`bun run tests/__helpers__/acl-fixture-runner.ts <scenario>`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the test suite.
- [`../__helpers__/AGENTS.md`](../__helpers__/AGENTS.md) — the behavioural harness.
- [`../../src/tools/acl/AGENTS.md`](../../src/tools/acl/AGENTS.md) — the scorer that consumes `acl/*.json`.
- [`../../AGENTS.md`](../../AGENTS.md) — `apps/server/` MCP server internals.

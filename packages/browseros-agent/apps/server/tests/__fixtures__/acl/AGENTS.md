# `tests/__fixtures__/acl/` — ACL scorer element captures

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/__fixtures__/`.

## What's here

Five JSON files, each a captured browser element (and its page context) used
as an input to the reference ACL scorer in
[`../../../src/tools/acl/`](../../../src/tools/acl/AGENTS.md). They are the
only test data for the removed ACL enforcement feature, kept so the scoring
pipeline stays documented and regression-testable.

## Contents

| File | Scenario |
|---|---|
| `semantic-safe.json` | A benign element — expected not to be blocked. |
| `semantic-delete.json` | A destructive action. |
| `semantic-payment.json` | A financial action. |
| `semantic-send-email.json` | An outbound message action. |
| `submit-button.json` | A plain submit control, matched by exact/fuzzy terms rather than semantics. |

## Rules

**AF1 — Filenames are the contract.** `acl-fixture-runner.ts` takes the
fixture name as its argument, and `tests/tools/acl-scorer.test.ts` references
the files by name. Renaming one breaks both.

**AF2 — These fixtures exercise scoring, not enforcement.** The scorer is
reference-only; nothing in the server runtime calls it. A fixture here is not
evidence that the browser will block a "delete" button.

**AF3 — Keep the JSON shape stable.** The scorer reads specific element
fields (text, selector, description) and a page URL for site filtering. A
field rename in `acl-scorer.ts` must be applied to all five fixtures together.

**AF4 — The semantic path needs the embedding model.** Scoring a fixture
loads `Xenova/bge-small-en-v1.5` via `@huggingface/transformers` on first use
(~33 MB, cached per process). `bun run test:tools:acl` runs
`tests/tools/acl-scorer.test.ts` directly; the standalone runner deliberately
deletes `ACL_EMBEDDING_DISABLE`.

## Workflows

**Scoring one fixture:**
`bun run tests/__helpers__/acl-fixture-runner.ts semantic-payment`
(prints the decision with `LOG_LEVEL=silent`).

**Adding a case:** 1. Capture the element as JSON matching the existing shape.
2. Save it as `acl/<scenario>.json`. 3. Add an expectation to
`tests/tools/acl-scorer.test.ts` (confidence ≥ 0.4 ⇒ blocked).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — fixture conventions.
- [`../../../src/tools/acl/AGENTS.md`](../../../src/tools/acl/AGENTS.md) — the scorer and its weights.
- [`../../tools/AGENTS.md`](../../tools/AGENTS.md) — where `acl-scorer.test.ts` lives.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `apps/server/` MCP server internals.

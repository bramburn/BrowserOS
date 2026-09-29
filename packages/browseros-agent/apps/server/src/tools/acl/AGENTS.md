# `src/tools/acl/` — reference-only ACL scorer

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/tools/`.

## What's here

The old agentic contrastive-learning scorer, kept as **reference
documentation of a removed feature**. `acl/README.md` is explicit: "BrowserOS
no longer exposes ACL rule UI, APIs, shared contracts, or production tool
enforcement. Nothing in the server runtime should import or call the scorer."
`acl-scorer.ts` still runs the full matching pipeline for tests; the other
three files are its dependencies.

## Contents

| File | Purpose |
|---|---|
| `README.md` | Reference doc for the pipeline, weights, and the embedding model. |
| `acl-scorer.ts` | Local ACL types, site-pattern filtering, element feature extraction, weighted scoring, decision. |
| `acl-embeddings.ts` | Lazy-loaded `@huggingface/transformers` ONNX pipeline for semantic similarity. |
| `acl-edit-distance.ts` | Levenshtein edit-distance ratio used for fuzzy term matching. |
| `acl-stopwords.ts` | Static set of 198 English stopwords (NLTK corpus). |

## Rules

**ACL1 — Never import this folder from production server code.** There is no
runtime enforcement. A `grep` for `tools/acl` outside `tests/` is a bug.
`tests/tools/acl-scorer.test.ts` is the only legitimate consumer.

**ACL2 — The scoring model is documented, don't silently change it.** Site
filter → site-only rules → element scoring, where exact = 25%, fuzzy = 25%,
semantic = 50%, and a confidence ≥ **0.4** marks the decision blocked. If you
change a weight, update `README.md` in the same change.

**ACL3 — The semantic signal needs a model download.** It uses
`Xenova/bge-small-en-v1.5` (~33 MB ONNX) via `@huggingface/transformers`,
cached for the process lifetime and overridable with
`ACL_EMBEDDING_MODEL`. Tests that must stay offline set
`ACL_EMBEDDING_DISABLE` — `tests/__helpers__/acl-fixture-runner.ts` does the
opposite and clears it.

**ACL4 — Fixtures for this scorer live in
`tests/__fixtures__/acl/*.json`** (`semantic-safe`, `semantic-delete`,
`semantic-payment`, `semantic-send-email`, `submit-button`). Adding a scoring
case means adding a fixture there, not a bespoke inline object.

## Workflows

**Running the scorer against a fixture:**
`bun run test:tools:acl` (from `apps/server/`) — it runs
`test:cleanup` then `bun test ./tests/tools/acl-scorer.test.ts`.

**Scoring one fixture by hand:**
`bun run tests/__helpers__/acl-fixture-runner.ts <fixture-name>` — forces
`LOG_LEVEL=silent`, deletes `ACL_EMBEDDING_DISABLE`, and loads the embedding
pipeline before scoring.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `tools/` conventions and the registry rule.
- [`../../../../tests/tools/AGENTS.md`](../../../tests/tools/AGENTS.md) — where the scorer test lives.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `apps/server/` MCP server internals.

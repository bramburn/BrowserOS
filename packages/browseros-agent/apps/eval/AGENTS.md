# `apps/eval/` — Eval / benchmark harness

> Sub-package AGENTS file. Eval harness for the MCP tools. For the
> cross-repo architecture see
> [`../../../AGENTS-architecture.md`](../../../AGENTS-architecture.md).
> For the parent monorepo see [`../../AGENTS.md`](../../AGENTS.md).

## What's here

Benchmarks + LLM-judged scoring for the MCP tools. Two layers:
1. **ACL scoring** — agentic contrastive learning scorer, evaluates
 tool-choice vs. ground-truth trajectories. See
 `apps/server/src/tools/acl/`.
2. **Monitoring judge** — at-runtime scoring of agent behavior. See
 `apps/server/src/monitoring/judge/`.

```
apps/eval/
├── README.md
├── package.json, tsconfig.json
├── src/                            ← harness source
├── tests/                          ← harness tests
├── configs/                        ← eval config per tool family
├── data/                           ← reference trajectories
└── scripts/                        ← runner scripts
```

## Opinionated rules

### E1 — Reference trajectories live in `data/`
A reference trajectory is a sequence of MCP tool calls that achieves a
known goal. They're committed to the repo as JSON files and used to
score the LLM's choices.

### E2 — Configs are per-tool-family
`configs/<family>.yaml` declares the tools in scope, the LLM to
evaluate against, and the pass/fail criteria.

### E3 — Reuse the `acl/` scorer
Don't reimplement scoring; import from `apps/server/src/tools/acl/`.
The eval harness is just the runner + visualizer.

## Opinionated workflow

### "Add a new eval scenario"
1. Drop a reference trajectory at `data/<scenario>.json`.
2. Add config at `configs/<scenario>.yaml`.
3. Add the scenario to `src/index.ts` (or `scripts/run.ts`).
4. Run: `bun run src/index.ts --scenario <scenario>`.

### "Add a new scoring criterion"
1. Add the criterion at `apps/server/src/tools/acl/<criterion>.ts`.
2. Register in `apps/server/src/tools/acl/acl-scorer.ts`.
3. The eval harness picks it up automatically.

## Run

```bash
# From packages/browseros-agent
bun run --filter '*eval' start
# or directly
cd apps/eval && bun run src/index.ts --config configs/<family>.yaml
```

## Cross-references

- [`../../AGENTS.md`](../../AGENTS.md) — Bun monorepo parent.
- [`../../../AGENTS-architecture.md`](../../../AGENTS-architecture.md) — overall map.
- [`../server/src/tools/acl/acl-scorer.ts`](../server/src/tools/acl/acl-scorer.ts) — the scorer.
- [`../server/src/monitoring/judge/`](../server/src/monitoring/judge/) — runtime judge.

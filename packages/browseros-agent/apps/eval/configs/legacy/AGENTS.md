# `configs/legacy/` — Legacy eval configs

> Part of [`../AGENTS.md`](../AGENTS.md) in `configs/`; validated by `src/types/config.ts`.

## What's here

Eleven run configs in the older `EvalConfigSchema` format, kept because
existing results and CI jobs reference them. `src/suites/config-adapter.ts`
converts any of them into the suite/variant model before the runner sees it, so
they behave identically to a `configs/suites/*.json` file.

```
configs/legacy/
├── browseros-agent-weekly.json             ← 3 workers, kimi-k2.5 via OpenRouter
├── browseros-oe-agent-weekly.json          ← weekly orchestrator/executor variant
├── browseros-oe-clado-weekly.json          ← weekly Clado visual-action variant
├── browseros-agent-kimi-k2-5-agisdk-real.json
├── browseros-agent-opus-4-6-agisdk-real.json
├── claude-code-agisdk-real.json            ← agent.type "claude-code"
├── agisdk-real.json, agisdk-real-smoke.json
├── infinity-hard-50.json                   ← WebArena-Infinity dataset
├── test-mind2web.json, test-webvoyager.json
```

## Rules

### CLG1 — Read-only in practice; add new work as suites
`EvalSuiteSchema` is the forward format. Extend `configs/suites/`, not this
directory (see rule CFG1 in [`../AGENTS.md`](../AGENTS.md)).

### CLG2 — `snake_case` keys, and a different vocabulary
`num_workers` (not `workers`), `restart_server_per_task` (not
`restartBrowserPerTask`), `timeout_ms` (not `timeoutMs`), and the agent type is
`single` where a suite would say `tool-loop`. The adapter is the only translation
point — do not hand-edit a legacy file into suite syntax.

### CLG3 — `apiKey` is an env var name, never a value
Files like `browseros-agent-weekly.json` carry `"apiKey": "OPENROUTER_API_KEY"`.
`src/utils/config-validator.ts` checks those names against `process.env` and
warns when unset. `claude-code-agisdk-real.json` has no key at all: the Claude
Code agent authenticates through the CLI.

### CLG4 — The agent type determines the adapter branch
`claude-code-agisdk-real.json` uses `type: "claude-code"`, which the adapter
maps to `{ type: 'claude-code' }` and `resolveVariant` short-circuits (no
`EVAL_AGENT_MODEL` required). The orchestrator-executor weekly configs map to
`{ type: 'orchestrated', executorBackend }`, where `clado-action` is what selects
the Clado backend.

### CLG5 — Same port-exclusivity rule as suites
All files share the 9010/9110/9310 base ports and `workers`/`num_workers` fans
out from them. Two legacy configs cannot run concurrently on one host, and
neither can run while a suite is running.

## Workflows

### Running one
```bash
cd apps/eval
bun run eval run --config configs/legacy/browseros-agent-weekly.json
# equivalent legacy form (no subcommand):
bun run eval -c configs/legacy/browseros-agent-weekly.json
```

### Migrating one to a suite
1. Create `configs/suites/<same-name>.json` with `id`, `dataset`,
   `agent.type` (`single` → `single`, or `orchestrated` + `executorBackend`),
   `graders`, `workers`, `restartBrowserPerTask`, `timeoutMs`, `browseros`,
   `captcha`.
2. Drop `apiKey`/`provider`/`model` — the variant supplies them at run time.
3. `bun run eval run --suite configs/suites/<name>.json --variant <id>` and
   compare `summary.json` with a legacy run of the same dataset.
4. Update the CI workflow reference before deleting the legacy file.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — config-format rules.
- [`../../src/suites/AGENTS.md`](../../src/suites/AGENTS.md) — the adapter that reads these.
- [`../../src/types/AGENTS.md`](../../src/types/AGENTS.md) — `EvalConfigSchema`.
- [`../../data/AGENTS.md`](../../data/AGENTS.md) — the datasets these configs point at.
- [`../../.env.example`](../../.env.example) — the key variables referenced here.
- [`../../../AGENTS.md`](../../../AGENTS.md) — Bun monorepo parent.

# `tests/utils/` — Helper tests

> Part of [`../AGENTS.md`](../AGENTS.md) in `tests/`.

## What's here

One file, `resolve-provider-config.test.ts`, covering the two branches of
`resolveProviderConfig`: the `browseros` provider (which requires
`BROWSEROS_CONFIG_URL` and fetches the remote config) and every other provider
(whose `apiKey`, `accessKeyId`, `secretAccessKey`, `sessionToken` and `region`
fields go through `resolveEnvValue`).

```
tests/utils/
└── resolve-provider-config.test.ts   ← browseros vs direct-provider resolution
```

## Rules

### TU1 — Stub the gateway, not the network
The `browseros` branch calls `fetchBrowserOSConfig` /
`getLLMConfigFromProvider` from `@browseros/server/lib/clients/gateway`. Stub
at that boundary (module mock or a fake config object) — never point the test at
a real `BROWSEROS_CONFIG_URL`.

### TU2 — Never use real env var names
Fixtures use obviously fake names (`TEST_API_KEY`) and a literal `env` object
wherever the helper accepts one. `resolveEnvValue` itself reads `process.env`,
so a test for it must set and restore a synthetic variable rather than relying
on a developer's shell.

### TU3 — Assert the failure message
`BROWSEROS_CONFIG_URL environment variable is required for BrowserOS provider`
is user-facing (it surfaces through the CLI's error path). Keep it asserted.

### TU4 — The rest of `src/utils/` is untested by design
`mcp-client`, `with-eval-timeout`, `token-usage`, `sleep` and
`dataset-metadata` have no unit tests here; their behaviour is covered
indirectly through runs and through the graders/metrics that consume their
output. Add a file in this directory if a change to one of them needs a
regression test.

## Workflows

### Adding a provider branch
1. Extend `resolveProviderConfig` in `src/utils/resolve-provider-config.ts`.
2. Add a case to `resolve-provider-config.test.ts` asserting the returned
   `ResolvedProviderConfig` (including `upstreamProvider` where applicable).
3. Confirm `LLM_PROVIDERS` in `@browseros/shared/schemas/llm` lists it — that
   schema gates both the config and the suite variant.

## Cross-references

- [`../../src/utils/AGENTS.md`](../../src/utils/AGENTS.md) — the folder under test.
- [`../../src/suites/AGENTS.md`](../../src/suites/AGENTS.md) — variant resolution, the other provider entry point.
- [`../AGENTS.md`](../AGENTS.md) — test-tree rules.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.

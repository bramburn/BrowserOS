# `src/cli/` — Command-line surface

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/`; argv parsing and dispatch.

## What's here

`index.ts` exports `runCli(argv)` and `usage()`; `src/index.ts` is the `bun`
entry that calls it. `args.ts` turns argv into a discriminated
`EvalCliArgs` union without touching a browser, a file, or the network — which
is why `tests/cli/args.test.ts` can assert on the whole surface. Dispatch is a
plain `switch` over `command`, with each command implemented in
`commands/`.

```
cli/
├── index.ts     ← runCli dispatch + usage text
├── args.ts      ← parseEvalCliArgs, EvalCliArgs union
└── commands/
    ├── suite.ts   ← runSuiteCommand / resolveSuiteCommand (run + optional publish)
    ├── run.ts     ← runRunCommand (suite without publishing)
    ├── grade.ts   ← runGradeCommand (re-score existing artifacts)
    └── publish.ts ← runPublishCommand (R2 upload)
```

## Rules

### CLI1 — Five commands, one of them implicit
`suite`, `run`, `grade`, `publish`, and the implicit `legacy` form
(`bun run eval -c <config.json>`, or bare `bun run eval` for the dashboard).
`args.ts` falls back to `parseLegacyArgs` when the first token is not in
`COMMANDS`, so any typo silently becomes a legacy invocation — check
`COMMANDS` before debugging a "wrong" behaviour.

### CLI2 — `--config` and `--suite` are mutually exclusive
`requireOne()` throws both `suite requires --config or --suite` and
`suite accepts either --config or --suite, not both`. Suite-like commands share
one parser (`parseSuiteLikeArgs`), so flags stay identical between `suite` and
`run`; only `--publish` is accepted on `suite`.

### CLI3 — Parsing is pure and must stay pure
`parseEvalCliArgs` may not read files, spawn processes, or start the
dashboard. Every validation message a user sees about their command line comes
from here.

### CLI4 — Only `r2` is a valid publish target
`publishTarget()` throws `Unsupported publish target: <x>` for anything else,
and `publish` requires both `--run` and `--target`. Adding a second backend
means widening `PublishTarget` and the guard together.

### CLI5 — Errors surface as a non-zero exit
`src/index.ts` catches, prints `error.message`, and exits 1. Messages are
therefore user-facing: keep them actionable and free of stack traces.

## Workflows

### Adding a command
1. Add the literal to `COMMANDS` and a variant to `EvalCliArgs` in `args.ts`.
2. Add `case` in `runCli` (`index.ts`) and a `commands/<name>.ts`.
3. Add the usage line in `usage()`.
4. Cover the parse in `tests/cli/args.test.ts` and the behaviour in
   `tests/cli/suite-command.test.ts`.

### Running a suite without publishing
`bun run eval run --suite <suite.json> --variant <id>` — identical to `suite`
minus `--publish`. Use `run` when you want artifacts on disk only; use `suite
--publish r2` for the full loop, which calls `publishRun` only after
`runEval` returns an `outputDir`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — harness map and config-format rules.
- [`commands/AGENTS.md`](commands/AGENTS.md) — the four command implementations.
- [`../suites/AGENTS.md`](../suites/AGENTS.md) — what `--suite` and `--config` resolve to.
- [`../../tests/cli/AGENTS.md`](../../tests/cli/AGENTS.md) — CLI tests.
- [`../../../AGENTS.md`](../../../AGENTS.md) — Bun monorepo parent.

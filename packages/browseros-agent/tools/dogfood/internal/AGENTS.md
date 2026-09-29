# `tools/dogfood/internal/` — Go internal packages

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/tools/dogfood`.

## What's here

The Go `internal/` boundary for the dogfood module: code that may be imported
by this module and nothing outside it. It currently holds a single package,
`fspath`, with one containment helper. The directory exists so any future
shared helper stays private to this binary.

## Contents

```
tools/dogfood/internal/
└── fspath/
    ├── path.go   ← IsSameOrChild(child, parent string) bool
    └── (no tests)
```

## Rules

**IN1 — `internal/` is not importable from `apps/cli` or `tools/dev`.** Those
are separate Go modules anyway; anything here stays private to
`browseros-dogfood` by the compiler.

**IN2 — Path helpers live here, not in `config/`.** `config` deals with the
user's configured values; `fspath` deals with filesystem containment
semantics. Keep the split.

**IN3 — `IsSameOrChild` cleans both inputs first.** It applies
`filepath.Clean` to `child` and `parent` before `filepath.Rel`, so `~/x/./y`
and trailing slashes can't produce a false negative.

**IN4 — A relative result that starts with `..` is "not contained".** The
helper returns `false` for `..`-prefixed relatives and for the `.` case
handled separately; don't relax it to a string `HasPrefix` check on the raw
paths.

**IN5 — Don't add a package here for a single-use helper.** A function used by
one caller belongs next to that caller; `fspath` earns its place because the
containment check is a safety invariant, reused by `../profile` and its tests.

## Workflows

**Adding a path helper:** 1. New file in `fspath/`. 2. Pure function, no I/O,
no globals. 3. Add a test — the current package has none, so a new helper
should establish the pattern.

**Guarding a destructive copy:** use `IsSameOrChild(dest, source)` and refuse
when it returns `true`, the way `../profile/import.go` does.

**Deciding where a helper goes:** shared across `cmd`/`profile`/`config` →
`internal/fspath` (or a new `internal/<name>`); used once → the caller's
package.

**Debugging a false "inside the source dir" error:** print the `Clean`ed
`child` and `parent`. The usual cause is a symlinked dev profile that
lexically sits under the source directory.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `browseros-dogfood` scope.
- [`fspath/AGENTS.md`](fspath/AGENTS.md) — the helper.
- [`../profile/AGENTS.md`](../profile/AGENTS.md) — the safety-critical caller.
- [`../cmd/AGENTS.md`](../cmd/AGENTS.md) — `refresh-profile`, which triggers the import.

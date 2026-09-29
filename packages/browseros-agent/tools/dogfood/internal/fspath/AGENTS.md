# `tools/dogfood/internal/fspath/` — path containment guard

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/tools/dogfood`.

## What's here

One file, `path.go`, with one function: `IsSameOrChild(child, parent string)
bool`. It answers "is this path the same as, or inside, that path?" after
cleaning both. It is a safety invariant, not a formatting helper — the profile
import uses it to refuse copying a dev profile onto, or inside, the user's
real profile.

## Contents

```
tools/dogfood/internal/fspath/
└── path.go   ← IsSameOrChild(child, parent string) bool
```

## Rules

**FS1 — Both arguments are `filepath.Clean`ed first.** Without that,
`dir/../source` and `dir/` produce wrong answers. Keep the clean.

**FS2 — Equality counts as contained.** `IsSameOrChild(x, x) == true`; the
caller's refusal message ("must not equal or live inside source user-data
dir") relies on this.

**FS3 — Use `filepath.Rel` + `..` prefix test, never string prefixes.**
`/data/foo-bar` must not be treated as inside `/data/foo`. The current form
(`rel != "." && !strings.HasPrefix(rel, "..")`) is the correct one.

**FS4 — No symlink resolution.** This is a lexical check. A symlinked dev
profile can still fool it; that's a known limitation, not something to
silently "fix" with `EvalSymlinks` (which would break on not-yet-created
destinations).

**FS5 — Keep it pure.** No I/O, no globals, no error return — the boolean is
the whole contract.

**FS6 — Argument order is `(child, parent)`.** Swapping them inverts the
check; the call site in `../profile/import.go` is
`IsSameOrChild(cfg.DevUserDataDir, cfg.SourceUserDataDir)`.

## Workflows

**Guarding a new destructive operation:** call
`fspath.IsSameOrChild(destination, protectedPath)` first and abort on `true`
before creating or removing anything.

**Adding a path helper here:** 1. New file in this package. 2. Pure,
lexical, no I/O. 3. Add a test — the package has none today, so start the
file with a table test covering: equal paths, direct child, nested child,
sibling with a shared prefix, and `..` escapes.

**Debugging a refusal:** the import error prints the configured
source/dev user-data dirs. Compare them here with
`filepath.Clean` applied; a trailing separator or a `.` segment is the usual
cause.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `internal/` scope (rules IN1–IN5).
- [`path.go`](path.go) — the only file.
- [`../../profile/AGENTS.md`](../../profile/AGENTS.md) — the safety-critical caller.
- [`../../profile/import.go`](../../profile/import.go) — rule PF2, the refusal.

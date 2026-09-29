# `tools/dogfood/profile/` — profile import and discovery

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/tools/dogfood`.

## What's here

Everything about moving data between the user's real BrowserOS profile and
the dogfood dev profile. `import.go` performs the copy — an explicit
**allowlist** of profile entries (extension state, logins, cookies,
bookmarks, preferences, history, and `IndexedDB/chrome-extension_*`) rather
than a wholesale directory copy, with warnings written to a replaceable
`io.Writer`. `local_state.go` parses the source `Local State` file so
`init` can show the user their real profile names.

## Contents

```
tools/dogfood/profile/
├── import.go        ← ImportConfig, profileAllowlist, profileGlobAllowlist, Import(cfg)
├── local_state.go   ← BrowserProfile, localState, ReadProfiles(userDataDir)
├── import_test.go   ← allowlist + containment tests
└── local_state_test.go
```

## Rules

**PF1 — The copy is allowlist-only.** `profileAllowlist` and
`profileGlobAllowlist` are exhaustive. Caches, GPU caches, and broad site
storage are intentionally *not* copied; adding a broad entry silently
triples the size of a dogfood profile and reintroduces stale state.

**PF2 — Refuse to copy a profile onto itself.** `Import` errors with
`dev user-data dir must not equal or live inside source user-data dir`,
checked with `internal/fspath.IsSameOrChild` before anything is touched.

**PF3 — Never write to the source.** Everything in `Import` writes under
`DevUserDataDir`. The source profile is input only.

**PF4 — `Local State` parse failures are non-fatal.**
`ReadProfiles` returns a single `Default` profile when the file is missing,
unparseable, or has an empty `info_cache` — `init` must still work on a
fresh install.

**PF5 — Profile names come from `info_cache`, falling back to the directory
name.** That's what makes the `init` picker show real names.

**PF6 — Warnings go to `warningOutput`, not `log.Fatal`.** The package must
stay testable; swap `warningOutput` in tests.

**PF7 — `import_test.go` is the guard on the allowlist.** If you add an entry,
add the assertion that proves it is (or isn't) copied.

## Workflows

**Importing a profile:** `browseros-dogfood start --refresh-profile` (or
`browseros-dogfood refresh-profile`) calls
`profile.Import(ImportConfig{SourceUserDataDir, SourceProfileDir,
DevUserDataDir, DevProfileDir})`. If the source is locked, the CLI asks you
to quit BrowserOS first.

**Adding a profile entry to the copy set:** 1. Append to `profileAllowlist`
(or `profileGlobAllowlist` for a glob). 2. Add a case in `import_test.go`. 3.
Re-read `../README.md` § "State And Profile Safety" and update it if the
change affects what users should expect.

**Listing profiles during `init`:** `profile.ReadProfiles(<source
user-data dir>)` returns `[]BrowserProfile{Dir, Name, Email}`; the CLI
prompts with the real names so the user picks the profile with their data.

**Debugging "source profile not found":** the error includes the resolved
`SourceUserDataDir/SourceProfileDir`; check `config.SourceProfilePath()` and
whether the source browser is actually installed at that path.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `browseros-dogfood` scope (rules DF1–DF2).
- [`import.go`](import.go) — the allowlist and copy logic.
- [`../internal/fspath/AGENTS.md`](../internal/fspath/AGENTS.md) — the containment guard.
- [`../cmd/AGENTS.md`](../cmd/AGENTS.md) — `init`, `refresh-profile`, `source-profile`.
- [`../README.md`](../README.md) — § "State And Profile Safety".

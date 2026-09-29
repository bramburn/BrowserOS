# `browsing_data/` — reporting-build guard

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/`) in
> [`packages/browseros/`](../../../../AGENTS.md).

## What's here

The smallest patch in this subtree: a 20-line diff that wraps one free helper
function in `chrome_browsing_data_remover_delegate_unittest.cc` in
`#if BUILDFLAG(ENABLE_REPORTING)`. The function is
`CreateUrlFilterFromOriginFilter()`, which turns an origin filter into a URL
filter for the download-removal tests. Without the guard it is an unused
function in builds where reporting is compiled out.

## Contents

```
browsing_data/
└── chrome_browsing_data_remover_delegate_unittest.cc   ←
    +#if BUILDFLAG(ENABLE_REPORTING)
     base::RepeatingCallback<bool(const GURL&)>
     CreateUrlFilterFromOriginFilter(const base::RepeatingCallback<bool(const url::Origin&)>&)
    +#endif  // BUILDFLAG(ENABLE_REPORTING)
```

## Rules

**BD1 — The guard is `#if BUILDFLAG(ENABLE_REPORTING)`, not `#ifdef`.**
`ENABLE_REPORTING` is a GN buildflag, so it must be tested with
`BUILDFLAG(...)` and the header
`chrome/browser/buildflags.h` must be reachable. A plain `#ifdef` silently
always-true and reintroduces the unused-function warning.

**BD2 — Both ends of the guard must move together.** The patch adds the
`#if` above the function and the matching `#endif  // BUILDFLAG(ENABLE_REPORTING)`
below it. Dropping only one is a syntax error at the next Chromium bump.

**BD3 — This is a test file; nothing here ships.** The only runtime effect is
on test binaries. Do not add behaviour here expecting it to reach users.

**BD4 — Keep the hunk to two lines.** This is a one-function, two-line
context diff against a 800+ line upstream test. Any additional change is a
merge conflict waiting for the next Chromium version bump.

**BD5 — `browseros/` does not add anything to browsing-data clearing.** If a
BrowserOS feature needs to exclude data from "Clear browsing data", that
belongs in `chrome/browsing_data/browsing_data_remover_delegate.cc` upstream
logic or in `../browseros/core/browseros_constants.h`, not in this test.

## Workflows

**Fixing an unused-function warning in a test build**
1. Identify whether the symbol is genuinely guarded upstream by a buildflag.
2. If so, wrap it in `#if BUILDFLAG(<FLAG>)` … `#endif  // BUILDFLAG(<FLAG>)`
   at the smallest possible scope.
3. Ensure the `buildflags.h` include is present.
4. Re-extract; keep the diff to the guard only.

**Checking whether a build variant is affected**
1. Find the `args.gn` that sets `enable_reporting`.
2. Confirm the flag reaches this target's compilation.
3. If the flag is on, the guard is inert and the function compiles as before.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/` overlay.
- [`../../common/AGENTS.md`](../../common/AGENTS.md) — where BrowserOS
  consts and switches are declared.
- [`../../../../build/features.yaml`](../../../../build/features.yaml) —
  patch manifest.

# `build/scripts/` — CI and asset helper scripts

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros/build`.

## What's here

Standalone helpers that are *not* part of the Typer CLI and are not
imported by it. `bump_version.py` is invoked by the nightly CI
workflow to increment the build offset and version before a release
build; `icon_generation/` generates every platform icon from one
source PNG. Both are run as scripts (`python build/scripts/...`), have
their own argparse/CLI surface, and never receive a `Context`.

## Contents

| File | What it does |
|---|---|
| `bump_version.py` | `bump_version(package_root, mode)` — edits `../config/BROWSEROS_BUILD_OFFSET` and `../../resources/BROWSEROS_VERSION`, prints the resulting semantic version. `BUMP_MODES = ("none", "offset-only", "offset+build", "offset+patch")`. Helpers `_read_int_setting`, `_replace_int_setting`, `_semantic_version`, `_bump_version_key`. |
| `bump_version_test.py` | Unit tests for the version arithmetic and the exactly-one-entry assertion. |
| `icon_generation/` | The icon generation system — see its own `AGENTS.md`. |

## Rules

**SCR1 — Scripts here must not import from `../common/` or
`../cli/`.** They run in CI with a bare interpreter and no Typer app.
`bump_version.py` imports only `argparse`, `re`, `sys`, `pathlib` and
`typing`. Keep it that way.

**SCR2 — The version file is `MAJOR=…/MINOR=…/BUILD=…/PATCH=` key-value
lines.** `_read_int_setting()` and `_replace_int_setting()` match them
with anchored regexes and raise if the count is not exactly 1. A
duplicated or renamed key is a hard error, by design — that is the
guard against a silent double-bump.

**SCR3 — `BROWSEROS_BUILD_OFFSET` is a bare integer plus newline.**
`bump_version.py` reads it with `.strip()` and writes `f"{offset + 1}\n"`.
Do not add a comment line or a key to that file.

**SCR4 — Mode semantics are additive, and mutually exclusive for
build/patch.** `none` changes nothing but still prints the version;
`offset-only` increments the offset; `offset+build` also bumps
`BROWSEROS_BUILD` and zeroes `BROWSEROS_PATCH`; `offset+patch` bumps
`BROWSEROS_PATCH`. `--mode` values outside `BUMP_MODES` raise.

**SCR5 — Path discovery is `package_root`, not CWD.** The script takes
the package root and joins `build/config/BROWSEROS_BUILD_OFFSET` and
`resources/BROWSEROS_VERSION`. Do not `os.chdir` to reach them.

**SCR6 — New scripts get a `*_test.py` beside them.** `bump_version_test.py`
is the pattern; the CLI has no test runner of its own.

## Workflows

**Bumping the version for a nightly build**
1. `python build/scripts/bump_version.py --package-root <packages/browseros> --mode offset+patch`
2. It prints the resulting semantic version; that string is what CI
   uses for artifact naming.
3. Commit `../config/BROWSEROS_BUILD_OFFSET` and
   `../../resources/BROWSEROS_VERSION` together — a bump that lands
   half-committed produces two different versions.

**Regenerating icons**
1. Follow `icon_generation/README.md` — replace
   `icon_generation/source/app_icon.png` (≥1024×1024) and run
   `python build/scripts/icon_generation/generate_icons.py`.
2. Output lands in `../../resources/icons/`; `copy_resources.yaml`
   picks it up on the next build.

**Adding a CI helper script**
1. Create `build/scripts/<name>.py` with an `argparse` or `click` entry
   point and a `main()`.
2. Do not register it in `../cli/` — scripts are invoked by path.
3. Add `<name>_test.py` beside it.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — build-system overview.
- [`icon_generation/AGENTS.md`](icon_generation/AGENTS.md) — the icon subsystem.
- [`../config/AGENTS.md`](../config/AGENTS.md) — `BROWSEROS_BUILD_OFFSET`, edited by `bump_version.py`.
- [`../../docs/AGENTS.md`](../docs/AGENTS.md) — the nightly CI workflow that invokes this script.
- [`../../resources/AGENTS.md`](../../resources/AGENTS.md) — `BROWSEROS_VERSION`, the other file this script edits.

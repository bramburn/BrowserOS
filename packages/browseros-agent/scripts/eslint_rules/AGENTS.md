# `scripts/eslint_rules/` — local ESLint plugin

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`.

## What's here

A two-file, self-contained ESLint plugin exposing one rule,
`check-license`. `local-plugin.js` is the plugin entry that registers the
rule; `check-license-rule.js` is the rule itself. It verifies that a source
file has an AGPL license header and, being `fixable: 'code'`, can insert a
correctly dated one at the top of the file.

## Contents

```
scripts/eslint_rules/
├── local-plugin.js         ← default export { rules: { 'check-license': checkLicenseRule } }
└── check-license-rule.js   ← the rule: meta {type 'layout', fixable 'code'}, message 'licenseRule'
```

## Rules

**ER1 — Plain JavaScript, no TypeScript, no build step.** Both files are `.js`
with an ESM `export default`; they are consumed by ESLint as-is.

**ER2 — The expected header is exact.** `@license` / `Copyright <year>
BrowserOS` / `SPDX-License-Identifier: AGPL-3.0-or-later`, wrapped in a block
comment. The year is `new Date().getFullYear()` at fix time, so a newly fixed
file gets the current year.

**ER3 — Shebangs are skipped.** The rule inspects only the first two comments
and explicitly ignores a shebang, so `#!/usr/bin/env bun` scripts can still
carry a license header.

**ER4 — Don't rename the message id.** It is `licenseRule`; ESLint configs and
`--fix` output key off it.

**ER5 — Register the rule in `local-plugin.js`,** not in a config file. A new
rule needs its own file, a `meta` block with `type`, `docs.description`,
`fixable`, `schema`, and `messages`, and an entry in that map.

## Workflows

**Adding a second local rule:** 1. New `<rule-name>-rule.js` beside this one
with the same meta shape. 2. Import it in `local-plugin.js` and add it to the
`rules` map. 3. Reference it as
`local-plugin/<rule-name>` from the ESLint config.

**Fixing files missing a header:** run ESLint with `--fix`; the rule inserts
the header itself, after any shebang.

**Checking why a file fails:** the rule only looks at comments — a file whose
header is missing, malformed, or not a comment (e.g. a string literal) will
report `Add license header.`

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `scripts/` scope (rules SR1–SR6).
- [`local-plugin.js`](local-plugin.js) — plugin registration point.
- [`../../CLAUDE.md`](../../CLAUDE.md) — repo-wide code style.
- [`../../LICENSE`](../../LICENSE) — the AGPL-3.0-or-later text this header names.

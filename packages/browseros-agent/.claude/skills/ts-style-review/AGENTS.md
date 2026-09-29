# `.claude/skills/ts-style-review/` — `/ts-style-review` style gate

> Part of [`../../../AGENTS.md`](../../../AGENTS.md) in `packages/browseros-agent`.

## What's here

A standalone review skill for TypeScript style. `SKILL.md` distils the Google
TypeScript Style Guide into nine rule groups — source-file structure,
variables & literals, classes, functions, control flow & error handling,
naming, type system, schema & type definitions, comments & documentation —
each pointing at a section of the bundled
[`google-ts-styleguide.md`](google-ts-styleguide.md) (28 KB) for the full
rules and examples. It is the only skill in this package that ships a
reference document alongside its entry point.

The ninth group is not from Google: it is a **team convention** (Zod schema
plus `z.infer`, with the schema at the top of the file that uses it unless
shared). The file labels it "(Team Convention)" — keep that distinction.

## Contents

```
ts-style-review/
├── SKILL.md                     ← 3.9 KB: name, description (no argument-hint)
└── google-ts-styleguide.md      ← 28.4 KB: the full guide + examples
```

## Rules

**TS1 — No `argument-hint`.** This is the only skill in `.claude/skills/`
without one — it takes no user input. Don't add `$ARGUMENTS` to it.

**TS2 — Keep detail in the reference file.** The body stays a nine-point
checklist; full rules and before/after examples go in
`google-ts-styleguide.md` and are linked by section number. Pasted-in prose
will rot; the linked sections will not.

**TS3 — Preserve the "(Team Convention)" marker on group 8.** The Zod /
`z.infer` / colocated-schema rules are this repo's addition, not Google's.
Losing the label makes reviewers treat them as upstream style.

**TS4 — This skill and
[`../../../CLAUDE.md`](../../../CLAUDE.md) are not the same authority.**
`CLAUDE.md` is authoritative for the repo: extensionless imports,
kebab-case filenames, no `[prefix]` logger tags, no `index.ts` re-exports.
The Google guide wants named exports only and no default exports — that
overlaps but is not a substitute. When they conflict on a real PR, `CLAUDE.md`
wins; say so rather than silently applying Google style.

**TS5 — Google's "no default exports" and this monorepo's "no `index.ts`
re-exports" are different rules.** Don't merge them.

**TS6 — Group 7 forbids `@ts-ignore` and `@ts-nocheck`.** Strict in this
codebase; a suppression needs an explicit, argued exception.

**TS7 — Group 6 requires naming acronyms as words** (`loadHttpUrl`, not
`loadHTTPURL`) even though elsewhere the guide uses `CONSTANT_CASE` for
constants. Both are intentional; apply each where it belongs.

## Workflows

**Reviewing changed TypeScript**
1. Run `/ts-style-review` (invoked automatically by
   [`dev5-review/`](../dev5-review/AGENTS.md) on TypeScript diffs).
2. Walk the nine groups in `SKILL.md` against the changed files.
3. Open the linked section of `google-ts-styleguide.md` for any rule about to
   be flagged — the checklist is a summary, not the authority.
4. Fold findings into the stage-5 review comments using the
   `[file:line] — severity` format.

**Adding a rule**
1. Add a condensed bullet to the relevant group in `SKILL.md`.
2. Add the full rationale and example to `google-ts-styleguide.md`.
3. Mark anything repo-specific as `(Team Convention)`.

## Cross-references

- [`SKILL.md`](SKILL.md) — the nine-group checklist.
- [`google-ts-styleguide.md`](google-ts-styleguide.md) — the full guide.
- [`../../../AGENTS.md`](../../../AGENTS.md) — Bun monorepo parent.
- [`../AGENTS.md`](../AGENTS.md) — the skills-tree overview.
- [`../dev5-review/AGENTS.md`](../dev5-review/AGENTS.md) — the stage that invokes this skill.
- [`../../commands/AGENTS.md`](../../commands/AGENTS.md) — `/browseros-review`,
  which has its own readability and type-safety checks that overlap here.
- [`../../../CLAUDE.md`](../../../CLAUDE.md) — the authoritative coding rules.

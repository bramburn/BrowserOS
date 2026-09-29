# `packages/shared/src/types/` — hand-written shared types

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/packages/shared`.

## What's here

Three type-only modules with no runtime behaviour and no zod — the plain
TypeScript half of the package. `logger.ts` defines the logger contract the
whole monorepo programs against. `server-config.ts` defines the shape of
`~/.browseros/server.json`, the file the server writes at startup and the CLI
reads for auto-discovery. `role-aware-agents.ts` defines the role-template,
boundary, and create-agent input types that `../constants/role-aware-agents.ts`
populates.

## Contents

```
shared/src/types/
├── logger.ts              ← LogLevel ('debug'|'info'|'warn'|'error'), LoggerInterface
├── server-config.ts       ← ServerDiscoveryConfig { server_port, cdp_port?, url, server_version,
│                             browseros_version?, chromium_version?, browseros_id? }
└── role-aware-agents.ts   ← BrowserOSAgentRoleId, BrowserOSRoleBoundary, BrowserOSRoleTemplate,
                             BrowserOSCustomRoleInput, RoleAwareCreateAgentInput, BrowserOSAgentRoleSummary
```

## Rules

**TY1 — `interface` for object shapes, `type` for unions/aliases.**
`LoggerInterface`, `ServerDiscoveryConfig`, `BrowserOSRoleTemplate` are
interfaces; `LogLevel` and `BrowserOSAgentRoleId` are string-literal unions.

**TY2 — Keep these importable with `import type`.** Nothing here may gain a
runtime value (no enum, no `as const` object); that belongs in
`../constants/`. A runtime export here would defeat type-only importing.

**TY3 — `logger.ts` is a contract, not an implementation.** Server rule S6
forbids `console.log` and requires this interface. If a consumer needs a new
level or an extra method, change the interface *and* every implementation
together, or use the existing `meta?: Record<string, unknown>` parameter
instead.

**TY4 — `server-config.ts` field names are snake_case on purpose.** They map
to the on-disk JSON that the separate Go CLI in `apps/cli` parses. Renaming a
key to camelCase breaks Go-side discovery.

**TY5 — `role-aware-agents.ts` and the constants file are one unit.** The
`BrowserOSRoleTemplate[]` literal in `../constants/role-aware-agents.ts` is
typed against these interfaces, so widening `BrowserOSAgentRoleId` forces the
constant to gain an entry (and vice versa).

**TY6 — No `[prefix]` tags in log messages.** Logger call sites pass a plain
message plus an optional meta object; the `meta` is how context is attached,
not a bracketed prefix (see `CLAUDE.md`).

## Workflows

**Adding a logger capability:** prefer the existing
`debug/info/warn/error(message, meta?)` shape. Only widen
`LoggerInterface` if the capability genuinely cannot be expressed as `meta` —
and then update every implementing class (server and agent) in the same
change.

**Extending server discovery:** add an optional field to
`ServerDiscoveryConfig` and write it in the server's
`lib/browseros-dir.ts`. Required fields are a breaking change for the Go CLI
in `apps/cli`; make new fields optional and document the default.

**Adding a built-in agent role:** 1. Add the id to `BrowserOSAgentRoleId`
here. 2. Add the template (with `bootstrap.agentsMd/soulMd/toolsMd` and
`boundaries`) to `../constants/role-aware-agents.ts`. 3. Confirm the
extension's role picker and the server's create-agent route accept it.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `src/` layout and the exports rule (SS1).
- [`../constants/AGENTS.md`](../constants/AGENTS.md) — the value half of this package.
- [`../../AGENTS.md`](../../AGENTS.md) — `@browseros/shared` package scope.
- [`../../../../apps/server/AGENTS.md`](../../../../apps/server/AGENTS.md) — rule S6, the logger consumer.
- [`../../../../apps/cli/AGENTS.md`](../../../../apps/cli/AGENTS.md) — Go consumer of `server.json`.

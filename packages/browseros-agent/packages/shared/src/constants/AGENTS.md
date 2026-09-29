# `packages/shared/src/constants/` — cross-cutting constants

> Part of [`../../AGENTS.md`](../../AGENTS.md) in
> `packages/browseros-agent/packages/shared`.

## What's here

Eight constant modules that are the single source of truth for every magic
number, URL, path, and exit status in the monorepo. `ports.ts` holds the
production / test / development port triples, `timeouts.ts` every timeout,
`limits.ts` the agent / tool / pagination / CDP / content limits, `urls.ts`
external service endpoints, `paths.ts` file-system and `.browseros` directory
names, `exit-codes.ts` the server's startup status codes, `hermes.ts` the
Hermes container identity and its supported provider list, and
`role-aware-agents.ts` the built-in role template catalog.

## Contents

```
shared/src/constants/
├── ports.ts                 ← DEFAULT_PORTS {cdp 9000, server 9100, extension 9300},
│                              TEST_PORTS 9005/9105/9305, DEV_PORTS 9010/9110/9310,
│                              OAUTH_CALLBACK_PORT 1455, type Ports
├── timeouts.ts              ← TIMEOUTS (tool/MCP/CDP/navigation/OAuth groups),
│                              KLAVIS_PROXY_RETRY_BACKOFF_MS, type TimeoutKey
├── limits.ts                ← AGENT_LIMITS (turns, compaction heuristics), TOOL_LIMITS,
│                              PAGINATION, CDP_LIMITS, CONTENT_LIMITS, AGENT_HARNESS_LIMITS
├── urls.ts                  ← EXTERNAL_URLS (Klavis proxy, PostHog, codegen, OpenAI/GitHub/Qwen OAuth)
├── paths.ts                 ← PATHS (.browseros dir names, browseros.sqlite, sessions, SOUL.md…)
├── exit-codes.ts            ← EXIT_CODES {SUCCESS 0, GENERAL_ERROR 1, PORT_CONFLICT 2, SIGNAL_KILL 3}
├── hermes.ts                ← HERMES_* container identity + HERMES_SUPPORTED_BROWSEROS_PROVIDER_TYPES
└── role-aware-agents.ts     ← BROWSEROS_ROLE_TEMPLATES + getBrowserOSRoleTemplate(id)
```

## Rules

**CN1 — `as const` objects, never TypeScript enums.** Every export here is an
object literal with `as const`, followed by a derived type alias
(`TimeoutKey`, `ExitCode`, `Ports`, `HermesSupportedBrowserosProviderType`).

**CN2 — Ports must stay in sync with Chromium.**
`packages/browseros-agent/packages/shared/src/constants/ports.ts` carries an
explicit "in sync with `chrome/browser/browseros/server/browseros_server_prefs.h`"
comment. Changing a port here is a two-repo change.

**CN3 — Consumers import the file, not the folder.**
`@browseros/shared/constants/ports`, `.../timeouts`, `.../limits`,
`.../urls`, `.../paths`, `.../exit-codes`, `.../hermes`,
`.../constants/role-aware-agents`. A new file needs a new `exports` entry in
`../../../package.json`.

**CN4 — Don't inline app-specific values here.** A value used by one app stays
in that app. Promotion here is a deliberate act, and once promoted, changing
it breaks every caller silently (rule H5 in the package `AGENTS.md`).

**CN5 — `role-aware-agents.ts` carries prompt text.** The `chief-of-staff`
template embeds `AGENTS.md` / `SOUL.md` / tooling markdown as module
constants. Editing a template changes real agent behaviour, not just a label.

**CN6 — `hermes.ts` is a two-sided contract.** Adding a provider to
`HERMES_SUPPORTED_BROWSEROS_PROVIDER_TYPES` without updating the backend
`hermes-provider-map` causes a 400 at agent-create time. Bedrock is
deliberately excluded (multi-env-var).

## Workflows

**Adding a timeout:** 1. Add it to the right group in `TIMEOUTS` (Agent/Tool,
MCP, CDP connection, External API, Navigation/DOM, OAuth) in `timeouts.ts`.
2. Import `@browseros/shared/constants/timeouts` — never re-declare the
number. 3. If it's a new *kind* of value, add a new file and a new `exports`
entry instead of growing this one.

**Changing a port:** 1. Edit `DEFAULT_PORTS` / `TEST_PORTS` / `DEV_PORTS` in
`ports.ts`. 2. Change the matching constant in Chromium's
`browseros_server_prefs.h`. 3. Grep for the literal in
`apps/server`, `apps/agent`, `apps/eval`, and `tools/`.

**Adding a role template:** 1. Add the markdown constants and an entry to
`BROWSEROS_ROLE_TEMPLATES` in `role-aware-agents.ts`. 2. Widen
`BrowserOSAgentRoleId` in `../types/role-aware-agents.ts`. 3. Confirm the
extension's role list and the server's create-agent input accept the new id.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `src/` layout and the exports rule (SS1).
- [`../types/AGENTS.md`](../types/AGENTS.md) — `logger`, `server-config`, role types.
- [`../../AGENTS.md`](../../AGENTS.md) — `@browseros/shared` package scope (H1–H5).
- [`../../../../CLAUDE.md`](../../../../CLAUDE.md) — why these exist (no magic constants).
- [`../../../../apps/server/AGENTS.md`](../../../../apps/server/AGENTS.md) — the main consumer.

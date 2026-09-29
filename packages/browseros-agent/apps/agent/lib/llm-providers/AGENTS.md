# `lib/llm-providers/` — LLM provider configuration

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

The single source of truth for which LLM the user talks to. This is a
**flat** directory of 12 TypeScript files plus one JSON cache and one
`graphql/` subfolder — there are no per-provider subdirectories. It owns
the provider type union, the stored `LlmProviderConfig` array, the
models.dev catalogue used to populate model pickers, the OAuth (device
code) flows for ChatGPT Pro / GitHub Copilot / Qwen Code, the connection
test call, and the one-way backup to BrowserOS prefs and the cloud backend.

## Contents

```
llm-providers/
├── types.ts                      ← ProviderType union (14 members) and
│                                   LlmProviderConfig (api key, baseUrl,
│                                   modelId, contextWindow, temperature,
│                                   Azure/Bedrock/ChatGPT-Pro extras),
│                                   LlmProvidersBackup
├── storage.ts                    ← providersStorage (local:llm-providers,
│                                   version 2 + migration bumping the
│                                   BrowserOS provider's context window),
│                                   defaultProviderIdStorage,
│                                   loadProviders(), createDefaultProvidersConfig(),
│                                   setupLlmProvidersBackupToBrowserOS(),
│                                   setupLlmProvidersSyncToBackend()
├── providerTemplates.ts          ← ProviderTemplate[] for quick setup, enriched
│                                   from models.dev; getProviderTemplate()
├── models-dev.ts                 ← typed accessor over the JSON catalogue
├── models-dev-data.json          ← vendored models.dev snapshot (regenerated
│                                   by `bun run generate:models` in the package)
├── providerIcons.tsx             ← ProviderType → icon map (@lobehub/icons,
│                                   lucide, assets/product_logo.svg);
│                                   BrowserOSIcon, ProviderIcon
├── useLlmProviders.ts            ← the CRUD hook: providers, defaultProviderId,
│                                   selectedProvider, save/setDefault/delete
├── testProvider.ts               ← POST {agentServerUrl}/test-provider, times it
├── useOAuthStatus.ts             ← polls {serverUrl}/oauth/{provider}/status
├── useOAuthProviderFlow.ts       ← orchestrates start/refresh/disconnect,
│                                   toasts, and analytics events
├── client-oauth.ts               ← RFC 8628 device-code flow (requestDeviceCode,
│                                   startTokenPolling) for WAF-blocked providers
├── uploadLlmProvidersToGraphql.ts← one-way sync of non-sensitive config
└── graphql/uploadLlmProviderDocument.ts
                                   ← Create/Update LlmProvider documents
```

## Rules

- **LPV1 — This folder is the only place provider types are declared.**
  `ProviderType` in `types.ts` is imported by `components/chat`, the
  settings routes, and the request builders. Adding a provider means:
  extend the union → add a `providerTemplates.ts` entry → add an icon in
  `providerIcons.tsx`. Nothing else lists providers.
- **LPV2 — Provider config is flat, not one folder per provider.** The parent
  `AGENTS.md` describes a `lib/llm-providers/<name>/{config,client,index}.ts`
  layout; that is **not** what the code does. Keep the flat layout.
- **LPV3 — API keys live in `providersStorage` only** and are stripped before
  the GraphQL upload. `uploadLlmProvidersToGraphql.ts` must never send
  `apiKey`, `accessKeyId`, `secretAccessKey`, or `sessionToken`.
- **LPV4 — Storage changes need a version bump.** `providersStorage` is at
  `version: 2` with a migration map; a shape change means adding version 3
  and a migration function, never mutating the stored shape in place.
- **LPV5 — `models-dev-data.json` is generated.** Refresh it with
  `bun run generate:models` from `packages/browseros-agent`; don't hand-edit.
- **LPV6 — `testProvider` deliberately bypasses the RPC client** and posts to
  `{agentServerUrl}/test-provider` so it exercises the same code path as a
  real chat request. Keep that property; don't "fix" it to use `getClient()`.
- **LPV7 — Analytics events for OAuth are named constants** in
  `../constants/analyticsEvents.ts`; the hook takes them as props
  (`startedEvent`, `completedEvent`, `disconnectedEvent`) rather than
  hard-coding per provider.

## Workflows

**Adding a new API-key LLM provider**
1. Add the `ProviderType` member in `types.ts`.
2. Add a `providerTemplates` entry with `defaultBaseUrl`, `defaultModelId`,
   `supportsImages`, `contextWindow`.
3. Add the icon in `providerIcons.tsx` (`null` means "fall back to
   BrowserOSIcon").
4. Nothing to register — `useLlmProviders()` iterates storage.

**Adding an OAuth (device-code) provider**
1. Add the `ProviderType` member and a template entry.
2. Add a `Feature.<X>_SUPPORT` gate in `lib/browseros/capabilities.ts` with a
   `minServerVersion` so old servers hide the button.
3. Add a `ClientAuthConfig` in `client-oauth.ts` terms
   (`deviceCodeEndpoint`, `tokenEndpoint`, `clientId`, `scopes`,
   `requiresPKCE`, `contentType`).
4. Call `useOAuthProviderFlow({ providerType, displayName, startedEvent,
   completedEvent, disconnectedEvent, clientAuth })` from the settings route.
5. Add the three `*_OAUTH_*_EVENT` constants in `../constants/analyticsEvents.ts`.

**Changing the stored provider shape**
1. Bump `version` and add a migration in `storage.ts`.
2. Update `LlmProviderConfig` in `types.ts`.
3. Re-check `uploadLlmProvidersToGraphql.ts` against the backend input type.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB2, LIB3).
- [`types.ts`](types.ts) — `ProviderType` and `LlmProviderConfig`.
- [`storage.ts`](storage.ts) — the storage wrapper and migrations.
- [`../browseros/AGENTS.md`](../browseros/AGENTS.md) — `BROWSEROS_PREFS.PROVIDERS` backup target.
- [`../../components/chat/AGENTS.md`](../../components/chat/AGENTS.md) — consumes `ProviderType` + icons.
- [`../../AGENTS.md`](../../AGENTS.md) — the (stale) per-provider folder claim this overrides.

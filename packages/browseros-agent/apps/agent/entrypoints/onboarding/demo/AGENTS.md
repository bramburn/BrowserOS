# `entrypoints/onboarding/demo/` — "try it now" demo

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/onboarding`.

## What's here

The end of the onboarding funnel. There is no user-driven app picker: on mount the
page reads `useGetUserMCPIntegrations()` and `onboardingProfileStorage`, derives a
suggestion list, and the user only **picks a suggestion** (or types their own).

The list is built in three steps:
1. `buildPersonalizedSuggestions(connectedApps)` — for each *authenticated*
   integration, take the **first** prompt only (`APP_PROMPTS[name][0]`), first
   occurrence per app name.
2. Append `buildCompanyPrompt(profile?.company)` — a company-scoped news query.
3. If step 1 produced nothing, fall back to `buildDefaultSuggestions(company)`.

Picking a suggestion (or submitting the free-text input) then:
`completeOnboarding()` → `chrome.tabs.create({ active: true })` → wait 500 ms →
`openSidePanelWithSearch('open', { query, mode })`. Skip just records completion and
navigates to `app.html#/home`.

`APP_PROMPTS` is a module-level `Record<string, Omit<DemoSuggestion,'appName'>[]>`
covering 8 apps (Gmail, Google Calendar, Notion, Slack, GitHub, Linear, Jira,
Google Docs) with two prompts each — but only the first of each is ever rendered.

## Contents

```
demo/
└── OnboardingDemo.tsx   ← suggestion list, free-text input, side-panel handoff, skip
```

## Rules

- **DM1 — The handoff is `openSidePanelWithSearch` from
  `@/lib/messaging/sidepanel/openSidepanelWithSearch`,** with a `{ mode }` of
  `'chat' | 'agent'`. Do not open the panel any other way; the background handler
  expects this message shape.
- **DM2 — Completion is recorded before the tab is opened.** The page writes
  `onboardingCompletedStorage` and fires `ONBOARDING_COMPLETED_EVENT`; keep that
  ahead of `chrome.tabs.create` so a failure to open the panel does not leave the
  user stuck in onboarding. It **reads** `onboardingProfileStorage` for
  `profile.company` — it never writes it here.
- **DM3 — `ONBOARDING_DEMO_TRIGGERED_EVENT` fires on a user action,** never on
  mount: suggestion click, custom-query submit, and Skip (which passes
  `{ skipped: true }`). Don't move it into the mount effect.
- **DM4 — Adding an app means adding an `APP_PROMPTS` key** and checking the icon
  mapping renders for it. The record type is
  `Omit<DemoSuggestion, 'appName'>`; `appName` is stamped on by
  `buildPersonalizedSuggestions`, not by a user selection — there is no picker.
- **DM5 — Reuse `McpServerIcon` and `useGetUserMCPIntegrations` from
  `../../app/connect-mcp/`.** They are imported here already; do not fork them into a
  second copy.

## Workflows

**Adding a canned prompt for an existing app**
1. Append an object to that app's array in `APP_PROMPTS`.
2. `label` is the button text; `query` is what is sent (they are often deliberately
   different — a short label, a fuller instruction).
3. Set `mode: 'agent'` for anything that must click around a page.

**Adding an app to the suggestion list**
1. Add the key to `APP_PROMPTS` with at least two prompts; only the **first** is
   ever shown, so put the strongest one first.
2. Confirm the icon resolves (`McpServerIcon` / `@/assets/mcp-icons/`).
3. The app only appears if `useGetUserMCPIntegrations()` reports it with
   `is_authenticated` — verify against real integration data, don't assume.

**Debugging the panel not opening**
1. Confirm the background handler for the open-with-search message is registered
   (`../../background/index.ts`).
2. Confirm the tab id resolves — the handler queries the active tab in the current
   window and no-ops without one.
3. There are two separate 500 ms waits, both load-bearing: this page waits after
   `chrome.tabs.create` so the new tab is the active one, and the background
   handler waits after the panel opens before writing `searchActionsStorage`.
   Removing either loses the prompt.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the onboarding route rules.
- [`../../app/connect-mcp/AGENTS.md`](../../app/connect-mcp/AGENTS.md) — `McpServerIcon` and the integrations hook.
- [`../../background/AGENTS.md`](../../background/AGENTS.md) — the side-panel open handler.
- [`../../../lib/onboarding/onboardingStorage.ts`](../../../lib/onboarding/onboardingStorage.ts) — completion storage.
- [`../../../lib/messaging/sidepanel/openSidepanelWithSearch.ts`](../../../lib/messaging/sidepanel/openSidepanelWithSearch.ts) — the handoff helper.

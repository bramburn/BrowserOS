# `lib/browseros/` — BrowserOS host adapter and feature gates

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

The extension's boundary with the host browser. `BrowserOSAdapter` wraps
the proprietary `chrome.browserOS.*` callback APIs in promises and feature-
detects each method; `capabilities.ts` turns the detected BrowserOS and
server versions into a `Feature` allowlist; `helpers.ts` resolves which
local port the agent/MCP/proxy servers are on; `prefs.ts` is the single
registry of `browseros.*` pref key strings. Every other module that needs
a port, a version, or a browser-only capability goes through here.

## Contents

```
browseros/
├── adapter.ts               ← BrowserOSAdapter singleton: getInteractiveSnapshot,
│                              click, inputText, clear, scrollToNode, sendKeys,
│                              getPageLoadStatus, getAccessibilityTree,
│                              captureScreenshot, getSnapshot, getVersion,
│                              getBrowserosVersion, logMetric,
│                              executeJavaScript, clickCoordinates,
│                              typeAtCoordinates, getPref/setPref/getAllPrefs,
│                              choosePath, isAPIAvailable, getAvailableAPIs;
│                              also re-exports the chrome.browserOS types and
│                              SCREENSHOT_SIZES
├── chrome-browser-os.d.ts   ← hand-written ambient types for the namespace
├── capabilities.ts          ← Feature enum, FEATURE_CONFIG version bounds,
│                              version compare, Capabilities.supports(),
│                              checkFeatureSupport(), resolveStaticFeatureSupport()
│                              + capabilities.test.ts
├── capabilities.test.ts     ← exercises the version gates without the dev short-circuit
├── useCapabilities.ts       ← React hook; pre-resolves every Feature once
├── useBrowserOSProviders.ts ← useAgentServerUrl(): baseUrl/isLoading/error,
│                              gated on capabilities finishing
├── helpers.ts               ← getAgentServerUrl(), getMcpServerUrl(),
│                              getProxyServerUrl(), getHealthCheckUrl(),
│                              getProxyPort(); AgentPortError / McpPortError /
│                              ProxyPortError
├── prefs.ts                 ← BROWSEROS_PREFS key registry (agent/mcp/proxy
│                              port, providers, vertical tabs, install id, …)
└── toggleSidePanel.ts       ← openSidePanel(tabId) / toggleSidePanel(tabId)
                               using chrome.sidePanel.browseros* APIs
```

## Rules

- **BOS1 — Never call `chrome.browserOS.*` outside this folder.** Go through
  `getBrowserOSAdapter()`. The adapter owns the `chrome.runtime.lastError`
  → `Error` conversion that every call site would otherwise duplicate.
- **BOS2 — Every adapter method must feature-detect.** The host may be plain
  Chrome. Use the `typeof chrome.browserOS.X !== 'function'` guard pattern
  already used for `getVersionNumber`, `getBrowserosVersionNumber`,
  `logMetric`, `executeJavaScript`, `clickCoordinates`, `typeAtCoordinates`,
  `getPref`, `setPref`, `getAllPrefs`, `choosePath`.
- **BOS3 — A new gated feature needs three edits:** the `Feature` enum member,
  its `FEATURE_CONFIG` entry (the mapped type enforces this), and a
  `useCapabilities()` check at the call site. Development builds enable
  everything, so the gate is untested locally — read the version bounds.
- **BOS4 — Pref keys are only declared in `prefs.ts`.** No inline
  `'browseros.server.mcp_port'` strings; the registry is the single source.
- **BOS5 — Ports come from `helpers.ts`.** `getAgentServerUrl()` switches
  between the legacy agent port and the unified MCP port based on
  `Feature.UNIFIED_PORT_SUPPORT`; `getMcpServerUrl()`/`getHealthCheckUrl()`
  switch to the proxy port on `Feature.PROXY_SUPPORT`. Never construct a
  `127.0.0.1:<port>` URL elsewhere.
- **BOS6 — `capabilities.ts` pre-initialises on import** and caches the
  promise. `Capabilities.reset()` exists for tests only.

## Workflows

**Adding a version-gated feature**
1. Add the member to `enum Feature` in `capabilities.ts`.
2. Add its `FEATURE_CONFIG` entry with `minBrowserOSVersion` /
   `minServerVersion` / `requiresAlphaFlag`.
3. Consume it with `useCapabilities()` in the component and hide the UI when
   `!supports(Feature.X)`.
4. Add a case to `capabilities.test.ts` for the boundary versions.

**Adding a new browser API wrapper**
1. Declare the signature in `chrome-browser-os.d.ts` (with every overload).
2. Implement it on `BrowserOSAdapter` with the `lastError` rejection and the
   `typeof chrome.browserOS.X !== 'function'` guard.
3. Add `isAPIAvailable('X')` checks where the feature is optional.

**Resolving the server base URL**
1. Call `await getAgentServerUrl()` (or `getMcpServerUrl()` / `getProxyServerUrl()`).
2. For React, prefer `useAgentServerUrl()` from `useBrowserOSProviders.ts`.
3. Handle `AgentPortError` / `McpPortError` / `ProxyPortError` explicitly —
   they mean "BrowserOS isn't running or hasn't told us its port".

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib-level rules (LIB3, LIB6).
- [`../rpc/AGENTS.md`](../rpc/AGENTS.md) — the only consumer of `getAgentServerUrl()`.
- [`../llm-providers/AGENTS.md`](../llm-providers/AGENTS.md) — reads `BROWSEROS_PREFS.PROVIDERS` for the backup.
- [`../../components/sidebar/AGENTS.md`](../../components/sidebar/AGENTS.md) — nav gating on `Feature`.
- [`../constants/productWebHost.ts`](../constants/productWebHost.ts) — a sibling `BROWSEROS_PREFS`-style registry.

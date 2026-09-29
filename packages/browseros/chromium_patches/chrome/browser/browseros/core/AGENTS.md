# `browseros/core/` — shared constants, switches and prefs

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

The single vocabulary every other BrowserOS patch includes. Four headers:
`browseros_constants.h` (bundled-extension IDs, the `chrome://browseros/*`
virtual-route table, config/update URLs, Sentry minidump URL),
`browseros_switches.h` (every `browseros-*` command-line switch),
`browseros_prefs.h`/`.cc` (the `browseros.*` profile prefs plus helper
predicates and two `Sync*` one-shot helpers), and `browseros_action_utils.h`
(which toolbar `actions::ActionId`s count as "BrowserOS actions").
`BUILD.gn` splits them into three targets: `:core` (header-only constants and
switches), `:prefs`, and `:action_utils`.

## Contents

```
core/
├── BUILD.gn                     ← source_set "core" / "prefs" / "action_utils"
├── browseros_constants.h        ← IsBrowserOSExtension(), IsBrowserOSPinnedExtension(),
│                                  IsBrowserOSLabelledExtension(), UsesContextualSidePanelToggle(),
│                                  GetBrowserOSExtensionIds(), kBrowserOSURLRoutes[],
│                                  GetBrowserOSExtensionURL(), GetBrowserOSVirtualURL(),
│                                  kBrowserOSHost = "browseros", kSentryMinidumpUrl
├── browseros_switches.h          ← --disable-browseros-server, --browseros-cdp-port,
│                                   --browseros-proxy-port, --browseros-extensions-url,
│                                   --disable-browseros-extensions, --browseros-sparkle-*, …
├── browseros_prefs.h / .cc       ← browseros::prefs::k* keys (show_llm_chat,
│                                   show_llm_hub, show_toolbar_labels,
│                                   vertical_tabs_enabled, providers,
│                                   custom_providers, default_provider_id,
│                                   ntp_focus_content), RegisterProfilePrefs(),
│                                   SyncVerticalTabsPref(), SyncDefaultTheme()
└── browseros_action_utils.h      ← kBrowserOSNativeActionIds, IsBrowserOSAction(),
                                   GetFeatureForBrowserOSAction()
```

## Rules

**CORE1 — Nothing here may depend on another BrowserOS directory.**
`browseros/core/` is the bottom of the dependency graph inside
`chrome/browser/browseros/`. `browseros_action_utils.h` is the exception and
pulls in `chrome/browser/ui/actions:actions_headers` and
`chrome/browser/ui/side_panel` for `ActionId` / `SidePanelEntryKey`; that is
deliberate, and it is why the `action_utils` target is separate from `core`.

**CORE2 — A new pref goes in `browseros::prefs` as
`inline constexpr char kX[]`, and in three places.** (a) the header, (b) a
`Register*` call in `browseros_prefs.cc`, (c) the `settingsPrivate`
allowlist in `../extensions/api/settings_private/prefs_util.cc` if the WebUI
must read or write it. Missing (c) is the usual bug: the pref registers fine
but `settingsPrivate` rejects it as unlisted.

**CORE3 — `SyncVerticalTabsPref()` and `SyncDefaultTheme()` are one-shot
callers, not observers.** `SyncVerticalTabsPref` mirrors the BrowserOS pref
into the upstream Chrome vertical-tabs pref; `SyncDefaultTheme` applies the
BrowserOS blue tonal-spot theme only when the user has not customised it.
Both are called from elsewhere (`../themes/theme_service.cc` calls
`SyncDefaultTheme` from `ThemeService::Init()`). Do not turn them into
`PrefChangeRegistrar` subscribers without checking the call sites.

**CORE4 — Route lookups are linear scans over a `constexpr` array, and that is
intended.** `kBrowserOSURLRoutes[]` / `kBrowserOSExtensions[]` are compile-time
tables of 2–3 entries; `FindBrowserOSRoute()` and
`FindBrowserOSExtensionInfo()` walk them. Do not introduce dynamic
registration — the point is that the table is `constexpr` and usable from
`browseros_switches.h` consumers with no initialisation.

**CORE5 — `IsURLOverridesDisabled()` is a switch read, not a pref.** It lives
next to the URL table because `GetBrowserOSExtensionURL()` in the same header
gates the virtual-URL machinery on it. As of this snapshot it has no consumer
outside `browseros_constants.h`, so a new caller elsewhere would be the first
— check for a duplicate switch read before adding one. If you add a second
runtime toggle for the virtual scheme, put it in `browseros_switches.h`, not as
a local helper.

**CORE6 — `::` prefixed feature flags in `GetFeatureForBrowserOSAction` are
`chrome/common/chrome_features.h` flags** (`kThirdPartyLlmPanel`,
`kClashOfGpts`). A new native BrowserOS action that is feature-gated needs a
row in that switch *and* an entry in `kBrowserOSNativeActionIds`, or
`IsBrowserOSAction()` will silently return `false` for it.

**CORE7 — Extension IDs are duplicated in exactly one other place.**
`bundled_extensions/BUILD.gn` lists the same three CRX filenames. If an ID
changes, both must change; `IsBrowserOSExtension()` is what the rest of the
codebase trusts, so a stale `BUILD.gn` list shows up as an extension that is
protected but never installed.

## Workflows

**Adding a `browseros.*` profile pref**
1. Add the `inline constexpr char kX[] = "browseros.x";` constant to
   `browseros::prefs` in `browseros_prefs.h` with its type and default in a
   comment (existing entries document `Boolean:` / `JSON string:` inline).
2. Register it in `browseros_prefs.cc:RegisterProfilePrefs` with the right
   `PrefRegistrySyncable` flags (`SYNCABLE_PREF` only if it should sync).
3. Add the matching `(*s_allowlist)[browseros::prefs::kX] = settings_api::PrefType::k…;`
   row in `../extensions/api/settings_private/prefs_util.cc`.
4. Extract all three diffs and list them under the same feature block in
   `../../../../../build/features.yaml`.

**Adding a virtual `chrome://browseros/*` route**
1. Append a `BrowserOSURLRoute` row to `kBrowserOSURLRoutes[]` in
   `browseros_constants.h` (`virtual_path`, `extension_id`, `extension_page`,
   `extension_hash`).
2. The forward and reverse mappings in the same file pick it up; no other
   change is needed for `chrome_content_browser_client.cc`.
3. Re-extract and update the same `features.yaml` block.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `browseros/` root overlay.
- [`../server/AGENTS.md`](../server/AGENTS.md) — consumes the server switches.
- [`../extensions/AGENTS.md`](../extensions/AGENTS.md) — consumes
  `kDisableExtensions` / `kExtensionsUrl` and the extension table.
- [`../../prefs/AGENTS.md`](../../prefs/AGENTS.md) — where
  `RegisterProfilePrefs` is wired in.
- [`../../extensions/api/settings_private/AGENTS.md`](../../extensions/api/settings_private/AGENTS.md)
  — the `settingsPrivate` allowlist.
- [`../../../../../build/features.yaml`](../../../../../build/features.yaml) —
  patch manifest.

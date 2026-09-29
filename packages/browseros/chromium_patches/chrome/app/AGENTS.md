# `chrome/app/` — app identity, command IDs, crash reporting, strings

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/`) in
> [`packages/browseros/`](../../../AGENTS.md).

## What's here

Process-level BrowserOS identity: the macOS bundle plist, the custom
`IDC_*` command IDs, the crash-reporter client (product name per platform
plus a Sentry minidump upload URL), and the GRIT string resources for the
Chat / Council side panels and the Chrome import dialog.

All files are unified diffs against the Chromium originals, except
`browser_os`-style brand-new entries which use `new file mode` hunks.

## Contents

```
app/
├── app-Info.plist                  ← Sparkle keys (SUPublicEDKey, auto-update,
│                                     3600 s check interval) + CrProductDirName=BrowserOS
├── chrome_command_ids.h            ← 4 BrowserOS command IDs in the 403xx range
├── chrome_crash_reporter_client.{cc,h}        ← per-platform product_name, GetUploadUrl
├── chrome_crash_reporter_client_win.{cc,h}   ← Windows: product_name + Sentry URL const
├── generated_resources.grd         ← IDS_THIRD_PARTY_LLM_TITLE, IDS_CLASH_OF_GPTS_*,
│                                     IDS_IMPORT_FROM_CHROME
└── settings_strings.grdp           ← IDS_SETTINGS_IMPORT_EXTENSIONS_CHECKBOX,
                                      IDS_SETTINGS_IMPORT_COOKIES_CHECKBOX
```

## Rules

**AP1 — New command IDs use the 403xx block only.** The four existing IDs
(`IDC_SHOW_THIRD_PARTY_LLM_SIDE_PANEL` 40304,
`IDC_CYCLE_THIRD_PARTY_LLM_PROVIDER` 40305, `IDC_OPEN_CLASH_OF_GPTS` 40306,
`IDC_TOGGLE_BROWSEROS_AGENT` 40307) sit in a reserved range. Do not
allocate near 40000–40100 (upstream commands).

**AP2 — An `IDC_*` constant is dead until it is routed.** Adding it here
also requires a `case` in
[`../browser/ui/browser_command_controller.cc`](../browser/ui/browser_command_controller.cc)
and, for a toolbar/app-menu entry, a registration in
`../browser/ui/browser_actions.cc`.

**AP3 — `GetUploadUrl()` must stay overridden on both clients.** The
cross-platform client returns `browseros::kSentryMinidumpUrl` (declared in
`chrome/browser/browseros/core/browseros_constants.h`); the Windows client
has its own `kSentryMinidumpUrl` constant with an embedded `sentry_key`.
Dropping the override silently reverts crash uploads to Google's endpoint.

**AP4 — Product names are per-platform and set explicitly.** Each branch sets
`BrowserOS_Mac`, `BrowserOS_Linux`, `BrowserOS_ChromeOS`,
`BrowserOS_Linux_ASan`, `BrowserOS_Android`, or `BrowserOS` — a new platform
needs a new branch or it reports as the wrong product.

**AP5 — A new GRIT string needs three edits.** Add the `<message>` to
`generated_resources.grd` or `settings_strings.grdp`, use the generated
`IDS_*` constant from C++, and — for settings-page strings — register it in
`../browser/ui/webui/settings/settings_localized_strings_provider.cc`.

**AP6 — `app-Info.plist` changes are macOS-only.** Sparkle keys are ignored
on Windows/Linux; do not use this file to express cross-platform identity.

## Workflows

**Adding a browser command**
1. Add `#define IDC_<NAME>  403xx` in `chrome_command_ids.h`.
2. Add the `case` in
   `../browser/ui/browser_command_controller.cc`, gated on the relevant
   `base::Feature`.
3. Register the action in `../browser/ui/browser_actions.cc` if it needs a
   toolbar button or app-menu item.
4. Add the keyboard binding in `../browser/ui/accelerator_table.cc` (and
   `../browser/global_keyboard_shortcuts_mac.mm` for macOS Cmd/Opt chords).

**Changing the crash-reporting backend**
1. Update the URL constant in
   `chrome_crash_reporter_client_win.cc` and/or
   `browseros_constants.h` (`kSentryMinidumpUrl`).
2. Confirm `ShouldEnableCrashReporting` still returns `true` on every
   platform branch.

**Changing the macOS bundle identity**
1. Edit `app-Info.plist` (`CrProductDirName`, Sparkle `SU*` keys).
2. Keep `SUPublicEDKey` in sync with the EdDSA key in
   `../install_static/chromium_install_modes.h` and the `enable_sparkle`
   block in `../browser/sparkle_buildflags.gni`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/` root overlay.
- [`../browser/ui/actions/chrome_action_ids`](../browser/ui/actions/AGENTS.md) —
  action IDs (the `actions::` counterpart to these `IDC_*` constants).
- [`../browser/ui/AGENTS.md`](../browser/ui/AGENTS.md) — where commands are
  dispatched and UI is registered.
- [`../../../AGENTS.md`](../../../AGENTS.md) — `packages/browseros/`.
- [`../../../../../AGENTS-build.md`](../../../../../AGENTS-build.md) — build
  pipeline detail.

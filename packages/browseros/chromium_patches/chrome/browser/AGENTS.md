# `chrome/browser/` — browser-process layer

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/`) in
> [`packages/browseros/`](../../../AGENTS.md).

## What's here

The browser-process surface: BrowserOS feature flags, the app-menu/toolbar
action registry, the Browser object (including hidden "agent workspace"
Browsers), first-run behaviour, macOS Sparkle glue, and the virtual
`chrome://browseros/*` URL handling. Roughly a dozen patch files sit
directly in this directory; ~30 more live in subdirectories (see below).

## Contents

```
browser/
├── BUILD.gn                      ← imports sparkle_buildflags.gni, defines
│                                   ENABLE_SPARKLE, deps on //chrome/browser/browseros,
│                                   adds mac/sparkle_glue.* when enable_sparkle
├── buildflags.gni                ← enable_update_notifications = is_chrome_branded ||
│                                   enable_sparkle
├── sparkle_buildflags.gni        ← NEW file: enable_sparkle = is_mac,
│                                   sparkle_framework_path, build_sparkle
├── about_flags.cc                ← registers enable-browseros-alpha-features and
│                                   enable-browseros-keyboard-shortcuts in chrome://flags
├── browser_features.{cc,h}       ← BASE_FEATURE kBrowserOsAlphaFeatures (disabled),
│                                   kBrowserOsKeyboardShortcuts (enabled)
├── flag_descriptions.h           ← name/description strings for both flags
├── browser_commands.cc           ← Copy URL rewrites extension URLs to
│                                   chrome://browseros/* virtual URLs
├── browser_command_controller.cc ← dispatches the 4 BrowserOS IDC_* commands
├── browser_list.{cc,h}           ← ShouldShowBrowserInUserInterface() +
│                                   BrowserList::GetUserVisibleBrowsers()
├── browser_finder.cc             ← hidden Browsers are never a find-any target
├── browser.cc / browser.h        ← CreateParams::hidden, Browser::is_hidden(),
│                                   PinHiddenTabVisibility()
├── browser_unittest.cc           ← IsHiddenReflectsCreateParams test
├── browser_ui_prefs.cc           ← pref defaults: hover-card off, home button on,
│                                   kPinSplitTabButton on
├── chrome_browser_main.cc        ← adds chrome://browseros-welcome first-run tab;
│                                   installs the iCloud Passwords native-messaging
│                                   manifest on EVERY macOS startup
├── chrome_browser_application_mac.mm ← dock-icon variant tinting (dev/alpha/beta)
├── chrome_content_browser_client.cc ← chrome://browseros/* ⇄ chrome-extension://
│                                      forward + reverse URL rewriting
├── global_keyboard_shortcuts_mac.mm  ← Cmd+Shift+K / Cmd+Shift+L / Option+A
├── (subdirs: browseros/, browsing_data/, devtools/, extensions/, importer/,
│  lifetime/, mac/, media/, metrics/, net/, prefs/, profiles/, resources/,
│  sessions/, sync/, themes/, ui/, upgrade_detector/)
```

Note: subdirectories of this tree (`browseros/`, `browsing_data/`, `devtools/`,
`extensions/`, `importer/`, `lifetime/`, `mac/`, `media/`, `metrics/`, `net/`,
`prefs/`, `profiles/`, `resources/`, `sessions/`, `sync/`, `themes/`, `ui/`,
`upgrade_detector/`) are **not** in this unit's list — see their own
`AGENTS.md` if present.

## Rules

**BR1 — A new `chrome://flags` entry needs four edits.** `browser_features.h`
(`BASE_DECLARE_FEATURE`), `browser_features.cc` (`BASE_FEATURE`),
`about_flags.cc` (flag row), and `flag_descriptions.h` (name + description
strings). Missing the description constants is a link error in
`about_flags.cc`.

**BR2 — `enable_sparkle` lives in exactly one place.**
`chrome/browser/sparkle_buildflags.gni` declares it; every consumer must
`import("//chrome/browser/sparkle_buildflags.gni")`. It defaults to
`is_mac`. In this overlay only three files reference it: `chrome/BUILD.gn`
and `chrome/browser/BUILD.gn` (which import it) and `chrome/browser/buildflags.gni`
(which forwards it). `chrome/browser/ui/BUILD.gn` does **not** read it — it
lists the mac-only `webui/help/sparkle_*` sources without a guard — so do not
assume a `ui/` guard exists.

**BR3 — The executable still gets `chrome` for Chromium's own build args,
but the installed binary is `browseros`.** Do not assume a `chrome.exe`
path from anything in this directory; the process name comes from
`../common/chrome_constants.cc`.

**BR4 — Hidden Browsers are agent-owned scratch space and must never appear
in user-facing enumerations.** `Browser::CreateParams::hidden` creates a
window with no taskbar entry, no Alt-Tab, no Mission Control. New
enumeration sites (tab search, window menus, drag/drop, extensions API)
must call `BrowserList::GetUserVisibleBrowsers()` or
`ShouldShowBrowserInUserInterface()` — not `BrowserList::GetInstance()`.

**BR5 — `browser.h` pins renderer visibility for hidden Browsers.**
`PinHiddenTabVisibility()` holds `ScopedClosureRunners` from
`WebContents::IncrementCapturerCount` so pages in a hidden window keep
`Visibility::kVisible`; they are cleared when the Browser is destroyed.
Do not remove the pins without understanding tab teardown ordering.

**BR6 — The iCloud Passwords manifest is installed on every startup, by
design.** `chrome_browser_main.cc` explains why (upgrades skip first run;
it self-heals if the manifest is deleted) and guards the cost with a
`PathExists` check. Do not move it into first-run-only code.

**BR7 — `chrome://browseros/*` is a virtual scheme.** The forward mapping
(`browseros::GetBrowserOSExtensionURL`) lives in
`chrome/browser/browseros/core/browseros_constants.h`; the reverse mapping
(`GetBrowserOSVirtualURL`) is used by `chrome_omnibox_client.cc` and
`browser_commands.cc`. A new virtual route must be added to
`kBrowserOSURLRoutes` there, not hard-coded here.

## Workflows

**Adding a browser-level feature flag**
1. Declare it in `browser_features.h`, define it in `browser_features.cc`
   with an explicit `FEATURE_DISABLED_BY_DEFAULT` / `FEATURE_ENABLED_BY_DEFAULT`.
2. Add the `chrome://flags` row in `about_flags.cc`.
3. Add name + description constants in `flag_descriptions.h`.
4. List all four files under a feature block in
   `packages/browseros/build/features.yaml` (`flags` or
   `chromium-ui-fixes`).

**Creating an offscreen agent Browser**
1. Set `Browser::CreateParams::hidden = true` at the call site.
2. If you added a new native widget path, set `params.headless =
   browser->is_hidden()` in
   `browser/ui/views/frame/browser_native_widget_{ash,aura}.cc` and
   `..._mac.mm`.
3. Ensure any new enumeration excludes it (BR4).

**Changing the first-run surface**
1. Edit the `AddFirstRunTabs({GURL("chrome://browseros-welcome")})` block in
   `chrome_browser_main.cc`.
2. Register the WebUI in
   `browser/ui/webui/chrome_web_ui_configs.cc` and add the constant in
   `../common/webui_url_constants.h`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/` root overlay.
- [`ui/AGENTS.md`](ui/AGENTS.md) — Browser object, commands, actions.
- [`browseros/core/AGENTS.md`](browseros/core/AGENTS.md) — prefs, switches,
  constants, action utils.
- [`mac/AGENTS.md`](mac/AGENTS.md) — Sparkle glue and extra browser parts.
- [`prefs/AGENTS.md`](prefs/AGENTS.md) — syncable pref registration.
- [`../../../AGENTS.md`](../../../AGENTS.md) — `packages/browseros/`.
- [`../../../../../AGENTS-build.md`](../../../../../AGENTS-build.md) — build
  pipeline.

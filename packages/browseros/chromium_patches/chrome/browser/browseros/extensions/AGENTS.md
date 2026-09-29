# `browseros/extensions/` — bundled-extension loader, installer, maintainer

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

The three classes that install and keep the bundled BrowserOS CRX files
(Agent, Bug Reporter, Controller) alive in a profile. `BrowserOSExtensionInstaller`
performs the first install (bundled CRX on disk first, remote config JSON as
fallback), `BrowserOSExtensionLoader` drives the `ExternalProvider`-facing
install flow and can force an immediate bundled/remote install, and
`BrowserOSExtensionMaintainer` runs a periodic 15-minute cycle that fetches the
remote config and repairs the installed set — uninstalling deprecated IDs,
reinstalling missing ones, re-enabling ones the user disabled. All three are
`new file` patches: Chromium has no counterpart.

## Contents

```
extensions/
├── browseros_extension_installer.{h,cc}  ← TryLoadFromBundled() then
│                                            FetchFromRemote(); net:: traffic
│                                            annotation on the remote fetch
├── browseros_extension_loader.{h,cc}     ← SetConfigUrl(), StartLoading(),
│                                            InstallBundledExtensionsNow() /
│                                            InstallRemoteExtensionsNow();
│                                            kImmediateInstallDelay = 2s
├── browseros_extension_maintainer.{h,cc} ← Start(config_url, …),
│                                            kMaintenanceInterval = 15min,
│                                            kInitialMaintenanceDelay = 60s;
│                                            UninstallDeprecated / ReinstallMissing /
│                                            ReenableDisabled / ForceUpdateCheck
└── (no BUILD.gn — see EXT2)
```

## Rules

**EXT1 — Bundled first, remote second, always.** `TryLoadFromBundled()` runs
before `FetchFromRemote()`, and `BrowserOSExtensionLoader::OnStartupComplete`
carries a `from_bundled` flag so the caller knows which path won. Reordering
these makes a first run depend on the network.

**EXT2 — There is no `BUILD.gn` in this directory on purpose.** The six files
are compiled as part of `//chrome/browser/extensions:extensions`, which lists
them by absolute GN path in `../../extensions/BUILD.gn`
(`"//chrome/browser/browseros/extensions/browseros_extension_*.cc"`). Do not add
a local `BUILD.gn`; update that target instead.

**EXT3 — Extension identity and pinning policy live in
`../core/browseros_constants.h`.** `IsBrowserOSExtension()`,
`IsBrowserOSPinnedExtension()` and `kBrowserOSExtensions[]` are the source of
truth. The maintainer's "which IDs should be installed" question is answered
from the remote config, but the "which IDs are protected from uninstall"
question is answered from that table.

**EXT4 — The remote fetch must carry a
`net::NetworkTrafficAnnotationTag`.** Both `browseros_extension_installer.cc` and
`browseros_extension_maintainer.cc` declare a `kTrafficAnnotation`. Adding a
new URLLoader without one is a presubmit failure, not a style nit.

**EXT5 — The maintainer's repair actions are deliberately narrow.** It only
uninstalls IDs that *left* the remote config, reinstalls IDs that are in the
config but missing locally, and re-enables IDs that are in the config but
disabled. It does not add arbitrary IDs and does not upgrade. Extending it to
"install whatever the config says" would let a remote config push code.

**EXT6 — The provider this installs into is
`ManifestLocation::kExternalComponent`, for both bundled and remote.**
`../../extensions/external_provider_impl.cc` constructs the
`ExternalProviderImpl` with `kExternalComponent` as *both* the CRX location and
the download location, and sets `set_auto_acknowledge(true)`,
`set_allow_updates(true)`, `set_install_immediately(true)`. Changing one
location without the other splits the install state.

**EXT7 — `--disable-browseros-extensions` and `--browseros-extensions-url`
gate this stack at the provider level.** They are read in
`../../extensions/external_provider_impl.cc`, not here. Both switches are
declared in `../core/browseros_switches.h`.

## Workflows

**Shipping a new version of a bundled extension**
1. Rebuild the CRX and drop it into the build inputs listed in
   `../bundled_extensions/BUILD.gn`.
2. Bump the version in `bundled_extensions.json` (a build-fetched file, not in
   this repo).
3. If the manifest lacks an `update_url`, add one — otherwise the
   maintainer's `ForceUpdateCheck()` will never fetch a replacement.
4. Do not change anything here; the update flows through
   `../../extensions/extension_management.cc:GetEffectiveUpdateURL()` and
   `../../extensions/updater/extension_updater.cc:InstallPendingNow()`.

**Changing the maintenance cadence**
1. Edit `kMaintenanceInterval` / `kInitialMaintenanceDelay` in
   `browseros_extension_maintainer.cc`.
2. Keep `ScheduleNextMaintenance()` as the only place the timer is re-armed,
   otherwise cycles stop after the first run.

**Adding a new remote-config-driven repair action**
1. Add a private method beside `ExecuteMaintenanceTasks()`.
2. Call it from `ExecuteMaintenanceTasks()` only, not from the config callback,
   so it cannot run on a partial config.
3. Log through the existing `LogExtensionHealth()` path.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `browseros/` root overlay.
- [`../core/AGENTS.md`](../core/AGENTS.md) — extension ID table and switches.
- [`../bundled_extensions/AGENTS.md`](../bundled_extensions/AGENTS.md) — the
  CRX build inputs these classes install.
- [`../../extensions/AGENTS.md`](../../extensions/AGENTS.md) — the
  `ExternalProviderImpl` that hosts this loader.
- [`../../extensions/updater/AGENTS.md`](../../extensions/updater/AGENTS.md) —
  `InstallPendingNow()`.
- [`../../../../../build/features.yaml`](../../../../../build/features.yaml) —
  patch manifest.

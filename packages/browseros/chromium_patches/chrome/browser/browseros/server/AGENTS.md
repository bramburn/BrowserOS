# `browseros/server/` — the bundled `browseros_server` process manager

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

BrowserOS launches a sidecar binary (`resources/bin/browseros_server` inside a
per-profile execution dir) and keeps it alive. `browseros_server_manager.{h,cc}`
is the singleton that owns the whole lifecycle: acquire a `server.lock` in the
execution dir (`server_utils::GetLockFilePath()`), resolve four ports, start
Chromium's CDP WebSocket server, bind a stable MCP proxy port that forwards to
the sidecar's ephemeral backend port, launch the process, poll `/health` every
30s, and restart on failure. Everything it touches is behind an injected
interface (`process_controller.h`, `health_checker.h`, `server_state_store.h`,
`server_updater.h`) so the whole thing is unit-testable — see `test/`.
`browseros_server_updater.{h,cc}` is the Sparkle-style OTA pipeline (appcast XML →
ZIP download → Ed25519 verify → extract to `versions/{version}/` → `--version`
smoke test → hot swap).

## Contents

```
server/
├── BUILD.gn                       ← source_set "server" (explicit sources list),
│                                   "test_support", "unit_tests",
│                                   group("browseros_server_resources"); runs
│                                   validate_resources.py at GN time
├── browseros_server_manager.{h,cc}  ← singleton; kHealthCheckInterval 30s,
│                                      kProcessCheckInterval 5s,
│                                      kStartupGracePeriod 30s,
│                                      kMaxStartupFailures 3, kBackLog 10
├── browseros_server_config.{h,cc}   ← ServerPorts / ServerPaths /
│                                      ServerIdentity / ServerLaunchConfig
├── browseros_server_prefs.{h,cc}    ← port prefs + RegisterLocalStatePrefs()
├── browseros_server_constants.h     ← appcast URLs, Ed25519 public key,
│                                      kUpdateCheckInterval 15min, size/timeout caps
├── browseros_server_proxy.{h,cc}    ← net::HttpServer delegate; IO thread only;
│                                      binds the stable port, forwards to backend
├── browseros_server_updater.{h,cc}  ← appcast → download → verify → extract → test
├── browseros_appcast_parser.{h,cc}  ← minimal libxml appcast reader
├── browseros_server_utils.{h,cc}    ← GetExecutionDir(), FindAvailablePort(),
│                                      IsPortAvailable(), GetLockFilePath(),
│                                      ReadStateFile()/WriteStateFile(),
│                                      ServerState (pid + creation time)
├── process_controller.h / _impl.{h,cc}   ← launch/terminate child process
├── health_checker.h / _impl.{h,cc}      ← GET /health, POST /shutdown
├── server_state_store.h / _impl.{h,cc}  ← pid/creation-time file for orphan recovery
├── server_updater.h                     ← interface for binary-path resolution
├── *_unittest.cc              ← appcast_parser / server_manager / server_utils tests
├── validate_resources.py      ← GN-time check that resources/bin/browseros_server exists
└── test/                      ← gmock fakes (see test/AGENTS.md)
```

## Rules

**SRV1 — New files must be added to `source_set("server")` in `BUILD.gn`.**
The sources list is explicit, not globbed. A file that compiles in isolation
but is not listed is dead code.

**SRV2 — Port defaults are Local State prefs, not build constants.**
`browseros_server_prefs.h` owns `kDefaultCDPPort = 9100`,
`kDefaultProxyPort = 9000`, `kDefaultServerPort = 9200`,
`kDefaultExtensionPort = 9300`. `LoadPortsFromPrefs()` falls back to those when
a stored value is `<= 0`, then `ResolvePortsForStartup()` re-resolves the
ephemeral sidecar ports with `server_utils::FindAvailablePort()` and
`SavePortsToPrefs()` writes them back. A change to port allocation must keep
all four in sync, or the sidecar and its proxy will disagree.

**SRV3 — `kMCPServerPort` is a deprecated mirror and is written on every
save.** `SavePortsToPrefs()` keeps the old `mcp_port` in sync with `server`
"for backward compat", and `LoadPortsFromPrefs()` migrates an old `mcp_port`
into `proxy` when `proxy` is unset. Removing either branch breaks upgrades from
older Local State files. Do not renumber or drop them.

**SRV4 — The proxy is IO-thread-only, and that is load-bearing.**
`browseros_server_proxy.h` documents the contract: the manager takes a
`SharedURLLoaderFactory` on the UI thread, `Clone()`s it into a
`PendingSharedURLFactory`, and calls `Start()` on the IO thread, which binds a
fresh `SharedURLLoaderFactory`. That keeps `net::HttpServer` and
`SimpleURLLoader` on one thread. Any new proxy method must not touch the
loader factory from the UI thread.

**SRV5 — Signature verification and size caps are not optional.**
`browseros_server_updater.cc` verifies the downloaded ZIP against the
Ed25519 key in `kServerUpdatePublicKey` and honours
`kMaxUpdatePackageSize` (200 MB) and `kMaxAppcastSize` (512 KB). Code that
writes to disk from an appcast-provided URL is a security regression; treat any
such change as a stop-and-report.

**SRV6 — Only `kMaxVersionsToKeep = 2` old versions are retained.**
`CleanupOldVersions()` prunes `versions/`. Raising the constant multiplies
disk use per profile; lowering it can delete a version that
`current_version` still points at. Change it deliberately, with the updater's
own unit test in mind.

**SRV7 — The restart loop gives up after `kMaxStartupFailures` (3).**
Between failures there is a 30s `kStartupGracePeriod` in which health is not
judged. Adding a restart path must preserve both, or a bad binary will spin
forever.

**SRV8 — `ServerPaths` is recomputed before every launch.**
`browseros_server_config.h` says so explicitly: "computed fresh before each
launch since the updater can change paths." Do not cache a `ServerPaths` in the
manager across a hot swap.

## Workflows

**Changing a port default or adding a port**
1. Add the constant to `browseros_server_prefs.h` and register it in
   `browseros_server_prefs.cc:RegisterLocalStatePrefs`.
2. Add the field to `ServerPorts` in `browseros_server_config.h` and handle it
   in `ResolvePortsForStartup()` / `SavePortsToPrefs()`.
3. Add the `--browseros-xxx-port` switch to `../core/browseros_switches.h` and
   handle it in `ApplyCommandLineOverrides()`.
4. Extract all touched diffs and update the same `features.yaml` block.

**Adding a probe to the health loop**
1. Extend the interface in `health_checker.h` (or add a new `*_impl` beside the
   existing one) — do not call `net::` directly from the manager.
2. Add the method to the gmock fake in `test/mock_health_checker.h`, or the
   `browseros_server_manager_unittest.cc` DI constructor will not compile.
3. Wire the callback in `CheckServerHealth()` / `OnHealthCheckComplete()`,
   keeping the 30s interval from `kHealthCheckInterval`.

**Adding an OTA step**
1. Add the stage method to `browseros_server_updater.h` in the existing
   callback-chain order (fetch → parse → download → verify → extract → test →
   hot-swap).
2. Every async stage must call `ResetState()` on failure and route errors
   through `OnError(stage, error)`.
3. Add coverage in `browseros_appcast_parser_unittest.cc` or
   `browseros_server_utils_unittest.cc` as appropriate.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `browseros/` root overlay.
- [`test/AGENTS.md`](test/AGENTS.md) — the gmock fakes every new interface needs.
- [`../core/AGENTS.md`](../core/AGENTS.md) — `browseros-*` command-line switches.
- [`../metrics/AGENTS.md`](../metrics/AGENTS.md) — the manager logs events through it.
- [`../../prefs/AGENTS.md`](../../prefs/AGENTS.md) — where
  `browseros_server::RegisterLocalStatePrefs` is wired in.
- [`../../../../../build/features.yaml`](../../../../../build/features.yaml) —
  patch manifest (`server` feature block).

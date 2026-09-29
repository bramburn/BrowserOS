# `browseros/server/test/` — gmock fakes for the server manager

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

The `test_support` source set from `../BUILD.gn`. `BrowserOSServerManager` takes
its collaborators through a DI constructor, and these are the fakes that make
`../browseros_server_manager_unittest.cc` and
`../browseros_appcast_parser_unittest.cc` runnable without a real child process,
a real socket, or a real CDN. `mock_implementations.cc` holds the
out-of-line method bodies the headers do not define, so each mock is a header
plus exactly one `.cc`.

## Contents

```
test/
├── mock_health_checker.h        ← gmock HealthChecker; scriptable CheckHealth /
│                                  RequestShutdown outcomes
├── mock_process_controller.h    ← gmock ProcessController; records Launch()/
│                                  Terminate() calls and canned LaunchResult
├── mock_server_state_store.h    ← gmock ServerStateStore; in-memory ServerState
├── mock_server_updater.h        ← gmock ServerUpdater; canned binary/resources paths
└── mock_implementations.cc      ← the .cc that defines every mock method body
```

## Rules

**SRVT1 — One interface added to the server directory means one mock here.**
`health_checker.h`, `process_controller.h`, `server_state_store.h` and
`server_updater.h` all exist so the manager never depends on a concrete
implementation. If you add a method to one of those interfaces, add the
matching `MOCK_METHOD` here in the same change or
`../browseros_server_manager_unittest.cc` stops compiling.

**SRVT2 — Every mock method body lives in `mock_implementations.cc`.** The
headers are include-only. Adding an inline body to a mock header breaks the
one-definition rule when two test binaries link `test_support`.

**SRVT3 — Register the new mock file in `../BUILD.gn`.** The
`source_set("test_support")` sources list is explicit and enumerates exactly
these five paths.

**SRVT4 — Mocks are gmock, not hand-rolled stubs.** Use
`EXPECT_CALL`/`ON_CALL`; the manager's behaviour under failure (3 startup
failures, grace period, restart-requested pref) is asserted entirely through
these expectations. A fake that silently succeeds destroys the signal the tests
depend on.

**SRVT5 — These mocks must not touch disk, the network, or the real clock.**
`mock_server_state_store.h` is the only place a `ServerState` may be invented;
`mock_process_controller.h` returns a `LaunchResult` struct, it does not spawn
anything. A mock that needs real I/O means the interface boundary is in the
wrong place.

## Workflows

**Adding a test for a new manager behaviour**
1. Add or extend the fake in this directory (SRVT1).
2. Use `BrowserOSServerManager`'s DI constructor — never `GetInstance()` — so
   the test does not depend on Local State or the lock file.
3. Drive the scenario through the existing `SetRunningForTesting()` and
   `OnHealthCheckComplete()` test seams declared in
   `../browseros_server_manager.h`.
4. Run it through the `unit_tests` target in `../BUILD.gn`.

**Adding a brand-new seam to the manager**
1. Declare the interface beside `../health_checker.h` with the same
   "Abstracted to enable unit testing" comment style.
2. Write the `*_impl` next to it.
3. Add a mock here plus a `mock_implementations.cc` body.
4. Add the new DI constructor parameter in `../browseros_server_manager.h` and
   update the existing test call sites.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — server directory, ports, updater, restart loop.
- [`../browseros_server_manager.h`](../browseros_server_manager.h) — the DI
  constructor and the public test seams.
- [`../BUILD.gn`](../BUILD.gn) — `test_support` / `unit_tests` targets.

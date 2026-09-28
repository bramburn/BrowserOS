# Plan — Feature gating + native localhost server

> **Status:** proposal, not implemented. Nothing here has been built or run.
> The Chromium tree is still not on disk (see
> [`docs/WINDOWS_BUILD.md`](../docs/WINDOWS_BUILD.md)), so Chromium-side
> signatures are **unverified at `148.0.7778.97`**. See
> [Verification gaps](#verification-gaps).
>
> **Scope:** Windows. Other OSes deferred until the Windows build is green.
>
> Two deliverables, in order:
> 1. **Part 1** — an opinionated feature-gating architecture over
>    `chrome://flags`, so any feature can be switched on and off.
> 2. **Part 2** — the first gated feature: a native, in-process HTTP server on
>    `127.0.0.1:1337` answering `hello world`, built as the first slice of
>    re-homing the MCP server from an extension into browser code.

## Why these two are one plan

The stated problem with the extension port of the MCP server is that it
**crashes and does not auto-reload**. Both halves of the answer are the same
move:

- A **`chrome://flags` toggle** makes the native path switchable, so a bad
  native build can be turned off in the field without shipping a fix.
- The **native server** is where the MCP port actually lands once it is
  reliable, because in-process code reloads with the browser.

Shipping the server before the gating would mean no way to turn it off.

---

# Part 1 — Feature gating

## What already exists (verified in this repo)

BrowserOS already has working `chrome://flags` integration. The pattern is a
**four-file contract**, and two BrowserOS flags are already registered through
it — `enable-browseros-alpha-features` and
`enable-browseros-keyboard-shortcuts`.

| # | File | Role | Existing BrowserOS content |
|---|---|---|---|
| 1 | `chrome/browser/browser_features.{h,cc}` | declare `BASE_FEATURE(kBrowserOsX, …)` in `namespace features` | `kBrowserOsAlphaFeatures`, `kBrowserOsKeyboardShortcuts` |
| 2 | `chrome/browser/flag_descriptions.{h,cc}` | human-readable name + description constants | `kBrowserOsAlphaFeaturesName` / `…Description` |
| 3 | `chrome/browser/about_flags.cc` | register the entry in `kFeatureEntries[]` | both flags registered |
| 4 | consumer | gate with `base::FeatureList::IsEnabled()` | used across the tree |

The registration shape, verbatim from
`chromium_patches/chrome/browser/about_flags.cc`:

```cpp
    {"enable-browseros-alpha-features",
     flag_descriptions::kBrowserOsAlphaFeaturesName,
     flag_descriptions::kBrowserOsAlphaFeaturesDescription, kOsDesktop,
     FEATURE_VALUE_TYPE(features::kBrowserOsAlphaFeatures)},
```

There is no `edge://flags`. Chromium is Chrome-forked but keeps
`chrome://flags`; the same surface serves both spellings and there is no
reason to introduce a second one.

## The ownership map — read this before you touch anything

This is the part that bites. Each Chromium file may be patched by **at most one**
feature block; a second block claiming the same file makes `git apply` fail and
aborts the entire prep phase.

| Chromium file | Owned by | When adding a flag you must… |
|---|---|---|
| `chrome/browser/browser_features.{h,cc}` | `chromium-ui-fixes` | **fold into that diff** |
| `chrome/browser/about_flags.cc` | `chromium-ui-fixes` | **fold into that diff** |
| `chrome/browser/flag_descriptions.{h,cc}` | `flags` | **fold into that diff** |
| `chrome/browser/ui/ui_features.{h,cc}` | `llm-chat` | prefer `browser_features`; only touch this if the feature is genuinely UI-scoped |
| `chrome/browser/browseros/core/**` (incl. `core/browseros_switches.h`) | `browseros-core` | **fold into that diff** |
| `chrome/browser/browseros/server/**` | `server` | **fold into that diff** — another reason not to extend it |
| `chrome/browser/chrome_browser_main.cc` | `first-run` | **fold into that diff** — do **not** add a second block |
| `chrome/browser/browseros/native_server/**` (new) | *unowned* | claim freely in your new block |

So **every new flag touches three already-owned files.** In practice that means
one new feature block plus three surgical edits to existing diffs. Plan for
that; it is not an oversight.

## Opinionated rules

These are the conventions the architecture *enforces*, not suggestions.

1. **One declaration site.** Feature declarations go in `browser_features.h` in
   `namespace features`, via `BASE_FEATURE`. Do not scatter `base::Feature`
   objects across feature folders. One header to read, one place to grep for
   "what can be switched off".
2. **`kBrowserOs` prefix, always.** `BASE_FEATURE(kBrowserOsNativeServer, …)`.
   The prefix is what makes the flag list greppable and keeps BrowserOS flags
   visually distinct from Chromium's own several hundred.
3. **Flag key is `enable-browseros-<kebab-case>`** and must match the feature
   name lowercased, with the `BrowserOs` prefix dropped:
   `kBrowserOsNativeServer` → `enable-browseros-native-server`. A mismatch
   between key and feature makes the toggle appear to work while gating
   nothing.
4. **Descriptions live in `flag_descriptions.{h,cc}`** as
   `kBrowserOs<CamelCase>Name` / `kBrowserOs<CamelCase>Description`. Never
   inline a string literal in `about_flags.cc`.
5. **`kOsDesktop` for anything shipping to users.** Use a narrower
   `kOsWindows` while a feature is Windows-only experimental, and widen it
   deliberately later.
6. **Default state must be stated in the feature block description.** New
   experimental features default **off**; the flag exists so a developer can
   turn it on. Anything default-on needs a written reason.
7. **A flag that gates a resource (a port, a thread, a file watcher) must have
   an observable off state** — see Part 2's stop path. "Turned off" that
   leaves a thread running is not off.
8. **No new flag for a bug fix.** Flags are for experiment and rollout, not for
   version skew. If a flag is needed to ship a fix, that is a signal the
   feature should not have shipped enabled.

## Adding a flag — checklist

1. Declare in `chrome/browser/browser_features.h`:
   ```cpp
   BASE_FEATURE(kBrowserOsNativeServer, "BrowserOsNativeServer",
                base::FEATURE_DISABLED_BY_DEFAULT);
   ```
2. Add `kBrowserOsNativeServerName` / `…Description` to
   `chrome/browser/flag_descriptions.{h,cc}`.
3. Register in `kFeatureEntries[]` in `chrome/browser/about_flags.cc`.
4. Fold 1–3 into the **existing** diffs owned by `chromium-ui-fixes` and
   `flags`.
5. Gate consumers with
   `base::FeatureList::IsEnabled(features::kBrowserOsNativeServer)`.
6. Add a line to the owning feature block's `description:` in
   `build/features.yaml` so the flag is traceable from the manifest.

## Pitfalls

| Pitfall | Consequence | Avoid by |
|---|---|---|
| Listing an already-patched file in a new block | `git apply` fails; **the whole prep phase aborts** | Check the ownership table above first |
| Feature name and flag key disagree | Toggle exists, gates nothing — silent no-op | Rule 3 |
| Feature read once and cached | Toggle has no effect until restart, looks broken | `FeatureList::IsEnabled` at use time, or observe the feature via `base::FeatureList::AddObserver` |
| Flag enabled but no off path | Cannot be disabled in the field | Rule 7 |
| Editing `about_flags.cc` in two features | Merge conflict every time either changes | Single owner: `chromium-ui-fixes` |

---

# Part 2 — Native server on `127.0.0.1:1337`

## Goal

A native HTTP server inside the browser binary, gated by
`enable-browseros-native-server`, bound to `127.0.0.1:1337`, answering
`hello world` on `GET /`. Exactly one instance across every browser process on
the machine. Survives handler failure. This is the first slice of the MCP port
re-homed from the extension into browser code.

## Placement — a new directory, deliberately

**`chrome/browser/browseros/native_server/`**, not an addition to
`chrome/browser/browseros/server/`.

The existing `server/` module manages a **child process**: it spawns
`browseros_server.exe`, health-checks it over HTTP, auto-restarts it, and runs
an appcast-based updater with signature verification. That is production,
signed, and shipped. The native server is an experiment that may be deleted
wholesale when the extension path is retired. Coupling them would mean the
experiment inherits the sidecar's update and signing machinery, and retiring
it would mean unpicking that later.

Keep the experiment isolated so it can be removed in one `git revert`.

## Reuse what already works

The repo already contains a proven implementation of every hard part. This is
not a from-scratch design.

| Requirement | Already solved by | Reuse as |
|---|---|---|
| HTTP server | `BrowserOSServerProxy` — implements `net::HttpServer::Delegate`, binds a stable port, runs on the IO thread | **Template.** Copy its `Start`/`Stop`/`OnHttpRequest` shape |
| Singleton across processes | `BrowserOSServerManager::AcquireLock()` — `base::File` opened `OPEN_ALWAYS\|READ\|WRITE` against `server.lock` in the exec dir, then `Lock()` | **Pattern**, with the improvement below |
| Crash recovery | `health_check_timer_`, `process_check_timer_`, `consecutive_startup_failures_`, `RestartBrowserOSProcess()` | **Pattern** for the watchdog |
| Orphan recovery | `RecoverFromOrphan()` | **Pattern** |
| Pref-backed config | `LoadPortsFromPrefs()` / `SavePortsToPrefs()` + `browseros_server_prefs.cc` | **Pattern** for the port pref |
| Feature flag | Part 1 | `features::kBrowserOsNativeServer` |

## Singleton — let the port be the lock

The existing lock file works, but for a fixed port there is a simpler and more
honest arbiter: **the bind itself**.

```
browser.exe (instance 1)          browser.exe (instance 2)
  bind 127.0.0.1:1337  ──► OK       bind 127.0.0.1:1337  ──► ERR_ADDRESS_IN_USE
  becomes owner                      writes its PID into server.lock
  writes its PID into server.lock    reports "owned by PID n"
```

The OS guarantees exactly one owner of a TCP port. No lock file is required to
*prevent* a second server — it is required only to answer *who owns it*.

This is better than the lock file alone because lock files are advisory and can
be orphaned by a hard kill, whereas a held socket is released by the kernel the
instant the process dies. Orphan recovery then becomes "try to bind; if it
fails, read the PID from the lock file and probe it".

Rules:
- Bind `127.0.0.1` only, never `0.0.0.0`. Loopback-only is a security control,
  not a default.
- On `ERR_ADDRESS_IN_USE`, **do not retry in a tight loop** — back off, and
  surface it. A second browser that silently fails to serve is a support
  ticket.
- Write `{pid, port, started_at}` into the lock file **after** a successful
  bind, and delete it on clean shutdown.
- If a later browser needs the endpoint rather than owning it, it should proxy
  to the owner. Out of scope for the hello-world slice; noted so the singleton
  decision does not paint us into a corner.

## Crash resilience

In-process, "crash" is not the process dying — it is a bad handler, a leaked
resource, or a wedged thread. Four defences:

1. **Own thread.** Run the server on a dedicated `base::Thread` with its own
   `base::MessagePumpForIO`. Nothing it does can block or kill the UI thread.
2. **No `CHECK()` in a request path.** A malformed request returns `400`; an
   unexpected handler error returns `500`. Neither unwinds into Chromium's
   crash handler, which would take the *browser* down and defeat the entire
   point of moving in-process.
3. **Watchdog with backoff.** A `base::RepeatingTimer` pings the server thread
   and verifies the listener is still bound. On failure it restarts the pump,
   capped by `consecutive_failures_` (mirror the existing manager's field). Give
   up after N and log loudly rather than thrash.
4. **Bound everything.** Max simultaneous connections, max request body, and a
   per-request timeout. An unbounded server in a browser process is a
   memory-growth bug reachable from any web page that can issue a request.

The failure that actually motivated this work — an extension crashing without
reloading — is structurally impossible for the native server: a handler that
misbehaves is contained by (1) and (2), and code changes ship with the browser
binary, so there is nothing left to reload.

## HTTP API

Deliberately tiny for the first slice. This is a liveness harness, not an API.

| Method | Path | Response |
|---|---|---|
| `GET` | `/` | `200` `text/plain` — `hello world` |
| `GET` | `/health` | `200` `application/json` — `{"status":"ok","pid":1234,"port":1337,"uptime_s":42}` |
| anything | anything else | `404` `text/plain` |

`/health` is what the watchdog and any future supervisor will poll, and it
matches the shape the existing sidecar health checker already expects — so
swapping the extension out for this later does not disturb the caller.

**Deliberately not in v1:** the MCP transport itself, auth, CORS, TLS,
persistence. Adding those now means the singleton and lifecycle work — the
parts that are actually novel — are entangled with protocol work that has a
known reference implementation in the extension to copy.

## Security

- Bind `127.0.0.1` explicitly. Never `0.0.0.0`.
- **Any local process can reach this port.** That is a real trust boundary:
  another process on the machine can call the endpoint. For `hello world` the
  blast radius is nil; before exposing MCP, add a token check, and note that
  a token in a browser binary is obfuscation, not authentication.
- Reject requests carrying an `Origin` header. A loopback port with no origin
  check is reachable from a malicious web page via `fetch`, because loopback is
  not a same-origin boundary. This is the single most important control here
  and it is cheap.
- Do not echo request content into responses or logs.

## File changes

**New — BrowserOS code** (as `new file` diffs under `chromium_patches/`):

```
chrome/browser/browseros/native_server/BUILD.gn
chrome/browser/browseros/native_server/browseros_native_server.h
chrome/browser/browseros/native_server/browseros_native_server.cc
chrome/browser/browseros/native_server/browseros_native_server_unittest.cc
```

| File | Responsibility |
|---|---|
| `browseros_native_server.h` | `net::HttpServer::Delegate`; `Start`/`Stop`/`IsRunning`/`GetBoundPort` |
| `…_unittest.cc` | route table, 404 path, response body, no-origin behaviour |

**New — repo artefacts:**

| Path | What |
|---|---|
| `packages/browseros/chromium_patches/chrome/browser/browseros/native_server/*` | the four diffs above |
| `packages/browseros/build/features.yaml` | new `native-server` block |

**Modified — existing diffs (fold in, do not create competing ones):**

| Chromium file | Owned by | Change |
|---|---|---|
| `chrome/browser/browser_features.{h,cc}` | `chromium-ui-fixes` | `BASE_FEATURE(kBrowserOsNativeServer, …)`, disabled by default |
| `chrome/browser/about_flags.cc` | `chromium-ui-fixes` | register `enable-browseros-native-server` |
| `chrome/browser/flag_descriptions.{h,cc}` | `flags` | name + description |
| `chrome/browser/browseros/BUILD.gn` | `browseros-core` | add `//chrome/browser/browseros/native_server` to the `browseros` group |

**Modified — startup hook (ownership confirmed):**

| Chromium file | Owned by | Change |
|---|---|---|
| `chrome/browser/chrome_browser_main.cc` | `first-run` | start the server when the feature is on; stop on shutdown. **Fold into the `first-run` diff** — a second block claiming this file aborts the prep phase. |

If the startup hook belongs in `browseros_server_manager.cc` instead, that file
is unowned today, but coupling the experiment to the sidecar manager is exactly
what [Placement](#placement--a-new-directory-deliberately) argues against.
Prefer a hook that does not drag in the sidecar lifecycle.

### `features.yaml`

```yaml
  native-server:
    description: >
      feat: native in-process server on 127.0.0.1:1337 (gated by
      enable-browseros-native-server, default off). Experiment: first slice of
      re-homing the MCP server from extension to browser code.
    files:
      - chrome/browser/browseros/native_server/
      # browser_features / about_flags / flag_descriptions are owned by
      # chromium-ui-fixes and flags respectively — fold edits into those diffs.
      # Listed here only to document the dependency.
```

### Port constant

`1337` belongs in a constant next to the other port defaults, following
`browseros_server_constants.h`'s style (`inline constexpr int kXxx = …`) — in
`browseros_native_server.h` rather than `browseros_server_constants.h`, for the
isolation reason above. A `--browseros-native-server-port` switch makes the
port overridable without a rebuild; put it in `core/browseros_switches.h`,
which is **owned by `browseros-core`** — fold that edit into that block's
existing diff.

## Implementation order

1. **Part 1 only** — add the `kBrowserOsNativeServer` flag, default off, with
   nothing consuming it. Build. Confirm the toggle appears in `chrome://flags`,
   flips, and survives a relaunch. This isolates gating from server work.
2. Create `native_server/` with `Start`/`Stop`/`OnHttpRequest` and the `/` +
   `/health` routes. No watchdog, no lock file yet.
3. Wire the `BUILD.gn` group into `chrome/browser/browseros/BUILD.gn`.
4. **Build checkpoint.** If `net::HttpServer` does not bind as
   `BrowserOSServerProxy` does, stop — the copied template is wrong and
   everything after is speculative.
5. Add the bind-fails-as-singleton path and the lock-file metadata.
6. Add the dedicated thread + watchdog + backoff.
7. Add the origin rejection and connection/request bounds.
8. Wire startup/shutdown behind the feature flag.
9. Add the unit test and the manual multi-instance test.

Steps 1–4 before 5–8 is the same discipline as the screenshot plan: prove the
riskiest external assumption (does `net::HttpServer` work in-process at this
Chromium version) before layering lifecycle machinery on top.

## Testing

| Level | How |
|---|---|
| Unit | `browseros_native_server_unittest.cc`: `/` returns `hello world`; unknown path returns `404`; a request with `Origin` is rejected |
| Feature flag | `chrome://flags` → toggle `enable-browseros-native-server` → relaunch → confirm present/absent from `netstat -ano` and from `GET /` |
| **Singleton** | Launch two browser instances; confirm exactly one binds. `netstat -ano \| findstr 1337` must show **one** PID, and the second browser's log must say it is not the owner |
| Crash containment | Add a temporary route that throws; confirm the browser stays up and `/health` still answers |
| Watchdog | Kill the server thread via debug break; confirm the timer restarts it and increments `consecutive_failures_` |
| Clean shutdown | Quit the browser; confirm `netstat` shows 1337 free and the lock file is gone |
| Leak | Hammer with 200 concurrent connections; confirm the bound rejects rather than growing unbounded |

The singleton and crash-containment rows are the two that actually prove the
requirements; treat them as release blockers, not nice-to-haves.

## Verification gaps

Honest limits:

- **The Chromium tree is not on disk.** `net::HttpServer`, `base::File::Lock`,
  `base::Thread` + `MessagePumpForIO`, and the exact `ERR_ADDRESS_IN_USE`
  plumbing are all inferred from the working `BrowserOSServerProxy` and
  `BrowserOSServerManager` in this repo, **not read from the tree.** The
  proxy-shaped HTTP parts are the most trustworthy; the watchdog and thread
  ownership are the most speculative.
- Ownership of `chrome_browser_main.cc` was not verified this pass. Check
  before assuming a startup hook is free.
- Port `1337` is taken as given; no conflict check has been run against
  whatever may already listen on it on this machine.
- The exact `BASE_FEATURE` macro form and where the `kFeatureEntries[]` array
  sits in `about_flags.cc` at this version are unconfirmed — the diff context
  shows line ~10898 and an `#endif`, but the surrounding guards are not read.
- Nothing here has been built, run, or measured. `/health` uptime and
  restart behaviour are design targets, not observations.

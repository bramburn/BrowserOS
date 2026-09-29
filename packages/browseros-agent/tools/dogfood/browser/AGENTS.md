# `tools/dogfood/browser/` — BrowserOS launch args and CDP readiness

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/tools/dogfood`.

## What's here

The two helpers that launch the *installed* BrowserOS app against a dogfood
environment. `args.go` builds the command line — most importantly
`--browseros-dock-icon=alpha`, which is what makes a dogfood window visually
distinct from your real browser — plus the dev server/extension port flags and
`--load-extension` pointing at the locally built extension. `cdp.go` polls the
DevTools endpoint until the browser is genuinely up.

## Contents

```
tools/dogfood/browser/
├── args.go        ← ArgsConfig{Binary, AgentRoot, UserDataDir, ProfileDir, Ports, Headless}, BuildArgs
├── args_test.go   ← asserts the produced flag list
└── cdp.go         ← WaitForCDP(ctx, port, maxAttempts) bool
```

## Rules

**BA1 — `--browseros-dock-icon=alpha` is non-negotiable.** It is the only
visual signal that a dogfood window is not your real profile. Losing it risks
acting on the wrong browser.

**BA2 — Server port is passed under all three switch aliases.**
`--browseros-mcp-port`, `--browseros-server-port`, and `--browseros-proxy-port`
are all set to `cfg.Ports.Server`, with an explicit comment: installed
BrowserOS apps have not converged on one switch. Dropping one re-breaks
discovery on older builds.

**BA3 — The bundled server is always disabled.**
`--disable-browseros-server` plus `--disable-browseros-extensions` keep the
*installed* server and extensions out of the way, so only the local build is
in play.

**BA4 — The extension path is derived, not configured:**
`filepath.Join(cfg.AgentRoot, "apps/agent/dist/chrome-mv3-dev")`. That path is
produced by `../pipeline` `Build`; if one changes, change both.

**BA5 — User data and profile are always passed.**
`--user-data-dir` and (when non-empty) `--profile-directory` point at the
*copied* dev profile. There is no mode that launches the real profile.

**BA6 — `chrome://newtab` is the final argument.** The session always opens on
a neutral page; don't add a start URL.

**BA7 — Readiness is a 200 from `/json/version`,** 500 ms apart, 1 s client
timeout, bounded by `maxAttempts` and the context — identical to the sibling
probe in `../../dev/browser/cdp.go`.

## Workflows

**Launching a dogfood browser:** `browser.BuildArgs(ArgsConfig{Binary: cfg.BrowserOSAppPath,
AgentRoot: cfg.AgentRoot(), UserDataDir: cfg.DevUserDataDir, ProfileDir:
cfg.DevProfileDir, Ports: <resolved>, Headless: <flag>})` → `proc.StartManaged`
→ `WaitForCDP`.

**Adding a flag:** append it in `BuildArgs` in `args.go` only, and extend
`args_test.go` in the same change. Flags that identify the dogfood instance
(dock icon, user-data-dir) are the ones that must never be optional.

**Debugging "the wrong browser opened":** confirm
`cfg.DevUserDataDir` resolves under `~/.config/browseros-dogfood/profile` and
not to the source profile. If it does, fix the config, not the flag order.

**Headless runs:** pass `Headless: true`; the flag appended is
`--headless=new`, never the legacy form.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `browseros-dogfood` scope.
- [`args.go`](args.go) — the flag list.
- [`../config/AGENTS.md`](../config/AGENTS.md) — binary path, dev user-data dir, ports.
- [`../pipeline/AGENTS.md`](../pipeline/AGENTS.md) — produces the built extension this loads.
- [`../../dev/browser/AGENTS.md`](../../dev/browser/AGENTS.md) — the sibling package (dev dock icon).

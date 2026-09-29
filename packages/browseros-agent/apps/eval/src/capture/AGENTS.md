# `src/capture/` — Task capture layer

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/`; everything a task writes about itself.

## What's here

`context.ts` (`CaptureContext`) is the façade every agent uses. It owns the
screenshot counter, the message log, the error/warning buffers, the active page
id, and the dashboard event fan-out. `trajectory-saver.ts` owns the on-disk
layout and the resume check. `screenshot.ts` captures PNGs either directly over
CDP or through the MCP `take_screenshot` tool. `message-logger.ts` appends to
`messages.jsonl`. `stream-text-accumulator.ts` parses SSE text into stream
events. `captcha-waiter.ts` blocks while a captcha is being solved.

```
capture/
├── context.ts                ← CaptureContext: the façade passed as AgentContext.capture
├── trajectory-saver.ts       ← TrajectorySaver + hasExistingGraderResults
├── screenshot.ts             ← ScreenshotCapture (CDP first, MCP fallback)
├── message-logger.ts         ← MessageLogger → messages.jsonl
├── stream-text-accumulator.ts← parseSSEEvents, StreamTextAccumulator
├── captcha-waiter.ts         ← CaptchaWaiter (reCAPTCHA / hCaptcha / Turnstile)
├── types.ts                  ← CaptureContextConfig
└── index.ts                  ← barrel re-export
```

## Rules

### CA1 — `TrajectorySaver.init()` wipes the task dir
`init()` does `rm -rf <outputDir>/<query_id>` then recreates it with
`screenshots/`. This is the mechanism that makes re-runs clean, and it is also
why the resume check (`hasExistingGraderResults`) must run *before* `init()` —
`runs/task-run-pipeline.ts` does exactly that. Reordering loses finished work.

### CA2 — Screenshots are numbered, and the number is the join key
`capture(pageId)` increments first, then writes `<n>.png`, and returns `n` even
when the capture failed. A gap in the numbering is normal; a renumbering is
not. Agents stamp that `n` onto the matching `tool-output-*` event so the viewer
can pair transcript and image.

### CA3 — Screenshot failures are warnings, never errors
`capture()` catches everything and returns the index; the CDP path additionally
retries against `listPages()[0]`. The MCP path reads the `image` content block.
Don't let a missing PNG fail a task.

### CA4 — Errors carry an `ErrorSource`
`addError(source, message, details?)` requires one of the
`types/errors.ts` `ErrorSource` values. `with-eval-timeout.ts` uses
`agent_execution`; the pipeline uses `navigation`; graders use `grader`. The
run summary aggregates by source, so new sources must be added to
`ErrorSourceSchema`.

### CA5 — `onEvent` is the dashboard channel and must stay cheap
`createStreamWriter()` parses SSE and re-emits each event through
`onEvent(taskId, event)`, stamping `screenshot` on tool-output events.
`runs/eval-runner.ts` wires that to `dashboardState.broadcastStreamEvent`.
Anything added to the hot path here is paid once per tool call per task.

### CA6 — Captcha handling is CDP-only and best-effort
`CaptchaWaiter` takes a `Browser` (not an MCP URL) and evaluates a detection
script; on any error it returns `detected: false`. It is only constructed when
`config.captcha` is set, and the API key is patched into the browser by
`BrowserOSAppManager.patchNopechaApiKey` before workers launch.

## Workflows

### Resuming an interrupted run
`runs/task-run-pipeline.ts` calls `hasExistingGraderResults(outputDir, queryId)`
first; if `metadata.json` has a non-empty `grader_results` map, the task is
reported from the stored metadata without re-running. To force a re-run, delete
the task dir (or use a fresh `--output`/`output_dir`).

### Re-grading without re-running
`cli/commands/grade.ts` reads `metadata.json` + `messages.jsonl` from every task
dir and re-runs the graders named in the existing `grader_results`, then writes
through `TrajectorySaver.updateGraderResults` (which updates both `grades.json`
and `metadata.json`). It needs no browser unless a grader requires one.

### Debugging a missing screenshot
1. Check `metadata.json` `screenshot_count` vs. the files in `screenshots/`.
2. A lower file count means a capture threw — the warning is in the run log,
   not in the artifact.
3. Screenshot timing is governed by `SCREENSHOT_TIMEOUT_MS` (65s) in
   `constants.ts` and the per-call MCP timeout in `utils/mcp-client.ts`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — harness-wide artifact rules.
- [`../agents/AGENTS.md`](../agents/AGENTS.md) — the callers of `CaptureContext`.
- [`../runs/AGENTS.md`](../runs/AGENTS.md) — where capture is created per task.
- [`../types/AGENTS.md`](../types/AGENTS.md) — `ErrorSource`, `Message` schemas.
- [`../../tests/capture/AGENTS.md`](../../tests/capture/AGENTS.md) — captcha-waiter tests.
- [`../../../AGENTS.md`](../../../AGENTS.md) — Bun monorepo parent.

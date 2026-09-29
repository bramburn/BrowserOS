# `tests/capture/` — Capture-layer tests

> Part of [`../AGENTS.md`](../AGENTS.md) in `tests/`.

## What's here

One test file, `captcha-waiter.test.ts`, covering the only capture helper with
enough branching to be worth testing without a browser: the poll loop that
waits while a reCAPTCHA / hCaptcha / Turnstile widget is still being solved, and
the early exits (no widget, already solved, wait timeout, evaluate throws).

The rest of `src/capture/` — `trajectory-saver.ts`, `message-logger.ts`,
`screenshot.ts`, `context.ts` — has no dedicated unit test. Their behaviour is
exercised indirectly by `tests/grading/*` (artifact layout) and by real runs.

```
tests/capture/
└── captcha-waiter.test.ts   ← detection, solved/unsolved paths, timeout, error
```

## Rules

### TC1 — `CaptchaWaiter` needs a fake `Browser`, not a real one
It only calls `browser.evaluate(pageId, script)`. Supply a stub that returns
`{ value: { type, solved } }` sequences; assert on the returned
`CaptchaWaitResult`, never on wall-clock time — drive the clock through
`waitTimeoutMs` / `pollIntervalMs` constructor values instead.

### TC2 — The error path is a documented outcome
`waitIfCaptchaPresent` catches everything and returns
`{ detected: false, type: 'none', solved: false }`. That is asserted behaviour,
not an untested branch; keep it when refactoring.

### TC3 — Add a capture test when you add a capture rule
The numbering/screenshot-stamp contract (`src/capture/AGENTS.md` CA2) and the
wipe-on-init behaviour (CA1) are cheap to test with a temp dir. If a change
touches either, add a file here rather than leaving it to manual verification.

## Workflows

### Changing captcha detection
1. Edit `DETECTION_SCRIPT` in `src/capture/captcha-waiter.ts`.
2. Update the stubbed evaluate results in `captcha-waiter.test.ts` to cover each
   new type it can return.
3. Check `CaptchaWaitResult['type']` still matches the
   `recaptcha | hcaptcha | turnstile | none` union used in the artifacts.

## Cross-references

- [`../../src/capture/AGENTS.md`](../../src/capture/AGENTS.md) — the folder under test.
- [`../agents/AGENTS.md`](../agents/AGENTS.md) — where the waiter is constructed.
- [`../AGENTS.md`](../AGENTS.md) — test-tree rules.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.

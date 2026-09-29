# `tests/tools/` — MCP tool tests (browser-backed)

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/`.

## What's here

Tests for the 61 MCP tools defined in `../../src/tools/`. Except for
`acl-scorer.test.ts` and the `filesystem/` subfolder, every test here drives a
**real BrowserOS process over CDP** through `withBrowser()` from
`../__helpers__/with-browser.ts`. They call the tool's `handler` via the
`execute()` the helper provides, so the full path — zod-free arg passing,
`ToolResponse` building, post-actions — is exercised against a live browser.

## Contents

| File | Covers |
|---|---|
| `navigation.test.ts` | `get_active_page`, `list_pages`, `navigate_page`, `new_page`, `show_page`, `close_page`, … |
| `navigation-newtab-guard.test.ts` | The new-tab guard behaviour specifically. |
| `observation.test.ts` | `take_snapshot`, `take_enhanced_snapshot`, `get_page_content`, `get_page_links`, `evaluate_script`. |
| `dom.test.ts` | `get_dom`, `search_dom` (also drives `evaluate_script` to set up page state). |
| `input.test.ts` | The 17 input tools. |
| `keyboard.test.ts` | Key event synthesis. |
| `page-actions.test.ts` | `save_pdf`, `save_screenshot`, `download_file`. |
| `windows.test.ts` | `list_windows`, `create_window`, `close_window`, `activate_window`, `set_window_visibility`. |
| `tab-groups.test.ts` | The 5 tab-group tools. |
| `bookmarks.test.ts` | The 6 bookmark tools. |
| `history.test.ts` | The 4 history tools. |
| `response.test.ts` | `ToolResponse` — content, structured data, post-actions, timeouts. **Offline.** |
| `acl-scorer.test.ts` | The reference ACL scorer. **Offline**, but loads the embedding model. |
| `filesystem/` | The agent-loop filesystem toolset. **Offline** — see [`filesystem/AGENTS.md`](filesystem/AGENTS.md). |

## Rules

**TT1 — Go through `withBrowser()`.** Import `withBrowser` and
`WithBrowserContext` from `../__helpers__`; call `execute(tool, args)`. Never
construct a `Browser` or a `CdpBackend` in a tool test — you will leak a
browser process per test file.

**TT2 — Set up page state through tools, not raw CDP.** Use
`navigate_page` / `new_page` / `evaluate_script` from `../../src/tools/` to
reach the precondition, exactly as a real agent would. A test that pokes CDP
directly can pass while the tool is broken.

**TT3 — Tests that write files must clean up.** `dom.test.ts` and
`page-actions.test.ts` create temp files (`tmp-shot-*/`, `tmp-upload-*/` are
in `.gitignore`) and remove them in teardown. Leave no artefacts.

**TT4 — An offline test does not belong in this folder's main body.** The two
exceptions (`response.test.ts`, `acl-scorer.test.ts`) are offline by nature
because they test pure logic. Anything you add that needs no browser is
either of that kind or belongs in `../lib/`.

**TT5 — `bun test` semantics, not Jest.** `describe` / `it` from `bun:test`,
`assert` from `node:assert` (some newer files use `expect`). No `jest.mock`,
no fake timers beyond Bun's.

## Workflows

**Running:** `bun run test:tools` from `apps/server/`. The browser group
spawns BrowserOS itself; `tests/__helpers__/test-env.ts` assigns the ports.

**Targeted runs:** `bun run test:tools:input`,
`bun run test:tools:filesystem`, `bun run test:tools:acl` — each runs
`test:cleanup` first, which is what frees a leftover browser on the test port.

**Adding a test for a new tool:** 1. `tests/tools/<tool>.test.ts`. 2. Import
the tool from `../../src/tools/<tool>` and `withBrowser` from `../__helpers__`.
3. Navigate to a fixture URL, run the tool, assert on the returned `ToolResult`
content. 4. Tear down anything you created.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the test suite and group rules.
- [`../../src/tools/AGENTS.md`](../../src/tools/AGENTS.md) — the tools under test and the registry rule.
- [`../__helpers__/AGENTS.md`](../__helpers__/AGENTS.md) — the browser harness.
- [`./filesystem/AGENTS.md`](filesystem/AGENTS.md) — the offline subfolder.

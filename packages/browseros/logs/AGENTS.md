# `packages/browseros/logs/` — generated build logs (gitignored)

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

Build log files, one per CLI invocation. This directory is **not
source** — it is created on demand, is covered by the `**/logs` rule in
the repo root `.gitignore`, and is not tracked. As of this writing it
holds 9 files from a single 2026-09-19 session on this Windows host
(`build_2026-09-19_15-19-02.log` through
`build_2026-09-19_16-26-44.log`); the directory did not exist in a
clean checkout.

Nothing here is written by hand. There is nothing to edit, nothing to
review in a diff, and nothing to preserve.

## Contents

```
logs/
└── build_<YYYY-MM-DD_HH-MM-SS>.log   ← one per run
```

Format: a header line `BrowserOS Build Log - Started at <timestamp>`,
a rule of 80 `=`, then one line per log call:

```
[2026-09-19 15:28:33] INFO: 🚀 BrowserOS Build System
[2026-09-19 15:28:33] ERROR: DIRECT MODE: chromium_src does not exist: ...
```

## Rules

**LOG1 — Never edit or commit a file here.** The directory is matched
by `**/logs` in the root `.gitignore`. `git add` on these paths should
require `-f`, and doing so is a mistake.

**LOG2 — One file per process, never reused.** The name is
`build_%Y-%m-%d_%H-%M-%S.log` from
`../../build/common/logger.py::_ensure_log_file()`. The handle is a
module-level `_log_file` created on first log call and closed by
`close_log_file()`. A second run in the same second would truncate the
first; that is accepted, not a bug to fix here.

**LOG3 — Console and file output are the same events.**
`log_info` / `log_warning` / `log_error` / `log_success` each `typer.echo`
(or `secho`) *and* append to the file. `log_debug` only writes when
`enabled=True`. There is no separate verbose log level.

**LOG4 — The directory path is `get_package_root() / "logs"`,** i.e.
always `packages/browseros/logs/`, resolved by walking up for
`pyproject.toml` — never CWD. Moving this directory or symlinking it
breaks the resolver.

**LOG5 — Truncation is a feature.** `_ensure_log_file()` opens with
mode `"w"`. A log is the record of one run, not an accumulating journal.

## Workflows

**Finding the log for a failed build**
1. `ls -t logs/ | head` (or sort by LastWriteTime in the Explorer).
2. Read the newest `build_*.log`; the header carries the start time.
3. Cross-check against the CI run's own log — this file captures the
   Python layer only, not autoninja or `gn` output unless it was
   invoked through `run_command()`.

**Diagnosing a DIRECT-mode failure**
1. The first meaningful line is often the mode banner. "DIRECT MODE:
   chromium_src does not exist: <path>" means no `--config` was passed
   and `CHROMIUM_SRC` / `--chromium-src` did not resolve.
2. In CONFIG mode the equivalent failure names the missing key from the
   YAML.

**Cleaning up**
1. Delete the whole directory; it is recreated on the next log call.
2. Do not add a `.gitkeep` — an empty tracked directory here would be
   the only thing that could accidentally pin log files into git.

**Adding a new log sink**
1. Extend `../../build/common/logger.py`, not the modules.
2. Modules must keep calling `log_info` / `log_warning` / `log_error` /
   `log_success` from `../../build/common/utils.py` re-exports; never
   call `typer.echo` directly for something that should be recorded.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — package-level view.
- [`../build/common/AGENTS.md`](../build/common/AGENTS.md) — `logger.py`, the writer.
- [`../build/common/utils.py`](../build/common/utils.py) — the `log_*` helpers modules import.
- [`../build/docs/AGENTS.md`](../build/docs/AGENTS.md) — CI operations docs.

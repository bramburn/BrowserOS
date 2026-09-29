# `build/modules/release/` — post-build release automation

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`packages/browseros/build/modules/`](../AGENTS.md).

## What's here

The `browseros release` sub-command's five steps. Unlike the build
pipeline, these run *after* artifacts already exist: they read the
release metadata from R2, list what is published, render Sparkle
appcast snippets, create a GitHub release from CDN artifacts, copy
versioned files to the stable `download/` URLs, and pull artifacts back
down. `common.py` holds the shared R2 reads, platform naming and
release-note rendering.

## Contents

| File | What it does |
|---|---|
| `common.py` | `PLATFORMS = ["macos", "win", "linux"]`, `PLATFORM_DISPLAY_NAMES`, `DOWNLOAD_PATH_MAPPING`, `fetch_all_release_metadata()`, `list_all_versions()`, `format_size()`, `generate_appcast_item()`, `generate_release_notes()`, `get_repo_from_git()`, `check_gh_cli()`. |
| `list.py` | `class ListModule` — list release artifacts from R2. |
| `appcast.py` | `class AppcastModule` — generate Sparkle appcast XML snippets. |
| `github.py` | `create_github_release()`, `download_file()`, `upload_to_github_release()`, `normalize_version()`, `download_and_upload_artifacts()` + `class GithubModule`. |
| `publish.py` | `copy_to_download_path()` + `class PublishModule` — copy versioned artifacts to the stable `download/` URLs. |
| `download.py` | `class DownloadModule` — download release artifacts from the CDN. |
| `__init__.py` | Re-exports all of the above plus `AVAILABLE_MODULES = {list, appcast, github, publish, download}`. |

All five module classes use `produces = []` and `requires = []`.

## Rules

**REL1 — Platform keys are exactly `macos`, `win`, `linux`.** Note
`win`, not `windows`. `PLATFORMS` and `DOWNLOAD_PATH_MAPPING` are keyed
on these strings; `get_platform()` in `../../common/utils.py` returns
`windows`, so there is a translation somewhere — preserve it rather
than "fixing" one side to match the other.

**REL2 — `DOWNLOAD_PATH_MAPPING` is the stable-URL contract.** Adding a
platform or an artifact variant means adding an entry here, or
`publish` will silently have nowhere to copy it.

**REL3 — `gh` must be installed for the `github` step.**
`check_gh_cli()` gates it. Do not shell out to `gh` from `common.py`
without that check.

**REL4 — `get_repo_from_git()` derives the repo from the working
tree.** Do not hardcode an `owner/repo` string; forks and CI checkouts
must resolve correctly.

**REL5 — `list_all_versions()` merges R2 keys across platforms.** A
version present on only some platforms still appears. Consumers must
not assume a version is complete.

**REL6 — These modules read R2 through `../storage/`.** Use
`get_release_json()` and `get_r2_client()` from `../storage/r2.py`;
do not create a second boto3 client here.

**REL7 — Publishing is a separate step from release creation.**
`PublishModule` copies into `download/`; `GithubModule` creates the
GitHub release. Running one does not run the other.

## Workflows

**Listing what is live**
1. `browseros release --list` (optionally `--version <ver>`).
2. Requires R2 credentials (`ctx.env.has_r2_config()`).

**Creating a GitHub release from published artifacts**
1. `browseros release github create --version <ver>`.
2. It downloads from the CDN (`download_and_upload_artifacts()`),
   normalises the version, and calls `gh release create`.
3. Verify the release page; R2 objects are unchanged by this step.

**Pointing the stable download URLs at a new version**
1. `browseros release --publish --version <ver>`.
2. `copy_to_download_path()` uses `DOWNLOAD_PATH_MAPPING`; a missing
   mapping entry is a silent no-op, so check the output.

**Rendering release notes / an appcast snippet**
1. `browseros release --appcast --version <ver>`.
2. `generate_release_notes()` builds the body from the per-platform
   metadata; `generate_appcast_item()` renders one `<item>`.

**Pulling a published artifact back for inspection**
1. `browseros release --download --version <ver> --os <macos|windows|linux>`.
2. `DownloadModule` maps those names through its own `OS_NAME_MAP` to the
   `macos` / `win` / `linux` keys in rule REL1.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — all 14 module areas.
- [`../storage/AGENTS.md`](../storage/AGENTS.md) — the R2 client this area reads through.
- [`../../cli/release.py`](../../cli/release.py) — the `browseros release` commands.
- [`../ota/AGENTS.md`](../ota/AGENTS.md) — the Server OTA feed, a separate appcast.
- [`../sign/sparkle.py`](../sign/sparkle.py) — signs the DMGs these modules publish.

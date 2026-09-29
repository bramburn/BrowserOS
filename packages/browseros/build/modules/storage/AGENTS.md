# `build/modules/storage/` — Cloudflare R2 upload and download

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`packages/browseros/build/modules/`](../AGENTS.md).

## What's here

The R2 (S3-compatible) side of the build. `r2.py` is the shared client
layer; `download.py` pulls the BrowserOS Server resource bundles from
R2 before a build; `upload.py` publishes finished artifacts and
maintains the `release.json` index. Two unit-test files cover the
metadata merge and the artifact-zip extraction.

Config is declarative: `../../config/download_resources.yaml` decides
what gets fetched, `../../common/env.py::EnvConfig` supplies the
credentials.

## Contents

| File | What it does |
|---|---|
| `r2.py` | `BOTO3_AVAILABLE`, `get_r2_client()`, `upload_file_to_r2()`, `download_file_from_r2()`, `download_from_r2()`, `get_release_json()`. |
| `download.py` | `ARTIFACT_ZIP_DOWNLOAD = "artifact_zip"`, `ARTIFACT_METADATA_NAME = "artifact-metadata.json"`, `COPY_CHUNK_SIZE`, `DownloadResourcesModule` (`description = "Download resources from Cloudflare R2"`), plus `extract_artifact_zip()` and its private helpers (`_read_artifact_metadata_bytes`, `_parse_artifact_metadata`, `_extract_artifact_files`, `_parse_artifact_entry`, `_normalize_artifact_path`, `_restore_zip_file_mode`, `_clear_destination`). |
| `upload.py` | `UploadModule` (`description = "Upload build artifacts to Cloudflare R2"`), `detect_artifacts(ctx)`, `upload_release_artifacts()`, `generate_release_json()`, `merge_release_metadata()`, `_get_artifact_key()`, `_get_linux_artifact_key()`, `_get_platform()`. |
| `download_test.py` | Unit tests for `extract_artifact_zip()`. |
| `upload_test.py` | Unit tests for the release-metadata merge. |
| `__init__.py` | Re-exports the R2 utilities and both module classes. |

Both modules use `produces = []` and `requires = []`.

## Rules

**STO1 — `boto3` is an optional import.** `r2.py` wraps it in
`try/except ImportError` and sets `BOTO3_AVAILABLE = False`.
`get_r2_client()` returns `None` (and logs "run: pip install boto3")
rather than raising. Callers must handle `None`; do not remove the
guard and make boto3 a hard dependency.

**STO2 — Credentials come from `EnvConfig` only.**
`get_r2_client(env)` uses `env.r2_endpoint_url`, `env.r2_access_key_id`,
`env.r2_secret_access_key`, and bails out if
`env.has_r2_config()` is false. Never accept raw keys as arguments.

**STO3 — `artifact_zip` downloads are validated, not blindly
extracted.** `_parse_artifact_metadata()` reads `artifact-metadata.json`
from the archive and `_normalize_artifact_path()` / `_parse_artifact_entry()`
reject absolute paths and traversal before anything is written. This is
a security boundary; do not bypass it with a plain `zipfile.extractall`.

**STO4 — Executable bits are restored after extraction.**
`_restore_zip_file_mode()` is required because zip does not preserve
permissions. A download that skips it produces non-executable server
binaries.

**STO5 — The destination is cleared before extraction.**
`_clear_destination()` guarantees "latest", not "merged". A stale file
from a previous build must not survive a resource download.

**STO6 — Release metadata is merged, never overwritten.**
`merge_release_metadata(existing, new)` preserves other platforms'
entries so publishing one platform does not drop the rest. Do not
replace it with a plain overwrite of `release.json`.

**STO7 — Artifact keys are derived, not hardcoded per call.**
`_get_artifact_key(filename, platform)` and
`_get_linux_artifact_key(filename)` encode the R2 layout;
`ctx.get_release_path(platform)` gives the `releases/<version>/<platform>/`
prefix. Add new artifacts there, not as literals at the call site.

**STO8 — `detect_artifacts(ctx)` reads the registry, not the
filesystem.** Artifacts appear in the upload only if a packaging module
registered them (see `../package/AGENTS.md` rule PKG1).

## Workflows

**Pre-building: fetching server resources**
1. Add the entry to `../../config/download_resources.yaml` with
   `r2_key`, `destination`, and `download_type: artifact_zip`.
2. `browseros build -m download_resources`.
3. `../resources/resources.py` then copies them per
   `os:` / `arch:` into the Chromium tree.

**Publishing a build**
1. Package first so the artifacts are in `ctx.artifact_registry`.
2. `browseros build -m upload`.
3. `generate_release_json()` + `merge_release_metadata()` update
   `release.json`; `get_release_json()` is the read side used by
   `../release/`.

**Adding a new artifact type to the R2 layout**
1. Add a branch to `_get_artifact_key()` (or
   `_get_linux_artifact_key()` for Linux variants).
2. Extend `generate_release_json()` so the field appears in the index.
3. Extend `merge_release_metadata()` so the field is preserved when
   another platform publishes.

**Debugging an R2 failure**
1. `get_r2_client()` logs "R2 configuration not set" when
   `has_r2_config()` is false — that is a credentials problem, not a
   network one.
2. "boto3 not installed" means the optional dependency is missing.
3. A metadata parse error means the archive did not contain
   `artifact-metadata.json`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — all 14 module areas.
- [`../../config/AGENTS.md`](../../config/AGENTS.md) — `download_resources.yaml`, this area's input.
- [`../release/AGENTS.md`](../release/AGENTS.md) — reads `release.json` through this area.
- [`../../cli/storage.py`](../../cli/storage.py) — `browseros upload lima|bun`, a separate third-party upload path.
- [`../../common/env.py`](../../common/env.py) — `R2_*` credentials.

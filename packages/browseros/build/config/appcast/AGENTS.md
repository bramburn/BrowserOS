# `build/config/appcast/` — Sparkle appcast seed XML

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

Two appcast files for the BrowserOS **Server** OTA channel, not for the
browser itself. `appcast-server.xml` is a minimal item-free Sparkle RSS
document — an empty `<channel>` with a title, a CDN link and a description.
`appcast-server.alpha.xml` is the same shell plus the published `<item>`
history for the alpha channel.

These files are *read* by the OTA module as a starting point, not
written by the build. `../modules/ota/common.py::parse_existing_appcast()`
parses one to recover any items already published, and
`generate_server_appcast()` writes the merged result (new item +
preserved history) elsewhere for upload.

## Contents

| File | Channel |
|---|---|
| `appcast-server.xml` | Production/stable. `<title>BrowserOS Server</title>`, links `https://cdn.browseros.com/appcast-server.xml`. |
| `appcast-server.alpha.xml` | Alpha channel, selected via `get_appcast_path(channel="alpha")` in `../modules/ota/common.py`. |

Both use the Sparkle namespace
`http://www.andymatuschak.org/xml-namespaces/sparkle` declared on `<rss>`,
matching `SPARKLE_NS` in `../modules/ota/common.py`.

## Rules

**AC1 — Keep `appcast-server.xml` item-free.** An `<item>` in the stable
shell is a hand-inserted release that the generator will then duplicate
or resurrect. `appcast-server.alpha.xml` legitimately carries `<item>`
history; that is the generator's output, not a hand edit. Publish through
`browseros ota server release-appcast`, which regenerates the file.

**AC2 — The `<link>` URL and the file name must stay in sync.** The
`<link>` is what clients fetch; the filename is what
`get_appcast_path()` resolves on disk. Renaming one without the other
breaks the update check for that channel.

**AC3 — Do not add platform enclosures by hand.** The five target
platforms (`darwin_arm64`, `darwin_x64`, `linux_arm64`, `linux_x64`,
`windows_x64`) come from `SERVER_PLATFORMS` in
`../modules/ota/common.py`; `ENCLOSURE_TEMPLATE` renders each one.
Hand-written enclosures will not match signed artifact names.

**AC4 — This is not the browser appcast.** The macOS browser's Sparkle
signing lives in `../../modules/sign/sparkle.py` and keys off
`SPARKLE_PRIVATE_KEY`; do not merge the two formats.

## Workflows

**Publishing a Server OTA release**
1. Build and sign the server bundle for each platform in
   `SERVER_PLATFORMS`.
2. Run `browseros ota server release` — it reads the correct seed file
   via `get_appcast_path(channel)`, preserves existing items, appends
   the new one, and uploads.
3. Verify the uploaded appcast, not the local file.

**Adding a new server target platform**
1. Add a dict to `SERVER_PLATFORMS` in `../modules/ota/common.py`
   (`name`, `binary`, `target`, `os`, `arch`).
2. Add the matching R2 `r2_key` to `../download_resources.yaml`.
3. No change is needed here — the generator renders the enclosure.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — build-system overview.
- [`../../modules/ota/AGENTS.md`](../../modules/ota/AGENTS.md) — the reader/writer of these files.
- [`../../modules/sign/sparkle.py`](../../modules/sign/sparkle.py) — browser-side Sparkle signing (different feed).
- [`../../common/sparkle.py`](../../common/sparkle.py) — `sparkle_sign_file()`.

# BrowserOS Update Design

How BrowserOS updates itself, and why it does not use Chromium's updater.

## Summary

BrowserOS ships its own updater in `chrome/browser/browseros/update/`. It polls a
JSON manifest from an S3-compatible bucket over plain HTTPS, downloads the
installer, verifies it twice (SHA-256 then Authenticode), and hands off to the
platform installer. The About page shows status and offers a **Check for
updates** button.

Windows is implemented. macOS has a stub that fails closed. Linux has no
in-browser updater and never will — Chromium does not provide one.

## Why not Chromium's updater

The fork is unbranded, and Chromium's updater is a Chrome-branded feature. Three
independent facts, each verified against Chromium 148.0.7778.97 (the pinned
upstream), make it unusable:

1. **It is not compiled in.** `chrome/browser/buildflags.gni`:

   ```gn
   # By default, only branded builds integrate with automatic updates.
   enable_updater = is_chrome_branded && target_os != "android"
   enable_update_notifications = is_chrome_branded
   ```

   with the upstream comment *"Chromium does not use an auto-updater."* For
   BrowserOS both are `false`. There is no build flag that yields a working
   in-browser updater without also declaring the build Chrome-branded.

2. **The check is a signed POST, not a GET.** `components/update_client/` speaks
   Omaha 4.0 JSON. `request_sender.cc` calls
   `PostRequest(url, body, "application/json", ...)` and then
   `signer_.ValidateResponse(...)`. `chrome/updater/external_constants_default.cc`
   has `bool UseCUP() const override { return true; }` — unconditional — so
   responses are verified against ECDSA P-256 / ML-DSA-44 public keys compiled
   into the binary. A static object store can serve neither the POST nor a valid
   signature.

3. **Windows payloads must be CRX3.** `external_constants_default.cc` sets
   `CrxVerifierFormat::CRX3_WITH_PUBLISHER_PROOF`; the `update_client` test data
   is `ChromeRecovery.crx3`. A bare `.exe` with `name`/`size`/`hash` attributes
   is not an acceptable package.

The legacy Omaha 3.0 XML that most write-ups describe is gone. There is no XML
parser left in `components/update_client` — only `protocol_parser_json.cc`,
`protocol_serializer_json.cc`, `protocol_handler.cc` and `request_sender.cc` —
and every test fixture is `.json`.

## What this replaced

Earlier revisions of this repo shipped a pipeline that published Omaha 3.0 XML
to the bucket (`tools/release/generate_update_manifests.py` wrote
`update_check.xml`; `docs/CI_AND_RELEASES.md` described an "Omaha-4" design).
It could not have worked, for the reasons above. It has been removed rather than
patched. If you find an `update_check.xml` in the bucket, it is dead.

Two further bugs were fixed along the way:

- The old generator wrote `<daystart elapsed_days="...">` from `tm_yday`
  (1–366). The Omaha V3 spec defines `elapsed_days` as days since 2007-01-01,
  and requires servers to always send `elapsed_seconds`. Both were wrong.
- The old About page patch had drifted from upstream (`hidden$=` where upstream
  now has `hidden=`) and no longer applied. It has been regenerated against the
  pinned commit.

## Architecture

```
release-windows.yml
    |  produces BrowserOS_v<ver>_win-x64.exe, Authenticode-signed
    v
update-manifest.yml
    |  browseros/windows/<channel>/latest.json            max-age=60
    |  browseros/windows/<version>/BrowserOS_v<ver>_win-x64.exe   immutable
    v
BrowserOSUpdateService  (singleton, UI thread)
    |  1. GET latest.json, no-cache
    |  2. Manifest::ParseForPlatform  — schema + host allowlist
    |  3. DecideUpdate                — rollback guard
    |  4. Downloader::Start           — .partial, then rename
    |  5. Verifier::VerifySha256      — background thread
    |  6. Verifier::VerifySignature   — Authenticode + publisher pin
    |  7. stage -> kReadyToInstall
    v
UpdateProxy (mojom)  ->  About page card
                            [Check for updates] / [Restart to update]
                                 |
                                 v
                        Installer::ApplyBlocking
```

### The manifest

`tools/release/generate_update_manifests.py` writes `latest.json`:

```json
{
  "schema": 1,
  "channel": "stable",
  "version": "0.1.0",
  "chromium_version": "148.0.7778.97",
  "published_at": "2026-09-28T21:00:00+00:00",
  "notes": "https://github.com/.../releases/tag/browseros-windows-v0.1.0",
  "platforms": {
    "windows-x64": {
      "url": "https://cdn.bramburn.com/browseros/windows/0.1.0/BrowserOS_v0.1.0_win-x64.exe",
      "sha256": "<64 lowercase hex>",
      "size": 178257920,
      "min_os_version": "10.0.17763",
      "publisher": "BrowserOS Ltd"
    }
  }
}
```

`Manifest::ParseForPlatform()` rejects anything that does not fit this shape:
unknown `schema`, missing or unparseable `version`, empty `channel`, a platform
entry that is absent or not an object, a `sha256` that is not 64 lowercase hex,
`size <= 0`, a URL that is not `https`, or a URL whose host is not in the
compiled-in allowlist.

### The kill switch

Set `"channel": "paused"` and the client reports *updates paused* and offers
nothing. This is deliberate: the channel lives in the **object path**
(`.../windows/stable/latest.json`), not in the body, so pausing a channel is one
upload and cannot be hidden behind a cached manifest for a different channel.

```
bun packages/build-tools/scripts/upload-to-r2.ts \
  --file paused.json \
  --key browseros/windows/stable/latest.json \
  --content-type application/json \
  --cache-control "public, max-age=60"
```

To resume, re-publish the previous version. The manifest TTL is 60 seconds, so
this takes effect on the next poll without shipping a browser build.

## Security

Two independent gates, both required, in this order:

1. **SHA-256** against the manifest. Catches truncation and corruption. This is
   *not* a security boundary: an attacker who can write to the bucket can
   replace the manifest and the binary together and this check still passes.
2. **Authenticode signature plus publisher-subject match.** This is the gate
   that actually establishes provenance, which is why it is never skipped and
   why the non-Windows stub **fails closed** rather than returning success.

Other invariants:

- **Rollback protection.** `DecideUpdate()` only ever offers a strictly newer
  version. Equal is a no-op; older is refused. Covered by
  `browseros_update_checker_unittest.cc`.
- **Host allowlist is compiled in**, not read from a pref or a flag, via
  `browseros_update_config.h`. Override per build with
  `gn args browseros_update_base_url=...`.
- **Hostile manifests cannot redirect a download.** The URL is validated before
  it is ever fetched, and the staging filename is stripped of path separators
  regardless.
- **Installers are never killed mid-write.** A wedged installer is left alone;
  the browser exits and re-checks on next launch. Killing a half-written
  installer is how a browser gets bricked.
- **Exit codes are advisory.** The authoritative success test is whether
  `Application\<version>\` appeared under the install root.
- **Interrupted installs are detected.** A verified-but-unapplied version is
  recorded in `kUpdateStagedVersion`; on next launch, if that version is not
  running, the marker is cleared and the update restages from scratch.

## Configuration

Build-time, via `gn args`:

| Arg | Default | Purpose |
| --- | --- | --- |
| `browseros_update_base_url` | `https://cdn.bramburn.com/browseros` | Manifest base URL |
| `browseros_update_installer_args` | `--silent --install --do-not-launch-browser` | Quiet install flags |

CI variables:

| Name | Purpose |
| --- | --- |
| `BROWSEROS_S3_PREFIX` | Object key prefix (default `browseros`) |
| `BROWSEROS_CDN_BASE` | Public CDN base URL |
| `BROWSEROS_PUBLISHER` | Expected Authenticode publisher subject |
| `BROWSEROS_UPDATE_CHANNEL` | Default channel when not passed explicitly |

The storage layer speaks the S3 API for both Cloudflare R2 and real AWS S3. Set
`S3_ENDPOINT_URL` and `S3_REGION` to point at AWS; leave them unset and set
`R2_ACCOUNT_ID` for R2. `S3_*` credentials take precedence over `R2_*`.

## Failure behaviour

| Failure | Result |
| --- | --- |
| Network down, manifest 404, or malformed | Log, settle to a terminal state, retry next poll |
| Version not strictly newer | Refuse (rollback guard) |
| `channel: "paused"` | Refuse to offer anything |
| SHA-256 mismatch | Delete the payload, never run the installer |
| Authenticode invalid or publisher mismatch | Same, logged loudly |
| Download interrupted | `.partial` discarded on next start |
| Installer fails or times out | Old install untouched, marker cleared |
| Build has no allowlisted host | Every manifest rejected |

The governing rule: **the updater must never be able to brick a browser.** Every
path above ends in a browser that still starts.

## Testing

```bash
# Manifest schema and host allowlist
out/<build>/browseros_update_manifest_unittest

# Version comparison and the rollback guard
out/<build>/browseros_update_checker_unittest
```

`browseros_update_manifest_unittest.cc` also pins the canonical JSON
serialisation against what the Python generator emits, so the two cannot drift.

## Known gaps

- **No resumable download.** A partial transfer is discarded and restarted.
  `SimpleURLLoader` cannot append, so resume needs a ranged request via
  `net::URLRequestFactory`. The installer URL embeds the version and is
  immutable, so this is a bandwidth cost, not a correctness one.
- **macOS is a stub.** `browseros_update_verifier_stub.cc` and
  `browseros_update_installer_stub.cc` fail closed. The fork already carries
  Sparkle patches (`mac-sparkle-updater` in `build/features.yaml`) but
  `SUFeedURL` is set nowhere in the repo and `third_party/sparkle/` contains
  only a `BUILD.gn`, so Sparkle has neither a feed to poll nor a framework to
  load.
- **The About page binding is not wired.** `browseros_update.mojom` and
  `UpdateProxy` exist and the About page markup is in place, but nothing yet
  hands a `PendingRemote` to the WebUI, registers `kUpdate*` prefs, or calls
  `UpdateService::Initialize()`. Until those three exist the service never
  runs, and the card never updates.
- **Authenticode pinning is cert-coupled.** A publisher-subject match survives
  renewal (the subject is stable across re-issues of the same key) but not a
  change of signing identity. An Ed25519 manifest signature — mirroring
  `build/modules/ota/common.py`, which already does this for the server binary —
  removes the coupling. Not done.

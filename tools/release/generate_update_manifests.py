#!/usr/bin/env python3
"""Generate BrowserOS update manifests for the bramburn fork.

Emits two XML files and a small JSON pointer:

  - update_check.xml — Omaha-4 response that Chromium's
    `components/update_client` polls. Drives the in-browser
    "restart to update" toast.

  - appcast.xml — Sparkle-style RSS feed that `browseros-cli` and
    external tools poll. Compatible with the existing
    `appcast-server.xml` template at
    `packages/browseros/build/config/appcast/`.

  - latest.json — tiny `{ "version": ..., "url": ..., "sha256": ... }`
    pointer for quick CLI checks without XML parsing.

Run as part of `.github/workflows/update-manifest.yml`. See
`docs/UPDATE_SERVER.md` for the schemas and the R2 layout.
"""

from __future__ import annotations

import argparse
import datetime as _dt
import hashlib
import json
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import NamedTuple


class _Args(NamedTuple):
    version: str
    sha256: str
    size: int
    app_id: str
    r2_prefix: str
    cdn_base: str
    output_dir: Path
    pub_date: str


def _parse_args() -> _Args:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--version", required=True, help="Semver, e.g. 0.1.0")
    p.add_argument(
        "--sha256",
        required=True,
        help="Hex SHA-256 of the .exe binary (no prefix, lowercase)",
    )
    p.add_argument("--size", required=True, type=int, help="Size in bytes")
    p.add_argument(
        "--app-id",
        default="{B49B6F23-3D5C-4F7D-9F0F-1F5E5D4C4D9F}",
        help="Bramburn BrowserOS Windows app GUID. Stable per fork.",
    )
    p.add_argument(
        "--r2-prefix",
        default="browseros",
        help="R2 key prefix for browseros artifacts (no trailing slash)",
    )
    p.add_argument(
        "--cdn-base",
        default="https://cdn.bramburn.com",
        help="Public CDN base URL (R2 fronted by Cloudflare)",
    )
    p.add_argument(
        "--output-dir", type=Path, default=Path("/tmp/out"), help="Where to write files"
    )
    p.add_argument(
        "--pub-date",
        default=None,
        help="RFC 822 pub date for the appcast item (default: now UTC)",
    )
    ns = p.parse_args()

    if not re.match(r"^[0-9a-f]{64}$", ns.sha256):
        raise SystemExit(f"--sha256 must be 64 lowercase hex chars, got: {ns.sha256!r}")
    if ns.size <= 0:
        raise SystemExit(f"--size must be positive, got: {ns.size}")

    return _Args(
        version=ns.version,
        sha256=ns.sha256,
        size=ns.size,
        app_id=ns.app_id,
        r2_prefix=ns.r2_prefix.rstrip("/"),
        cdn_base=ns.cdn_base.rstrip("/"),
        output_dir=ns.output_dir,
        pub_date=ns.pub_date or _dt.datetime.now(_dt.timezone.utc).strftime(
            "%a, %d %b %Y %H:%M:%S +0000"
        ),
    )


def _binary_url(args: _Args) -> str:
    return f"{args.cdn_base}/{args.r2_prefix}/{args.version}/BrowserOS_v{args.version}_win-x64.exe"


def _write_omaha_update_check(args: _Args, out_dir: Path) -> Path:
    """Omaha-4 `update_check` response.

    Format reference: https://chromium.googlesource.com/chromium/src/+/master/components/update_client/
    The browseros patch wires `update_client` at this URL (configured at build
    time via the fork's update URL override).
    """
    binary_url = _binary_url(args)
    response = ET.Element("response", attrib={"protocol": "3.0", "server": "prod"})
    # daystart — required by some Chromium clients for sanity checks
    daystart = ET.SubElement(
        response,
        "daystart",
        attrib={"elapsed_days": str(_dt.datetime.now(_dt.timezone.utc).timetuple().tm_yday)},
    )
    app = ET.SubElement(
        response,
        "app",
        attrib={"appid": args.app_id, "status": "ok"},
    )
    updatecheck = ET.SubElement(app, "updatecheck", attrib={"status": "ok"})
    urls = ET.SubElement(updatecheck, "urls")
    ET.SubElement(urls, "url", attrib={"codebase": f"{args.cdn_base}/{args.r2_prefix}/"})
    manifest = ET.SubElement(updatecheck, "manifest", attrib={"version": args.version})
    packages = ET.SubElement(manifest, "packages")
    ET.SubElement(
        packages,
        "package",
        attrib={
            "hash": f"sha256={args.sha256}",
            "name": f"BrowserOS_v{args.version}_win-x64.exe",
            "size": str(args.size),
            "required": "true",
        },
    )

    tree = ET.ElementTree(response)
    ET.indent(tree, space="  ")
    out_path = out_dir / "update_check.xml"
    tree.write(out_path, encoding="utf-8", xml_declaration=True)
    return out_path


def _write_appcast(args: _Args, out_dir: Path) -> Path:
    """Sparkle-style RSS appcast for browseros-cli / external consumers.

    Compatible with the existing `appcast-server.xml` template. browseros-cli
    can poll this URL to detect new BrowserOS builds.
    """
    binary_url = _binary_url(args)
    sparkle_ns = "http://www.andymatuschak.org/xml-namespaces/sparkle"
    ET.register_namespace("sparkle", sparkle_ns)

    rss = ET.Element("rss", attrib={"version": "2.0"})
    channel = ET.SubElement(rss, "channel")
    ET.SubElement(channel, "title").text = "BrowserOS Windows"
    ET.SubElement(channel, "link").text = (
        f"https://github.com/{args.r2_prefix.split('/')[0] if '/' in args.r2_prefix else 'bramburn'}/BrowserOS/releases"
    )
    ET.SubElement(channel, "description").text = (
        "BrowserOS Windows builds for the bramburn/BrowserOS fork. "
        "Tagged at browseros-windows-v<version>, packaged from upstream "
        "Chromium with our patches + bundled MCP server."
    )
    ET.SubElement(channel, "language").text = "en"

    item = ET.SubElement(channel, "item")
    ET.SubElement(item, "title").text = f"BrowserOS {args.version}"
    ET.SubElement(item, "link").text = (
        f"https://github.com/bramburn/BrowserOS/releases/tag/browseros-windows-v{args.version}"
    )
    ET.SubElement(item, f"{{{sparkle_ns}}}version").text = args.version
    ET.SubElement(item, f"{{{sparkle_ns}}}shortVersionString").text = args.version
    ET.SubElement(item, f"{{{sparkle_ns}}}minSystemVersion").text = "10.0.17763"
    ET.SubElement(item, "pubDate").text = args.pub_date
    ET.SubElement(
        item,
        "enclosure",
        attrib={
            "url": binary_url,
            f"{{{sparkle_ns}}}os": "windows-x64",
            "length": str(args.size),
            "type": "application/octet-stream",
        },
    )
    ET.SubElement(item, "description").text = (
        f"BrowserOS Windows v{args.version}. "
        f"SHA-256: {args.sha256}. See the GitHub release notes for what's new."
    )

    tree = ET.ElementTree(rss)
    ET.indent(tree, space="  ")
    out_path = out_dir / "appcast.xml"
    tree.write(out_path, encoding="utf-8", xml_declaration=True)
    return out_path


def _write_latest_pointer(args: _Args, out_dir: Path) -> Path:
    """Small JSON pointer for `browseros-cli --check-update` style flows."""
    payload = {
        "version": args.version,
        "url": _binary_url(args),
        "sha256": args.sha256,
        "size": args.size,
        "channel": "stable",
        "app_id": args.app_id,
        "published_at": _dt.datetime.now(_dt.timezone.utc).isoformat(),
    }
    out_path = out_dir / "latest.json"
    out_path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    return out_path


def main() -> int:
    args = _parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    omaha = _write_omaha_update_check(args, args.output_dir)
    appcast = _write_appcast(args, args.output_dir)
    latest = _write_latest_pointer(args, args.output_dir)

    for path in (omaha, appcast, latest):
        digest = hashlib.sha256(path.read_bytes()).hexdigest()
        print(f"{path}  {path.stat().st_size} bytes  sha256={digest}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
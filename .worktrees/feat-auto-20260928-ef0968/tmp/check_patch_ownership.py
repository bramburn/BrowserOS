"""Which Chromium files are targeted by more than one patch in
chromium_patches/?

PatchesModule (build/modules/patches/patches.py) -> apply_all_patches ->
find_patch_files() applies EVERY file under chromium_patches/ with
`git apply -p1` (with a --3way fallback), in sorted order. features.yaml is
NOT consulted by this path, so features.yaml ownership is irrelevant to
whether prep succeeds. What matters is whether two diffs target the same
Chromium path.

Duplicate targets are legal when the later diff was generated against the
already-patched tree, and fatal when it was not. This lists them so a prep
failure can be localised fast.
"""
import os
import re
from collections import defaultdict

ROOT = r"D:\BrowserOs\packages\browseros\chromium_patches"

# --- collect patch files exactly the way find_patch_files() does ---------
patches = []
for dirpath, _dirnames, filenames in os.walk(ROOT):
    for name in filenames:
        if (name.endswith(".deleted") or name.endswith(".binary")
                or name.endswith(".rename") or name.startswith(".")):
            continue
        patches.append(os.path.join(dirpath, name))
patches.sort()

targets = defaultdict(list)   # chromium path -> [patch relpaths]
no_header = []

diff_re = re.compile(r"^diff --git a/(.+?) b/(.+?)\s*$")
new_re = re.compile(r"^\+\+\+ b/(.+?)\s*$")

for p in patches:
    rel = os.path.relpath(p, ROOT).replace("\\", "/")
    seen = set()
    with open(p, "r", encoding="utf-8", errors="replace") as fh:
        for line in fh:
            m = diff_re.match(line)
            if m:
                path = m.group(2)
                if path == "/dev/null":
                    continue
                if path not in seen:
                    seen.add(path)
                    targets[path].append(rel)
                continue
            m = new_re.match(line)
            if m and m.group(1) != "/dev/null":
                path = m.group(1)
                if path not in seen:
                    seen.add(path)
                    targets[path].append(rel)
    if not seen:
        no_header.append(rel)

print("patch files scanned : %d" % len(patches))
print("distinct chromium paths targeted: %d" % len(targets))
print("patch files with no parseable diff --git/+++ header: %d" % len(no_header))
for n in no_header[:20]:
    print("    %s" % n)
print()

dupes = {k: v for k, v in targets.items() if len(v) > 1}
print("=== PATHS TARGETED BY MORE THAN ONE PATCH: %d ===" % len(dupes))
for path in sorted(dupes):
    print("  %s" % path)
    for src in dupes[path]:
        print("      <- %s" % src)

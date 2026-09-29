"""Extract the five signatures the two blocked plans guess at, from the real
pinned Chromium tree. Prints file:line for every hit so nothing is quoted
from memory.
"""
import os
import re
import sys

SRC = r"D:\browseros-build\src"


def show(relpath, patterns, ctx=0, limit=40, flags=0):
    p = os.path.join(SRC, relpath)
    print("=" * 78)
    print("### %s" % relpath)
    if not os.path.exists(p):
        print("    !!! NOT ON DISK: %s" % p)
        return
    with open(p, "r", encoding="utf-8", errors="replace") as fh:
        lines = fh.readlines()
    print("    (%d lines)" % len(lines))
    shown = 0
    for i, line in enumerate(lines, 1):
        for pat in patterns:
            if re.search(pat, line, flags):
                lo = max(0, i - 1 - ctx)
                hi = min(len(lines), i + ctx)
                for j in range(lo, hi):
                    print("  %5d | %s" % (j + 1, lines[j].rstrip("\n")))
                if ctx:
                    print("  %5d | %s" % (i, line.rstrip("\n")))
                else:
                    print("  ----")
                shown += 1
                break
        if shown >= limit:
            print("  ... (truncated at %d hits)" % limit)
            break


def region(relpath, start, end):
    p = os.path.join(SRC, relpath)
    print("=" * 78)
    print("### %s  lines %d-%d" % (relpath, start, end))
    if not os.path.exists(p):
        print("    !!! NOT ON DISK")
        return
    with open(p, "r", encoding="utf-8", errors="replace") as fh:
        lines = fh.readlines()
    for j in range(max(0, start - 1), min(len(lines), end)):
        print("  %5d | %s" % (j + 1, lines[j].rstrip("\n")))


# ---- 1. renderer_context_menu.cc -----------------------------------------
show("chrome/browser/renderer_context_menu.cc",
     [r"^\s*void RenderViewContextMenu::AppendMenuItems",
      r"AppendMenuItems\(\)"])

print()
print(">>> command dispatch: who executes a menu command id")
show("chrome/browser/renderer_context_menu.cc",
     [r"RenderViewContextMenu::ExecuteCommand",
      r"RenderViewContextMenu::OnMenuItemShown",
      r"MenuModel::ExecuteCommand"], ctx=3, limit=12)

# ---- 2. CopyFromSurface ---------------------------------------------------
show("content/public/browser/render_widget_host_view.h",
     [r"CopyFromSurface"], ctx=4, limit=30)

# ---- 3. about_flags.cc ----------------------------------------------------
show("chrome/browser/about_flags.cc",
     [r"^\s*(?:static\s+)?const\s+FeatureEntry\s+kFeatureEntries\[\]",
      r"^\s*// static.*entries",
      r"kFeatureEntries\[\]"], ctx=6, limit=6)

show("chrome/browser/browser_features.h",
     [r"^BASE_FEATURE\("], ctx=2, limit=4)

# ---- 4. chrome_command_ids.h ---------------------------------------------
print()
print(">>> 403xx command ids in the pristine tree")
show("chrome/app/chrome_command_ids.h", [r"\b403\d\d\b"], limit=60)

# ---- 5. stock page-context-menu screenshot item ---------------------------
print()
print(">>> stock page context-menu screenshot")
show("chrome/browser/renderer_context_menu.cc",
     [r"[Ss]creenshot"], ctx=2, limit=20)

show("chrome/app/chromium_strings.grd",
     [r"IDS_SHOW_SCREENSHOT|Screenshot\.\.\.|name=\"IDS_.*SCREENSHOT"],
     ctx=2, limit=20)

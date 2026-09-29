diff --git a/chrome/browser/browseros/update/browseros_update_prefs.h b/chrome/browser/browseros/update/browseros_update_prefs.h
new file mode 100644
index 0000000000000..0000000000000
--- /dev/null
+++ b/chrome/browser/browseros/update/browseros_update_prefs.h
@@ -0,0 +1,33 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+#ifndef CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_PREFS_H_
+#define CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_PREFS_H_
+
+class PrefRegistrySimple;
+
+namespace browseros_update {
+
+// How often the background check runs, and the default channel.
+inline constexpr int kDefaultUpdateCheckIntervalHours = 6;
+inline constexpr char kDefaultUpdateChannel[] = "stable";
+
+// Preference keys, stored in Local State.
+//
+// These are browser-wide, not per-profile: the update applies to the
+// installation, not to a set of tabs. A second profile on the same machine
+// must see the same update state.
+extern const char kUpdateChannel[];
+extern const char kUpdateAutoCheckEnabled[];
+extern const char kUpdateLastCheckTime[];
+extern const char kUpdateLastError[];
+extern const char kUpdateSkippedVersion[];
+extern const char kUpdateStagedVersion[];
+
+// Registers BrowserOS update preferences in Local State.
+void RegisterLocalStatePrefs(PrefRegistrySimple* registry);
+
+}  // namespace browseros_update
+
+#endif  // CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_PREFS_H_

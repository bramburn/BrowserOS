diff --git a/chrome/browser/browseros/update/browseros_update_prefs.cc b/chrome/browser/browseros/update/browseros_update_prefs.cc
new file mode 100644
index 0000000000000..0000000000000
--- /dev/null
+++ b/chrome/browser/browseros/update/browseros_update_prefs.cc
@@ -0,0 +1,44 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+#include "chrome/browser/browseros/update/browseros_update_prefs.h"
+
+#include <string>
+
+#include "components/pref_registry/pref_registry_simple.h"
+
+namespace browseros_update {
+
+// Defined in the header. Deliberately not routed through
+// chrome/common/pref_names.h: these are Local State (browser-wide) prefs owned
+// entirely by this module, and the fork already keeps its prefs local
+// (see chrome/browser/browseros/core/browseros_prefs.h).
+const char kUpdateChannel[] = "browseros.update.channel";
+const char kUpdateAutoCheckEnabled[] = "browseros.update.auto_check_enabled";
+const char kUpdateLastCheckTime[] = "browseros.update.last_check_time";
+const char kUpdateLastError[] = "browseros.update.last_error";
+const char kUpdateSkippedVersion[] = "browseros.update.skipped_version";
+const char kUpdateStagedVersion[] = "browseros.update.staged_version";
+
+void RegisterLocalStatePrefs(PrefRegistrySimple* registry) {
+  registry->RegisterStringPref(kUpdateChannel, kDefaultUpdateChannel);
+  registry->RegisterBooleanPref(kUpdateAutoCheckEnabled, true);
+
+  // Stored as a double (seconds since the Windows epoch). Zero means
+  // "never checked".
+  registry->RegisterDoublePref(kUpdateLastCheckTime, 0.0);
+
+  // Empty means "no error recorded".
+  registry->RegisterStringPref(kUpdateLastError, std::string());
+
+  // Empty means "nothing skipped".
+  registry->RegisterStringPref(kUpdateSkippedVersion, std::string());
+
+  // Set when an update has been downloaded and verified but not yet applied.
+  // Cleared once the new version is observed running, which is how a crash
+  // mid-install is detected on the next startup.
+  registry->RegisterStringPref(kUpdateStagedVersion, std::string());
+}
+
+}  // namespace browseros_update

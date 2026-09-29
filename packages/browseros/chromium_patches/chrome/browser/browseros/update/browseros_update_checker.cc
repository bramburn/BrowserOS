diff --git a/chrome/browser/browseros/update/browseros_update_checker.cc b/chrome/browser/browseros/update/browseros_update_checker.cc
new file mode 100644
index 0000000000000..0000000000000
--- /dev/null
+++ b/chrome/browser/browseros/update/browseros_update_checker.cc
@@ -0,0 +1,70 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+#include "chrome/browser/browseros/update/browseros_update_checker.h"
+
+#include <string>
+
+#include "base/version.h"
+
+namespace browseros_update {
+
+const char* UpdateDecisionToString(UpdateDecision decision) {
+  switch (decision) {
+    case UpdateDecision::kUpdateAvailable:
+      return "update-available";
+    case UpdateDecision::kUpToDate:
+      return "up-to-date";
+    case UpdateDecision::kRollbackBlocked:
+      return "rollback-blocked";
+    case UpdateDecision::kPaused:
+      return "paused";
+    case UpdateDecision::kNotSupported:
+      return "not-supported";
+    case UpdateDecision::kSkippedByUser:
+      return "skipped-by-user";
+    case UpdateDecision::kInvalidVersion:
+      return "invalid-version";
+  }
+  return "unknown";
+}
+
+UpdateDecision DecideUpdate(const base::Version& current_version,
+                            const base::Version& remote_version,
+                            bool is_paused,
+                            const std::string& skipped_version) {
+  // The kill switch wins over everything. An operator who paused a channel
+  // must be able to stop updates even if the version comparison would have
+  // offered one.
+  if (is_paused) {
+    return UpdateDecision::kPaused;
+  }
+
+  if (!current_version.IsValid() || !remote_version.IsValid()) {
+    return UpdateDecision::kInvalidVersion;
+  }
+
+  const int comparison = remote_version.CompareTo(current_version);
+
+  // Strictly newer only. Equal means there is nothing to do, and older is
+  // refused outright. Do not relax this into ">=" — that is the rollback
+  // vulnerability.
+  if (comparison <= 0) {
+    return comparison == 0 ? UpdateDecision::kUpToDate
+                           : UpdateDecision::kRollbackBlocked;
+  }
+
+  // A newer version is still offered if it is the one the user skipped;
+  // anything newer than the skipped one is fine.
+  if (!skipped_version.empty()) {
+    const base::Version skipped(skipped_version);
+    if (skipped.IsValid() && remote_version.CompareTo(skipped) <= 0) {
+      return UpdateDecision::kSkippedByUser;
+    }
+  }
+
+  return UpdateDecision::kUpdateAvailable;
+}
+
+}  // namespace browseros_update

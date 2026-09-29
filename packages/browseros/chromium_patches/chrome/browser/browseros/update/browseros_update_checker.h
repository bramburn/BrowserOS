diff --git a/chrome/browser/browseros/update/browseros_update_checker.h b/chrome/browser/browseros/update/browseros_update_checker.h
new file mode 100644
index 0000000000000..0000000000000
--- /dev/null
+++ b/chrome/browser/browseros/update/browseros_update_checker.h
@@ -0,0 +1,66 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+#ifndef CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_CHECKER_H_
+#define CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_CHECKER_H_
+
+#include <string>
+
+#include "base/version.h"
+
+namespace browseros_update {
+
+// What the updater should do with a manifest it has just fetched.
+//
+// Kept separate from UpdateService::State so the decision is pure data
+// that can be tested exhaustively without a network, a profile, or a
+// browser process.
+enum class UpdateDecision {
+  // Remote version is strictly newer. Offer the update.
+  kUpdateAvailable,
+
+  // Remote version equals the running version. Nothing to do.
+  kUpToDate,
+
+  // Remote version is older than the running one. Refuse. This is the
+  // rollback guard: a compromised or misconfigured manifest must never be
+  // able to talk the browser back into an older, less patched build.
+  kRollbackBlocked,
+
+  // The manifest is the kill switch. Offer nothing, report "paused".
+  kPaused,
+
+  // This build cannot update itself (wrong platform, or no artifact for
+  // it in the manifest).
+  kNotSupported,
+
+  // The user explicitly skipped this version.
+  kSkippedByUser,
+
+  // Either version is unparseable.
+  kInvalidVersion,
+};
+
+// Human-readable reason for a decision, for logs and the About page.
+// Returns a static string.
+const char* UpdateDecisionToString(UpdateDecision decision);
+
+// Decides what to do with a fetched manifest.
+//
+// Pure function: no I/O, no globals, no clock. All policy that can be
+// expressed as a comparison lives here so it is covered by unit tests
+// rather than discovered in the field.
+//
+// |skipped_version| is the version the user chose to skip, or empty. Pass
+// the parsed value; an unparseable string is treated as "nothing skipped"
+// rather than an error, because it only ever makes the updater *more*
+// willing to update, never less.
+UpdateDecision DecideUpdate(const base::Version& current_version,
+                            const base::Version& remote_version,
+                            bool is_paused,
+                            const std::string& skipped_version);
+
+}  // namespace browseros_update
+
+#endif  // CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_CHECKER_H_

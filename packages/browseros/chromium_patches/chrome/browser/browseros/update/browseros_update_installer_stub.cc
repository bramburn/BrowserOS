diff --git a/chrome/browser/browseros/update/browseros_update_installer_stub.cc b/chrome/browser/browseros/update/browseros_update_installer_stub.cc
new file mode 100644
index 0000000000000..0000000000000
--- /dev/null
+++ b/chrome/browser/browseros/update/browseros_update_installer_stub.cc
@@ -0,0 +1,37 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+// Non-Windows. Fails closed.
+//
+// Windows is the first supported platform. macOS lands next and will need a
+// real implementation (launching the .app installer, or handing off to the
+// Sparkle updater the fork already carries). Until one exists, every apply
+// fails and the service can never reach kAwaitingRestart here.
+
+#include "chrome/browser/browseros/update/browseros_update_installer.h"
+
+#include <string>
+
+#include "base/logging.h"
+#include "base/version.h"
+
+namespace browseros_update {
+
+base::FilePath Installer::GetInstallRoot() {
+  return base::FilePath();
+}
+
+bool Installer::IsVersionInstalled(const base::Version& version) {
+  return false;
+}
+
+bool Installer::ApplyBlocking(const base::FilePath& installer_path,
+                              const base::Version& expected_version,
+                              base::TimeDelta timeout,
+                              std::string* error) {
+  *error = "installing updates is not implemented for this platform";
+  return false;
+}
+
+}  // namespace browseros_update

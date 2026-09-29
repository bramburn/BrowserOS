diff --git a/chrome/browser/browseros/update/browseros_update_installer.h b/chrome/browser/browseros/update/browseros_update_installer.h
new file mode 100644
index 0000000000000..0000000000000
--- /dev/null
+++ b/chrome/browser/browseros/update/browseros_update_installer.h
@@ -0,0 +1,54 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+#ifndef CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_INSTALLER_H_
+#define CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_INSTALLER_H_
+
+#include <string>
+
+#include "base/files/file_path.h"
+#include "base/version.h"
+
+namespace browseros_update {
+
+// Applies a verified installer.
+//
+// The guiding rule is that the running installation is never mutated in
+// place. The Chrome-style installer writes a new versioned
+// Application\<version>\ directory and updates the pointers, so an install
+// that fails halfway leaves the previous version intact and launchable.
+//
+// A non-zero exit code is treated as advisory only. The real success test is
+// whether the expected version directory appeared, because installers
+// routinely exit non-zero after a successful write (relaunch denied, cleanup
+// warning, pending-reboot notice).
+class Installer {
+ public:
+  // Runs |installer_path| and waits for it to exit.
+  //
+  // |expected_version| is the version the manifest advertised. On success,
+  // the install root is expected to contain Application\<expected_version>\.
+  //
+  // |timeout| bounds the wait. Exceeding it is a failure; a wedged installer
+  // holding a handle on the old directory would otherwise block the relaunch
+  // indefinitely.
+  //
+  // Blocking. Do not call on the UI thread.
+  static bool ApplyBlocking(const base::FilePath& installer_path,
+                            const base::Version& expected_version,
+                            base::TimeDelta timeout,
+                            std::string* error);
+
+  // Root of the versioned install tree, e.g.
+  // %LOCALAPPDATA%\browseros. Empty when the platform has no such layout.
+  static base::FilePath GetInstallRoot();
+
+  // True when Application\<version>\ exists under the install root. This is
+  // the authoritative "did the update land" check.
+  static bool IsVersionInstalled(const base::Version& version);
+};
+
+}  // namespace browseros_update
+
+#endif  // CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_INSTALLER_H_

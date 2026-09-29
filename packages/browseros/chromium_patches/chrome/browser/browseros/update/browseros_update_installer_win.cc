diff --git a/chrome/browser/browseros/update/browseros_update_installer_win.cc b/chrome/browser/browseros/update/browseros_update_installer_win.cc
new file mode 100644
index 0000000000000..0000000000000
--- /dev/null
+++ b/chrome/browser/browseros/update/browseros_update_installer_win.cc
@@ -0,0 +1,96 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+// Windows-only. See browseros_update_installer_stub.cc for every other
+// platform, which fails closed.
+
+#include "chrome/browser/browseros/update/browseros_update_installer.h"
+
+#include <string>
+#include <vector>
+
+#include "base/files/file_util.h"
+#include "base/logging.h"
+#include "base/path_service.h"
+#include "base/process/launch.h"
+#include "base/time/time.h"
+#include "chrome/browser/browseros/update/browseros_update_config.h"
+#include "chrome/common/chrome_paths.h"
+
+namespace browseros_update {
+
+base::FilePath Installer::GetInstallRoot() {
+  // DIR_USER_DATA is <install root>\User Data, so the versioned
+  // Application\ tree that holds the binaries is its parent.
+  return base::PathService::Get(chrome::DIR_USER_DATA).DirName();
+}
+
+bool Installer::IsVersionInstalled(const base::Version& version) {
+  if (!version.IsValid()) {
+    return false;
+  }
+  const base::FilePath candidate =
+      GetInstallRoot().AppendASCII("Application").AppendASCII(version.ToString());
+  return base::DirectoryExists(candidate);
+}
+
+bool Installer::ApplyBlocking(const base::FilePath& installer_path,
+                              const base::Version& expected_version,
+                              base::TimeDelta timeout,
+                              std::string* error) {
+  if (!expected_version.IsValid()) {
+    *error = "refusing to install an invalid version";
+    return false;
+  }
+  if (!base::PathExists(installer_path)) {
+    *error = "installer is missing";
+    return false;
+  }
+
+  const std::vector<std::string> args = config::GetInstallerArgs();
+  if (args.empty()) {
+    LOG(ERROR) << "BrowserOS update: no installer arguments configured; this "
+                  "build cannot update silently";
+  }
+
+  base::LaunchOptions options;
+  options.current_directory = installer_path.DirName();
+
+  base::Process process = base::LaunchProcess(installer_path, args, options);
+  if (!process.IsValid()) {
+    *error = "could not start the installer";
+    return false;
+  }
+
+  const bool exited = process.WaitForExit(timeout.InMillisecondsFlt());
+  if (!exited) {
+    // Deliberately do not kill it. It may be mid-write, and killing a
+    // half-written installer is how a browser gets bricked. The browser exits
+    // regardless, and the next launch re-checks which version is installed.
+    LOG(ERROR) << "BrowserOS update: installer did not exit within the timeout";
+    *error = "installer timed out";
+    return false;
+  }
+
+  const int exit_code = process.exit_code();
+  if (exit_code != 0) {
+    // Advisory only. Installers exit non-zero after a good write often enough
+    // (relaunch denied, pending reboot, cleanup warning) that treating the
+    // code as fatal would strand users on old versions.
+    LOG(WARNING) << "BrowserOS update: installer exited with " << exit_code
+                 << "; checking the install root anyway";
+  }
+
+  // The authoritative check. Exit codes lie; the versioned directory does not.
+  if (!IsVersionInstalled(expected_version)) {
+    LOG(ERROR) << "BrowserOS update: version " << expected_version
+               << " is not under the install root after install";
+    *error = "the installer did not produce the expected version";
+    return false;
+  }
+
+  return true;
+}
+
+}  // namespace browseros_update

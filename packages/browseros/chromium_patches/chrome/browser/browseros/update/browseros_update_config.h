diff --git a/chrome/browser/browseros/update/browseros_update_config.h b/chrome/browser/browseros/update/browseros_update_config.h
new file mode 100644
index 0000000000000..0000000000000
--- /dev/null
+++ b/chrome/browser/browseros/update/browseros_update_config.h
@@ -0,0 +1,70 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+#ifndef CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_CONFIG_H_
+#define CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_CONFIG_H_
+
+#include <string>
+#include <vector>
+
+// Build-time update configuration.
+//
+// Every macro below can be overridden from GN, e.g.
+//
+//   gn gen out/Release --args='browseros_update_base_url="https://staging.example.com/browseros"'
+//
+// which is how a dev build points at a staging bucket without editing source.
+// The defaults are the production values.
+//
+// Nothing here is read at runtime from a pref or a command line. The host
+// allowlist in particular is the boundary that stops a hostile manifest from
+// redirecting a download somewhere else, so it has to be compiled in.
+
+// Base for the channel manifest, which resolves to e.g.
+//   https://cdn.bramburn.com/browseros/windows/stable/latest.json
+#ifndef BROWSEROS_UPDATE_BASE_URL
+#define BROWSEROS_UPDATE_BASE_URL "https://cdn.bramburn.com/browseros"
+#endif
+
+// Hosts permitted to serve a manifest or an installer. A manifest naming any
+// other host is rejected at parse time.
+#ifndef BROWSEROS_UPDATE_ALLOWED_HOSTS
+#define BROWSEROS_UPDATE_ALLOWED_HOSTS {"cdn.bramburn.com"}
+#endif
+
+// Arguments passed to the downloaded installer for a quiet in-place update.
+//
+// This must match what .github/workflows/release-windows.yml actually emits.
+// If the installer does not accept these the update still lands but shows UI.
+// Leaving it empty is safe: the installer runs interactively and the service
+// still confirms the outcome via Installer::IsVersionInstalled() rather than
+// trusting the exit code.
+#ifndef BROWSEROS_UPDATE_INSTALLER_ARGS
+#define BROWSEROS_UPDATE_INSTALLER_ARGS \
+  {"--silent", "--install", "--do-not-launch-browser"}
+#endif
+
+namespace browseros_update {
+namespace config {
+
+// Base URL for manifests.
+inline constexpr char kBaseUrl[] = BROWSEROS_UPDATE_BASE_URL;
+
+// Hosts the updater will fetch from. An empty list rejects every manifest,
+// which is the correct behaviour for a build with no update host.
+inline const std::vector<std::string>& GetAllowedHosts() {
+  static const std::vector<std::string> hosts = BROWSEROS_UPDATE_ALLOWED_HOSTS;
+  return hosts;
+}
+
+// Arguments for the downloaded installer.
+inline const std::vector<std::string>& GetInstallerArgs() {
+  static const std::vector<std::string> args = BROWSEROS_UPDATE_INSTALLER_ARGS;
+  return args;
+}
+
+}  // namespace config
+}  // namespace browseros_update
+
+#endif  // CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_CONFIG_H_

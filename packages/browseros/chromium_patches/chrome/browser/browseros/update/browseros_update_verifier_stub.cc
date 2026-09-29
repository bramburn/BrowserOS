diff --git a/chrome/browser/browseros/update/browseros_update_verifier_stub.cc b/chrome/browser/browseros/update/browseros_update_verifier_stub.cc
new file mode 100644
index 0000000000000..0000000000000
--- /dev/null
+++ b/chrome/browser/browseros/update/browseros_update_verifier_stub.cc
@@ -0,0 +1,29 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+// Non-Windows. Fails closed.
+//
+// The browser updater ships for Windows first; macOS is the next platform and
+// will need its own trust chain (Sparkle EdDSA or Developer ID) added here.
+// Until then, every signature check fails, which means the service can never
+// reach kReadyToInstall on a platform without an implemented trust chain.
+// Returning success here would be the dangerous choice: it would let an
+// unsigned binary through on any platform we had not thought about.
+
+#include "chrome/browser/browseros/update/browseros_update_verifier.h"
+
+#include <string>
+
+#include "base/logging.h"
+
+namespace browseros_update {
+
+VerifyResult Verifier::VerifySignature(const base::FilePath& path,
+                                       const std::string& expected_publisher) {
+  LOG(ERROR) << "BrowserOS update: no signature verifier for this platform";
+  return VerifyResult::Failure(
+      "no code-signature verifier is implemented for this platform");
+}
+
+}  // namespace browseros_update

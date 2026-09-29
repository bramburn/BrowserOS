diff --git a/chrome/browser/browseros/update/browseros_update_verifier.h b/chrome/browser/browseros/update/browseros_update_verifier.h
new file mode 100644
index 0000000000000..0000000000000
--- /dev/null
+++ b/chrome/browser/browseros/update/browseros_update_verifier.h
@@ -0,0 +1,62 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+#ifndef CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_VERIFIER_H_
+#define CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_VERIFIER_H_
+
+#include <string>
+
+#include "base/files/file_path.h"
+
+namespace browseros_update {
+
+// Outcome of verifying a downloaded installer.
+struct VerifyResult {
+  bool ok = false;
+  std::string error;
+
+  static VerifyResult Success() { return VerifyResult{true, std::string()}; }
+  static VerifyResult Failure(std::string reason) {
+    return VerifyResult{false, std::move(reason)};
+  }
+};
+
+// Verifies a downloaded installer.
+//
+// Two independent gates, both required, in this order:
+//
+//  1. SHA-256 against the value from the manifest. This catches truncation
+//     and corruption. It is NOT a security boundary: an attacker who can
+//     write to the bucket can replace the manifest and the binary together
+//     and this check still passes.
+//  2. Authenticode signature with a publisher-subject match (Windows), or a
+//     stub that always fails elsewhere. This is the gate that actually
+//     establishes provenance, which is why it is not optional and why the
+//     platform stub fails closed rather than returning success.
+//
+// |expected_publisher| may be empty, in which case a valid Authenticode
+// signature is required but the subject is not pinned.
+//
+// Blocks the calling thread; callers must not run this on the UI thread.
+// Use base::ThreadTaskRunner to hop off it.
+class Verifier {
+ public:
+  // Hashes |path| and compares against |expected_sha256| (64 lowercase hex).
+  static VerifyResult VerifySha256(const base::FilePath& path,
+                                   const std::string& expected_sha256);
+
+  // Platform signature check. Always fails on platforms with no updater.
+  static VerifyResult VerifySignature(const base::FilePath& path,
+                                      const std::string& expected_publisher);
+
+  // Runs both gates in order. |expected_sha256| and |expected_publisher| come
+  // from the validated manifest.
+  static VerifyResult Verify(const base::FilePath& path,
+                             const std::string& expected_sha256,
+                             const std::string& expected_publisher);
+};
+
+}  // namespace browseros_update
+
+#endif  // CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_VERIFIER_H_

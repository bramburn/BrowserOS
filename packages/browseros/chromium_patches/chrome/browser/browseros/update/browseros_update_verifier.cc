diff --git a/chrome/browser/browseros/update/browseros_update_verifier.cc b/chrome/browser/browseros/update/browseros_update_verifier.cc
new file mode 100644
index 0000000000000..0000000000000
--- /dev/null
+++ b/chrome/browser/browseros/update/browseros_update_verifier.cc
@@ -0,0 +1,96 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+#include "chrome/browser/browseros/update/browseros_update_verifier.h"
+
+#include <memory>
+#include <optional>
+#include <string>
+#include <utility>
+#include <vector>
+
+#include "base/files/file.h"
+#include "base/files/file_util.h"
+#include "base/logging.h"
+#include "base/memory/ref_counted.h"
+#include "base/numerics/safe_conversions.h"
+#include "crypto/hash.h"
+
+namespace browseros_update {
+
+namespace {
+
+// Chunk size for streaming the hash. Large enough that syscall overhead is
+// irrelevant, small enough to stay off the large-allocation path.
+constexpr int64_t kReadChunkSize = 64 * 1024;
+
+}  // namespace
+
+VerifyResult Verifier::VerifySha256(const base::FilePath& path,
+                                    const std::string& expected_sha256) {
+  if (expected_sha256.size() != 64) {
+    return VerifyResult::Failure("manifest sha256 is not 64 hex characters");
+  }
+
+  base::File file(path, base::File::OPEN, base::File::READ);
+  if (!file.IsValid()) {
+    return VerifyResult::Failure("could not open downloaded file");
+  }
+
+  std::unique_ptr<crypto::Hash> hasher = crypto::CreateHasher(crypto::SHA256);
+  if (!hasher) {
+    return VerifyResult::Failure("could not create SHA-256 hasher");
+  }
+
+  std::vector<uint8_t> buffer(base::checked_cast<size_t>(kReadChunkSize));
+  while (true) {
+    const int read = file.Read(buffer.data(), buffer.size());
+    if (read < 0) {
+      return VerifyResult::Failure("read error while hashing");
+    }
+    if (read == 0) {
+      break;
+    }
+    hasher->Update(buffer.data(), static_cast<size_t>(read));
+  }
+  file.Close();
+
+  std::optional<std::vector<uint8_t>> digest = hasher->Finish(/*hex_output=*/true);
+  if (!digest) {
+    return VerifyResult::Failure("SHA-256 finalization failed");
+  }
+  // crypto::Hash with hex_output=true emits lowercase hex, and the manifest
+  // is validated to be lowercase hex at parse time, so a direct string
+  // compare is correct and avoids depending on a binary-digest signature
+  // helper whose signature has changed across versions.
+  const std::string actual(digest->begin(), digest->end());
+
+  if (actual != expected_sha256) {
+    LOG(ERROR) << "BrowserOS update: SHA-256 mismatch for " << path;
+    return VerifyResult::Failure("download failed checksum verification");
+  }
+  return VerifyResult::Success();
+}
+
+VerifyResult Verifier::Verify(const base::FilePath& path,
+                              const std::string& expected_sha256,
+                              const std::string& expected_publisher) {
+  // Cheap gate first: a truncated file is rejected without paying for an
+  // Authenticode chain walk.
+  VerifyResult hash = VerifySha256(path, expected_sha256);
+  if (!hash.ok) {
+    return hash;
+  }
+
+  // Expensive gate second. This is the one that establishes provenance, so
+  // it must never be skipped, even when the manifest omits a publisher.
+  VerifyResult signature = VerifySignature(path, expected_publisher);
+  if (!signature.ok) {
+    LOG(ERROR) << "BrowserOS update: signature check failed for " << path;
+    return signature;
+  }
+  return VerifyResult::Success();
+}
+
+}  // namespace browseros_update

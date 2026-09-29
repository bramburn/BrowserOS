diff --git a/chrome/browser/browseros/update/browseros_update_downloader.h b/chrome/browser/browseros/update/browseros_update_downloader.h
new file mode 100644
index 0000000000000..0000000000000
--- /dev/null
+++ b/chrome/browser/browseros/update/browseros_update_downloader.h
@@ -0,0 +1,101 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+#ifndef CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_DOWNLOADER_H_
+#define CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_DOWNLOADER_H_
+
+#include <memory>
+#include <string>
+
+#include "base/files/file_path.h"
+#include "base/memory/raw_ptr.h"
+#include "base/memory/ref_counted.h"
+#include "base/memory/weak_ptr.h"
+#include "url/gurl.h"
+
+namespace network {
+class SimpleURLLoader;
+class URLRequestContextFactory;
+}  // namespace network
+
+namespace browseros_update {
+
+// Downloads one installer to a staging directory.
+//
+// Invariants, in order of importance:
+//
+//  1. A partially written file is never mistaken for a complete one. The
+//     download lands on "<name>.partial" and is renamed to "<name>" only
+//     after the byte count matches the manifest.
+//  2. An interrupted download cannot poison a later one. Stale ".partial"
+//     files are removed on construction rather than resumed, because a
+//     resumed transfer into a file whose provenance we cannot re-establish
+//     is a downgrade hazard.
+//  3. Nothing outside |staging_dir| is written.
+//
+// This is not a resumable downloader. Re-adding resume later means issuing a
+// Range request through net::URLRequestFactory (SimpleURLLoader cannot append)
+// and re-verifying the full-file sha256 before the resumed bytes are trusted;
+// the size check below already catches the common case.
+class Downloader {
+ public:
+  class Delegate {
+   public:
+    virtual ~Delegate() = default;
+
+    // |received| and |total| are bytes. |total| may be -1 when the server
+    // does not send a content length, in which case the UI shows an
+    // indeterminate progress state.
+    virtual void OnDownloadProgress(int64_t received, int64_t total) = 0;
+
+    // |path| is the fully written, size-checked file. The delegate owns it
+    // from this point and is responsible for deleting it if verification
+    // fails.
+    virtual void OnDownloadComplete(const base::FilePath& path) = 0;
+
+    virtual void OnDownloadFailed(const std::string& error) = 0;
+  };
+
+  Downloader(Delegate* delegate,
+             scoped_refptr<network::URLRequestContextFactory> url_request_context_factory);
+  ~Downloader();
+
+  Downloader(const Downloader&) = delete;
+  Downloader& operator=(const Downloader&) = delete;
+
+  // Starts a download of |url| into |staging_dir| as |filename|.
+  //
+  // |expected_size| comes from the manifest and must match the transferred
+  // byte count exactly; a mismatch is a failure, not a warning, because a
+  // truncated installer must never reach the verifier.
+  void Start(const GURL& url,
+             const base::FilePath& staging_dir,
+             const std::string& filename,
+             int64_t expected_size);
+
+  // Cancels any in-flight download. Safe to call when idle.
+  void Cancel();
+
+  bool is_downloading() const { return is_downloading_; }
+
+ private:
+  void OnResponseReceived(const base::FilePath& partial_path);
+  void ReportProgress(int64_t received, int64_t total);
+  void CleanUpPartial();
+
+  raw_ptr<Delegate> delegate_;
+  scoped_refptr<network::URLRequestContextFactory> url_request_context_factory_;
+  std::unique_ptr<network::SimpleURLLoader> loader_;
+  base::FilePath partial_path_;
+  base::FilePath final_path_;
+  int64_t expected_size_ = 0;
+  int64_t received_ = 0;
+  bool is_downloading_ = false;
+
+  base::WeakPtrFactory<Downloader> weak_factory_{this};
+};
+
+}  // namespace browseros_update
+
+#endif  // CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_DOWNLOADER_H_

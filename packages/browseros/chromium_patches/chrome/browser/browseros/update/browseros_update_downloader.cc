diff --git a/chrome/browser/browseros/update/browseros_update_downloader.cc b/chrome/browser/browseros/update/browseros_update_downloader.cc
new file mode 100644
index 0000000000000..0000000000000
--- /dev/null
+++ b/chrome/browser/browseros/update/browseros_update_downloader.cc
@@ -0,0 +1,164 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+#include "chrome/browser/browseros/update/browseros_update_downloader.h"
+
+#include <string>
+#include <utility>
+
+#include "base/files/file.h"
+#include "base/files/file_util.h"
+#include "base/logging.h"
+#include "base/memory/weak_ptr.h"
+#include "chrome/browser/browseros/update/browseros_update_downloader.h"
+#include "net/traffic_annotation/network_traffic_annotation.h"
+#include "url/gurl.h"
+
+namespace browseros_update {
+
+namespace {
+
+constexpr char kPartialSuffix[] = ".partial";
+
+// The updater talks only to the manifest's host, which Manifest::Parse()
+// allow-lists. The annotation records that no browser state is attached.
+constexpr net::NetworkTrafficAnnotationTag kUpdateTrafficAnnotation =
+    net::NetworkTrafficAnnotationTag(
+        "BrowserOS update download",
+        R"(
+          semantically {
+            destination: OTHER
+            description: "Downloads a BrowserOS browser update installer."
+          }
+          policy {
+            cookies_allowed: NO
+            set_cookie: NO
+            policy: kBrowserUpdateEnabled
+            policy_description: "The update service, not a page, initiates this."
+          })");
+
+}  // namespace
+
+Downloader::Downloader(
+    Delegate* delegate,
+    scoped_refptr<network::URLRequestContextFactory> url_request_context_factory)
+    : delegate_(delegate),
+      url_request_context_factory_(std::move(url_request_context_factory)) {}
+
+Downloader::~Downloader() {
+  // Drop the loader without calling back into the delegate: the owner may
+  // already have been destroyed.
+  loader_.reset();
+}
+
+void Downloader::Start(const GURL& url,
+                       const base::FilePath& staging_dir,
+                       const std::string& filename,
+                       int64_t expected_size) {
+  if (is_downloading_) {
+    delegate_->OnDownloadFailed("download already in progress");
+    return;
+  }
+  if (!url.is_valid() || !url.SchemeIs("https")) {
+    delegate_->OnDownloadFailed("refusing non-https download");
+    return;
+  }
+  if (expected_size <= 0) {
+    delegate_->OnDownloadFailed("refusing download with unknown size");
+    return;
+  }
+
+  if (!base::CreateDirectory(staging_dir)) {
+    delegate_->OnDownloadFailed("could not create staging directory");
+    return;
+  }
+
+  final_path_ = staging_dir.AppendASCII(filename);
+  partial_path_ = staging_dir.AppendASCII(filename + kPartialSuffix);
+
+  // Never resume into a file left by a previous attempt. The installer URL
+  // embeds the version, so it is immutable and worth caching at the CDN, but
+  // a stale local file is a downgrade hazard we cannot re-verify cheaply.
+  CleanUpPartial();
+
+  loader_ = network::SimpleURLLoader::Create(
+      kUpdateTrafficAnnotation, url_request_context_factory_,
+      base::BindOnce(&Downloader::OnResponseReceived, weak_factory_.GetWeakPtr()));
+  if (!loader_) {
+    delegate_->OnDownloadFailed("could not create download");
+    return;
+  }
+
+  loader_->SetDownloadToFile(partial_path_, base::File::PATTERN_NORMAL,
+                             base::File::ShowInfoInFolder::kNo,
+                             base::OnceClosure());
+  loader_->SetReportByteCount(true);
+  loader_->SetProgressThrottleInterval(base::Seconds(1));
+  // Do not let a stopped network silently resume a download that we already
+  // consider failed.
+  loader_->SetAllowNetworkRequestOnDownloadStopped(false);
+
+  is_downloading_ = true;
+  received_ = 0;
+
+  loader_->Start(base::BindRepeating(&Downloader::ReportProgress,
+                                     weak_factory_.GetWeakPtr()));
+}
+
+void Downloader::Cancel() {
+  if (loader_) {
+    loader_->Cancel();
+    loader_.reset();
+  }
+  is_downloading_ = false;
+  CleanUpPartial();
+}
+
+void Downloader::ReportProgress(int64_t received, int64_t total) {
+  received_ = received;
+  delegate_->OnDownloadProgress(received, total);
+}
+
+void Downloader::OnResponseReceived(const base::FilePath& partial_path) {
+  is_downloading_ = false;
+  loader_.reset();
+
+  if (partial_path.empty()) {
+    CleanUpPartial();
+    delegate_->OnDownloadFailed("download failed");
+    return;
+  }
+
+  // Size is the cheap integrity gate. It runs before sha256 so a truncated
+  // transfer never costs a full-file hash.
+  int64_t size = -1;
+  if (!base::GetFileSize(partial_path, &size)) {
+    CleanUpPartial();
+    delegate_->OnDownloadFailed("could not stat downloaded file");
+    return;
+  }
+  if (size != expected_size_) {
+    LOG(ERROR) << "BrowserOS update: size mismatch, expected "
+               << expected_size_ << " got " << size;
+    CleanUpPartial();
+    delegate_->OnDownloadFailed("downloaded file has the wrong size");
+    return;
+  }
+
+  if (!base::Move(partial_path, final_path_)) {
+    CleanUpPartial();
+    delegate_->OnDownloadFailed("could not move downloaded file into place");
+    return;
+  }
+
+  delegate_->OnDownloadComplete(final_path_);
+}
+
+void Downloader::CleanUpPartial() {
+  if (!partial_path_.empty()) {
+    base::DeleteFile(partial_path_);
+  }
+}
+
+}  // namespace browseros_update

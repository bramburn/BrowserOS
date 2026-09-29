diff --git a/chrome/browser/browseros/update/browseros_update_proxy.cc b/chrome/browser/browseros/update/browseros_update_proxy.cc
new file mode 100644
index 0000000000000..0000000000000
--- /dev/null
+++ b/chrome/browser/browseros/update/browseros_update_proxy.cc
@@ -0,0 +1,105 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+#include "chrome/browser/browseros/update/browseros_update_proxy.h"
+
+#include <string>
+#include <utility>
+
+#include "base/logging.h"
+#include "base/version.h"
+#include "chrome/browser/browseros/update/browseros_update_service.h"
+
+namespace browseros_update {
+
+namespace {
+
+// Maps a service state onto the string the page switches on. Kept as a
+// function rather than reusing the enum name so the WebUI contract is stable
+// even if the internal enum is reordered.
+std::string StateKindToString(UpdateService::State state) {
+  switch (state) {
+    case UpdateService::State::kIdle:
+      return "idle";
+    case UpdateService::State::kChecking:
+      return "checking";
+    case UpdateService::State::kUpdateAvailable:
+      return "update-available";
+    case UpdateService::State::kDownloading:
+      return "downloading";
+    case UpdateService::State::kVerifying:
+      return "verifying";
+    case UpdateService::State::kReadyToInstall:
+      return "ready-to-install";
+    case UpdateService::State::kInstalling:
+      return "installing";
+    case UpdateService::State::kAwaitingRestart:
+      return "awaiting-restart";
+    case UpdateService::State::kPaused:
+      return "paused";
+    case UpdateService::State::kNotSupported:
+      return "not-supported";
+    case UpdateService::State::kFailed:
+      return "failed";
+  }
+  return "idle";
+}
+
+mojom::BrowserOSUpdateStatePtr ToMojoState(const UpdateService::StateSnapshot& s) {
+  auto state = mojom::BrowserOSUpdateState::New();
+  state->state = StateKindToString(s.state);
+  state->current_version = s.current_version;
+  state->available_version = s.available_version;
+  state->channel = s.channel;
+  state->notes_url = s.notes_url;
+  state->error = s.error;
+  state->download_progress = s.download_progress;
+  return state;
+}
+
+}  // namespace
+
+// static
+mojo::PendingRemote<mojom::BrowserOSUpdateHandler> UpdateProxy::Create() {
+  return mojo::MakeReceiverWithRemote<mojom::BrowserOSUpdateHandler,
+                                      UpdateProxy>(
+      mojo::PendingRemote<mojom::BrowserOSUpdateHandler>());
+}
+
+UpdateProxy::UpdateProxy() {
+  UpdateService::GetInstance()->AddObserver(this);
+}
+
+UpdateProxy::~UpdateProxy() {
+  UpdateService::GetInstance()->RemoveObserver(this);
+}
+
+void UpdateProxy::PushState(const StateSnapshot& snapshot) {
+  if (!receiver_.is_bound()) {
+    return;
+  }
+  receiver_->NotifyUpdateState(ToMojoState(snapshot));
+}
+
+void UpdateProxy::OnUpdateStateChanged(const StateSnapshot& snapshot) {
+  PushState(snapshot);
+}
+
+void UpdateProxy::GetUpdateState(GetUpdateStateCallback callback) {
+  std::move(callback).Run(ToMojoState(UpdateService::GetInstance()->GetSnapshot()));
+}
+
+void UpdateProxy::CheckForUpdate() {
+  UpdateService::GetInstance()->CheckNow();
+}
+
+void UpdateProxy::TriggerInstall() {
+  UpdateService::GetInstance()->TriggerInstall();
+}
+
+void UpdateProxy::SkipVersion(const std::string& version) {
+  UpdateService::GetInstance()->SkipVersion(base::Version(version));
+}
+
+}  // namespace browseros_update

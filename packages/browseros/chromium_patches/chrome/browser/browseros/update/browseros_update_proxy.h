diff --git a/chrome/browser/browseros/update/browseros_update_proxy.h b/chrome/browser/browseros/update/browseros_update_proxy.h
new file mode 100644
index 0000000000000..0000000000000
--- /dev/null
+++ b/chrome/browser/browseros/update/browseros_update_proxy.h
@@ -0,0 +1,61 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+#ifndef CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_PROXY_H_
+#define CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_PROXY_H_
+
+#include <string>
+
+#include "chrome/browser/browseros/update/browseros_update.mojom.h"
+#include "chrome/browser/browseros/update/browseros_update_service.h"
+#include "mojo/public/cpp/bindings/remote.h"
+#include "mojo/public/mojom/message_pipe.h"
+
+namespace browseros_update {
+
+// Browser-side endpoint of the BrowserOSUpdateHandler interface.
+//
+// The browser implements the interface; the About page is the client and
+// receives a PendingRemote from the WebUI host. This class is a thin adapter:
+// it forwards requests to UpdateService and mirrors service state changes back
+// to the page. It owns no update state of its own.
+class UpdateProxy : public UpdateService::Observer,
+                    public mojom::BrowserOSUpdateHandler {
+ public:
+  // Builds a receiver/remote pair for the WebUI host to hand to the page.
+  //
+  // Ownership: the returned receiver is the browser-side implementation. The
+  // remote must be passed to the WebUI, which forwards it to the page. The
+  // handler unregisters itself from the service when the pipe closes, so a
+  // navigating-away About page does not leak an observer.
+  static mojo::PendingRemote<mojom::BrowserOSUpdateHandler> Create();
+
+  UpdateProxy(const UpdateProxy&) = delete;
+  UpdateProxy& operator=(const UpdateProxy&) = delete;
+
+  // UpdateService::Observer:
+  void OnUpdateStateChanged(const StateSnapshot& snapshot) override;
+
+  // mojom::BrowserOSUpdateHandler:
+  void GetUpdateState(GetUpdateStateCallback callback) override;
+  void CheckForUpdate() override;
+  void TriggerInstall() override;
+  void SkipVersion(const std::string& version) override;
+
+ protected:
+  // Called when the page's pipe closes.
+  virtual void OnConnectionLost() {}
+
+ private:
+  UpdateProxy();
+  ~UpdateProxy() override;
+
+  void PushState(const StateSnapshot& snapshot);
+
+  mojo::Remote<mojom::BrowserOSUpdateHandler> receiver_;
+};
+
+}  // namespace browseros_update
+
+#endif  // CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_PROXY_H_

diff --git a/chrome/browser/browseros/update/browseros_update_service.h b/chrome/browser/browseros/update/browseros_update_service.h
new file mode 100644
index 0000000000000..0000000000000
--- /dev/null
+++ b/chrome/browser/browseros/update/browseros_update_service.h
@@ -0,0 +1,174 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+#ifndef CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_SERVICE_H_
+#define CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_SERVICE_H_
+
+#include <memory>
+#include <optional>
+#include <string>
+
+#include "base/files/file_path.h"
+#include "base/memory/raw_ptr.h"
+#include "base/memory/weak_ptr.h"
+#include "base/observer_list.h"
+#include "base/time/time.h"
+#include "base/timer/timer.h"
+#include "base/version.h"
+#include "chrome/browser/browseros/update/browseros_update_downloader.h"
+#include "chrome/browser/browseros/update/browseros_update_manifest.h"
+#include "chrome/browser/browseros/update/browseros_update_verifier.h"
+#include "url/gurl.h"
+
+class PrefService;
+namespace network {
+class SimpleURLLoader;
+class URLRequestContextFactory;
+}  // namespace network
+
+namespace browseros_update {
+
+// Owns the browser's own update state machine.
+//
+// Chromium's built-in updater is compiled out of unbranded builds
+// (chrome/browser/buildflags.gni: enable_updater = is_chrome_branded), so
+// this is the whole mechanism. See docs/UPDATE_DESIGN.md for why the Omaha
+// contract was abandoned.
+//
+// Ownership: this class is the single owner of update state. The Mojo proxy
+// subscribes as an Observer and never mutates anything but through the public
+// methods here.
+//
+// Threading: every method must be called on the UI thread. Verification and
+// installation hop to a background thread and return via weak pointers.
+class UpdateService : public Downloader::Delegate {
+ public:
+  enum class State {
+    kIdle,
+    kChecking,
+    kUpdateAvailable,
+    kDownloading,
+    kVerifying,
+    kReadyToInstall,
+    kInstalling,
+    kAwaitingRestart,
+    kPaused,
+    kNotSupported,
+    kFailed,
+  };
+
+  // Everything the About page needs to render its card. Cheap to copy.
+  struct StateSnapshot {
+    State state = State::kIdle;
+    std::string current_version;
+    std::string available_version;
+    std::string channel;
+    std::string notes_url;
+    std::string error;
+    // 0..100, or -1 when indeterminate.
+    int download_progress = -1;
+  };
+
+  class Observer {
+   public:
+    virtual ~Observer() = default;
+    virtual void OnUpdateStateChanged(const StateSnapshot& snapshot) = 0;
+  };
+
+  // Returns the process-wide instance. Never null, but the instance is inert
+  // until Initialize() runs, so call that during startup.
+  static UpdateService* GetInstance();
+
+  // Must be called once, on the UI thread, before Start().
+  //
+  // |local_state| holds the update prefs and outlives the service.
+  // |url_request_context_factory| must be the browser context's, so cookies
+  // and proxy settings match the rest of the browser.
+  void Initialize(PrefService* local_state,
+                  scoped_refptr<network::URLRequestContextFactory>
+                      url_request_context_factory);
+
+  // Begins the periodic check and runs crash recovery. Idempotent.
+  void Start();
+
+  // Shuts down timers and cancels in-flight work. The observer list is not
+  // cleared; the browser is going away.
+  void Shutdown();
+
+  // User-initiated check, from the About page. Bypasses the interval gate but
+  // never starts a second concurrent check.
+  void CheckNow();
+
+  // Applies a staged update. Only valid in kReadyToInstall.
+  void TriggerInstall();
+
+  // Records the version the user chose to skip. A newer version than this is
+  // still offered.
+  void SkipVersion(const base::Version& version);
+
+  StateSnapshot GetSnapshot() const { return snapshot_; }
+  base::Version GetCurrentVersion() const { return current_version_; }
+
+  void AddObserver(Observer* observer);
+  void RemoveObserver(Observer* observer);
+
+  // Downloader::Delegate:
+  void OnDownloadProgress(int64_t received, int64_t total) override;
+  void OnDownloadComplete(const base::FilePath& path) override;
+  void OnDownloadFailed(const std::string& error) override;
+
+  UpdateService(const UpdateService&) = delete;
+  UpdateService& operator=(const UpdateService&) = delete;
+
+ private:
+  UpdateService();
+  ~UpdateService() override;
+
+  static const char* StateToString(State state);
+
+  void OnCheckTimer();
+  bool ShouldCheckNow() const;
+  void RecordCheckTime();
+
+  GURL BuildManifestUrl() const;
+  void FetchManifest();
+  void OnManifestFetched(std::optional<std::string> body);
+
+  void BeginDownload(const Manifest& manifest);
+  void BeginVerify();
+  void OnVerifyComplete(VerifyResult result);
+  void BeginInstall();
+  void OnInstallComplete(bool success, const std::string& error);
+
+  // Clears kUpdateStagedVersion when the staged version is now running, which
+  // is how an interrupted install is detected on the next launch.
+  void RecoverFromInterruptedInstall();
+
+  void SetState(State state);
+  void SetError(const std::string& error);
+  void Publish();
+
+  base::FilePath GetStagingDir() const;
+
+  raw_ptr<PrefService> local_state_ = nullptr;
+  scoped_refptr<network::URLRequestContextFactory> url_request_context_factory_;
+  std::unique_ptr<network::SimpleURLLoader> manifest_loader_;
+  std::unique_ptr<Downloader> downloader_;
+
+  base::RepeatingTimer check_timer_;
+  base::Version current_version_;
+
+  // Held between kUpdateAvailable and kReadyToInstall.
+  std::optional<Manifest> manifest_;
+
+  State state_ = State::kIdle;
+  StateSnapshot snapshot_;
+
+  base::ObserverList<Observer> observers_;
+  base::WeakPtrFactory<UpdateService> weak_factory_{this};
+};
+
+}  // namespace browseros_update
+
+#endif  // CHROME_BROWSER_BROWSEROS_UPDATE_BROWSEROS_UPDATE_SERVICE_H_

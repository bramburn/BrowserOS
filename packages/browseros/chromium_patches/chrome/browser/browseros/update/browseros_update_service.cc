diff --git a/chrome/browser/browseros/update/browseros_update_service.cc b/chrome/browser/browseros/update/browseros_update_service.cc
new file mode 100644
index 0000000000000..0000000000000
--- /dev/null
+++ b/chrome/browser/browseros/update/browseros_update_service.cc
@@ -0,0 +1,516 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+#include "chrome/browser/browseros/update/browseros_update_service.h"
+
+#include <memory>
+#include <optional>
+#include <string>
+#include <utility>
+
+#include "base/functional/bind.h"
+#include "base/location.h"
+#include "base/logging.h"
+#include "base/memory/weak_ptr.h"
+#include "base/path_service.h"
+#include "base/strings/string_number_conversions.h"
+#include "base/task/single_thread_task_runner.h"
+#include "base/task/thread_pool.h"
+#include "base/time/time.h"
+#include "chrome/browser/browseros/update/browseros_update_checker.h"
+#include "chrome/browser/browseros/update/browseros_update_config.h"
+#include "chrome/browser/browseros/update/browseros_update_installer.h"
+#include "chrome/browser/browseros/update/browseros_update_prefs.h"
+#include "chrome/browser/browseros/update/browseros_update_verifier.h"
+#include "chrome/common/chrome_paths.h"
+#include "chrome/common/version.h"
+#include "net/traffic_annotation/network_traffic_annotation.h"
+#include "url/gurl.h"
+
+namespace browseros_update {
+
+namespace {
+
+constexpr char kPlatformDirWindows[] = "windows";
+constexpr char kPlatformDirMac[] = "mac";
+
+// Bounds the one long blocking step. A wedged installer must not leave the
+// browser stuck in kInstalling forever.
+constexpr base::TimeDelta kInstallTimeout = base::Minutes(10);
+
+// Manifests are small and must not be cached, so that publishing a new one
+// takes effect on the next poll rather than whenever the CDN feels like it.
+constexpr net::NetworkTrafficAnnotationTag kManifestTrafficAnnotation =
+    net::NetworkTrafficAnnotationTag(
+        "BrowserOS update manifest fetch",
+        R"(
+          semantically {
+            destination: OTHER
+            description: "Fetches the BrowserOS update manifest."
+          }
+          policy {
+            cookies_allowed: NO
+            set_cookie: NO
+            policy: kBrowserUpdateEnabled
+            policy_description: "The update service, not a page, initiates this."
+          })");
+
+// "windows" / "mac", matching the directory the manifest generator writes.
+const char* PlatformDir() {
+  const std::string key = Manifest::CurrentPlatformKey();
+  if (key == "windows-x64") {
+    return kPlatformDirWindows;
+  }
+  if (key == "mac-arm64") {
+    return kPlatformDirMac;
+  }
+  return nullptr;
+}
+
+}  // namespace
+
+// static
+UpdateService* UpdateService::GetInstance() {
+  static UpdateService* instance = new UpdateService();
+  return instance;
+}
+
+UpdateService::UpdateService() {
+  current_version_ = version_info::GetVersion();
+}
+
+UpdateService::~UpdateService() {
+  Shutdown();
+}
+
+void UpdateService::Initialize(
+    PrefService* local_state,
+    scoped_refptr<network::URLRequestContextFactory>
+        url_request_context_factory) {
+  local_state_ = local_state;
+  url_request_context_factory_ = std::move(url_request_context_factory);
+  downloader_ = std::make_unique<Downloader>(this, url_request_context_factory_);
+}
+
+void UpdateService::Start() {
+  if (!local_state_ || !downloader_) {
+    LOG(ERROR) << "BrowserOS update: Start() before Initialize()";
+    return;
+  }
+  if (check_timer_.IsRunning()) {
+    return;
+  }
+
+  RecoverFromInterruptedInstall();
+
+  const std::string platform = Manifest::CurrentPlatformKey();
+  if (platform.empty()) {
+    SetState(State::kNotSupported);
+    return;
+  }
+
+  snapshot_.current_version = current_version_.ToString();
+  snapshot_.channel = local_state_->GetString(kUpdateChannel);
+
+  check_timer_.StartRepeating(
+      base::Hours(kDefaultUpdateCheckIntervalHours),
+      base::BindRepeating(&UpdateService::OnCheckTimer, weak_factory_.GetWeakPtr()));
+
+  // Check shortly after startup rather than waiting a full interval, but only
+  // if the last check is old enough. A user who restarts the browser twice in
+  // an afternoon should not ping the CDN twice in an afternoon.
+  if (ShouldCheckNow()) {
+    check_timer_.Start(base::Seconds(30), base::BindOnce(&UpdateService::OnCheckTimer,
+                                                         weak_factory_.GetWeakPtr()));
+  }
+}
+
+void UpdateService::Shutdown() {
+  check_timer_.Stop();
+  if (manifest_loader_) {
+    manifest_loader_->Cancel();
+    manifest_loader_.reset();
+  }
+  if (downloader_) {
+    downloader_->Cancel();
+  }
+}
+
+const char* UpdateService::StateToString(State state) {
+  switch (state) {
+    case State::kIdle:
+      return "idle";
+    case State::kChecking:
+      return "checking";
+    case State::kUpdateAvailable:
+      return "update-available";
+    case State::kDownloading:
+      return "downloading";
+    case State::kVerifying:
+      return "verifying";
+    case State::kReadyToInstall:
+      return "ready-to-install";
+    case State::kInstalling:
+      return "installing";
+    case State::kAwaitingRestart:
+      return "awaiting-restart";
+    case State::kPaused:
+      return "paused";
+    case State::kNotSupported:
+      return "not-supported";
+    case State::kFailed:
+      return "failed";
+  }
+  return "unknown";
+}
+
+bool UpdateService::ShouldCheckNow() const {
+  if (!local_state_->GetBoolean(kUpdateAutoCheckEnabled)) {
+    return false;
+  }
+  const double last = local_state_->GetDouble(kUpdateLastCheckTime);
+  if (last <= 0.0) {
+    return true;
+  }
+  const base::Time last_time = base::Time::FromSecondsSinceUnixEpoch(
+      static_cast<int64_t>(last));
+  return base::Time::Now() - last_time >=
+         base::Hours(kDefaultUpdateCheckIntervalHours);
+}
+
+void UpdateService::RecordCheckTime() {
+  if (local_state_) {
+    local_state_->SetDouble(
+        kUpdateLastCheckTime,
+        static_cast<double>(base::Time::ToSecondsSinceUnixEpoch(base::Time::Now())));
+  }
+}
+
+void UpdateService::OnCheckTimer() {
+  if (state_ == State::kChecking || state_ == State::kDownloading ||
+      state_ == State::kVerifying || state_ == State::kInstalling) {
+    // Never run a check while one is already in flight. This is also what
+    // stops the About page from spawning a second one.
+    return;
+  }
+  if (!local_state_->GetBoolean(kUpdateAutoCheckEnabled)) {
+    return;
+  }
+  FetchManifest();
+}
+
+void UpdateService::CheckNow() {
+  if (state_ == State::kChecking || state_ == State::kDownloading ||
+      state_ == State::kVerifying || state_ == State::kInstalling) {
+    return;
+  }
+  FetchManifest();
+}
+
+GURL UpdateService::BuildManifestUrl() const {
+  const char* dir = PlatformDir();
+  if (!dir) {
+    return GURL();
+  }
+  const std::string channel = local_state_->GetString(kUpdateChannel);
+  return GURL(std::string(config::kBaseUrl) + "/" + dir + "/" + channel +
+              "/latest.json");
+}
+
+void UpdateService::FetchManifest() {
+  const GURL url = BuildManifestUrl();
+  if (!url.is_valid()) {
+    SetState(State::kNotSupported);
+    return;
+  }
+
+  SetState(State::kChecking);
+
+  // A no-cache loader: a cached manifest would keep serving a version we have
+  // already rejected, or one the operator has just paused.
+  auto loader = network::SimpleURLLoader::Create(
+      kManifestTrafficAnnotation, url_request_context_factory_,
+      base::BindOnce(
+          &UpdateService::OnManifestFetched, weak_factory_.GetWeakPtr()));
+  if (!loader) {
+    SetError("could not create the manifest request");
+    return;
+  }
+  loader->SetAllowCache(false);
+  loader->SetMaxBodySize(256 * 1024);
+
+  manifest_loader_ = std::move(loader);
+  manifest_loader_->Start();
+}
+
+void UpdateService::OnManifestFetched(std::optional<std::string> body) {
+  manifest_loader_.reset();
+  RecordCheckTime();
+
+  if (!body) {
+    SetError("couldn't reach the update server");
+    return;
+  }
+
+  std::optional<Manifest> manifest =
+      Manifest::Parse(*body, config::GetAllowedHosts());
+  if (!manifest) {
+    SetError("the update manifest was not valid");
+    return;
+  }
+
+  const std::string skipped = local_state_->GetString(kUpdateSkippedVersion);
+  const UpdateDecision decision =
+      DecideUpdate(current_version_, manifest->version(), manifest->IsPaused(), skipped);
+
+  snapshot_.available_version = manifest->version().ToString();
+  snapshot_.channel = manifest->channel();
+  snapshot_.notes_url = manifest->notes_url();
+
+  switch (decision) {
+    case UpdateDecision::kPaused:
+      SetState(State::kPaused);
+      return;
+    case UpdateDecision::kUpToDate:
+      manifest_.reset();
+      SetState(State::kIdle);
+      return;
+    case UpdateDecision::kRollbackBlocked:
+      LOG(WARNING) << "BrowserOS update: refusing manifest for older version "
+                   << manifest->version() << " (running "
+                   << current_version_ << ")";
+      SetError("the update server offered an older version; refusing it");
+      return;
+    case UpdateDecision::kSkippedByUser:
+      manifest_.reset();
+      SetState(State::kIdle);
+      return;
+    case UpdateDecision::kNotSupported:
+      SetState(State::kNotSupported);
+      return;
+    case UpdateDecision::kInvalidVersion:
+      SetError("the update manifest had an unusable version");
+      return;
+    case UpdateDecision::kUpdateAvailable:
+      break;
+  }
+
+  manifest_ = std::move(manifest);
+  SetError(std::string());
+  BeginDownload(*manifest_);
+}
+
+base::FilePath UpdateService::GetStagingDir() const {
+  return base::PathService::Get(chrome::DIR_USER_DATA)
+      .AppendASCII("BrowserOSUpdate")
+      .AppendASCII("staging");
+}
+
+void UpdateService::BeginDownload(const Manifest& manifest) {
+  const PlatformEntry* entry = manifest.GetPlatformForCurrentBuild();
+  if (!entry) {
+    SetError("the manifest had no entry for this platform");
+    return;
+  }
+
+  // Extract the filename from the validated URL and use it as the staging
+  // filename. The URL passed the host allowlist, so this cannot point outside
+  // the staging directory, but strip any path separators anyway rather than
+  // trusting that.
+  std::string filename = entry->url.ExtractFileName();
+  if (filename.empty() ||
+      filename.find('/') != std::string::npos ||
+      filename.find('\\') != std::string::npos) {
+    SetError("the manifest had an unusable installer filename");
+    return;
+  }
+
+  SetState(State::kDownloading);
+  downloader_->Start(entry->url, GetStagingDir(), filename, entry->size);
+}
+
+void UpdateService::OnDownloadProgress(int64_t received, int64_t total) {
+  if (state_ != State::kDownloading) {
+    return;
+  }
+  snapshot_.download_progress =
+      total > 0 ? static_cast<int>((received * 100) / total) : -1;
+  Publish();
+}
+
+void UpdateService::OnDownloadComplete(const base::FilePath& path) {
+  snapshot_.download_progress = 100;
+  BeginVerify();
+
+  const PlatformEntry* entry =
+      manifest_ ? manifest_->GetPlatformForCurrentBuild() : nullptr;
+  if (!entry) {
+    base::DeleteFile(path);
+    SetError("the manifest changed underneath the download");
+    return;
+  }
+
+  // Hashing and Authenticode both walk the whole file; neither belongs on the
+  // UI thread. Post to the blocking pool, then hop back to the UI thread to
+  // publish the result.
+  base::ThreadPool::PostTask(
+      FROM_HERE, base::BindOnce(
+                     [](base::WeakPtr<UpdateService> weak, base::FilePath file,
+                        std::string sha256, std::string publisher) {
+                       VerifyResult result = Verifier::Verify(file, sha256, publisher);
+                       // The payload is ours to clean up either way: on
+                       // success the installer needs it, so only delete when
+                       // verification failed.
+                       if (!result.ok) {
+                         base::DeleteFile(file);
+                       }
+                       base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
+                           FROM_HERE,
+                           base::BindOnce(&UpdateService::OnVerifyComplete,
+                                          weak, result));
+                     },
+                     weak_factory_.GetWeakPtr(), path, entry->sha256,
+                     entry->publisher));
+}
+
+void UpdateService::BeginVerify() {
+  SetState(State::kVerifying);
+}
+
+void UpdateService::OnVerifyComplete(VerifyResult result) {
+  if (!result.ok) {
+    SetError(result.error);
+    return;
+  }
+
+  if (!manifest_) {
+    SetError("the manifest disappeared before install");
+    return;
+  }
+
+  // Remember that a verified payload exists so an interrupted install is
+  // detectable next launch.
+  local_state_->SetString(kUpdateStagedVersion, manifest_->version().ToString());
+  SetState(State::kReadyToInstall);
+}
+
+void UpdateService::TriggerInstall() {
+  if (state_ != State::kReadyToInstall || !manifest_) {
+    LOG(WARNING) << "BrowserOS update: TriggerInstall() in state "
+                 << StateToString(state_);
+    return;
+  }
+
+  const base::Version version = manifest_->version();
+  const std::string filename = manifest_->GetPlatformForCurrentBuild()
+                                   ->url.ExtractFileName();
+  const base::FilePath installer = GetStagingDir().AppendASCII(filename);
+
+  if (!base::PathExists(installer)) {
+    SetError("the staged installer is missing; check for updates again");
+    return;
+  }
+
+  SetState(State::kInstalling);
+
+  // Launching the installer blocks for as long as it takes; keep it off the
+  // UI thread and come back to it to publish the outcome.
+  base::ThreadPool::PostTask(
+      FROM_HERE, base::BindOnce(
+                     [](base::WeakPtr<UpdateService> weak, base::FilePath path,
+                        base::Version v) {
+                       std::string error;
+                       const bool ok =
+                           Installer::ApplyBlocking(path, v, kInstallTimeout, &error);
+                       base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
+                           FROM_HERE,
+                           base::BindOnce(&UpdateService::OnInstallComplete, weak,
+                                          ok, error));
+                     },
+                     weak_factory_.GetWeakPtr(), installer, version));
+}
+
+void UpdateService::OnInstallComplete(bool success, const std::string& error) {
+  if (!success) {
+    // The old installation is untouched, so clearing the staged marker is
+    // safe: the next check will simply fetch again.
+    local_state_->SetString(kUpdateStagedVersion, std::string());
+    SetError(error.empty() ? "the update could not be installed" : error);
+    return;
+  }
+
+  SetState(State::kAwaitingRestart);
+  SetError(std::string());
+}
+
+void UpdateService::RecoverFromInterruptedInstall() {
+  const std::string staged = local_state_->GetString(kUpdateStagedVersion);
+  if (staged.empty()) {
+    return;
+  }
+
+  if (current_version_ == base::Version(staged)) {
+    // The install completed; we are running it now.
+    local_state_->SetString(kUpdateStagedVersion, std::string());
+    return;
+  }
+
+  // Staged but not applied. The payload may be gone, so drop the marker and
+  // let the next check restage. Never try to install it from here: doing so
+  // during startup, before the profile is ready, is how a browser ends up
+  // in a half-updated state.
+  LOG(WARNING) << "BrowserOS update: found an unapplied staged version "
+               << staged << "; clearing it";
+  local_state_->SetString(kUpdateStagedVersion, std::string());
+}
+
+void UpdateService::SkipVersion(const base::Version& version) {
+  if (!version.IsValid()) {
+    return;
+  }
+  local_state_->SetString(kUpdateSkippedVersion, version.ToString());
+  manifest_.reset();
+  SetState(State::kIdle);
+}
+
+void UpdateService::SetState(State state) {
+  state_ = state;
+  LOG(INFO) << "BrowserOS update: state -> " << StateToString(state);
+  if (state == State::kPaused) {
+    snapshot_.download_progress = -1;
+  }
+  Publish();
+}
+
+void UpdateService::SetError(const std::string& error) {
+  snapshot_.error = error;
+  if (!error.empty() && local_state_) {
+    local_state_->SetString(kUpdateLastError, error);
+  }
+  // An error always resolves to a settled state, never a busy one, so the
+  // About page never sits on a spinner forever.
+  if (state_ != State::kPaused) {
+    state_ = State::kFailed;
+  }
+  snapshot_.download_progress = -1;
+  Publish();
+}
+
+void UpdateService::Publish() {
+  snapshot_.state = state_;
+  snapshot_.current_version = current_version_.ToString();
+  for (Observer& observer : observers_) {
+    observer.OnUpdateStateChanged(snapshot_);
+  }
+}
+
+void UpdateService::AddObserver(Observer* observer) {
+  observers_.AddObserver(observer);
+}
+
+void UpdateService::RemoveObserver(Observer* observer) {
+  observers_.RemoveObserver(observer);
+}
+
+}  // namespace browseros_update

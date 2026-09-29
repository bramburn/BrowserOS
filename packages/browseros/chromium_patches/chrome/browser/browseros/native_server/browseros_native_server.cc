diff --git a/chrome/browser/browseros/native_server/browseros_native_server.cc b/chrome/browser/browseros/native_server/browseros_native_server.cc
new file mode 100644
index 0000000000000..29f16ab50d81cbe078b6175f7d332f5432b60c21
--- /dev/null
+++ b/chrome/browser/browseros/native_server/browseros_native_server.cc
@@ -0,0 +1,594 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+#include "chrome/browser/browseros/native_server/browseros_native_server.h"
+
+#include <memory>
+#include <string>
+#include <string_view>
+#include <utility>
+
+#include "base/command_line.h"
+#include "base/feature_list.h"
+#include "base/files/file_util.h"
+#include "base/functional/bind.h"
+#include "base/location.h"
+#include "base/logging.h"
+#include "base/memory/weak_ptr.h"
+#include "base/path_service.h"
+#include "base/process/process.h"
+#include "base/strings/string_number_conversions.h"
+#include "base/strings/string_util.h"
+#include "base/strings/stringprintf.h"
+#include "base/synchronization/waitable_event.h"
+#include "base/threading/scoped_blocking_call.h"
+#include "base/threading/thread.h"
+#include "base/time/time.h"
+#include "base/timer/timer.h"
+#include "chrome/browser/browser_features.h"
+#include "chrome/browser/browseros/core/browseros_switches.h"
+#include "chrome/common/chrome_paths.h"
+#include "net/base/ip_endpoint.h"
+#include "net/base/net_errors.h"
+#include "net/http/http_status_code.h"
+#include "net/log/net_log_source.h"
+#include "net/server/http_server_request_info.h"
+#include "net/server/http_server_response_info.h"
+#include "net/socket/tcp_server_socket.h"
+#include "net/traffic_annotation/network_traffic_annotation.h"
+
+namespace browseros {
+
+namespace {
+
+// Status codes come from net::HttpStatusCode, which at this Chromium version
+// is X-macro generated from net/http/http_status_code_list.h. Every code used
+// here is named there, including METHOD_NOT_ALLOWED and REQUEST_ENTITY_TOO_LARGE.
+constexpr int kStatusOK = net::HTTP_OK;
+constexpr int kStatusForbidden = net::HTTP_FORBIDDEN;
+constexpr int kStatusNotFound = net::HTTP_NOT_FOUND;
+constexpr int kStatusMethodNotAllowed = net::HTTP_METHOD_NOT_ALLOWED;
+constexpr int kStatusPayloadTooLarge = net::HTTP_REQUEST_ENTITY_TOO_LARGE;
+
+constexpr char kOwnershipFileName[] = "native_server.lock";
+
+net::NetworkTrafficAnnotationTag GetNativeServerTrafficAnnotation() {
+  return net::DefineNetworkTrafficAnnotation("browseros_native_server", R"(
+    semantics {
+      sender: "BrowserOS Native Server"
+      description:
+        "Responds to liveness requests from local processes against the "
+        "flag-gated in-process server bound to 127.0.0.1:1337."
+      trigger: "A local process issues GET / or GET /health on the native "
+               "server port."
+      data: "No request data is read or echoed. Response is a fixed string "
+            "or a status object."
+      destination: LOCAL
+    }
+    policy {
+      cookies_allowed: NO
+      cookies_store: "no cookie store"
+      setting: "Guarded by the enable-browseros-native-server flag, which "
+               "defaults to off."
+      policy_exception_justification:
+        "Experimental loopback liveness endpoint for the BrowserOS native "
+        "server. Requests carrying an Origin header are rejected."
+    })");
+}
+
+std::string_view StripQuery(std::string_view path) {
+  const size_t query = path.find('?');
+  return query == std::string_view::npos ? path : path.substr(0, query);
+}
+
+bool HasHeader(const net::HttpServerRequestInfo& info,
+               std::string_view header_name) {
+  for (const auto& header : info.headers) {
+    if (base::EqualsCaseInsensitiveASCII(header.first, header_name)) {
+      return true;
+    }
+  }
+  return false;
+}
+
+// Returns the header value, or an empty view when absent. net::HttpServer
+// normalises header names to lowercase and stores them in a map, so a missing
+// header and an empty-valued one are indistinguishable here -- which is fine,
+// because an empty Origin is never sent.
+std::string_view GetHeaderValue(const net::HttpServerRequestInfo& info,
+                                 std::string_view header_name) {
+  for (const auto& header : info.headers) {
+    if (base::EqualsCaseInsensitiveASCII(header.first, header_name)) {
+      return header.second;
+    }
+  }
+  return {};
+}
+
+}  // namespace
+
+bool IsBrowserPageRequest(bool has_origin,
+                          std::string_view sec_fetch_site) {
+  if (has_origin) {
+    return true;
+  }
+  return base::EqualsCaseInsensitiveASCII(sec_fetch_site, "cross-site") ||
+         base::EqualsCaseInsensitiveASCII(sec_fetch_site, "same-site") ||
+         base::EqualsCaseInsensitiveASCII(sec_fetch_site, "same-origin");
+}
+
+NativeServerResponse BuildNativeServerResponse(std::string_view method,
+                                               std::string_view path,
+                                               bool from_web_page,
+                                               size_t body_size,
+                                               int bound_port,
+                                               int64_t uptime_s) {
+  // Loopback is not a same-origin boundary, so a page on any site could
+  // otherwise reach this endpoint. IsBrowserPageRequest() is what makes that
+  // safe; do not bypass it, and do not add a route with side effects without
+  // re-reading it.
+  if (from_web_page) {
+    return {kStatusForbidden, "Browser page request rejected", "text/plain"};
+  }
+
+  if (body_size > kNativeServerMaxRequestBodyBytes) {
+    return {kStatusPayloadTooLarge, "Request body too large", "text/plain"};
+  }
+
+  const std::string_view route = StripQuery(path);
+  const bool is_get = base::EqualsCaseInsensitiveASCII(method, "GET");
+
+  if (route == "/") {
+    if (!is_get) {
+      return {kStatusMethodNotAllowed, "Method Not Allowed", "text/plain"};
+    }
+    return {kStatusOK, "hello world", "text/plain"};
+  }
+
+  if (route == "/health") {
+    if (!is_get) {
+      return {kStatusMethodNotAllowed, "Method Not Allowed", "text/plain"};
+    }
+    // Same shape the sidecar health checker already expects, so swapping the
+    // extension out for this later does not disturb the caller.
+    std::string body = base::StringPrintf(
+        "{\"status\":\"ok\",\"pid\":%d,\"port\":%d,\"uptime_s\":%lld}",
+        base::Process::Current().Pid(), bound_port,
+        static_cast<long long>(uptime_s));
+    return {kStatusOK, std::move(body), "application/json"};
+  }
+
+  return {kStatusNotFound, "Not Found", "text/plain"};
+}
+
+// static
+BrowserOsNativeServer* BrowserOsNativeServer::GetInstance() {
+  static base::NoDestructor<BrowserOsNativeServer> server;
+  return server.get();
+}
+
+BrowserOsNativeServer::BrowserOsNativeServer() = default;
+
+BrowserOsNativeServer::~BrowserOsNativeServer() {
+  Stop();
+}
+
+bool BrowserOsNativeServer::Start() {
+  if (IsRunning()) {
+    return true;
+  }
+
+  // Checked here as well as at the call site. The flag gates a resource, and a
+  // gate that a future caller can forget is not a gate.
+  if (!base::FeatureList::IsEnabled(features::kBrowserOsNativeServer)) {
+    LOG(INFO) << "browseros: native server not started - "
+                 "enable-browseros-native-server is off";
+    return false;
+  }
+
+  const int port = ResolvePort();
+
+  {
+    // Binding and the handshake below block briefly. Nothing in a request path
+    // ever blocks.
+    base::ScopedBlockingCall allow_blocking(FROM_HERE, base::BlockingType::WILL_BLOCK);
+
+    // base::Thread has no Options-taking constructor. The pump type is passed
+    // to StartWithOptions() instead, and it has to be IO: net::HttpServer's
+    // socket watchers require an IO message pump.
+    server_thread_ = std::make_unique<base::Thread>("BrowserOsNativeServer");
+    server_thread_->StartWithOptions(
+        base::Thread::Options(base::MessagePumpType::IO, 0));
+
+    // Bind on the server thread and wait for the result, so a bind failure is
+    // reported to the caller instead of disappearing into a log.
+    //
+    // base::WaitableEvent, not base::SyncEvent: SyncEvent was removed from base
+    // and its header no longer exists at this Chromium version.
+    base::WaitableEvent bound;
+    bool ok = false;
+    // Unretained is safe here: the UI thread blocks in bound.Wait() directly
+    // below, so this task has completed before Start() can return, and the
+    // singleton outlives every caller.
+    server_thread_->task_runner()->PostTask(
+        FROM_HERE, base::BindOnce(&BrowserOsNativeServer::BindAndSignal,
+                                  base::Unretained(this), port, &bound, &ok));
+    bound.Wait();
+
+    if (!ok) {
+      server_thread_->Stop();
+      server_thread_.reset();
+      return false;
+    }
+  }
+
+  heartbeat_.store(base::TimeTicks::Now().ToInternalValue(),
+                   std::memory_order_relaxed);
+  heartbeat_at_ping_ = 0;
+  ping_outstanding_ = false;
+  consecutive_failures_ = 0;
+
+  // The receiver/method overload avoids base::BindRepeatingTask, which is not
+  // in base/functional/bind.h at this version.
+  watchdog_timer_.Start(FROM_HERE, kNativeServerWatchdogInterval, this,
+                        &BrowserOsNativeServer::OnWatchdogTick);
+
+  owns_port_ = true;
+  WriteOwnershipFile(port);
+  return true;
+}
+
+void BrowserOsNativeServer::Stop() {
+  watchdog_timer_.Stop();
+  ping_outstanding_ = false;
+
+  if (server_thread_) {
+    if (server_thread_->IsRunning()) {
+      // Destroying net::HttpServer on its own thread keeps its thread-affinity
+      // checks satisfied.
+      // Unretained is safe here: server_thread_->Stop() below joins the thread,
+      // so this task has run before Stop() returns.
+      server_thread_->task_runner()->PostTask(
+          FROM_HERE, base::BindOnce(&BrowserOsNativeServer::UnbindOnServerThread,
+                                    base::Unretained(this)));
+
+      // base::Thread::Stop() joins and has no timeout. If the server thread is
+      // genuinely wedged this blocks shutdown. That is a deliberate trade: a
+      // thread cannot be abandoned in-process, and one we cannot join is not
+      // one we could safely keep running either.
+      server_thread_->Stop();
+    }
+    server_thread_.reset();
+  }
+
+  running_.store(false, std::memory_order_relaxed);
+  bound_port_.store(0, std::memory_order_relaxed);
+  // Only remove the file we wrote. Another instance may own the port -- this
+  // one failed to bind it and must not delete the real owner's record, or the
+  // next instance to collide logs "already owned by PID 0".
+  if (owns_port_) {
+    RemoveOwnershipFile();
+    owns_port_ = false;
+  }
+  consecutive_failures_ = 0;
+}
+
+bool BrowserOsNativeServer::BindOnServerThread(int port) {
+  if (server_) {
+    LOG(WARNING) << "browseros: native server already listening on port "
+                 << GetBoundPort();
+    return true;
+  }
+
+  auto server_socket =
+      std::make_unique<net::TCPServerSocket>(nullptr, net::NetLogSource());
+  const int result = server_socket->ListenWithAddressAndPort(
+      kNativeServerBindAddress, port, kNativeServerBacklog);
+  if (result != net::OK) {
+    LOG(ERROR) << "browseros: native server could not bind "
+               << kNativeServerBindAddress << ":" << port << " - "
+               << net::ErrorToString(result);
+    if (result == net::ERR_ADDRESS_IN_USE) {
+      // The bind is the lock. A held socket is released by the kernel the
+      // instant the owning process dies, so this is never a stale lock: either
+      // the owner is alive and serving, or it is gone and the bind will
+      // succeed on the next launch. There is deliberately no retry loop here -
+      // a second browser that silently fails to serve is a support ticket.
+      //
+      // A future need to *use* the endpoint from a non-owning instance (proxy
+      // to the owner) is the reason not to fold this into the sidecar manager.
+      LOG(WARNING) << "browseros: " << kNativeServerBindAddress << ":" << port
+                   << " is already owned by PID " << GetOwnerPid()
+                   << "; this instance will not serve it";
+    }
+    return false;
+  }
+
+  server_ = std::make_unique<net::HttpServer>(std::move(server_socket), this);
+  bound_port_.store(port, std::memory_order_relaxed);
+  running_.store(true, std::memory_order_relaxed);
+  start_time_ = base::Time::Now();
+
+  LOG(INFO) << "browseros: native server listening on "
+            << kNativeServerBindAddress << ":" << port;
+  return true;
+}
+
+void BrowserOsNativeServer::UnbindOnServerThread() {
+  open_connections_.clear();
+  server_.reset();
+  bound_port_.store(0, std::memory_order_relaxed);
+  running_.store(false, std::memory_order_relaxed);
+}
+
+void BrowserOsNativeServer::BindAndSignal(int port,
+                                          base::WaitableEvent* bound,
+                                          bool* ok) {
+  *ok = BindOnServerThread(port);
+  bound->Signal();
+}
+
+void BrowserOsNativeServer::RecordHeartbeat() {
+  heartbeat_.store(base::TimeTicks::Now().ToInternalValue(),
+                   std::memory_order_relaxed);
+}
+
+void BrowserOsNativeServer::RestartListener(int port) {
+  UnbindOnServerThread();
+  if (!BindOnServerThread(port)) {
+    LOG(ERROR) << "browseros: native server listener restart failed";
+  }
+}
+
+void BrowserOsNativeServer::SendResponse(
+    int connection_id,
+    const NativeServerResponse& response) {
+  if (!server_) {
+    return;
+  }
+  net::HttpServerResponseInfo info(
+      static_cast<net::HttpStatusCode>(response.status_code));
+  info.SetBody(response.body, response.content_type);
+  server_->SendResponse(connection_id, info,
+                        GetNativeServerTrafficAnnotation());
+}
+
+void BrowserOsNativeServer::OnConnect(int connection_id) {
+  if (open_connections_.size() >= kNativeServerMaxConnections) {
+    // Refuse rather than grow. A connection table that only ever grows is a
+    // memory-growth bug reachable from any page that can open a socket.
+    LOG(WARNING) << "browseros: native server refusing connection "
+                 << connection_id << " - limit of "
+                 << kNativeServerMaxConnections << " reached";
+    server_->Close(connection_id);
+    return;
+  }
+  open_connections_.insert(connection_id);
+}
+
+void BrowserOsNativeServer::OnHttpRequest(
+    int connection_id,
+    const net::HttpServerRequestInfo& info) {
+  if (!server_) {
+    return;
+  }
+
+  // The loopback bind already guarantees this. Kept anyway because it is the
+  // control that would stop holding the moment someone widens the bind address.
+  if (!info.peer.address().IsLoopback()) {
+    SendResponse(connection_id,
+                 {kStatusForbidden, "Non-loopback peer rejected", "text/plain"});
+    return;
+  }
+
+  const base::TimeDelta uptime = base::Time::Now() - start_time_;
+  SendResponse(
+      connection_id,
+      BuildNativeServerResponse(
+          info.method, info.path,
+          IsBrowserPageRequest(HasHeader(info, "origin"),
+                               GetHeaderValue(info, "sec-fetch-site")),
+          info.data.size(), GetBoundPort(), uptime.InSeconds()));
+}
+
+void BrowserOsNativeServer::OnWebSocketRequest(
+    int connection_id,
+    const net::HttpServerRequestInfo& info) {
+  if (!server_) {
+    return;
+  }
+  server_->Close(connection_id);
+}
+
+void BrowserOsNativeServer::OnWebSocketMessage(int connection_id,
+                                               std::string data) {
+  if (!server_) {
+    return;
+  }
+  server_->Close(connection_id);
+}
+
+void BrowserOsNativeServer::OnClose(int connection_id) {
+  open_connections_.erase(connection_id);
+}
+
+void BrowserOsNativeServer::OnWatchdogTick() {
+  // Keyed on the thread, not on IsRunning(). A restart that fails to rebind
+  // leaves IsRunning() false, and gating on that would silently disable the
+  // watchdog instead of escalating to the give-up cap.
+  if (!server_thread_ || !server_thread_->IsRunning()) {
+    return;
+  }
+
+  const base::TimeTicks now = base::TimeTicks::Now();
+
+  if (ping_outstanding_) {
+    if (now < ping_deadline_) {
+      return;
+    }
+    ping_outstanding_ = false;
+
+    // Distinguish "the server thread is wedged" from "the watchdog timer was
+    // itself delayed". Only the first counts as a server failure.
+    if (heartbeat_.load(std::memory_order_relaxed) == heartbeat_at_ping_) {
+      OnWatchdogFailure();
+    } else {
+      consecutive_failures_ = 0;
+    }
+    if (!server_thread_ || !server_thread_->IsRunning()) {
+      return;  // the watchdog gave up and stopped the timer
+    }
+  }
+
+  // A live thread that is not serving is a failure in its own right: the
+  // listener dropped, or a restart could not rebind. Counting it here is what
+  // makes kNativeServerMaxConsecutiveFailures reachable.
+  if (!IsRunning()) {
+    OnWatchdogFailure();
+    if (!server_thread_ || !server_thread_->IsRunning()) {
+      return;
+    }
+  }
+
+  heartbeat_at_ping_ = heartbeat_.load(std::memory_order_relaxed);
+  ping_outstanding_ = true;
+  ping_deadline_ = now + kNativeServerPingTimeout;
+
+  base::WeakPtr<BrowserOsNativeServer> weak = weak_factory_.GetWeakPtr();
+  server_thread_->task_runner()->PostTask(
+      FROM_HERE, base::BindOnce(&BrowserOsNativeServer::RecordHeartbeat, weak));
+}
+
+void BrowserOsNativeServer::OnWatchdogFailure() {
+  ++consecutive_failures_;
+  LOG(ERROR) << "browseros: native server thread did not answer a ping "
+                "(failure "
+             << consecutive_failures_ << " of "
+             << kNativeServerMaxConsecutiveFailures << ")";
+
+  if (consecutive_failures_ >= kNativeServerMaxConsecutiveFailures) {
+    // Stop pretending. Report an off state so the log, IsRunning() and any
+    // supervisor agree with what is actually bound.
+    LOG(ERROR) << "browseros: giving up on the native server watchdog; the "
+                  "listener is unresponsive and will not be restarted";
+    running_.store(false, std::memory_order_relaxed);
+    watchdog_timer_.Stop();
+    return;
+  }
+
+  // Bounce the listener. If the thread itself is wedged this post never runs,
+  // which the next tick detects as a second failure.
+  base::WeakPtr<BrowserOsNativeServer> weak = weak_factory_.GetWeakPtr();
+  const int port = GetBoundPort();
+  server_thread_->task_runner()->PostTask(
+      FROM_HERE, base::BindOnce(&BrowserOsNativeServer::RestartListener, weak,
+                                port));
+}
+
+int BrowserOsNativeServer::ResolvePort() const {
+  const base::CommandLine* command_line =
+      base::CommandLine::ForCurrentProcess();
+  if (command_line->HasSwitch(browseros::kNativeServerPort)) {
+    int port = 0;
+    if (base::StringToInt(
+            command_line->GetSwitchValueASCII(browseros::kNativeServerPort),
+            &port) &&
+        port > 0 && port <= 65535) {
+      return port;
+    }
+    LOG(WARNING) << "browseros: ignoring invalid --"
+                 << browseros::kNativeServerPort;
+  }
+  return kNativeServerDefaultPort;
+}
+
+base::FilePath BrowserOsNativeServer::GetOwnershipFilePath() const {
+  base::FilePath user_data_dir;
+  if (!base::PathService::Get(chrome::DIR_USER_DATA, &user_data_dir)) {
+    return base::FilePath();
+  }
+  return user_data_dir.AppendASCII(".browseros")
+      .AppendASCII(kOwnershipFileName);
+}
+
+bool BrowserOsNativeServer::WriteOwnershipFile(int port) {
+  base::ScopedBlockingCall allow_blocking(FROM_HERE, base::BlockingType::WILL_BLOCK);
+
+  const base::FilePath path = GetOwnershipFilePath();
+  if (path.empty()) {
+    LOG(ERROR) << "browseros: cannot resolve user data dir for the native "
+                  "server ownership file";
+    return false;
+  }
+  if (!base::PathExists(path.DirName()) &&
+      !base::CreateDirectory(path.DirName())) {
+    LOG(ERROR) << "browseros: cannot create " << path.DirName();
+    return false;
+  }
+
+  const std::string contents = base::StringPrintf(
+      "{\"pid\":%d,\"port\":%d,\"started_at\":%lld}\n",
+      base::Process::Current().Pid(), port,
+      static_cast<long long>(
+          base::Time::Now().ToDeltaSinceWindowsEpoch().InSeconds()));
+  if (!base::WriteFile(path, contents)) {
+    LOG(ERROR) << "browseros: cannot write " << path;
+    return false;
+  }
+  return true;
+}
+
+void BrowserOsNativeServer::RemoveOwnershipFile() {
+  base::ScopedBlockingCall allow_blocking(FROM_HERE, base::BlockingType::WILL_BLOCK);
+  const base::FilePath path = GetOwnershipFilePath();
+  if (path.empty()) {
+    return;
+  }
+  if (!base::DeleteFile(path)) {
+    LOG(WARNING) << "browseros: could not remove " << path;
+  }
+}
+
+int BrowserOsNativeServer::GetOwnerPid() const {
+  base::ScopedBlockingCall allow_blocking(FROM_HERE, base::BlockingType::WILL_BLOCK);
+
+  const base::FilePath path = GetOwnershipFilePath();
+  if (path.empty()) {
+    return 0;
+  }
+
+  std::string contents;
+  if (!base::ReadFileToStringWithMaxSize(path, &contents, 512)) {
+    return 0;
+  }
+
+  // The file is ours, but it is on disk and therefore untrusted input. Pull the
+  // digits out by hand rather than running a parser over it.
+  const size_t key = contents.find("\"pid\"");
+  if (key == std::string::npos) {
+    return 0;
+  }
+  const size_t colon = contents.find(':', key);
+  if (colon == std::string::npos) {
+    return 0;
+  }
+
+  size_t begin = colon + 1;
+  while (begin < contents.size() && contents[begin] == ' ') {
+    ++begin;
+  }
+  size_t end = begin;
+  while (end < contents.size() && contents[end] >= '0' && contents[end] <= '9') {
+    ++end;
+  }
+
+  int pid = 0;
+  if (end == begin ||
+      !base::StringToInt(contents.substr(begin, end - begin), &pid)) {
+    return 0;
+  }
+  return pid;
+}
+
+}  // namespace browseros

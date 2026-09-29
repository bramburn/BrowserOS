diff --git a/chrome/browser/browseros/native_server/browseros_native_server.h b/chrome/browser/browseros/native_server/browseros_native_server.h
new file mode 100644
index 0000000000000..45a644a94527b31c9adffd08f2aaf435afac9ceb
--- /dev/null
+++ b/chrome/browser/browseros/native_server/browseros_native_server.h
@@ -0,0 +1,200 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+#ifndef CHROME_BROWSER_BROWSEROS_NATIVE_SERVER_BROWSEROS_NATIVE_SERVER_H_
+#define CHROME_BROWSER_BROWSEROS_NATIVE_SERVER_BROWSEROS_NATIVE_SERVER_H_
+
+#include <atomic>
+#include <cstddef>
+#include <cstdint>
+#include <memory>
+#include <set>
+#include <string>
+#include <string_view>
+
+#include "base/containers/flat_set.h"
+#include "base/files/file_path.h"
+#include "base/memory/raw_ptr.h"
+#include "base/memory/weak_ptr.h"
+#include "base/no_destructor.h"
+#include "base/synchronization/waitable_event.h"
+#include "base/threading/thread.h"
+#include "base/time/time.h"
+#include "base/timer/timer.h"
+#include "net/server/http_server.h"
+namespace browseros {
+
+// Default port for the experimental native server. Overridable at runtime with
+// --browseros-native-server-port so the port can be moved without a rebuild.
+inline constexpr int kNativeServerDefaultPort = 1337;
+
+// Loopback only, never 0.0.0.0. A loopback bind is a security control here, not
+// a default: this endpoint is unauthenticated and any local process can reach
+// it, so it must not be reachable from the network.
+inline constexpr char kNativeServerBindAddress[] = "127.0.0.1";
+
+// Listen backlog. Small on purpose -- a real server sits behind this one.
+inline constexpr int kNativeServerBacklog = 10;
+
+// Refuse rather than grow without bound. An unbounded server inside the browser
+// process is a memory-growth bug reachable from any page that can issue a
+// request.
+inline constexpr size_t kNativeServerMaxConnections = 32;
+
+// Anything larger is rejected with 413 before it is buffered into a response.
+inline constexpr size_t kNativeServerMaxRequestBodyBytes = 64 * 1024;
+
+// Watchdog cadence and the grace period a server thread gets to answer a ping
+// before it is considered wedged.
+inline constexpr base::TimeDelta kNativeServerWatchdogInterval =
+    base::Seconds(3);
+inline constexpr base::TimeDelta kNativeServerPingTimeout = base::Seconds(2);
+
+// After this many consecutive failures the watchdog stops trying and reports a
+// hard off state, rather than thrashing a listener that is not coming back.
+inline constexpr int kNativeServerMaxConsecutiveFailures = 3;
+
+// The result of routing one request. Kept free of net types so the route table
+// can be unit tested without binding a socket.
+struct NativeServerResponse {
+  int status_code = 0;
+  std::string body;
+  std::string content_type;
+};
+
+// Returns true if the request was initiated by a web page rather than a local
+// process.
+//
+// An Origin header alone is not sufficient: Chrome does not send Origin for
+// no-cors subresource loads, so <img src="http://127.0.0.1:1337/"> and <form>
+// reach the server with no Origin at all. Sec-Fetch-Site is sent on every
+// browser-initiated request including no-cors ones, so it closes that gap.
+// A "same-origin" value is only possible from a document this server did not
+// serve, so it is treated as page traffic too.
+bool IsBrowserPageRequest(bool has_origin,
+                          std::string_view sec_fetch_site);
+
+// Pure routing decision. No I/O, no browser state -- everything the route table
+// needs is passed in. Returns 404 for an unknown path, 405 for a known path
+// with the wrong method, 403 when the request came from a page, and 413 when
+// the body is over the cap.
+NativeServerResponse BuildNativeServerResponse(std::string_view method,
+                                               std::string_view path,
+                                               bool from_web_page,
+                                               size_t body_size,
+                                               int bound_port,
+                                               int64_t uptime_s);
+
+// Experimental, flag-gated, in-process HTTP server on 127.0.0.1:1337.
+//
+// Threading: the listener and every net::HttpServer callback run on a private
+// base::Thread with an IO message pump, so a bad handler can never block or
+// crash the UI thread. The watchdog timer runs on the UI thread, because a
+// wedged server thread cannot be used to detect that it is wedged.
+//
+// Singleton: the bind itself is the lock. A second browser instance gets
+// ERR_ADDRESS_IN_USE and is told which PID owns the port; there is no retry loop,
+// because a second browser that silently fails to serve is a support ticket.
+// The ownership file exists only to answer *who* owns it, never to prevent a
+// second server -- the kernel releases a held socket the instant the owning
+// process dies, which no lock file can promise.
+class BrowserOsNativeServer : public net::HttpServer::Delegate {
+ public:
+  static BrowserOsNativeServer* GetInstance();
+
+  BrowserOsNativeServer();
+  ~BrowserOsNativeServer() override;
+
+  BrowserOsNativeServer(const BrowserOsNativeServer&) = delete;
+  BrowserOsNativeServer& operator=(const BrowserOsNativeServer&) = delete;
+
+  // Starts the server when features::kBrowserOsNativeServer is enabled.
+  // UI thread only. Returns true if the port is bound when the call returns.
+  bool Start();
+
+  // Stops the server, releases the port, and removes the ownership file.
+  // Safe to call when never started. UI thread only.
+  void Stop();
+
+  bool IsRunning() const { return running_.load(std::memory_order_relaxed); }
+  int GetBoundPort() const { return bound_port_.load(std::memory_order_relaxed); }
+
+  // Process id recorded in the ownership file, or 0 when it is unreadable.
+  int GetOwnerPid() const;
+
+ private:
+  friend base::NoDestructor<BrowserOsNativeServer>;
+
+  // net::HttpServer::Delegate. Server thread only.
+  void OnConnect(int connection_id) override;
+  void OnHttpRequest(int connection_id,
+                     const net::HttpServerRequestInfo& info) override;
+  void OnWebSocketRequest(int connection_id,
+                          const net::HttpServerRequestInfo& info) override;
+  void OnWebSocketMessage(int connection_id, std::string data) override;
+  void OnClose(int connection_id) override;
+
+  // Server thread only.
+  bool BindOnServerThread(int port);
+  void UnbindOnServerThread();
+
+  // Server thread only. Small member functions rather than lambdas, because
+  // these are what get handed to base::BindOnce for PostTask: binding a
+  // capturing lambda to a OnceCallback does not compile at this Chromium
+  // version, and binding a member function is the supported form anyway.
+  void BindAndSignal(int port, base::WaitableEvent* bound, bool* ok);
+  void RecordHeartbeat();
+  void RestartListener(int port);
+
+  void SendResponse(int connection_id, const NativeServerResponse& response);
+
+  // UI thread only.
+  void OnWatchdogTick();
+  void OnWatchdogFailure();
+
+  // UI thread only. Blocking file I/O.
+  bool WriteOwnershipFile(int port);
+  void RemoveOwnershipFile();
+  base::FilePath GetOwnershipFilePath() const;
+  int ResolvePort() const;
+
+  std::unique_ptr<base::Thread> server_thread_;
+
+  // Server-thread affine. Destroyed on the server thread via UnbindOnServerThread
+  // so net::HttpServer's thread checks stay satisfied.
+  std::unique_ptr<net::HttpServer> server_;
+  base::flat_set<int> open_connections_;
+
+  base::RepeatingTimer watchdog_timer_;
+
+  std::atomic<int> bound_port_{0};
+  std::atomic<bool> running_{false};
+
+  // Bumped by the server thread on every task it runs, so the UI thread can
+  // tell "busy" from "wedged".
+  std::atomic<int64_t> heartbeat_{0};
+
+  bool ping_outstanding_ = false;
+  base::TimeTicks ping_deadline_;
+  int64_t heartbeat_at_ping_ = 0;
+  int consecutive_failures_ = 0;
+
+  // Server-thread affine.
+  base::Time start_time_;
+
+  // True only between a successful bind and Stop(). Guards RemoveOwnershipFile:
+  // a second browser instance that failed to bind still runs Stop() at shutdown,
+  // and must not delete the lock file belonging to the instance that owns the
+  // port.
+  bool owns_port_ = false;
+
+  // Must be the last member: a WeakPtrFactory referring to its own class has to
+  // be destroyed after everything it can invalidate a pointer to. Chromium
+  // enforces this with a style check.
+  base::WeakPtrFactory<BrowserOsNativeServer> weak_factory_{this};
+};
+
+}  // namespace browseros
+
+#endif  // CHROME_BROWSER_BROWSEROS_NATIVE_SERVER_BROWSEROS_NATIVE_SERVER_H_

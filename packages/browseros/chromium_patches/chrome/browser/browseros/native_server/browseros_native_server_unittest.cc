diff --git a/chrome/browser/browseros/native_server/browseros_native_server_unittest.cc b/chrome/browser/browseros/native_server/browseros_native_server_unittest.cc
new file mode 100644
index 0000000000000..27afb6800744d28e612bded85881519fee164aa5
--- /dev/null
+++ b/chrome/browser/browseros/native_server/browseros_native_server_unittest.cc
@@ -0,0 +1,150 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+#include "chrome/browser/browseros/native_server/browseros_native_server.h"
+
+#include <string>
+
+#include "testing/gtest/include/gtest/gtest.h"
+
+// These tests cover the route table only. Binding a real socket would make the
+// suite depend on a free port and a live event loop, and would test Chromium's
+// net::HttpServer rather than our code. The route table is the part that is
+// ours and the part that can silently regress.
+
+namespace browseros {
+namespace {
+
+constexpr int kTestPort = 1337;
+constexpr int64_t kTestUptime = 42;
+
+NativeServerResponse Route(const std::string& method, const std::string& path) {
+  return BuildNativeServerResponse(method, path, /*from_web_page=*/false,
+                                   /*body_size=*/0, kTestPort, kTestUptime);
+}
+
+// === Happy path ===
+
+TEST(BrowserOsNativeServerRouteTest, RootReturnsHelloWorld) {
+  const NativeServerResponse response = Route("GET", "/");
+  EXPECT_EQ(200, response.status_code);
+  EXPECT_EQ("hello world", response.body);
+  EXPECT_EQ("text/plain", response.content_type);
+}
+
+TEST(BrowserOsNativeServerRouteTest, HealthReturnsStatusJson) {
+  const NativeServerResponse response = Route("GET", "/health");
+  EXPECT_EQ(200, response.status_code);
+  EXPECT_EQ("application/json", response.content_type);
+  EXPECT_NE(std::string::npos, response.body.find("\"status\":\"ok\""));
+  EXPECT_NE(std::string::npos, response.body.find("\"port\":1337"));
+  EXPECT_NE(std::string::npos, response.body.find("\"uptime_s\":42"));
+  EXPECT_NE(std::string::npos, response.body.find("\"pid\":"));
+}
+
+TEST(BrowserOsNativeServerRouteTest, QueryStringIsIgnored) {
+  EXPECT_EQ(200, Route("GET", "/?debug=1").status_code);
+  EXPECT_EQ(200, Route("GET", "/health?x=1").status_code);
+}
+
+TEST(BrowserOsNativeServerRouteTest, MethodIsCaseInsensitive) {
+  EXPECT_EQ(200, Route("get", "/").status_code);
+}
+
+// === 404 ===
+
+TEST(BrowserOsNativeServerRouteTest, UnknownPathReturns404) {
+  const NativeServerResponse response = Route("GET", "/nope");
+  EXPECT_EQ(404, response.status_code);
+  EXPECT_EQ("text/plain", response.content_type);
+}
+
+TEST(BrowserOsNativeServerRouteTest, RootWithSubpathReturns404) {
+  EXPECT_EQ(404, Route("GET", "/index.html").status_code);
+}
+
+// === 405 ===
+
+TEST(BrowserOsNativeServerRouteTest, WrongMethodOnKnownRouteReturns405) {
+  EXPECT_EQ(405, Route("POST", "/").status_code);
+  EXPECT_EQ(405, Route("DELETE", "/health").status_code);
+}
+
+TEST(BrowserOsNativeServerRouteTest, WrongMethodOnUnknownPathStillReturns404) {
+  EXPECT_EQ(404, Route("POST", "/nope").status_code);
+}
+
+// === Browser page requests: the control that must not be dropped ===
+
+TEST(BrowserOsNativeServerRouteTest, BrowserPageRequestIsRejected) {
+  const NativeServerResponse response = BuildNativeServerResponse(
+      "GET", "/", /*from_web_page=*/true, /*body_size=*/0, kTestPort, kTestUptime);
+  EXPECT_EQ(403, response.status_code);
+}
+
+TEST(BrowserOsNativeServerRouteTest, PageRequestIsRejectedOnEveryRoute) {
+  EXPECT_EQ(403, BuildNativeServerResponse("GET", "/health", true, 0, kTestPort,
+                                           kTestUptime)
+                   .status_code);
+  EXPECT_EQ(403, BuildNativeServerResponse("GET", "/nope", true, 0, kTestPort,
+                                           kTestUptime)
+                   .status_code);
+}
+
+// An Origin header alone does not cover no-cors subresource loads: Chrome sends
+// none for <img>/<script>/<form>, so those must be caught by Sec-Fetch-Site.
+TEST(BrowserOsNativeServerPageRequestTest, OriginAloneMarksPageRequest) {
+  EXPECT_TRUE(IsBrowserPageRequest(/*has_origin=*/true, ""));
+}
+
+TEST(BrowserOsNativeServerPageRequestTest, SecFetchSiteCoversNoCorsLoads) {
+  EXPECT_TRUE(IsBrowserPageRequest(false, "cross-site"));
+  EXPECT_TRUE(IsBrowserPageRequest(false, "same-site"));
+  EXPECT_TRUE(IsBrowserPageRequest(false, "same-origin"));
+}
+
+TEST(BrowserOsNativeServerPageRequestTest, SecFetchSiteIsCaseInsensitive) {
+  EXPECT_TRUE(IsBrowserPageRequest(false, "Cross-Site"));
+}
+
+TEST(BrowserOsNativeServerPageRequestTest, LocalProcessIsNotAPageRequest) {
+  // A local client (curl, the MCP host) sends neither header.
+  EXPECT_FALSE(IsBrowserPageRequest(/*has_origin=*/false, ""));
+  EXPECT_FALSE(IsBrowserPageRequest(false, "none"));
+}
+
+// === Body bound ===
+
+TEST(BrowserOsNativeServerRouteTest, OversizedBodyIsRejected) {
+  const NativeServerResponse response = BuildNativeServerResponse(
+      "POST", "/", /*has_origin=*/false,
+      kNativeServerMaxRequestBodyBytes + 1, kTestPort, kTestUptime);
+  EXPECT_EQ(413, response.status_code);
+}
+
+TEST(BrowserOsNativeServerRouteTest, BodyAtTheLimitIsAccepted) {
+  const NativeServerResponse response =
+      BuildNativeServerResponse("GET", "/", false,
+                                kNativeServerMaxRequestBodyBytes, kTestPort,
+                                kTestUptime);
+  EXPECT_EQ(200, response.status_code);
+}
+
+// === Constants ===
+
+TEST(BrowserOsNativeServerBoundTest, BindsLoopbackOnly) {
+  EXPECT_EQ("127.0.0.1", std::string(kNativeServerBindAddress));
+}
+
+TEST(BrowserOsNativeServerBoundTest, DefaultPortIs1337) {
+  EXPECT_EQ(1337, kNativeServerDefaultPort);
+}
+
+TEST(BrowserOsNativeServerBoundTest, ConnectionLimitIsBounded) {
+  EXPECT_GT(kNativeServerMaxConnections, 0u);
+  EXPECT_LT(kNativeServerMaxConnections, 1024u);
+}
+
+}  // namespace
+}  // namespace browseros

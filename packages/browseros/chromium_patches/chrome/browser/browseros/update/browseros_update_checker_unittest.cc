diff --git a/chrome/browser/browseros/update/browseros_update_checker_unittest.cc b/chrome/browser/browseros/update/browseros_update_checker_unittest.cc
new file mode 100644
index 0000000000000..0000000000000
--- /dev/null
+++ b/chrome/browser/browseros/update/browseros_update_checker_unittest.cc
@@ -0,0 +1,106 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+#include "chrome/browser/browseros/update/browseros_update_checker.h"
+
+#include <string>
+
+#include "base/version.h"
+#include "testing/gtest.h"
+
+namespace browseros_update {
+
+namespace {
+
+UpdateDecision Decide(const std::string& current,
+                      const std::string& remote,
+                      bool paused = false,
+                      const std::string& skipped = "") {
+  return DecideUpdate(base::Version(current), base::Version(remote), paused,
+                      skipped);
+}
+
+}  // namespace
+
+TEST(BrowserOSUpdateCheckerTest, NewerIsOffered) {
+  EXPECT_EQ(Decide("1.0.0", "1.0.1"), UpdateDecision::kUpdateAvailable);
+  EXPECT_EQ(Decide("1.0.0", "2.0.0"), UpdateDecision::kUpdateAvailable);
+  EXPECT_EQ(Decide("1.0.0", "1.1.0"), UpdateDecision::kUpdateAvailable);
+}
+
+TEST(BrowserOSUpdateCheckerTest, EqualIsUpToDate) {
+  EXPECT_EQ(Decide("1.2.3", "1.2.3"), UpdateDecision::kUpToDate);
+}
+
+TEST(BrowserOSUpdateCheckerTest, OlderIsRefused) {
+  // The rollback guard. These must never become kUpdateAvailable.
+  EXPECT_EQ(Decide("2.0.0", "1.0.0"), UpdateDecision::kRollbackBlocked);
+  EXPECT_EQ(Decide("1.0.1", "1.0.0"), UpdateDecision::kRollbackBlocked);
+}
+
+TEST(BrowserOSUpdateCheckerTest, DifferingComponentCounts) {
+  // base::Version compares component-wise, so a longer version with more
+  // components wins only if the shared prefix is equal.
+  EXPECT_EQ(Decide("0.1.0", "0.1.0.1"), UpdateDecision::kUpdateAvailable);
+  EXPECT_EQ(Decide("0.1.0.1", "0.1.0"), UpdateDecision::kRollbackBlocked);
+  EXPECT_EQ(Decide("0.1.0.1", "0.1.0.1"), UpdateDecision::kUpToDate);
+  EXPECT_EQ(Decide("0.1.0.1", "0.1.0.2"), UpdateDecision::kUpdateAvailable);
+}
+
+TEST(BrowserOSUpdateCheckerTest, PausedBeatsEverything) {
+  // Even a strictly newer version must not be offered on a paused channel.
+  EXPECT_EQ(Decide("1.0.0", "9.9.9", /*paused=*/true),
+            UpdateDecision::kPaused);
+  // Including when the remote version is older or equal.
+  EXPECT_EQ(Decide("1.0.0", "1.0.0", /*paused=*/true),
+            UpdateDecision::kPaused);
+  EXPECT_EQ(Decide("9.9.9", "1.0.0", /*paused=*/true),
+            UpdateDecision::kPaused);
+}
+
+TEST(BrowserOSUpdateCheckerTest, InvalidVersionIsRefused) {
+  EXPECT_EQ(Decide("", "1.0.0"), UpdateDecision::kInvalidVersion);
+  EXPECT_EQ(Decide("1.0.0", ""), UpdateDecision::kInvalidVersion);
+  EXPECT_EQ(Decide("not.a.version", "1.0.0"), UpdateDecision::kInvalidVersion);
+}
+
+TEST(BrowserOSUpdateCheckerTest, SkippedVersionIsNotOffered) {
+  EXPECT_EQ(Decide("1.0.0", "1.1.0", false, "1.1.0"),
+            UpdateDecision::kSkippedByUser);
+  // The same version twice over is also "skipped", not an update.
+  EXPECT_EQ(Decide("1.0.0", "1.1.0", false, "1.1.0"),
+            UpdateDecision::kSkippedByUser);
+}
+
+TEST(BrowserOSUpdateCheckerTest, NewerThanSkippedIsOffered) {
+  EXPECT_EQ(Decide("1.0.0", "1.2.0", false, "1.1.0"),
+            UpdateDecision::kUpdateAvailable);
+}
+
+TEST(BrowserOSUpdateCheckerTest, EmptySkipOffersUpdate) {
+  EXPECT_EQ(Decide("1.0.0", "1.1.0", false, ""),
+            UpdateDecision::kUpdateAvailable);
+}
+
+TEST(BrowserOSUpdateCheckerTest, GarbageSkipIsIgnored) {
+  // An unparseable skipped version must not silently block updates.
+  EXPECT_EQ(Decide("1.0.0", "1.1.0", false, "not-a-version"),
+            UpdateDecision::kUpdateAvailable);
+}
+
+TEST(BrowserOSUpdateCheckerTest, SkipNeverOverridesRollbackGuard) {
+  // Skipping cannot be used to talk the browser into an older build.
+  EXPECT_EQ(Decide("2.0.0", "1.0.0", false, "1.0.0"),
+            UpdateDecision::kRollbackBlocked);
+}
+
+TEST(BrowserOSUpdateCheckerTest, DecisionToStringIsStable) {
+  EXPECT_STREQ(UpdateDecisionToString(UpdateDecision::kUpdateAvailable),
+               "update-available");
+  EXPECT_STREQ(UpdateDecisionToString(UpdateDecision::kRollbackBlocked),
+               "rollback-blocked");
+  EXPECT_STREQ(UpdateDecisionToString(UpdateDecision::kPaused), "paused");
+}
+
+}  // namespace browseros_update

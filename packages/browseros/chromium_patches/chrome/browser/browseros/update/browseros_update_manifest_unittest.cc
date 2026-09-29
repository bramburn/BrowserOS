diff --git a/chrome/browser/browseros/update/browseros_update_manifest_unittest.cc b/chrome/browser/browseros/update/browseros_update_manifest_unittest.cc
new file mode 100644
index 0000000000000..0000000000000
--- /dev/null
+++ b/chrome/browser/browseros/update/browseros_update_manifest_unittest.cc
@@ -0,0 +1,229 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+#include "chrome/browser/browseros/update/browseros_update_manifest.h"
+
+#include <optional>
+#include <string>
+#include <vector>
+
+#include "base/values.h"
+#include "testing/gtest.h"
+
+namespace browseros_update {
+
+namespace {
+
+constexpr char kHost[] = "cdn.example.com";
+constexpr char kSha[] =
+    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
+
+const std::vector<std::string> AllowedHosts() {
+  return {kHost};
+}
+
+// Builds a well-formed manifest. Each test either uses this and tampers with
+// exactly one field, or assembles its own dict when it needs to omit a key.
+std::string ValidManifest(const std::string& channel = "stable",
+                          const std::string& version = "1.2.3",
+                          const std::string& url =
+                              "https://cdn.example.com/browseros/x/inst.exe",
+                          const std::string& sha256 = kSha,
+                          const std::string& platform_key = "windows-x64") {
+  base::Value::Dict platform;
+  platform.Set("url", base::Value(url));
+  platform.Set("sha256", base::Value(sha256));
+  platform.Set("size", base::Value(static_cast<double>(178257920)));
+  platform.Set("min_os_version", base::Value("10.0.17763"));
+  platform.Set("publisher", base::Value("BrowserOS Ltd"));
+
+  base::Value::Dict platforms;
+  platforms.Set(platform_key, base::Value(std::move(platform)));
+
+  base::Value::Dict root;
+  root.Set("schema", base::Value(static_cast<double>(1)));
+  root.Set("channel", base::Value(channel));
+  root.Set("version", base::Value(version));
+  root.Set("notes", base::Value("https://example.com/notes"));
+  root.Set("platforms", base::Value(std::move(platforms)));
+  return base::Value(std::move(root)).ToJsonString();
+}
+
+std::optional<Manifest> ParseFor(const std::string& json,
+                                 const std::string& key = "windows-x64") {
+  return Manifest::ParseForPlatform(json, AllowedHosts(), key);
+}
+
+}  // namespace
+
+TEST(BrowserOSUpdateManifestTest, ParsesValidManifest) {
+  std::optional<Manifest> manifest = ParseFor(ValidManifest());
+  ASSERT_TRUE(manifest.has_value());
+  EXPECT_TRUE(manifest->IsValid());
+  EXPECT_FALSE(manifest->IsPaused());
+  EXPECT_EQ(manifest->version(), base::Version("1.2.3"));
+  EXPECT_EQ(manifest->channel(), "stable");
+  EXPECT_EQ(manifest->notes_url(), "https://example.com/notes");
+
+  const PlatformEntry* entry = manifest->GetPlatform("windows-x64");
+  ASSERT_NE(entry, nullptr);
+  EXPECT_EQ(entry->sha256, kSha);
+  EXPECT_EQ(entry->size, 178257920);
+  EXPECT_EQ(entry->min_os_version, "10.0.17763");
+  EXPECT_EQ(entry->publisher, "BrowserOS Ltd");
+  EXPECT_TRUE(entry->url.SchemeIs("https"));
+}
+
+TEST(BrowserOSUpdateManifestTest, PausedManifestIsValidAndEmpty) {
+  base::Value::Dict root;
+  root.Set("schema", base::Value(static_cast<double>(1)));
+  root.Set("channel", base::Value("paused"));
+  root.Set("version", base::Value("0.9.0"));
+  root.Set("platforms", base::Value::Dict());
+  const std::string json = base::Value(std::move(root)).ToJsonString();
+
+  std::optional<Manifest> manifest = ParseFor(json);
+  ASSERT_TRUE(manifest.has_value());
+  EXPECT_TRUE(manifest->IsPaused());
+  EXPECT_EQ(manifest->version(), base::Version("0.9.0"));
+  // A paused manifest offers nothing to download.
+  EXPECT_EQ(manifest->GetPlatformForCurrentBuild(), nullptr);
+}
+
+TEST(BrowserOSUpdateManifestTest, RejectsMalformedJson) {
+  EXPECT_FALSE(ParseFor("{not json").has_value());
+  EXPECT_FALSE(ParseFor("").has_value());
+  EXPECT_FALSE(ParseFor("[]").has_value());
+}
+
+TEST(BrowserOSUpdateManifestTest, RejectsWrongSchema) {
+  base::Value::Dict root;
+  root.Set("schema", base::Value(static_cast<double>(2)));
+  root.Set("channel", base::Value("stable"));
+  root.Set("version", base::Value("1.0.0"));
+  root.Set("platforms", base::Value::Dict());
+  const std::string json = base::Value(std::move(root)).ToJsonString();
+  EXPECT_FALSE(ParseFor(json).has_value());
+}
+
+TEST(BrowserOSUpdateManifestTest, RejectsMissingSchema) {
+  base::Value::Dict root;
+  root.Set("channel", base::Value("stable"));
+  root.Set("version", base::Value("1.0.0"));
+  const std::string json = base::Value(std::move(root)).ToJsonString();
+  EXPECT_FALSE(ParseFor(json).has_value());
+}
+
+TEST(BrowserOSUpdateManifestTest, RejectsEmptyChannel) {
+  base::Value::Dict root;
+  root.Set("schema", base::Value(static_cast<double>(1)));
+  root.Set("channel", base::Value(""));
+  root.Set("version", base::Value("1.0.0"));
+  const std::string json = base::Value(std::move(root)).ToJsonString();
+  EXPECT_FALSE(ParseFor(json).has_value());
+}
+
+TEST(BrowserOSUpdateManifestTest, RejectsMissingVersion) {
+  base::Value::Dict root;
+  root.Set("schema", base::Value(static_cast<double>(1)));
+  root.Set("channel", base::Value("stable"));
+  const std::string json = base::Value(std::move(root)).ToJsonString();
+  EXPECT_FALSE(ParseFor(json).has_value());
+}
+
+TEST(BrowserOSUpdateManifestTest, RejectsMissingPlatformEntry) {
+  base::Value::Dict root;
+  root.Set("schema", base::Value(static_cast<double>(1)));
+  root.Set("channel", base::Value("stable"));
+  root.Set("version", base::Value("1.0.0"));
+  root.Set("platforms", base::Value::Dict());
+  const std::string json = base::Value(std::move(root)).ToJsonString();
+  EXPECT_FALSE(ParseFor(json).has_value());
+}
+
+TEST(BrowserOSUpdateManifestTest, RejectsPlatformNotMatchingThisBuild) {
+  // A manifest carrying only a macOS artifact is not an error by itself, but
+  // it is useless here, so it must not resolve to a valid manifest.
+  EXPECT_FALSE(ParseFor(ValidManifest("stable", "1.2.3",
+                                      "https://cdn.example.com/x.zip", kSha,
+                                      "mac-arm64"))
+                   .has_value());
+}
+
+TEST(BrowserOSUpdateManifestTest, RejectsNonHttpsUrl) {
+  EXPECT_FALSE(ParseFor(ValidManifest("stable", "1.2.3",
+                                      "http://cdn.example.com/x/inst.exe", kSha))
+                   .has_value());
+}
+
+TEST(BrowserOSUpdateManifestTest, RejectsHostOutsideAllowlist) {
+  EXPECT_FALSE(ParseFor(ValidManifest("stable", "1.2.3",
+                                      "https://evil.example.net/x/inst.exe", kSha))
+                   .has_value());
+}
+
+TEST(BrowserOSUpdateManifestTest, EmptyAllowlistRejectsEverything) {
+  const std::string json = ValidManifest();
+  EXPECT_FALSE(
+      Manifest::ParseForPlatform(json, {}, "windows-x64").has_value());
+}
+
+TEST(BrowserOSUpdateManifestTest, RejectsBadSha256Length) {
+  EXPECT_FALSE(
+      ParseFor(ValidManifest("stable", "1.2.3",
+                             "https://cdn.example.com/x/inst.exe", "abcd"))
+          .has_value());
+}
+
+TEST(BrowserOSUpdateManifestTest, RejectsUppercaseSha256) {
+  std::string upper(kSha);
+  for (char& c : upper) {
+    if (c >= 'a' && c <= 'f') {
+      c = static_cast<char>(c - 'a' + 'A');
+    }
+  }
+  EXPECT_FALSE(ParseFor(ValidManifest("stable", "1.2.3",
+                                      "https://cdn.example.com/x/inst.exe", upper))
+                   .has_value());
+}
+
+TEST(BrowserOSUpdateManifestTest, RejectsNonHexSha256) {
+  std::string bad(kSha);
+  bad[0] = 'z';
+  EXPECT_FALSE(ParseFor(ValidManifest("stable", "1.2.3",
+                                      "https://cdn.example.com/x/inst.exe", bad))
+                   .has_value());
+}
+
+TEST(BrowserOSUpdateManifestTest, RejectsNonPositiveSize) {
+  base::Value::Dict platform;
+  platform.Set("url", base::Value("https://cdn.example.com/x/inst.exe"));
+  platform.Set("sha256", base::Value(kSha));
+  platform.Set("size", base::Value(static_cast<double>(0)));
+  base::Value::Dict platforms;
+  platforms.Set("windows-x64", base::Value(std::move(platform)));
+  base::Value::Dict root;
+  root.Set("schema", base::Value(static_cast<double>(1)));
+  root.Set("channel", base::Value("stable"));
+  root.Set("version", base::Value("1.0.0"));
+  root.Set("platforms", base::Value(std::move(platforms)));
+  const std::string json = base::Value(std::move(root)).ToJsonString();
+  EXPECT_FALSE(ParseFor(json).has_value());
+}
+
+TEST(BrowserOSUpdateManifestTest, CanonicalFormMatchesPythonOutput) {
+  // The Python generator writes with sort_keys=True and no insignificant
+  // whitespace. base::Value::Dict iterates sorted and ToJsonString() is
+  // compact, so the two must agree byte for byte or signatures will not
+  // verify. This pins the exact expected string.
+  base::Value::Dict root;
+  root.Set("schema", base::Value(static_cast<double>(1)));
+  root.Set("channel", base::Value("stable"));
+  root.Set("version", base::Value("1.2.3"));
+  const std::string canonical = Manifest::CanonicalizeJsonForSigning(root);
+  EXPECT_EQ(canonical,
+            R"({"channel":"stable","schema":1,"version":"1.2.3"})");
+}
+
+}  // namespace browseros_update

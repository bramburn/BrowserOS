diff --git a/chrome/browser/browseros/update/browseros_update_manifest.cc b/chrome/browser/browseros/update/browseros_update_manifest.cc
new file mode 100644
index 0000000000000..0000000000000
--- /dev/null
+++ b/chrome/browser/browseros/update/browseros_update_manifest.cc
@@ -0,0 +1,249 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+#include "chrome/browser/browseros/update/browseros_update_manifest.h"
+
+#include <optional>
+#include <string>
+#include <string_view>
+#include <utility>
+#include <vector>
+
+#include "base/logging.h"
+#include "base/values.h"
+#include "build/build_config.h"
+#include "url/gurl.h"
+
+#if BUILDFLAG(IS_MAC)
+#include "base/mac/mac_util.h"
+#endif
+#if BUILDFLAG(IS_WIN)
+#include "base/win/win_util.h"
+#endif
+
+namespace browseros_update {
+
+namespace {
+
+constexpr char kKeyChannel[] = "channel";
+constexpr char kKeyMinOsVersion[] = "min_os_version";
+constexpr char kKeyNotes[] = "notes";
+constexpr char kKeyPlatforms[] = "platforms";
+constexpr char kKeyPublisher[] = "publisher";
+constexpr char kKeySchema[] = "schema";
+constexpr char kKeySha256[] = "sha256";
+constexpr char kKeySize[] = "size";
+constexpr char kKeyUrl[] = "url";
+constexpr char kKeyVersion[] = "version";
+
+// Exactly 64 lowercase hex characters.
+bool IsLowercaseSha256Hex(const std::string& value) {
+  if (value.size() != 64) {
+    return false;
+  }
+  for (const char c : value) {
+    const bool is_digit = c >= '0' && c <= '9';
+    const bool is_lower_hex = c >= 'a' && c <= 'f';
+    if (!is_digit && !is_lower_hex) {
+      return false;
+    }
+  }
+  return true;
+}
+
+bool HostIsAllowed(const GURL& url, const std::vector<std::string>& allowed) {
+  if (allowed.empty()) {
+    return false;
+  }
+  const std::string host = url.host();
+  for (const std::string& candidate : allowed) {
+    if (host == candidate) {
+      return true;
+    }
+  }
+  return false;
+}
+
+// Parses and validates the "platforms" -> key entry.
+std::optional<PlatformEntry> ParsePlatformEntry(
+    const base::Value::Dict& root,
+    const std::string& key,
+    const std::vector<std::string>& allowed_hosts) {
+  const base::Value* platforms_value = root.Find(kKeyPlatforms);
+  const base::Value::Dict* platforms = platforms_value ? platforms_value->GetIfDict()
+                                                      : nullptr;
+  if (!platforms) {
+    LOG(ERROR) << "BrowserOS update manifest: \"platforms\" is not an object";
+    return std::nullopt;
+  }
+
+  const base::Value* entry_value = platforms->Find(key);
+  const base::Value::Dict* entry = entry_value ? entry_value->GetIfDict() : nullptr;
+  if (!entry) {
+    LOG(ERROR) << "BrowserOS update manifest: no object for platform " << key;
+    return std::nullopt;
+  }
+
+  PlatformEntry result;
+
+  const std::string* url_string = entry->FindString(kKeyUrl);
+  if (!url_string) {
+    LOG(ERROR) << "BrowserOS update manifest: platform " << key << " has no url";
+    return std::nullopt;
+  }
+  const GURL url(*url_string);
+  if (!url.is_valid() || !url.SchemeIs("https")) {
+    LOG(ERROR) << "BrowserOS update manifest: platform " << key
+               << " url is not https";
+    return std::nullopt;
+  }
+  if (!HostIsAllowed(url, allowed_hosts)) {
+    LOG(ERROR) << "BrowserOS update manifest: platform " << key
+               << " url host is not allow-listed: " << url.host();
+    return std::nullopt;
+  }
+  result.url = url;
+
+  const std::string* sha256 = entry->FindString(kKeySha256);
+  if (!sha256 || !IsLowercaseSha256Hex(*sha256)) {
+    LOG(ERROR) << "BrowserOS update manifest: platform " << key
+               << " has a malformed sha256";
+    return std::nullopt;
+  }
+  result.sha256 = *sha256;
+
+  std::optional<int64_t> size = entry->FindInt(kKeySize);
+  if (!size || *size <= 0) {
+    LOG(ERROR) << "BrowserOS update manifest: platform " << key
+               << " has a missing or non-positive size";
+    return std::nullopt;
+  }
+  result.size = *size;
+
+  if (const std::string* min_os = entry->FindString(kKeyMinOsVersion)) {
+    result.min_os_version = *min_os;
+  }
+  if (const std::string* publisher = entry->FindString(kKeyPublisher)) {
+    result.publisher = *publisher;
+  }
+
+  return result;
+}
+
+}  // namespace
+
+std::string Manifest::CurrentPlatformKey() {
+#if BUILDFLAG(IS_WIN)
+  if (base::win::GetCurrentArchitecture() == base::win::ARCHITECTURE_X64) {
+    return "windows-x64";
+  }
+  return std::string();
+#elif BUILDFLAG(IS_MAC)
+  if (base::mac::IsArm()) {
+    return "mac-arm64";
+  }
+  return std::string();
+#else
+  // Linux has no in-browser updater; see docs/UPDATE_DESIGN.md.
+  return std::string();
+#endif
+}
+
+std::optional<Manifest> Manifest::Parse(
+    std::string_view json,
+    const std::vector<std::string>& expected_hosts) {
+  return ParseForPlatform(json, expected_hosts, CurrentPlatformKey());
+}
+
+std::optional<Manifest> Manifest::ParseForPlatform(
+    std::string_view json,
+    const std::vector<std::string>& expected_hosts,
+    const std::string& platform_key) {
+  base::Value::Dict root;
+  if (!base::ParseFromJson(json, root)) {
+    LOG(ERROR) << "BrowserOS update manifest: not valid JSON";
+    return std::nullopt;
+  }
+
+  std::optional<int> schema = root.FindInt(kKeySchema);
+  if (!schema || *schema != kSchemaVersion) {
+    LOG(ERROR) << "BrowserOS update manifest: unsupported schema";
+    return std::nullopt;
+  }
+
+  Manifest manifest;
+
+  const std::string* version_string = root.FindString(kKeyVersion);
+  if (!version_string || version_string->empty()) {
+    LOG(ERROR) << "BrowserOS update manifest: missing version";
+    return std::nullopt;
+  }
+  manifest.version_ = base::Version(*version_string);
+  if (!manifest.version_.IsValid()) {
+    LOG(ERROR) << "BrowserOS update manifest: unparseable version "
+               << *version_string;
+    return std::nullopt;
+  }
+
+  const std::string* channel = root.FindString(kKeyChannel);
+  if (!channel || channel->empty()) {
+    LOG(ERROR) << "BrowserOS update manifest: missing channel";
+    return std::nullopt;
+  }
+  manifest.channel_ = *channel;
+
+  if (const std::string* notes = root.FindString(kKeyNotes)) {
+    manifest.notes_url_ = *notes;
+  }
+
+  // A paused manifest is the kill switch. It carries no platforms but must
+  // still be well-formed, so stop before demanding a download entry.
+  if (manifest.IsPaused()) {
+    manifest.valid_ = true;
+    return manifest;
+  }
+
+  if (platform_key.empty()) {
+    LOG(WARNING) << "BrowserOS update manifest: no updatable platform here";
+    return std::nullopt;
+  }
+
+  std::optional<PlatformEntry> entry =
+      ParsePlatformEntry(root, platform_key, expected_hosts);
+  if (!entry) {
+    return std::nullopt;
+  }
+  manifest.platforms_.emplace_back(platform_key, std::move(*entry));
+  manifest.valid_ = true;
+  return manifest;
+}
+
+bool Manifest::IsPaused() const {
+  return channel_ == kChannelPaused;
+}
+
+const PlatformEntry* Manifest::GetPlatform(const std::string& key) const {
+  for (const auto& [name, entry] : platforms_) {
+    if (name == key) {
+      return &entry;
+    }
+  }
+  return nullptr;
+}
+
+const PlatformEntry* Manifest::GetPlatformForCurrentBuild() const {
+  const std::string current = CurrentPlatformKey();
+  return current.empty() ? nullptr : GetPlatform(current);
+}
+
+std::string Manifest::CanonicalizeJsonForSigning(const base::Value::Dict& manifest) {
+  // Must match canonical_manifest_bytes() in
+  // tools/release/generate_update_manifests.py byte for byte. base::Value::Dict
+  // iterates in sorted key order and ToJsonString() emits no insignificant
+  // whitespace, which is what the Python side produces with sort_keys=True and
+  // separators=(",", ":").
+  return base::Value(std::move(manifest)).ToJsonString();
+}
+
+}  // namespace browseros_update

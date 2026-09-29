diff --git a/chrome/browser/browseros/update/browseros_update_verifier_win.cc b/chrome/browser/browseros/update/browseros_update_verifier_win.cc
new file mode 100644
index 0000000000000..0000000000000
--- /dev/null
+++ b/chrome/browser/browseros/update/browseros_update_verifier_win.cc
@@ -0,0 +1,148 @@
+// Copyright 2024 The Chromium Authors
+// Use of this source code is governed by a BSD-style license that can be
+// found in the LICENSE file.
+
+// Windows-only. See browseros_update_verifier_stub.cc for every other
+// platform, which fails closed.
+
+#include "chrome/browser/browseros/update/browseros_update_verifier.h"
+
+#include <optional>
+#include <string>
+#include <vector>
+
+#include "base/i18n/i18n.h"
+#include "base/logging.h"
+#include "base/win/windows_hids.h"
+#include "winbase.h"
+#include "wincrypt.h"
+#include "wintrust.h"
+
+namespace browseros_update {
+
+namespace {
+
+GUID kWinTrustActionGenericVerifyV2 = WINTRUST_ACTION_GENERIC_VERIFY_V2;
+
+// Extracts the leaf (signing) certificate's subject from a completed
+// WinVerifyTrust call and returns it as a simple string.
+//
+// The trust provider must be replayed with WTD_STATEACTION_VERIFY to get at
+// the provider state data; forgetting this is the classic way to get a
+// silently empty subject and a check that always passes.
+std::optional<std::wstring> GetSignerSubject(const WINTRUST_DATA& wintrust_data) {
+  WINTRUST_DATA replay = wintrust_data;
+  replay.dwStateAction = WTD_STATEACTION_VERIFY;
+  replay.hWVTStateData = wintrust_data.hWVTStateData;
+
+  GUID action = kWinTrustActionGenericVerifyV2;
+  const LONG replayed = ::WinVerifyTrust(INVALID_HANDLE_VALUE, &action, &replay);
+  if (replayed != ERROR_SUCCESS) {
+    return std::nullopt;
+  }
+
+ CRYPT_PROVIDER_DATA* provider_data =
+      ::WTHelperProvDataFromStateData(replay.hWVTStateData);
+  if (!provider_data) {
+    return std::nullopt;
+  }
+
+ CRYPT_PROVIDER_SGNR* signer = ::WTHelperGetProvSignerFromChain(provider_data, 0, TRUE, 0);
+  if (!signer) {
+    return std::nullopt;
+  }
+
+  CRYPT_PROVIDER_CERT* provider_cert =
+      ::WTHelperGetProvCertFromChain(signer, 0);
+  if (!provider_cert) {
+    return std::nullopt;
+  }
+
+  // provider_cert is a decoded CERT_INFO; the raw encoded bytes are at
+  // pbCertEncoded with cbCertEncoded.
+  PCCERT_CONTEXT cert = CertCreateCertificateContext(
+      X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, provider_cert->pbCertEncoded,
+      static_cast<DWORD>(provider_cert->cbCertEncoded));
+  if (!cert) {
+    return std::nullopt;
+  }
+
+  std::wstring subject;
+  DWORD chars = ::CertGetNameStringW(cert->pCertInfo, CERT_NAME_SIMPLE_STRING_TYPE,
+                                     0, nullptr, nullptr, 0);
+  if (chars > 0) {
+    subject.resize(chars);
+    chars = ::CertGetNameStringW(cert->pCertInfo, CERT_NAME_SIMPLE_STRING_TYPE, 0,
+                                 nullptr, subject.data(), chars);
+    if (chars > 0) {
+      subject.resize(chars - 1);
+    } else {
+      subject.clear();
+    }
+  }
+  ::CertFreeCertificateContext(cert);
+  return subject;
+}
+
+}  // namespace
+
+VerifyResult Verifier::VerifySignature(const base::FilePath& path,
+                                       const std::string& expected_publisher) {
+  WINTRUST_FILE_INFO file_info = {};
+  file_info.cbStruct = sizeof(WINTRUST_FILE_INFO);
+  file_info.pcwszFilePath = path.value().c_str();
+
+  WINTRUST_DATA wintrust_data = {};
+  wintrust_data.cbStruct = sizeof(WINTRUST_DATA);
+  wintrust_data.dwUIChoice = WTD_UI_NONE;
+  wintrust_data.fdwRevocationChecks = WTD_REVOKE_NONE;
+  // Chain building must succeed. Without this, an offline machine with a
+  // cached root would silently fail every update.
+  wintrust_data.dwUnionChoice = WTD_CHOICE_FILE;
+  wintrust_data.pFile = &file_info;
+  wintrust_data.dwStateAction = WTD_STATEACTION_VERIFY;
+  wintrust_data.dwProvFlags =
+      WTD_SAFER_FLAG | WTD_REVOCATION_CHECK_NONE | WTD_CACHE_ONLY_URL_RETRIEVAL;
+
+  GUID action = kWinTrustActionGenericVerifyV2;
+  const LONG status =
+      ::WinVerifyTrust(INVALID_HANDLE_VALUE, &action, &wintrust_data);
+
+  // The provider state stays valid until WTD_STATEACTION_CLOSE, so read the
+  // signer subject before closing. Closing first invalidates hWVTStateData and
+  // GetSignerSubject() would return nothing, turning a publisher check into a
+  // silent pass.
+  std::optional<std::wstring> subject;
+  if (status == ERROR_SUCCESS && !expected_publisher.empty()) {
+    subject = GetSignerSubject(wintrust_data);
+  }
+
+  WINTRUST_DATA close = wintrust_data;
+  close.dwStateAction = WTD_STATEACTION_CLOSE;
+  ::WinVerifyTrust(INVALID_HANDLE_VALUE, &action, &close);
+
+  if (status != ERROR_SUCCESS) {
+    return VerifyResult::Failure("installer is not correctly Authenticode signed");
+  }
+
+  if (expected_publisher.empty()) {
+    return VerifyResult::Success();
+  }
+
+  if (!subject) {
+    return VerifyResult::Failure("could not read the installer signer subject");
+  }
+
+  // Compare case-insensitively: certificate subjects have no reliable case
+  // and Windows returns them as stored, which varies by issuer.
+  const std::wstring expected = base::i18n::ToWide(expected_publisher);
+  if (_wcsicmp(subject->c_str(), expected.c_str()) != 0) {
+    LOG(ERROR) << "BrowserOS update: signer subject mismatch, wanted "
+               << expected_publisher;
+    return VerifyResult::Failure("installer signer is not the expected publisher");
+  }
+
+  return VerifyResult::Success();
+}
+
+}  // namespace browseros_update

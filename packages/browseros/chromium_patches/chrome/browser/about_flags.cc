diff --git a/chrome/browser/about_flags.cc b/chrome/browser/about_flags.cc
index 659379c7d74a1..a819194debde4 100644
--- a/chrome/browser/about_flags.cc
+++ b/chrome/browser/about_flags.cc
@@ -10898,6 +10898,21 @@ const FeatureEntry kFeatureEntries[] = {
     {"bookmarks-tree-view", flag_descriptions::kBookmarksTreeViewName,
      flag_descriptions::kBookmarksTreeViewDescription, kOsDesktop,
      FEATURE_VALUE_TYPE(features::kBookmarksTreeView)},
+
+    {"enable-browseros-alpha-features",
+     flag_descriptions::kBrowserOsAlphaFeaturesName,
+     flag_descriptions::kBrowserOsAlphaFeaturesDescription, kOsDesktop,
+     FEATURE_VALUE_TYPE(features::kBrowserOsAlphaFeatures)},
+
+    {"enable-browseros-keyboard-shortcuts",
+     flag_descriptions::kBrowserOsKeyboardShortcutsName,
+     flag_descriptions::kBrowserOsKeyboardShortcutsDescription, kOsDesktop,
+     FEATURE_VALUE_TYPE(features::kBrowserOsKeyboardShortcuts)},
+
+    {"enable-browseros-native-server",
+     flag_descriptions::kBrowserOsNativeServerName,
+     flag_descriptions::kBrowserOsNativeServerDescription, kOsDesktop,
+     FEATURE_VALUE_TYPE(features::kBrowserOsNativeServer)},
 #endif
 
 #if BUILDFLAG(IS_ANDROID)

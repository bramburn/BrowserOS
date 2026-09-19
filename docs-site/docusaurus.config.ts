import { themes as prismThemes } from "prism-react-renderer";
import type { Config } from "@docusaurus/types";
import type * as Preset from "@docusaurus/preset-classic";

const config: Config = {
  title: "bramburn/BrowserOS",
  tagline: "A self-hostable Chromium fork with an Edge/Chrome-grade auto-update workflow.",
  favicon: "img/favicon.ico",

  url: "https://bramburn.github.io",
  baseUrl: "/BrowserOS/",

  organizationName: "bramburn",
  projectName: "BrowserOS",

  onBrokenLinks: "throw",
  onBrokenMarkdownLinks: "warn",

  i18n: {
    defaultLocale: "en",
    locales: ["en"],
  },

  presets: [
    [
      "classic",
      {
        docs: {
          routeBasePath: "/",
          path: "docs",
          sidebarPath: "./sidebars.ts",
          editUrl:
            "https://github.com/bramburn/BrowserOS/edit/main/docs-site/",
          remarkPlugins: [],
          rehypePlugins: [],
          showLastUpdateTime: true,
        },
        blog: false,
        theme: {
          image: "img/social-card.png",
          colorMode: {
            defaultMode: "dark",
            respectPrefersColorScheme: true,
          },
        },
      } satisfies Preset.Options,
    ],
  ],

  themeConfig: {
    image: "img/social-card.png",
    colorMode: {
      defaultMode: "dark",
      respectPrefersColorScheme: true,
    },
    prism: {
      theme: prismThemes.github,
      darkTheme: prismThemes.dracula,
      additionalLanguages: ["powershell", "typescript", "bash", "yaml"],
    },
    navbar: {
      title: "bramburn/BrowserOS",
      logo: {
        alt: "bramburn/BrowserOS logo",
        src: "img/logo.svg",
      },
      items: [
        {
          type: "docSidebar",
          sidebarId: "mainSidebar",
          position: "left",
          label: "Docs",
        },
        {
          href: "https://github.com/bramburn/BrowserOS",
          label: "GitHub",
          position: "right",
        },
        {
          href: "https://github.com/bramburn/BrowserOS/releases",
          label: "Releases",
          position: "right",
        },
      ],
    },
    footer: {
      style: "dark",
      links: [
        {
          title: "Fork",
          items: [
            {
              label: "GitHub",
              href: "https://github.com/bramburn/BrowserOS",
            },
            {
              label: "WHY_FORK.md",
              to: "/why-fork",
            },
          ],
        },
        {
          title: "Upstream",
          items: [
            {
              label: "browseros-ai/BrowserOS",
              href: "https://github.com/browseros-ai/BrowserOS",
            },
            {
              label: "docs.browseros.com",
              href: "https://docs.browseros.com",
            },
          ],
        },
        {
          title: "License",
          items: [
            {
              label: "AGPL-3.0",
              href: "https://github.com/bramburn/BrowserOS/blob/main/LICENSE",
            },
          ],
        },
      ],
      copyright: `Fork maintained by <a href="https://github.com/bramburn">bramburn</a>. Built on top of <a href="https://github.com/browseros-ai/BrowserOS">browseros-ai/BrowserOS</a> (AGPL-3.0).`,
    },
  } satisfies Preset.ThemeConfig,
};

export default config;
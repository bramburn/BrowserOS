import type { SidebarsConfig } from "@docusaurus/plugin-content-docs";

const sidebars: SidebarsConfig = {
  mainSidebar: [
    "intro",
    {
      type: "category",
      label: "Roadmap",
      collapsed: false,
      items: ["roadmap", "why-fork"],
    },
    {
      type: "category",
      label: "Build & Release",
      collapsed: false,
      items: ["build", "ubuntu-dev", "toolchain", "release", "update-server", "advance-installer"],
    },
    {
      type: "category",
      label: "Architecture",
      collapsed: true,
      items: ["architecture", "mcp-tool-spec", "agents"],
    },
    {
      type: "category",
      label: "Operations",
      collapsed: true,
      items: ["ci-runners", "runbook"],
    },
  ],
};

export default sidebars;
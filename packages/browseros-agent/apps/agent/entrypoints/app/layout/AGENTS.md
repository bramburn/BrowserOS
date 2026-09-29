# `entrypoints/app/layout/` — route layouts for the app shell

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/app`.

## What's here

Three React Router layout components used as `<Route element={...}>` wrappers in
`app/App.tsx`. A layout renders chrome and an `<Outlet />`; it never renders a page's
content itself. All three mount `RpcClientProvider`, which is the client that talks
to the local agent server — anything under a layout can call the server.

## Contents

```
layout/
├── AuthLayout.tsx          ← centred logo + <Outlet/>; wraps login/logout/profile/magic-link
├── SidebarLayout.tsx       ← main shell: AppSidebar rail, hover-expand, mobile sheet, shortcuts dialog
└── SettingsSidebarLayout.tsx ← settings shell: SettingsSidebar + SETTINGS_PAGE_VIEWED tracking
```

## Rules

- **LY1 — A layout provides context, not content.** If you are about to import a page
  component here, it belongs on a route in `App.tsx` instead.
- **LY2 — `RpcClientProvider` goes in the layout, not the page.** Pages under a layout
  assume the RPC client exists. Adding a second provider inside a page creates two
  clients and duplicated request state.
- **LY3 — `SidebarLayout` uses hover-to-expand with a 150 ms collapse delay**
  (`COLLAPSE_DELAY`) and a `pl-14` body offset for the collapsed rail width. Change
  one and the other must follow, or content slides under the rail.
- **LY4 — `/home/chat` gets a different scroll container.** `SidebarLayout` renders a
  full-height `overflow-hidden` main only for that path; every other route gets a
  normal scrolling page. Keep the special case when adding routes.
- **LY5 — Analytics for settings views live in `SettingsSidebarLayout`,**
  keyed on `location.pathname` with `SETTINGS_PAGE_VIEWED_EVENT`. Adding a settings
  route therefore needs no extra tracking code — but removing the effect would.
- **LY6 — `ShortcutsDialog` is mounted by `SidebarLayout`,** not by the pages that open
  it. It is imported from `../../../newtab/index/ShortcutsDialog`; keep that import
  path working or move the dialog.

## Workflows

**Adding a new layout**
1. Create `<Name>Layout.tsx` here.
2. Mount `RpcClientProvider` around the shell.
3. Render `<Outlet />` in the content slot.
4. Add `<Route element={<YourLayout />}> … </Route>` in `app/App.tsx`.
5. Add the nav entry in the matching `components/sidebar/*` component.

**Fixing responsive behaviour**
1. `isMobile` comes from `@/hooks/use-mobile`.
2. The mobile branch renders a top bar + `Sheet`; the desktop branch a fixed rail.
3. Both branches must wrap `RpcClientProvider` — do not hoist it and forget one branch.

**Adding a keyboard shortcut**
1. Add the key handling where the dialog is opened (`SidebarLayout`).
2. The dialog itself is `../../../newtab/index/ShortcutsDialog` — edit it there.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — where each layout is mounted.
- [`../../../../components/sidebar/AppSidebar.tsx`](../../../components/sidebar/AppSidebar.tsx) — the main nav.
- [`../../../../components/sidebar/SettingsSidebar.tsx`](../../../components/sidebar/SettingsSidebar.tsx) — settings nav.
- [`../../../../lib/rpc/RpcClientProvider.tsx`](../../../lib/rpc/RpcClientProvider.tsx) — the provider every layout mounts.
- [`../../../newtab/index/AGENTS.md`](../../newtab/index/AGENTS.md) — owner of `ShortcutsDialog`.

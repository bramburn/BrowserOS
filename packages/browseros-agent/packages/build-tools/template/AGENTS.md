# `packages/build-tools/template/` — Lima VM template

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/packages/build-tools`.

## What's here

One file: `browseros-vm.yaml`, the committed Lima template that defines the
BrowserOS VM. There is no build step — `limactl` consumes this file directly
at runtime, both in the dev loop and in production server artifacts. It
describes an Ubuntu 24.04 minimal cloud image (arm64 + amd64), a `vz` VM with
4 CPU / 4 GiB RAM / 20 GiB disk, Lima-managed **rootless** containerd, a
one-shot provision script, a `nerdctl` readiness probe, and a port forward for
the containerd socket.

## Contents

```
build-tools/template/
└── browseros-vm.yaml   ← the only file; guarded by ../../tests/vm-template.test.ts
```

## Rules

**TM1 — Rootless containerd, always.** `containerd: { system: false, user:
true }` and the probe's `nerdctl info` loop. The host socket is forwarded
from `/run/user/{{.UID}}/containerd-rootless/containerd.sock` to
`{{.Dir}}/sock/containerd.sock`. A system containerd socket path will not
work with the server's runtime code.

**TM2 — No podman, ever.** `tests/vm-template.test.ts` explicitly asserts the
template does not contain `podman`, `debian`, `sudo nerdctl`, or
`/var/run/containerd/containerd.sock`.

**TM3 — Provision must be idempotent.** The script guards on
`/etc/browseros-vm-provisioned` and only writes
`/etc/browseros-vm-version` with `runtime:containerd-rootless` plus a UTC
timestamp, so repeat VM starts are cheap and deterministic.

**TM4 — Images are digest-pinned first.** The arm64 and amd64 entries carry
`sha256:` digests for the 20260415 minimal release; the two unpinned entries
below them are fallbacks. If you change an image, change the digest in the
same edit and update the test's expected filename strings.

**TM5 — Keep guest-side services off.** The provision script disables
`unattended-upgrades`, `apt-daily*`, and `snapd` so the guest cannot reboot
or update itself mid-session.

**TM6 — Edit the test with the template.** Any intentional change to
`template/browseros-vm.yaml` must be reflected in
`../tests/vm-template.test.ts` in the same change, and that test is the only
guard on these values.

## Workflows

**Dev-loop smoke test:** `limactl start --name browseros-vm-dev
packages/browseros-agent/packages/build-tools/template/browseros-vm.yaml`,
then `limactl shell browseros-vm-dev nerdctl info`, verify
`test -S "$(limactl list browseros-vm-dev --format '{{.Dir}}')/sock/containerd.sock"`,
then `limactl delete --force browseros-vm-dev`.

**Changing VM sizing:** 1. Edit `cpus` / `memory` / `disk` here. 2. Delete and
recreate any existing VM — Lima does not re-provision an existing instance.
3. Run `bun test packages/build-tools/tests/vm-template.test.ts`.

**Shipping the VM with a server artifact:** nothing to do in this folder — the
staging rules live in `../../../../scripts/build/config/server-prod-resources.json`
and the bundled `limactl` + guest agents are uploaded to R2 separately.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — build-tools scope (rules BT1–BT6).
- [`../README.md`](../README.md) — Lima dev loop and guest-agent layout.
- [`../tests/vm-template.test.ts`](../tests/vm-template.test.ts) — the guard test.
- [`../tests/AGENTS.md`](../tests/AGENTS.md) — the bun test suite for this package.

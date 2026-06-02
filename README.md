# Krit-Faslo Source Workspace

This branch contains the source rebuild package for the Krit-Faslo ASA plugin.

Primary project:

```text
05_rebuild_workspace/FasolaFlintScaler/rebuild-source/
```

The repository is intentionally source-first. Compiled plugin binaries, PDBs, build folders, and old release packages are not kept in this branch.

## What Is Included

```text
05_rebuild_workspace/FasolaFlintScaler/rebuild-source/
  CMakeLists.txt
  config.json
  config_commented.json
  include/FasolaFlintScaler/
  src/
  cmake/
  docs/
  packaging/
  scripts/
  third_party/nlohmann/

03_license_admin/
  KritFaslo-LicenseTool.ps1
  krit-faslo-public-key.json
  README.md
  PLUGIN_INTEGRATION.md
```

## Review

Start with:

```text
05_rebuild_workspace/FasolaFlintScaler/rebuild-source/README.md
05_rebuild_workspace/FasolaFlintScaler/rebuild-source/config.json
05_rebuild_workspace/FasolaFlintScaler/rebuild-source/config_commented.json
05_rebuild_workspace/FasolaFlintScaler/rebuild-source/docs/SDK_WIRING.md
05_rebuild_workspace/FasolaFlintScaler/rebuild-source/src/ArkApiBridge.cpp
```

## Build Status

Do not compile from this branch until the compile step is explicitly requested.

The non-compile preflight script is:

```powershell
cd "05_rebuild_workspace/FasolaFlintScaler/rebuild-source"
.\scripts\Preflight.ps1
```

That script checks config shape, SDK paths, and confirms no DLLs exist in the source package. It does not build.

# ASA SDK Wiring

This project is ready for the ASA runtime library path, but exact harvest hook detour signatures still require the ASA API developer headers.

## Known Runtime Root

The remote server currently has a usable runtime/library root:

```text
C:\Users\Administrator\Downloads\AsaApi_1.19
```

That folder contains:

```text
ArkApi\AsaApi.dll
Lib\AsaApi.lib
```

It does not appear to contain developer headers.

## Matching Header Source

The matching ASA API 1.19 source headers were found locally under:

```text
C:\Users\sutto\Downloads\AsaApi-1.19\AsaApi-1.19
```

For source wiring, point `ASA_API_ROOT` to that folder. It contains the public/private API headers needed to inspect command and hook signatures.

When headers and runtime files are in different folders, use:

```powershell
$env:ASA_API_ROOT = "C:\Users\sutto\Downloads\AsaApi-1.19\AsaApi-1.19"
$env:ASA_API_LIB_ROOT = "C:\Users\Administrator\Downloads\AsaApi_1.19"
```

## Required Developer Headers

Exact hook wiring needs files such as:

```text
AsaApi.h
ArkApi.h
IHooks.h
Commands.h
API\ARK\Ark.h
```

Without these headers, the bridge can document and centralize recovered hook names, but should not guess detour signatures.

## Recovered Hook Names

The hook names are centralized in `src/ArkApiBridge.cpp`:

```text
UPrimalItem.IncrementItemQuantity(int,bool,bool,bool,bool,bool)
UPrimalInventoryComponent.AddItem(FItemNetInfo&,bool,bool,bool,FItemNetID*,bool,bool,bool,AShooterCharacter*,bool,bool,bool,bool)
UPrimalInventoryComponent.AddItemObject(UPrimalItem*)
UPrimalInventoryComponent.AddItemObjectEx
```

`AddItemObjectEx` was recovered from the original binary but is not present in the ASA API 1.19 headers. The source does not guess its detour signature.

## Preflight

Run this without compiling:

```powershell
$env:ASA_API_ROOT = "C:\Users\sutto\Downloads\AsaApi-1.19\AsaApi-1.19"
$env:ASA_API_LIB_ROOT = "C:\Users\Administrator\Downloads\AsaApi_1.19"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Preflight.ps1
```

The preflight checks config parsing, runtime API files, header availability, and confirms no DLL is present under the source root.

## CMake Finder Behavior

`cmake/FindAsaApi.cmake` supports split SDK roots:

- `ASA_API_ROOT` is used for headers.
- `ASA_API_LIB_ROOT` is used for `AsaApi.lib` and `AsaApi.dll`.

The finder adds both ASA public and private header folders to the plugin include path when they exist. This is required for the 1.19 source layout, where command/hook headers are split between:

```text
AsaApi\Core\Public
AsaApi\Core\Private
```

The runtime DLL is reported during configure as a readiness check, but the import library is the required linker input.

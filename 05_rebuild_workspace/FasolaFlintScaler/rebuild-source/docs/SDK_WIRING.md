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
UPrimalItem.IncrementItemQuantity
UPrimalInventoryComponent.AddItem
UPrimalInventoryComponent.AddItemObject
UPrimalInventoryComponent.AddItemObjectEx
```

## Preflight

Run this without compiling:

```powershell
$env:ASA_API_ROOT = "C:\Users\Administrator\Downloads\AsaApi_1.19"
.\scripts\Preflight.ps1
```

The preflight checks config parsing, runtime API files, header availability, and confirms no DLL is present under the source root.

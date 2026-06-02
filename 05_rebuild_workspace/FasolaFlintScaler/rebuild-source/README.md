# Krit-Faslo

Krit-Faslo is a native Windows x64 plugin for ArkServerAPI / ASA Server API. The plugin monitors Fasola harvest flows and adjusts flint rewards toward a configurable target range.

The internal module name is `FasolaFlintScaler`; the release DLL name is `Krit-Faslo.dll`.

## Project Layout

```text
rebuild-source/
  CMakeLists.txt
  config.json
  config_commented.json
  include/FasolaFlintScaler/
  src/
  third_party/nlohmann/
  packaging/
```

## Requirements

- Windows x64
- Visual Studio 2022 or compatible MSVC toolchain
- CMake 3.24 or newer
- ArkServerAPI / ASA Server API headers and import library
- Microsoft Visual C++ Redistributable installed on the server

Set `ASA_API_ROOT` to the folder that contains the ASA API headers. Set `ASA_API_LIB_ROOT` to the folder that contains `AsaApi.lib` and the server runtime DLL when those files live somewhere else.

```powershell
$env:ASA_API_ROOT = "C:\Users\sutto\Downloads\AsaApi-1.19\AsaApi-1.19"
$env:ASA_API_LIB_ROOT = "C:\Users\Administrator\Downloads\AsaApi_1.19"
```

The CMake finder checks common ASA API source and install layouts below those roots, including public/private source headers, `ArkApi`, `AsaApi`, `Binaries/Win64`, `Lib`, and common Visual Studio output folders.

## Configuration

The runtime file is `config.json`. Keep it as strict JSON. Use `config_commented.json` as the readable reference.

| Setting | Description |
| --- | --- |
| `Enabled` | Enables or disables plugin behavior. |
| `LicenseKey` | Server license token. |
| `HarvestMultiplier` | Fallback harvest multiplier. |
| `ReadHarvestAmountMultiplierFromIni` | Reads `HarvestAmountMultiplier` from `GameUserSettings.ini` when enabled. |
| `GameUserSettingsPath` | Optional explicit path to `GameUserSettings.ini`. |
| `BaselineOfficialFlint` | Official baseline flint amount used by the scaler. |
| `TargetFlintMin` | Minimum target flint amount. |
| `TargetFlintMax` | Maximum target flint amount. |
| `MinTriggerAmount` | Minimum incoming stack amount that can be adjusted. |
| `MaxTriggerAmount` | Maximum incoming stack amount that can be adjusted. |
| `VariancePercent` | Allowed tolerance around the selected target amount. |
| `AntiDupeEnabled` | Enables anti-dupe guardrails before extra flint is added. |
| `LogAntiDupeEvents` | Logs anti-dupe rejection details when suspicious events are blocked. |
| `MaxExtraFlintPerStack` | Maximum extra flint one stack adjustment can add. |
| `MaxExtraFlintPerSecond` | Maximum extra flint the plugin may add per second. |
| `DuplicateDetectionWindowMs` | Time window used to catch repeated identical events too close together. |
| `BlockNegativeOrZeroAdjustments` | Rejects zero, negative, or invalid extra-flint adjustments. |
| `RequireFasolaOwnerMatch` | Requires the item owner or inventory to match a configured Fasola class. |
| `ShowHudNotificationForExtraFlint` | Sends a HUD notice when extra flint is added. |
| `DebugLogging` | Writes diagnostics to `FasolaFlintScaler.debug.log`. |
| `FasolaClasses` | Character blueprint classes treated as Fasola targets. |
| `FlintClass` | Flint resource blueprint class. |

Anti-dupe guardrails are evaluated after the scaler chooses an adjustment and before the plugin writes extra flint to the inventory. Blocked adjustments are skipped without modifying the stack.

## Commands

Run these from the server console:

```text
reload.faslo
reload.fasola
faslo.status
```

## Licensing

`LicenseKey` must use this token format:

```text
KFASLO-v1.<base64url-payload-json>.<base64url-rsa-signature>
```

The plugin validates the RSA-3072/SHA-256 PKCS#1 signature, product, version, and expiry before enabling behavior.

Only public verification material belongs in the plugin. Keep private signing material and license issuing tools off the game server.

Expected payload fields:

| Field | Description |
| --- | --- |
| `product` | Must be `Krit-Faslo`. |
| `version` | Must be `1`. |
| `customer` | Display/audit value. |
| `issuedUtc` | UTC issue timestamp. |
| `expires` | Expiry date in `YYYY-MM-DD` format. |
| `serverId` | Optional metadata. |
| `nonce` | Unique token id. |

## Installation

After the approved compile step creates `Krit-Faslo.dll`, install the plugin folder here:

```text
ShooterGame/Binaries/Win64/ArkApi/Plugins/Krit-Faslo/
```

The installed folder should contain:

```text
Krit-Faslo/
  Krit-Faslo.dll
  config.json
```

Optional documentation files can also be kept beside the runtime files:

```text
Krit-Faslo/
  config_commented.json
  README.md
```

## Build

Do not build from this workspace until the compile step has been explicitly requested.

Before building, run the non-compile preflight:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Preflight.ps1 `
  -AsaApiRoot "C:\Users\sutto\Downloads\AsaApi-1.19\AsaApi-1.19" `
  -AsaApiLibRoot "C:\Users\Administrator\Downloads\AsaApi_1.19"
```

When building is intended, configure from the project root:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

The output DLL is named `Krit-Faslo.dll`.

The verified ASA 1.19 harvest hook names and detour signatures are centralized in `src/ArkApiBridge.cpp`.

## SDK Wiring Status

The runtime package path found on the server is:

```powershell
$env:ASA_API_ROOT = "C:\Users\Administrator\Downloads\AsaApi_1.19"
```

That runtime package supplies `AsaApi.dll` and `AsaApi.lib`.

The matching ASA API 1.19 source headers were found locally under:

```powershell
$env:ASA_API_ROOT = "C:\Users\sutto\Downloads\AsaApi-1.19\AsaApi-1.19"
```

If the headers and runtime package are separate, set the library/runtime root too:

```powershell
$env:ASA_API_LIB_ROOT = "C:\Users\Administrator\Downloads\AsaApi_1.19"
```

`src/ArkApiBridge.cpp` uses the confirmed 1.19 signatures for `IncrementItemQuantity`, `AddItem`, and `AddItemObject`. `AddItemObjectEx` was recovered from the original binary but is not declared in the ASA API 1.19 headers, so it is documented but not hooked. See `docs/SDK_WIRING.md`.

Before the first live server test, review `docs/PRE_LIVE_TEST_CHECKLIST.md`.

## Troubleshooting

- If CMake cannot find ASA API, verify `ASA_API_ROOT`.
- If the plugin loads but does not apply behavior, check `LicenseKey`, `Enabled`, and the Fasola/flint blueprint class names.
- If harvest values look wrong, verify `HarvestAmountMultiplier` in `GameUserSettings.ini` or set `ReadHarvestAmountMultiplierFromIni` to `false`.
- Enable `DebugLogging` only while diagnosing server behavior. Debug logs show each harvest event amount, item class, inventory class, owner class, trigger range, target range, rejection reason, and extra flint amount added.

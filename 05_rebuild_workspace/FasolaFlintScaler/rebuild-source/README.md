# Krit-Faslo

Krit-Faslo is a native Windows x64 plugin for ArkServerAPI / ASA Server API. The plugin monitors Fasola harvest flows and adjusts flint rewards toward a configurable target range.

The internal module name is `FasolaFlintScaler`; the release DLL name is `Krit-Faslo.dll`.

## Project Layout

```text
rebuild-source/
  CMakeLists.txt
  config.json
  CONFIG-COMMENTED.jsonc
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

Set `ASA_API_ROOT` to the folder that contains the ASA API headers and `AsaApi.lib`.

```powershell
$env:ASA_API_ROOT = "C:\Path\To\ArkApi"
```

The CMake finder checks common ASA API source and install layouts below that root, including `include`, `ArkApi`, `AsaApi`, `Binaries/Win64`, and common Visual Studio output folders.

## Configuration

The runtime file is `config.json`. Keep it as strict JSON. Use `CONFIG-COMMENTED.jsonc` or `config_commented.json` as the readable reference.

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
| `ShowHudNotificationForExtraFlint` | Sends a HUD notice when extra flint is added. |
| `DebugLogging` | Writes diagnostics to `FasolaFlintScaler.debug.log`. |
| `FasolaClasses` | Character blueprint classes treated as Fasola targets. |
| `FlintClass` | Flint resource blueprint class. |

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

Install the release folder here:

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
  CONFIG-COMMENTED.jsonc
  README.md
```

## Build

Do not build from this workspace until the compile step has been requested.

When building is intended, configure from the project root:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

The output DLL is named `Krit-Faslo.dll`.

The recovered harvest hook names are centralized in `src/ArkApiBridge.cpp`. Final detour signatures must be matched against the installed ASA API headers before compiling.

## Troubleshooting

- If CMake cannot find ASA API, verify `ASA_API_ROOT`.
- If the plugin loads but does not apply behavior, check `LicenseKey`, `Enabled`, and the Fasola/flint blueprint class names.
- If harvest values look wrong, verify `HarvestAmountMultiplier` in `GameUserSettings.ini` or set `ReadHarvestAmountMultiplierFromIni` to `false`.
- Enable `DebugLogging` only while diagnosing server behavior.

# Krit-Faslo

Krit-Faslo is an ARK Survival Ascended plugin for ArkServerAPI / ASA Server API.

## Install

Install this release folder here on the ASA server:

```text
ShooterGame/Binaries/Win64/ArkApi/Plugins/Krit-Faslo/
```

The folder should contain:

```text
Krit-Faslo/
  Krit-Faslo.dll
  config.json
```

`AsaApi.dll` is expected to be provided by the ArkServerAPI / ASA Server API installation. It is not bundled with this plugin release.

## Files

- `Krit-Faslo.dll` is the compiled Windows x64 plugin binary.
- `config.json` is the runtime configuration file used by the plugin.
- `CONFIG-COMMENTED.jsonc` is a human-readable commented example of the configuration.

Do not edit, decompile, patch, or rebuild `Krit-Faslo.dll` as part of normal installation. The DLL is binary and will not display as readable source code in a text editor.

Keep `config.json` as valid JSON. JSON comments are not valid in the runtime file unless the plugin explicitly supports JSONC.

## Licensing

Paste the issued Krit-Faslo license string into `LicenseKey` in `config.json`.

```json
"LicenseKey": "KFASLO-v1..."
```

License keys should be issued by the plugin owner using the admin-side license tooling. The private signing key must not be shipped with the plugin or placed on a game server.

This release DLL verifies `LicenseKey` at runtime. If the key is missing, malformed, expired, or signed by a different private key, Krit-Faslo logs a license rejection and disables plugin behavior.

## Runtime Testing

Test this plugin on a spare or local ASA server before installing it on a production server.

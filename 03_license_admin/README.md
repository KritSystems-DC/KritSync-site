# Krit-Faslo License Admin

This folder contains admin-side tooling for issuing Krit-Faslo license keys.

Do not ship this folder to plugin users, and do not put the private key in the
server plugin folder.

## Create a Signing Keypair

Run once from this folder:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\KritFaslo-LicenseTool.ps1 -Command init
```

This creates:

- `krit-faslo-private-key.json`
- `krit-faslo-public-key.json`

Keep `krit-faslo-private-key.json` private. The public key is what the plugin
source should embed for runtime verification.

## Issue a License

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\KritFaslo-LicenseTool.ps1 -Command issue -Customer "CustomerName" -Expires 2026-12-31
```

Optional server id metadata:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\KritFaslo-LicenseTool.ps1 -Command issue -Customer "CustomerName" -Expires 2026-12-31 -ServerId "SERVER-ID-HERE"
```

The current plugin records `serverId` in the signed payload but does not enforce
server locking because no stable ASA server identifier has been selected.

The command prints a license string beginning with `KFASLO-v1.`. Give that full
string to the user and have them paste it into `LicenseKey` in `config.json`.

For the test server workflow, see `TEST_LICENSE_WORKFLOW.md`.

## Verify a License

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\KritFaslo-LicenseTool.ps1 -Command verify -LicenseKey "KFASLO-v1...."
```

Verification checks the signature, product name, version, and expiry date.

## Plugin Requirement

This tool only generates and verifies licenses externally. For licensing to be
enforced by the ASA plugin, the Krit-Faslo plugin source must read
`LicenseKey` from `config.json`, verify the token using the public key, and
disable plugin behavior when the license is missing, invalid, or expired.

# Test License Workflow

Use this workflow to prepare a valid `LicenseKey` for the test server.

## Current Key State

The repository includes:

```text
03_license_admin/krit-faslo-public-key.json
```

The private key is intentionally ignored by Git:

```text
03_license_admin/krit-faslo-private-key.json
```

Do not commit, upload, or copy the private key to a game server.

## Check Whether This Machine Can Issue A License

From the repository root:

```powershell
Test-Path "03_license_admin\krit-faslo-private-key.json"
```

If this returns `False`, this machine cannot issue a valid license for the public key currently embedded in the plugin source.

## Issue A Test License

Run this only on the machine that has the matching private key:

```powershell
cd "C:\Users\sutto\test pull\KritSync-site\03_license_admin"

powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\KritFaslo-LicenseTool.ps1 `
  -Command issue `
  -Customer "TheKriticals-Test" `
  -Expires 2026-12-31 `
  -ServerId "TEST-ASA-SERVER" `
  -OutFile ".\krit-faslo-test-license.txt"
```

The output starts with:

```text
KFASLO-v1.
```

## Verify The Test License

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\KritFaslo-LicenseTool.ps1 `
  -Command verify `
  -LicenseKey (Get-Content ".\krit-faslo-test-license.txt" -Raw)
```

The command should print the signed payload JSON. If verification fails, do not put the license into `config.json`.

## Put The License Into The Test Config

Paste the full `KFASLO-v1...` value into:

```text
05_rebuild_workspace/FasolaFlintScaler/rebuild-source/config.json
```

Set:

```json
"LicenseKey": "KFASLO-v1..."
```

Do not commit a customer or test license unless that is explicitly intended.

## Confirm With Preflight

From the source package:

```powershell
cd "C:\Users\sutto\test pull\KritSync-site\05_rebuild_workspace\FasolaFlintScaler\rebuild-source"

.\scripts\Preflight.ps1 `
  -AsaApiRoot "C:\Users\sutto\Downloads\AsaApi-1.19\AsaApi-1.19" `
  -AsaApiLibRoot "C:\Users\Administrator\Downloads\AsaApi_1.19"
```

The `LicenseKey present` line should change from `MISSING` to `OK`.

# Pre-Live-Test Checklist

Use this checklist before the first server compile or live test pass.

## Source State

- Confirm the branch is `krit-faslo-full-build`.
- Confirm the working tree is clean before copying or compiling anything.
- Confirm no DLL exists under `rebuild-source/`.
- Confirm the source package is reviewed from:

```text
05_rebuild_workspace/FasolaFlintScaler/rebuild-source/
```

## Configuration

- Open `config.json` and confirm `Enabled` is set for the intended test.
- Confirm `LicenseKey` is populated with a valid `KFASLO-v1` license before expecting runtime behavior.
- Confirm `HarvestMultiplier`, `TargetFlintMin`, `TargetFlintMax`, `MinTriggerAmount`, `MaxTriggerAmount`, and `VariancePercent` match the test server target.
- Confirm `FasolaClasses` contains the exact Fasola blueprint class names expected on the test server.
- Confirm `FlintClass` matches the flint resource class used by the server.
- Use `config_commented.json` only as the readable reference; keep `config.json` strict JSON.

## Anti-Dupe Settings

- Confirm `AntiDupeEnabled` is `true` for the first controlled test.
- Confirm `MaxExtraFlintPerStack` is high enough for the expected target range but low enough to block runaway adds.
- Confirm `MaxExtraFlintPerSecond` is suitable for the number of players/harvest events expected during the first test.
- Confirm `DuplicateDetectionWindowMs` is not so high that normal repeated harvests are blocked.
- Confirm `BlockNegativeOrZeroAdjustments` is `true`.
- Confirm `RequireFasolaOwnerMatch` is `true` unless deliberately testing non-Fasola detection.
- Confirm `LogAntiDupeEvents` is `true` for the first live test pass.

## SDK Paths

- Confirm the ASA header root is available:

```text
C:\Users\sutto\Downloads\AsaApi-1.19\AsaApi-1.19
```

- Confirm the ASA runtime/library root is available on the server:

```text
C:\Users\Administrator\Downloads\AsaApi_1.19
```

- If headers and libraries are in separate folders, set both variables before any future build:

```powershell
$env:ASA_API_ROOT = "C:\Users\sutto\Downloads\AsaApi-1.19\AsaApi-1.19"
$env:ASA_API_LIB_ROOT = "C:\Users\Administrator\Downloads\AsaApi_1.19"
```

## Logging

- Set `DebugLogging` to `true` for the first live test.
- Watch `FasolaFlintScaler.debug.log` for harvest event lines.
- Watch normal plugin logs for startup, license validation, hook registration, and anti-dupe block messages.
- After the first controlled pass, set `DebugLogging` back to `false` unless more diagnosis is needed.

## First Server Boot Checks

- Confirm the plugin loads without missing dependency errors.
- Confirm the license validation log reports accepted status.
- Confirm the harvest hooks register for `IncrementItemQuantity`, `AddItem`, and `AddItemObject`.
- Confirm no warning appears for a hook other than the documented `AddItemObjectEx` note.
- Confirm no unexpected anti-dupe blocks appear during normal Fasola harvesting.

## First Harvest Test

- Spawn or use one known Fasola target.
- Harvest a small controlled sample with `DebugLogging` enabled.
- Confirm the debug log shows `flint_match=true` and `fasola_match=true`.
- Confirm extra flint is only added after the anti-dupe checks pass.
- Confirm rejected events include a clear reason and do not modify the stack.
- Record the original amount, target amount, and added amount from the log.

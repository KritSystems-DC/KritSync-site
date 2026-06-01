Krit-Faslo Clean Release Workspace
==================================

This workspace contains the minimal verified Krit-Faslo plugin release.

Ship this folder:

  01_release/Krit-Faslo/

Install target:

  ShooterGame/Binaries/Win64/ArkApi/Plugins/Krit-Faslo/

Required files included:

  Krit-Faslo.dll
    Main plugin DLL.

  config.json
    Plugin configuration, including a LicenseKey placeholder.

  README.md
    Release install and licensing notes.

  CONFIG-COMMENTED.jsonc
    Commented config example for humans.

Admin-only files included:

  03_license_admin/
    Local tooling for creating signing keys, issuing licenses, and verifying
    licenses. Do not ship the private key or place it on a game server.

Not included:

  AsaApi.dll
    Expected to come from the ArkServerAPI / ASA Server API installation.

  Poco/OpenSSL/minizip/z DLLs
    Static dependency checks show Krit-Faslo.dll does not directly import them.
    Keep the old full package only as a rollback option until this minimal
    package has been tested on a spare/local ASA server.

Before live deployment:

  1. Install 01_release/Krit-Faslo/ on a spare/local server.
  2. Restart the server.
  3. Check ArkApi logs for plugin load, missing DLL, config, or runtime errors.
  4. If using licensing, issue a test key and confirm the plugin accepts it in
     ArkApi logs.

No DLLs were patched, decompiled, or reverse engineered while creating this
clean workspace. The current DLL was rebuilt from the FasolaFlintScaler source
project to enforce LicenseKey with the generated public key.

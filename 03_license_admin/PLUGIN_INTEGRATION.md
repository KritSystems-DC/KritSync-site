# Krit-Faslo Plugin License Integration

The current clean release DLL has been rebuilt from the FasolaFlintScaler source
project with license enforcement.

The runtime verification contract is:

- Config field: `LicenseKey`
- Token format: `KFASLO-v1.<base64url-payload-json>.<base64url-rsa-signature>`
- Signature algorithm: RSA 3072, SHA-256, PKCS#1 v1.5
- Required payload fields:
  - `product`: must equal `Krit-Faslo`
  - `version`: must equal `1`
  - `customer`: display/audit value
  - `issuedUtc`: UTC issue timestamp
  - `expires`: `YYYY-MM-DD`
  - `serverId`: optional metadata, not enforced by the current DLL
  - `nonce`: unique token id

Runtime behavior:

- On plugin load or config reload, read `LicenseKey` from `config.json`.
- Reject empty, malformed, invalid-signature, wrong-product, unsupported-version,
  and expired tokens.
- Current enforcement checks signature, product, version, and expiry.
- Add server binding later only after choosing a stable ASA server identifier.
- Log a clear ArkApi message when the license is accepted or rejected.
- Disable Krit-Faslo behavior when the license is rejected.

Only embed `krit-faslo-public-key.json` data in the plugin source. Never embed
or distribute `krit-faslo-private-key.json`.

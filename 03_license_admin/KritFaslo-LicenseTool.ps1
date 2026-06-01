param(
  [Parameter(Mandatory = $true)]
  [ValidateSet('init', 'issue', 'verify')]
  [string]$Command,

  [string]$PrivateKeyPath = '.\krit-faslo-private-key.json',
  [string]$PublicKeyPath = '.\krit-faslo-public-key.json',
  [string]$Customer = '',
  [string]$Expires = '',
  [string]$ServerId = '',
  [string]$LicenseKey = '',
  [string]$OutFile = ''
)

$ErrorActionPreference = 'Stop'
$Product = 'Krit-Faslo'
$TokenPrefix = 'KFASLO-v1'

function ConvertTo-Base64Url {
  param([byte[]]$Bytes)
  [Convert]::ToBase64String($Bytes).TrimEnd('=').Replace('+', '-').Replace('/', '_')
}

function ConvertFrom-Base64Url {
  param([string]$Text)
  $padded = $Text.Replace('-', '+').Replace('_', '/')
  switch ($padded.Length % 4) {
    2 { $padded += '==' }
    3 { $padded += '=' }
    0 { }
    default { throw "Invalid base64url value." }
  }
  [Convert]::FromBase64String($padded)
}

function ConvertFrom-RsaJson {
  param([string]$Path)
  $json = Get-Content -LiteralPath $Path -Raw | ConvertFrom-Json
  $params = New-Object System.Security.Cryptography.RSAParameters
  $params.Modulus = ConvertFrom-Base64Url $json.n
  $params.Exponent = ConvertFrom-Base64Url $json.e
  if ($json.d) {
    $params.D = ConvertFrom-Base64Url $json.d
    $params.P = ConvertFrom-Base64Url $json.p
    $params.Q = ConvertFrom-Base64Url $json.q
    $params.DP = ConvertFrom-Base64Url $json.dp
    $params.DQ = ConvertFrom-Base64Url $json.dq
    $params.InverseQ = ConvertFrom-Base64Url $json.qi
  }
  $rsa = New-Object System.Security.Cryptography.RSACryptoServiceProvider
  $rsa.PersistKeyInCsp = $false
  $rsa.ImportParameters($params)
  $rsa
}

function ConvertTo-RsaJson {
  param(
    [System.Security.Cryptography.RSAParameters]$Params,
    [bool]$IncludePrivate
  )
  $obj = [ordered]@{
    kty = 'RSA'
    alg = 'RS256'
    n = ConvertTo-Base64Url $Params.Modulus
    e = ConvertTo-Base64Url $Params.Exponent
  }
  if ($IncludePrivate) {
    $obj.d = ConvertTo-Base64Url $Params.D
    $obj.p = ConvertTo-Base64Url $Params.P
    $obj.q = ConvertTo-Base64Url $Params.Q
    $obj.dp = ConvertTo-Base64Url $Params.DP
    $obj.dq = ConvertTo-Base64Url $Params.DQ
    $obj.qi = ConvertTo-Base64Url $Params.InverseQ
  }
  $obj | ConvertTo-Json -Depth 4
}

function New-LicenseKey {
  if (-not $Customer) { throw 'Customer is required for issue.' }
  if (-not $Expires) { throw 'Expires is required for issue. Use YYYY-MM-DD.' }

  $expiry = [DateTime]::ParseExact($Expires, 'yyyy-MM-dd', [Globalization.CultureInfo]::InvariantCulture)
  $payload = [ordered]@{
    product = $Product
    version = 1
    customer = $Customer
    issuedUtc = [DateTime]::UtcNow.ToString('yyyy-MM-ddTHH:mm:ssZ')
    expires = $expiry.ToString('yyyy-MM-dd')
    serverId = $ServerId
    nonce = [Guid]::NewGuid().ToString('N')
  }

  $payloadJson = $payload | ConvertTo-Json -Compress
  $payloadBytes = [Text.Encoding]::UTF8.GetBytes($payloadJson)
  $rsa = ConvertFrom-RsaJson $PrivateKeyPath
  try {
    $sha256 = [System.Security.Cryptography.SHA256]::Create()
    $signature = $rsa.SignData($payloadBytes, $sha256)
  }
  finally {
    if ($sha256) { $sha256.Dispose() }
    $rsa.Dispose()
  }

  $token = "$TokenPrefix.$(ConvertTo-Base64Url $payloadBytes).$(ConvertTo-Base64Url $signature)"
  if ($OutFile) {
    Set-Content -LiteralPath $OutFile -Value $token -NoNewline
  }
  $token
}

function Test-LicenseKey {
  if (-not $LicenseKey -and $OutFile) {
    $LicenseKey = Get-Content -LiteralPath $OutFile -Raw
  }
  if (-not $LicenseKey) { throw 'LicenseKey is required for verify.' }

  $parts = $LicenseKey.Trim() -split '\.'
  if ($parts.Count -ne 3 -or $parts[0] -ne $TokenPrefix) { throw 'Invalid license token format.' }

  $payloadBytes = ConvertFrom-Base64Url $parts[1]
  $signature = ConvertFrom-Base64Url $parts[2]
  $rsa = ConvertFrom-RsaJson $PublicKeyPath
  try {
    $sha256 = [System.Security.Cryptography.SHA256]::Create()
    $valid = $rsa.VerifyData($payloadBytes, $sha256, $signature)
  }
  finally {
    if ($sha256) { $sha256.Dispose() }
    $rsa.Dispose()
  }
  if (-not $valid) { throw 'License signature is invalid.' }

  $payloadJson = [Text.Encoding]::UTF8.GetString($payloadBytes)
  $payload = $payloadJson | ConvertFrom-Json
  if ($payload.product -ne $Product) { throw "License is for $($payload.product), not $Product." }

  $expiry = [DateTime]::ParseExact($payload.expires, 'yyyy-MM-dd', [Globalization.CultureInfo]::InvariantCulture)
  if ($expiry.Date -lt [DateTime]::UtcNow.Date) { throw "License expired on $($payload.expires)." }

  $payloadJson
}

switch ($Command) {
  'init' {
    if ((Test-Path -LiteralPath $PrivateKeyPath) -or (Test-Path -LiteralPath $PublicKeyPath)) {
      throw 'Refusing to overwrite existing key files.'
    }
    $rsa = New-Object System.Security.Cryptography.RSACryptoServiceProvider(3072)
    $rsa.PersistKeyInCsp = $false
    try {
      $privateParams = $rsa.ExportParameters($true)
      $publicParams = $rsa.ExportParameters($false)
      ConvertTo-RsaJson $privateParams $true | Set-Content -LiteralPath $PrivateKeyPath -Encoding ASCII
      ConvertTo-RsaJson $publicParams $false | Set-Content -LiteralPath $PublicKeyPath -Encoding ASCII
    }
    finally {
      $rsa.Dispose()
    }
    "Created private key: $PrivateKeyPath"
    "Created public key:  $PublicKeyPath"
    'Keep the private key secret. Embed only the public key in the plugin source.'
  }
  'issue' { New-LicenseKey }
  'verify' { Test-LicenseKey }
}

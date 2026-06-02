param(
  [string]$AsaApiRoot = $env:ASA_API_ROOT
)

$ErrorActionPreference = 'Stop'
$ProjectRoot = Split-Path -Parent $PSScriptRoot

function Write-Result {
  param(
    [string]$Name,
    [bool]$Ok,
    [string]$Detail = ''
  )

  $status = if ($Ok) { 'OK' } else { 'MISSING' }
  if ($Detail) {
    "{0,-8} {1} - {2}" -f $status, $Name, $Detail
  }
  else {
    "{0,-8} {1}" -f $status, $Name
  }
}

$configPath = Join-Path $ProjectRoot 'config.json'
$dlls = Get-ChildItem -Path $ProjectRoot -Recurse -Filter '*.dll' -ErrorAction SilentlyContinue

Write-Host 'Krit-Faslo source preflight'
Write-Host '==========================='
Write-Result 'Project root' (Test-Path $ProjectRoot) $ProjectRoot
Write-Result 'config.json' (Test-Path $configPath) $configPath

try {
  Get-Content -Raw -LiteralPath $configPath | ConvertFrom-Json | Out-Null
  Write-Result 'JSON parse' $true 'config.json is valid JSON'
}
catch {
  Write-Result 'JSON parse' $false $_.Exception.Message
}

Write-Result 'No DLLs' ($dlls.Count -eq 0) "$($dlls.Count) DLL file(s) under source root"

if ([string]::IsNullOrWhiteSpace($AsaApiRoot)) {
  Write-Result 'ASA_API_ROOT' $false 'set ASA_API_ROOT or pass -AsaApiRoot'
}
else {
  Write-Result 'ASA_API_ROOT' (Test-Path $AsaApiRoot) $AsaApiRoot

  $lib = Get-ChildItem -Path $AsaApiRoot -Recurse -Filter 'AsaApi.lib' -ErrorAction SilentlyContinue | Select-Object -First 1
  $dll = Get-ChildItem -Path $AsaApiRoot -Recurse -Filter 'AsaApi.dll' -ErrorAction SilentlyContinue | Select-Object -First 1
  $headers = Get-ChildItem -Path $AsaApiRoot -Recurse -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -in 'AsaApi.h', 'ArkApi.h', 'IHooks.h', 'Commands.h' }

  Write-Result 'AsaApi.lib' ($null -ne $lib) $(if ($lib) { $lib.FullName } else { 'not found below ASA_API_ROOT' })
  Write-Result 'AsaApi.dll' ($null -ne $dll) $(if ($dll) { $dll.FullName } else { 'not found below ASA_API_ROOT' })
  Write-Result 'Headers' ($headers.Count -gt 0) "$($headers.Count) key header file(s) found"

  if ($headers.Count -gt 0) {
    $headers | ForEach-Object { "         $($_.FullName)" }
  }
}

Write-Host ''
Write-Host 'This script does not compile or generate a DLL.'

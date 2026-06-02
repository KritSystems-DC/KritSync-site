param(
  [string]$AsaApiRoot = $env:ASA_API_ROOT,
  [string]$AsaApiLibRoot = $env:ASA_API_LIB_ROOT
)

$ErrorActionPreference = 'Stop'
$ProjectRoot = Split-Path -Parent $PSScriptRoot

function Write-Section {
  param([string]$Title)

  Write-Host ''
  Write-Host $Title
  Write-Host ('-' * $Title.Length)
}

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

function Write-Value {
  param(
    [string]$Name,
    [object]$Value
  )

  $display = if ($null -eq $Value) { '<missing>' } elseif ($Value -is [string] -and [string]::IsNullOrWhiteSpace($Value)) { '<empty>' } else { $Value }
  "{0,-32} {1}" -f $Name, $display
}

function Get-ConfigValue {
  param(
    [object]$Config,
    [string]$Name
  )

  if ($null -eq $Config) {
    return $null
  }

  $property = $Config.PSObject.Properties[$Name]
  if ($null -eq $property) {
    return $null
  }

  return $property.Value
}

function Find-FirstFile {
  param(
    [string]$Root,
    [string]$Name
  )

  if ([string]::IsNullOrWhiteSpace($Root) -or -not (Test-Path -LiteralPath $Root)) {
    return $null
  }

  return Get-ChildItem -LiteralPath $Root -Recurse -ErrorAction SilentlyContinue |
    Where-Object { -not $_.PSIsContainer -and $_.Name -eq $Name } |
    Select-Object -First 1
}

$configPath = Join-Path $ProjectRoot 'config.json'
$dlls = @(Get-ChildItem -LiteralPath $ProjectRoot -Recurse -Filter '*.dll' -ErrorAction SilentlyContinue)
$config = $null

Write-Host 'Krit-Faslo source preflight'
Write-Host '==========================='
Write-Result 'Project root' (Test-Path -LiteralPath $ProjectRoot) $ProjectRoot
Write-Result 'config.json' (Test-Path -LiteralPath $configPath) $configPath

Write-Section 'Config'
try {
  $config = Get-Content -Raw -LiteralPath $configPath | ConvertFrom-Json
  Write-Result 'JSON parse' $true 'config.json is valid JSON'
}
catch {
  Write-Result 'JSON parse' $false $_.Exception.Message
}

if ($null -ne $config) {
  $licenseKey = Get-ConfigValue $config 'LicenseKey'
  $licensePresent = -not [string]::IsNullOrWhiteSpace([string]$licenseKey)

  Write-Value 'Enabled' (Get-ConfigValue $config 'Enabled')
  Write-Value 'DebugLogging' (Get-ConfigValue $config 'DebugLogging')
  Write-Value 'LicenseKey' $(if ($licensePresent) { '<present>' } else { '<empty>' })
  Write-Result 'LicenseKey present' $licensePresent $(if ($licensePresent) { 'runtime behavior can pass license format validation' } else { 'plugin behavior will stay disabled until a license is set' })
  Write-Value 'TargetFlintMin' (Get-ConfigValue $config 'TargetFlintMin')
  Write-Value 'TargetFlintMax' (Get-ConfigValue $config 'TargetFlintMax')
  Write-Value 'MinTriggerAmount' (Get-ConfigValue $config 'MinTriggerAmount')
  Write-Value 'MaxTriggerAmount' (Get-ConfigValue $config 'MaxTriggerAmount')
  Write-Value 'VariancePercent' (Get-ConfigValue $config 'VariancePercent')

  Write-Section 'Anti-Dupe'
  Write-Value 'AntiDupeEnabled' (Get-ConfigValue $config 'AntiDupeEnabled')
  Write-Value 'LogAntiDupeEvents' (Get-ConfigValue $config 'LogAntiDupeEvents')
  Write-Value 'MaxExtraFlintPerStack' (Get-ConfigValue $config 'MaxExtraFlintPerStack')
  Write-Value 'MaxExtraFlintPerSecond' (Get-ConfigValue $config 'MaxExtraFlintPerSecond')
  Write-Value 'DuplicateDetectionWindowMs' (Get-ConfigValue $config 'DuplicateDetectionWindowMs')
  Write-Value 'BlockNegativeOrZeroAdjustments' (Get-ConfigValue $config 'BlockNegativeOrZeroAdjustments')
  Write-Value 'RequireFasolaOwnerMatch' (Get-ConfigValue $config 'RequireFasolaOwnerMatch')
}

Write-Section 'Source Package'
Write-Result 'No DLLs' ($dlls.Count -eq 0) "$($dlls.Count) DLL file(s) under source root"
if ($dlls.Count -gt 0) {
  $dlls | ForEach-Object { "         $($_.FullName)" }
}

Write-Section 'ASA SDK'
if ([string]::IsNullOrWhiteSpace($AsaApiLibRoot)) {
  $AsaApiLibRoot = $AsaApiRoot
}

$hasHeaderRoot = -not [string]::IsNullOrWhiteSpace($AsaApiRoot) -and (Test-Path -LiteralPath $AsaApiRoot)
$hasLibRoot = -not [string]::IsNullOrWhiteSpace($AsaApiLibRoot) -and (Test-Path -LiteralPath $AsaApiLibRoot)
$splitRoots = $hasHeaderRoot -and $hasLibRoot -and ((Resolve-Path -LiteralPath $AsaApiRoot).Path -ne (Resolve-Path -LiteralPath $AsaApiLibRoot).Path)

if ([string]::IsNullOrWhiteSpace($AsaApiRoot)) {
  Write-Result 'ASA_API_ROOT' $false 'set ASA_API_ROOT or pass -AsaApiRoot'
}
else {
  Write-Result 'ASA_API_ROOT' $hasHeaderRoot $AsaApiRoot
}

Write-Result 'ASA_API_LIB_ROOT' $hasLibRoot $(if ([string]::IsNullOrWhiteSpace($AsaApiLibRoot)) { 'set ASA_API_LIB_ROOT or pass -AsaApiLibRoot' } else { $AsaApiLibRoot })
Write-Value 'Separate SDK roots' $(if ($splitRoots) { 'true' } else { 'false' })

if ($hasHeaderRoot) {
  $headerNames = @('AsaApi.h', 'ArkApi.h', 'IHooks.h', 'Commands.h', 'Ark.h')
  $headers = @(Get-ChildItem -LiteralPath $AsaApiRoot -Recurse -ErrorAction SilentlyContinue |
    Where-Object { -not $_.PSIsContainer -and $headerNames -contains $_.Name })

  Write-Result 'Headers' ($headers.Count -gt 0) "$($headers.Count) key header file(s) found below ASA_API_ROOT"
  if ($headers.Count -gt 0) {
    $headers | ForEach-Object { "         $($_.FullName)" }
  }
}
else {
  Write-Result 'Headers' $false 'ASA_API_ROOT is not available'
}

if ($hasLibRoot) {
  $lib = Find-FirstFile $AsaApiLibRoot 'AsaApi.lib'
  $dll = Find-FirstFile $AsaApiLibRoot 'AsaApi.dll'

  Write-Result 'AsaApi.lib' ($null -ne $lib) $(if ($lib) { $lib.FullName } else { 'not found below ASA_API_LIB_ROOT' })
  Write-Result 'AsaApi.dll' ($null -ne $dll) $(if ($dll) { $dll.FullName } else { 'not found below ASA_API_LIB_ROOT' })
}
else {
  Write-Result 'AsaApi.lib' $false 'ASA_API_LIB_ROOT is not available'
  Write-Result 'AsaApi.dll' $false 'ASA_API_LIB_ROOT is not available'
}

Write-Host ''
Write-Host 'This script does not compile or generate a DLL.'

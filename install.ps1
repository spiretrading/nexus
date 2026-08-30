<#
.SYNOPSIS
  Installs the tools required to build Nexus on Windows.

.EXAMPLE
  .\install.ps1
#>
#Requires -Version 5.1
#Requires -RunAsAdministrator

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'

$UserAgent = 'nexus-install'
$GitVersion = '2.52.0'
$GitHash = 'd8de7a3152266c8bb13577eab850ea1df6dccf8c2aa48be5b4a1c58b7190d62c'
$CMakeVersion = '4.3.2'
$CMakeHash = '6915813bedf3a8a698b72fc858e0c2a99761be981745c3c3c99bd30e1477e142'
$NodeVersion = '24.20.0'
$NodeHash = '28b69132c35ccc033bf8f2a67cd10c9d75ef5822593363309da448f2afff2d8a'
$PythonVersion = '3.14.4'
$PythonHash = 'b571567bd11ea98fd7a2cf85791d2c8557a63b1e04e9d1dae665a275cac87f1b'
$NsisVersion = '3.12'
$NsisHash = '3bc2b06253a7e4957111be152ac6a536e0c7478a706e19da814038db5d706495'
$VisualStudioRange = '[18.0,19.0)'
$VisualStudioBootstrapper = 'https://aka.ms/vs/18/stable/vs_community.exe'
$VisualStudioComponents = @(
  'Microsoft.VisualStudio.Workload.NativeDesktop',
  'Microsoft.VisualStudio.Component.VC.Tools.x86.x64',
  'Microsoft.VisualStudio.Component.Windows11SDK.26100',
  'Microsoft.Component.MSBuild'
)

$RestartRequired = $false

function ConvertTo-Version {
  param([string] $Text)
  if (-not $Text) {
    return $null
  }
  $match = [regex]::Match($Text, '\d+(\.\d+){1,3}')
  if (-not $match.Success) {
    return $null
  }
  return [version] $match.Value
}

function Invoke-Native {
  param(
    [Parameter(Mandatory)] [string] $FilePath,
    [string[]] $ArgumentList = @()
  )
  $preference = $ErrorActionPreference
  $ErrorActionPreference = 'Continue'
  try {
    $output = & $FilePath @ArgumentList 2>$null
    $code = $LASTEXITCODE
  } finally {
    $ErrorActionPreference = $preference
  }
  return [PSCustomObject]@{
    Output = ($output -join [Environment]::NewLine)
    ExitCode = $code
  }
}

function Get-ApplicationPath {
  param([Parameter(Mandatory)] [string] $Name)
  $commands = @(Get-Command -Name $Name -CommandType Application `
    -ErrorAction SilentlyContinue)
  foreach ($command in $commands) {
    if ($command.Path -notmatch '\\WindowsApps\\') {
      return $command.Path
    }
  }
  return $null
}

function Get-ApplicationVersion {
  param(
    [Parameter(Mandatory)] [string] $Name,
    [string[]] $ArgumentList = @('--version')
  )
  $path = Get-ApplicationPath -Name $Name
  if (-not $path) {
    return $null
  }
  $result = Invoke-Native -FilePath $path -ArgumentList $ArgumentList
  if ($result.ExitCode -ne 0) {
    return $null
  }
  return ConvertTo-Version $result.Output
}

function Update-SessionPath {
  $paths = @(
    [Environment]::GetEnvironmentVariable('Path', 'Machine'),
    [Environment]::GetEnvironmentVariable('Path', 'User')
  )
  $env:Path = ($paths | Where-Object { $_ }) -join ';'
}

function Save-Download {
  param(
    [Parameter(Mandatory)] [string] $Uri,
    [Parameter(Mandatory)] [string] $Path,
    [string] $Sha256,
    [int] $Retries = 3
  )
  for ($attempt = 1; $attempt -le $Retries; $attempt += 1) {
    try {
      Invoke-WebRequest -Uri $Uri -OutFile $Path -UseBasicParsing `
        -UserAgent $UserAgent
      if (-not $Sha256) {
        return
      }
      $actual = (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash
      if ($actual -eq $Sha256) {
        return
      }
      throw "SHA256 mismatch for $(Split-Path -Leaf $Path). " +
        "Expected $Sha256, got $actual."
    } catch {
      Remove-Item -LiteralPath $Path -Force -ErrorAction SilentlyContinue
      if ($attempt -eq $Retries) {
        throw
      }
      Start-Sleep -Seconds (2 * $attempt)
    }
  }
}

function Invoke-Installer {
  param(
    [Parameter(Mandatory)] [string] $FilePath,
    [string[]] $ArgumentList = @(),
    [int[]] $SuccessCodes = @(0, 3010, 1641)
  )
  $parameters = @{
    FilePath = $FilePath
    PassThru = $true
    NoNewWindow = $true
    Wait = $true
  }
  if ($ArgumentList) {
    $parameters['ArgumentList'] = $ArgumentList
  }
  $process = Start-Process @parameters
  $process.WaitForExit()
  if ($SuccessCodes -notcontains $process.ExitCode) {
    throw "$(Split-Path -Leaf $FilePath) failed with exit code " +
      "$($process.ExitCode)."
  }
  if (@(3010, 1641) -contains $process.ExitCode) {
    $script:RestartRequired = $true
  }
  Update-SessionPath
}

function Test-Requirement {
  param(
    [Parameter(Mandatory)] [string] $Required,
    [version] $Installed,
    [switch] $Exact
  )
  if (-not $Installed) {
    return $false
  }
  $target = ConvertTo-Version $Required
  if ($Installed -lt $target) {
    return $false
  }
  return -not ($Exact -and $Installed -ne $target)
}

function Install-Git {
  param([Parameter(Mandatory)] [string] $Directory)
  if (Test-Requirement -Required $GitVersion `
      -Installed (Get-ApplicationVersion -Name 'git')) {
    return
  }
  $package = "Git-$GitVersion-64-bit.exe"
  $installer = Join-Path $Directory $package
  Save-Download -Sha256 $GitHash -Path $installer -Uri ('https://github.com/' +
    "git-for-windows/git/releases/download/v$GitVersion.windows.1/$package")
  $answers = Join-Path $Directory 'git.inf'
  @(
    '[Setup]',
    'Lang=default',
    "Dir=$(Join-Path $env:ProgramFiles 'Git')",
    'Group=Git',
    'NoIcons=1',
    'SetupType=default',
    'Components=gitlfs,assoc,assoc_sh',
    'Tasks=',
    'PathOption=Cmd',
    'SSHOption=OpenSSH',
    'CURLOption=OpenSSL',
    'CRLFOption=CRLFAlways',
    'BashTerminalOption=ConHost',
    'GitPullBehaviorOption=Merge',
    'UseCredentialManager=Enabled',
    'PerformanceTweaksFSCache=Enabled',
    'EnableSymlinks=Disabled',
    'EnableFSMonitor=Disabled'
  ) | Set-Content -LiteralPath $answers -Encoding ASCII
  Invoke-Installer -FilePath $installer -ArgumentList @(
    '/VERYSILENT', '/NORESTART', '/NOCANCEL', '/SP-', '/SUPPRESSMSGBOXES',
    '/CLOSEAPPLICATIONS', "/LOADINF=`"$answers`""
  )
}

function Install-CMake {
  param([Parameter(Mandatory)] [string] $Directory)
  if (Test-Requirement -Required $CMakeVersion `
      -Installed (Get-ApplicationVersion -Name 'cmake')) {
    return
  }
  $package = "cmake-$CMakeVersion-windows-x86_64.msi"
  $installer = Join-Path $Directory $package
  Save-Download -Sha256 $CMakeHash -Path $installer -Uri (
    'https://github.com/Kitware/CMake/releases/download/' +
    "v$CMakeVersion/$package")
  Invoke-Installer -FilePath 'msiexec.exe' -ArgumentList @(
    '/i', "`"$installer`"", '/qn', '/norestart', 'ADD_CMAKE_TO_PATH=System'
  )
}

function Install-Node {
  param([Parameter(Mandatory)] [string] $Directory)
  if (Test-Requirement -Required $NodeVersion -Exact `
      -Installed (Get-ApplicationVersion -Name 'node')) {
    return
  }
  $package = "node-v$NodeVersion-x64.msi"
  $installer = Join-Path $Directory $package
  Save-Download -Sha256 $NodeHash -Path $installer `
    -Uri "https://nodejs.org/dist/v$NodeVersion/$package"
  Invoke-Installer -FilePath 'msiexec.exe' -ArgumentList @(
    '/i', "`"$installer`"", '/qn', '/norestart'
  )
}

function Get-PythonVersion {
  $required = ConvertTo-Version $PythonVersion
  $installed = Get-ApplicationVersion -Name 'py' `
    -ArgumentList @("-$($required.Major).$($required.Minor)", '--version')
  if (-not $installed) {
    $installed = Get-ApplicationVersion -Name 'python'
  }
  if ($installed -and ($installed.Major -ne $required.Major -or
      $installed.Minor -ne $required.Minor)) {
    return $null
  }
  return $installed
}

function Install-Python {
  param([Parameter(Mandatory)] [string] $Directory)
  if (Test-Requirement -Required $PythonVersion `
      -Installed (Get-PythonVersion)) {
    return
  }
  $package = "python-$PythonVersion-amd64.exe"
  $installer = Join-Path $Directory $package
  Save-Download -Sha256 $PythonHash -Path $installer `
    -Uri "https://www.python.org/ftp/python/$PythonVersion/$package"
  Invoke-Installer -FilePath $installer -ArgumentList @(
    '/quiet', '/norestart', 'InstallAllUsers=1', 'InstallLauncherAllUsers=1',
    'PrependPath=1', 'Include_launcher=1', 'Include_pip=1',
    'Include_test=0', 'Include_doc=0', 'AssociateFiles=1', 'CompileAll=1'
  )
}

function Install-Nsis {
  param([Parameter(Mandatory)] [string] $Directory)
  $makensis = Join-Path ${env:ProgramFiles(x86)} 'NSIS\makensis.exe'
  $installed = $null
  if (Test-Path -LiteralPath $makensis) {
    $installed = ConvertTo-Version (Invoke-Native -FilePath $makensis `
      -ArgumentList @('/VERSION')).Output
  }
  if (Test-Requirement -Required $NsisVersion -Installed $installed) {
    return
  }
  $package = "nsis-$NsisVersion-setup.exe"
  $installer = Join-Path $Directory $package
  Save-Download -Sha256 $NsisHash -Path $installer -Uri (
    'https://downloads.sourceforge.net/project/nsis/' +
    "NSIS%203/$NsisVersion/$package")
  Invoke-Installer -FilePath $installer -ArgumentList @('/S')
}

function Get-VisualStudioProperty {
  param(
    [Parameter(Mandatory)] [string] $Property,
    [switch] $Complete
  )
  $vswhere = Join-Path ${env:ProgramFiles(x86)} `
    'Microsoft Visual Studio\Installer\vswhere.exe'
  if (-not (Test-Path -LiteralPath $vswhere)) {
    return $null
  }
  $arguments = @(
    '-products', '*',
    '-version', $VisualStudioRange,
    '-latest', '-property', $Property
  )
  if ($Complete) {
    $arguments += @('-requires') + $VisualStudioComponents
  }
  $result = Invoke-Native -FilePath $vswhere -ArgumentList $arguments
  if ($result.ExitCode -ne 0 -or -not $result.Output) {
    return $null
  }
  return ($result.Output -split "`n" | Select-Object -First 1).Trim()
}

function Install-VisualStudio {
  param([Parameter(Mandatory)] [string] $Directory)
  if (Get-VisualStudioProperty -Complete -Property 'installationPath') {
    return
  }
  $existing = Get-VisualStudioProperty -Property 'installationPath'
  $installer = Join-Path $Directory 'vs_community.exe'
  Save-Download -Uri $VisualStudioBootstrapper -Path $installer
  $arguments = @('--quiet', '--norestart', '--wait', '--nocache')
  foreach ($component in $VisualStudioComponents) {
    $arguments += @('--add', $component)
  }
  if ($existing) {
    $arguments = @('modify', '--installPath', "`"$existing`"") + $arguments
  }
  Invoke-Installer -FilePath $installer -ArgumentList $arguments
}

function Main {
  $architecture = $env:PROCESSOR_ARCHITEW6432
  if (-not $architecture) {
    $architecture = $env:PROCESSOR_ARCHITECTURE
  }
  if ($architecture -ne 'AMD64') {
    throw 'Nexus builds 64-bit binaries and requires an x64 host, but this ' +
      "machine reports '$architecture'."
  }
  if ([Net.ServicePointManager]::SecurityProtocol -ne 0) {
    [Net.ServicePointManager]::SecurityProtocol =
      [Net.ServicePointManager]::SecurityProtocol -bor
      [Net.SecurityProtocolType]::Tls12
  }
  $directory = Join-Path ([IO.Path]::GetTempPath()) "nexus-install-$PID"
  New-Item -ItemType Directory -Path $directory -Force | Out-Null
  try {
    Install-Git -Directory $directory
    Install-CMake -Directory $directory
    Install-Node -Directory $directory
    Install-Python -Directory $directory
    Install-Nsis -Directory $directory
    Install-VisualStudio -Directory $directory
  } finally {
    Remove-Item -LiteralPath $directory -Recurse -Force `
      -ErrorAction SilentlyContinue
  }
  if ($RestartRequired) {
    Write-Warning ('An installer reported that a restart is pending. ' +
      'Nothing was restarted, so restart the machine when convenient.')
  }
}

Main

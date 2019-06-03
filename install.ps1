$program_files_x86_path = (Get-Item -LiteralPath "C:\Progra~1").FullName
$program_files_path = (Get-Item -LiteralPath "C:\Progra~2").FullName
If("$program_files_x86_path" -eq "C:\Program Files") {
  $program_files_x86_path = "C:\Progra~2"
  $program_files_path = "C:\Progra~1"
} Else {
  $program_files_x86_path = "C:\Progra~1"
  $program_files_path = "C:\Progra~2"
}
[Net.ServicePointManager]::SecurityProtocol = "tls12, tls11, tls"
If((Get-Command "grep.exe" -ErrorAction SilentlyContinue) -eq $null) {
  Invoke-WebRequest -UseBasicParsing -Uri https://github.com/bmatzelle/gow/releases/download/v0.8.0/Gow-0.8.0.exe -UserAgent [Microsoft.PowerShell.Commands.PSUserAgent]::FireFox -OutFile Gow-0.8.0.exe
  .\Gow-0.8.0.exe /S | Out-Null
  $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
  rm Gow-0.8.0.exe -Force
}
If((Get-Command "unzip.exe" -ErrorAction SilentlyContinue) -eq $null) {
  Invoke-WebRequest -UseBasicParsing -Uri http://www2.cs.uidaho.edu/~jeffery/win32/unzip.exe -UserAgent [Microsoft.PowerShell.Commands.PSUserAgent]::FireFox -OutFile unzip.exe
  mv ./unzip.exe "$program_files_x86_path\Gow\bin"
}
If((Get-Command "git.exe" -ErrorAction SilentlyContinue) -eq $null) {
  New-Item git_config.inf -type file -force -value "[Setup]`r`nPathOption=Cmd"
  Invoke-WebRequest -UseBasicParsing -Uri https://github.com/git-for-windows/git/releases/download/v2.12.2.windows.2/Git-2.12.2.2-64-bit.exe -OutFile Git-2.12.2.2-64-bit.exe
  .\Git-2.12.2.2-64-bit.exe /SILENT /LOADINF="git_config.inf" | Out-Null
  rm .\Git-2.12.2.2-64-bit.exe -Force
  rm .\git_config.inf -Force
}
If((Get-Command "cmake.exe" -ErrorAction SilentlyContinue) -eq $null) {
  Invoke-WebRequest -UseBasicParsing -Uri https://github.com/Kitware/CMake/releases/download/v3.13.2/cmake-3.13.2-win64-x64.msi -OutFile cmake-3.13.2-win64-x64.msi
  msiexec /quiet /i cmake-3.13.2-win64-x64.msi | Out-Null
  $old_path = (Get-ItemProperty -Path "Registry::HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Session Manager\Environment" -Name PATH).path
  $new_path = "$old_path;C:\Program Files\CMake\bin"
  Set-ItemProperty -Path "Registry::HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Session Manager\Environment" -Name PATH –Value $new_path
  $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
  rm .\cmake-3.13.2-win64-x64.msi -Force
}
If(-Not (Test-Path "C:\Program Files (x86)\Microsoft Visual Studio")) {
  Invoke-WebRequest -UseBasicParsing -Uri "https://aka.ms/vs/15/release/vs_community.exe" -OutFile vs_Community.exe
  ./vs_Community.exe --passive --norestart --wait `
    --add Microsoft.VisualStudio.Workload.NativeDesktop `
    --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    --add Microsoft.VisualStudio.Component.Windows10SDK.15063.Desktop `
    --add Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Win81 `
    --add Microsoft.Component.MSBuild `
    --add Microsoft.VisualStudio.Component.VC.CoreIde `
    --add Microsoft.VisualStudio.Component.VC.Redist.14.Latest `
    --add Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Core | Out-Null
  rm vs_Community.exe
  $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
}
pushd "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build"
cmd /c "vcvars32.bat&set" |
foreach {
  if ($_ -match "=") {
    $v = $_.split("="); set-item -force -path "ENV:\$($v[0])"  -value "$($v[1])"
  }
}
popd

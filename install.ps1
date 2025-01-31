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
If((Get-Command "git.exe" -ErrorAction SilentlyContinue) -eq $null) {
  New-Item git_config.inf -type file -force -value "[Setup]`r`nPathOption=Cmd"
  Invoke-WebRequest -UseBasicParsing -Uri https://github.com/git-for-windows/git/releases/download/v2.47.1.windows.2/Git-2.47.1.2-64-bit.exe -OutFile Git-2.47.1.2-64-bit.exe
  .\Git-2.47.1.2-64-bit.exe /SILENT /LOADINF="git_config.inf" | Out-Null
  rm .\Git-2.47.1.2-64-bit.exe -Force
  rm .\git_config.inf -Force
}
If((Get-Command "cmake.exe" -ErrorAction SilentlyContinue) -eq $null) {
  Invoke-WebRequest -UseBasicParsing -Uri https://github.com/Kitware/CMake/releases/download/v3.31.5/cmake-3.31.5-windows-x86_64.msi -OutFile cmake-3.31.5-windows-x86_64.msi
  msiexec /quiet /i cmake-3.31.5-windows-x86_64.msi ADD_CMAKE_TO_PATH=User | Out-Null
  rm .\cmake-3.31.5-windows-x86_64.msi -Force
}
If((Get-Command "node.exe" -ErrorAction SilentlyContinue) -eq $null) {
  Invoke-WebRequest -UseBasicParsing -Uri https://nodejs.org/dist/v22.13.1/node-v22.13.1-x64.msi -OutFile node-v22.13.1-x64.msi
  msiexec /quiet /i node-v22.13.1-x64.msi | Out-Null
  rm .\node-v22.13.1-x64.msi
}
If(-Not (Test-Path "C:\Program Files\Microsoft Visual Studio")) {
  Invoke-WebRequest -UseBasicParsing -Uri "https://aka.ms/vs/17/release/vs_community.exe" -OutFile vs_Community.exe
  ./vs_Community.exe --passive --norestart --wait `
    --add Microsoft.Component.MSBuild `
    --add Microsoft.VisualStudio.Component.Debugger.JustInTime `
    --add Microsoft.VisualStudio.Component.Windows10SDK.10586 `
    --add Microsoft.VisualStudio.Component.Windows10SDK.20348 `
    --add Microsoft.VisualStudio.Component.Windows10SDK `
    --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    --add Microsoft.VisualStudio.Component.VC.CoreIde `
    --add Microsoft.VisualStudio.Component.VC.Redist.14.Latest `
    --add Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Core | Out-Null
  rm vs_Community.exe
}

#define MyAppName "NVMe Recovery Studio AI"
#define MyAppVersion "0.1.0"
#define MyAppExeName "NVMeRecoveryStudioAI.exe"
[Setup]
AppId={{9C7F76BC-65D0-4C8B-B88B-EA720D09C8AA}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
DefaultDirName={autopf}\NVMe Recovery Studio AI
DefaultGroupName={#MyAppName}
OutputDir=installer
OutputBaseFilename=NVMeRecoveryStudioAI_Setup_0.1.0_x64
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=lowest
[Files]
Source: "dist\NVMeRecoveryStudioAI.exe"; DestDir: "{app}"; Flags: ignoreversion
[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
[Tasks]
Name: "desktopicon"; Description: "Desktop-Verknüpfung erstellen"; GroupDescription: "Zusätzliche Aufgaben:"
[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "NVMe Recovery Studio AI starten"; Flags: nowait postinstall skipifsilent

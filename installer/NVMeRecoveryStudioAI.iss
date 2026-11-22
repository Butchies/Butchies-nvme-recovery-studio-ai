#define MyAppName "NVMe Recovery Studio AI"
#ifndef MyAppVersion
  #define MyAppVersion "0.5.0"
#endif
#define MyAppExeName "nvme-recovery-studio.exe"
[Setup]
AppId={{9C7F76BC-65D0-4C8B-B88B-EA720D09C8AA}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher=Butchies
DefaultDirName={autopf}\NVMe Recovery Studio AI
DefaultGroupName={#MyAppName}
OutputDir=..\out
OutputBaseFilename=NVMeRecoveryStudioAI_Setup_{#MyAppVersion}_x64
Compression=lzma2/ultra64
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
WizardStyle=modern
SetupIconFile=..\native\app\app.ico
WizardImageFile=wizard.bmp
WizardSmallImageFile=wizard-small.bmp
UninstallDisplayIcon={app}\{#MyAppExeName}
[Languages]
Name: "de"; MessagesFile: "compiler:Languages\German.isl"
[Tasks]
Name: "desktopicon"; Description: "Desktop-Verknüpfung erstellen"; GroupDescription: "Zusätzliche Aufgaben:"
[Files]
Source: "..\dist\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{#MyAppName} deinstallieren"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "NVMe Recovery Studio AI starten"; Flags: nowait postinstall skipifsilent shellexec

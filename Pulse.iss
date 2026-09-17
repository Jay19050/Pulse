#define MyAppName "Pulse"
#define MyAppVersion "0.1.0"
#define MyAppPublisher "Pulse"
#define MyAppExeName "Pulse.exe"

[Setup]
SetupIconFile=D:\Pulse\Source\Pulse.ico
AppId={{8F6A7D21-7C8B-4E0A-9B43-PULSE00000001}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\Pulse
DefaultGroupName=Pulse
DisableProgramGroupPage=yes
OutputDir=D:\Pulse\installer
OutputBaseFilename=PulseSetup
UninstallDisplayIcon={app}\Pulse.exe
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

[Files]
Source: "D:\Pulse\build\Pulse_artefacts\Release\Pulse.exe"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\Pulse"; Filename: "{app}\Pulse.exe"
Name: "{autodesktop}\Pulse"; Filename: "{app}\Pulse.exe"

[Run]
Filename: "{app}\Pulse.exe"; Description: "Launch Pulse"; Flags: nowait postinstall skipifsilent

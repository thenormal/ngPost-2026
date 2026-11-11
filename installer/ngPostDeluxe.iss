; Inno Setup script of the ngPost Deluxe installer (built by the GitHub workflow, see .github/workflows/build-windows.yml)
;   ISCC /DAppVersion=1.2 /DArch=x64 /DSourceDir=..\dist /DOutName=ngPostDeluxe-v1.2-win64-setup installer\ngPostDeluxe.iss
#ifndef AppVersion
  #define AppVersion "1.2"
#endif
#ifndef Arch
  #define Arch "x64"
#endif
#ifndef SourceDir
  #define SourceDir "..\dist"
#endif
#ifndef OutName
  #define OutName "ngPostDeluxe-setup"
#endif
#define AppName "ngPost Deluxe"
#define AppExe  "ngPostDeluxe.exe"
#define AppUrl  "https://github.com/thenormal/ngPost-Deluxe"

[Setup]
; the AppId identifies the program: it must never change (updates are installed over the previous version)
AppId={{6F0B4E52-3B1C-4D7E-9A58-2C7D61E0A4F3}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher=thenormal
AppPublisherURL={#AppUrl}
AppSupportURL={#AppUrl}/issues
AppUpdatesURL={#AppUrl}/releases
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
UninstallDisplayIcon={app}\{#AppExe}
SetupIconFile=..\src\ngPost.ico
LicenseFile=..\LICENSE
OutputDir=..\installer_out
OutputBaseFilename={#OutName}
Compression=lzma2/ultra
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
CloseApplications=yes
VersionInfoVersion={#AppVersion}.0.0
VersionInfoProductName={#AppName}
VersionInfoDescription={#AppName} setup
#if Arch == "x64"
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
#endif

[Languages]
Name: "english";    MessagesFile: "compiler:Default.isl"
Name: "italian";    MessagesFile: "compiler:Languages\Italian.isl"
Name: "french";     MessagesFile: "compiler:Languages\French.isl"
Name: "german";     MessagesFile: "compiler:Languages\German.isl"
Name: "spanish";    MessagesFile: "compiler:Languages\Spanish.isl"
Name: "dutch";      MessagesFile: "compiler:Languages\Dutch.isl"
Name: "portuguese"; MessagesFile: "compiler:Languages\Portuguese.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; the sample configuration file (ngPost.conf) is only for the portable ZIP: the installed program
; keeps its settings in %APPDATA%\ngPost Deluxe
Source: "{#SourceDir}\*"; DestDir: "{app}"; Excludes: "ngPost.conf,*diagnostics*.log"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
; the program starts in the Documents folder (the installation folder is read only)
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\{#AppExe}"; WorkingDir: "{userdocs}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExe}"; WorkingDir: "{userdocs}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppExe}"; Description: "{cm:LaunchProgram,{#AppName}}"; WorkingDir: "{userdocs}"; Flags: nowait postinstall skipifsilent

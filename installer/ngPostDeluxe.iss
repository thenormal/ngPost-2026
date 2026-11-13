; Inno Setup script of the ngPost Deluxe installer (built by the GitHub workflow, see .github/workflows/build-windows.yml)
;   ISCC /DAppVersion=1.3 /DArch=x64 /DSourceDir=..\dist /DOutName=ngPostDeluxe-v1.3-win64-setup installer\ngPostDeluxe.iss
#ifndef AppVersion
  #define AppVersion "1.3"
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

[CustomMessages]
english.DeleteUserData=Do you also want to delete your settings and the history of your posts?%n%n%1%n%nChoose No to keep them (for example to install the program again later).
italian.DeleteUserData=Vuoi cancellare anche le impostazioni e la cronologia dei post?%n%n%1%n%nScegli No per conservarle (ad esempio per reinstallare il programma in seguito).
french.DeleteUserData=Voulez-vous aussi supprimer vos paramètres et l'historique de vos envois ?%n%n%1%n%nChoisissez Non pour les conserver (par exemple pour réinstaller le programme plus tard).
german.DeleteUserData=Möchten Sie auch Ihre Einstellungen und den Verlauf Ihrer Posts löschen?%n%n%1%n%nWählen Sie Nein, um sie zu behalten (zum Beispiel für eine spätere Neuinstallation).
spanish.DeleteUserData=¿Quieres borrar también tus ajustes y el historial de tus publicaciones?%n%n%1%n%nElige No para conservarlos (por ejemplo, para volver a instalar el programa más tarde).
dutch.DeleteUserData=Wilt u ook uw instellingen en de geschiedenis van uw posts verwijderen?%n%n%1%n%nKies Nee om ze te bewaren (bijvoorbeeld om het programma later opnieuw te installeren).
portuguese.DeleteUserData=Quer apagar também as suas definições e o histórico das publicações?%n%n%1%n%nEscolha Não para as manter (por exemplo, para reinstalar o programa mais tarde).

[Code]
// The installed program keeps its settings and its history in %APPDATA%\ngPost Deluxe.
// They are kept by default (a silent uninstall never deletes them): the user is asked once, at the end.
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  DataDir: String;
begin
  if CurUninstallStep = usPostUninstall then
  begin
    DataDir := ExpandConstant('{userappdata}\ngPost Deluxe');
    if (not UninstallSilent) and DirExists(DataDir) then
      if MsgBox(FmtMessage(CustomMessage('DeleteUserData'), [DataDir]), mbConfirmation, MB_YESNO or MB_DEFBUTTON2) = IDYES then
        DelTree(DataDir, True, True, True);
  end;
end;

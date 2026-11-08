[Setup]
AppId={{B6E1F0D2-5B7A-4B8E-9C61-3A6E5F1D7A21}
AppName=Ganj Library
AppVersion=1.0.0
AppPublisher=Tip 116 Mechanized Infantry
DefaultDirName={autopf}\GanjLibrary
DefaultGroupName=Ganj Library
OutputDir=..\dist
OutputBaseFilename=GanjLibrarySetup
SetupIconFile=..\resources\ganj.ico
UninstallDisplayIcon={app}\Ganj.exe
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional icons:"

[Files]
Source: "..\deploy\*"; DestDir: "{app}"; Excludes: "vc_redist*.exe"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\deploy\vc_redist.x64.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall skipifsourcedoesntexist

[Icons]
Name: "{group}\Ganj Library"; Filename: "{app}\Ganj.exe"
Name: "{autodesktop}\Ganj Library"; Filename: "{app}\Ganj.exe"; Tasks: desktopicon

[Run]
Filename: "{tmp}\vc_redist.x64.exe"; Parameters: "/install /quiet /norestart"; StatusMsg: "Installing Microsoft Visual C++ Runtime..."; Check: VCRedistNeeded; Flags: waituntilterminated skipifdoesntexist
Filename: "{app}\Ganj.exe"; Description: "Launch Ganj Library"; Flags: nowait postinstall skipifsilent

[Code]
function VCRedistNeeded: Boolean;
var
  Installed: Cardinal;
begin
  Result := not (RegQueryDWordValue(HKLM64, 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64', 'Installed', Installed) and (Installed = 1));
end;

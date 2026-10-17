; ==============================================================================
; IT26 Workstation - Inno Setup Script
; Modern 64-bit Tracker Workstation for Windows
; Developed by Marton Voros aka C41 (based on Impulse Tracker by Jeffrey Lim)
; ==============================================================================

#define MyAppName "IT26 Workstation"
#define MyAppVersion "1.0.4"
#define MyAppPublisher "Marton Voros aka C41"
#define MyAppURL "https://github.com/martonvoros/it26"
#define MyAppExeName "IT26.exe"

[Setup]
AppId={{D68F2314-87A1-4192-9F2B-A6BE639EF910}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} v{#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}/issues
AppUpdatesURL={#MyAppURL}/releases
DefaultDirName={autopf}\IT26
DefaultGroupName=IT26 Workstation
AllowNoIcons=yes
LicenseFile=LICENSE
OutputDir=dist
OutputBaseFilename=IT26-Windows-x64-Setup-v1.0.4
SetupIconFile=res\impulse.ico
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayIcon={app}\{#MyAppExeName}
ChangesAssociations=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "trustcert"; Description: "Install developer root certificate to prevent Windows SmartScreen prompts"; GroupDescription: "Security & Trust:"
Name: "associatefiles"; Description: "Associate tracker module files (.it, .xm, .s3m, .mod) with IT26"; GroupDescription: "File Associations:"

[Files]
Source: "IT26.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "SDL3.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "head.jpg"; DestDir: "{app}"; Flags: ignoreversion
Source: "run.bat"; DestDir: "{app}"; Flags: ignoreversion
Source: "LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "MartonVoros_C41.cer"; DestDir: "{app}"; Flags: ignoreversion
Source: "assets\*"; DestDir: "{app}\assets"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Registry]
; .it files
Root: HKA; Subkey: "Software\Classes\.it"; ValueType: string; ValueName: ""; ValueData: "IT26.Module"; Flags: uninsdeletevalue; Tasks: associatefiles
Root: HKA; Subkey: "Software\Classes\IT26.Module"; ValueType: string; ValueName: ""; ValueData: "Impulse Tracker Module"; Flags: uninsdeletekey; Tasks: associatefiles
Root: HKA; Subkey: "Software\Classes\IT26.Module\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"; Tasks: associatefiles
Root: HKA; Subkey: "Software\Classes\IT26.Module\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""; Tasks: associatefiles

; .xm files
Root: HKA; Subkey: "Software\Classes\.xm"; ValueType: string; ValueName: ""; ValueData: "IT26.ModuleXM"; Flags: uninsdeletevalue; Tasks: associatefiles
Root: HKA; Subkey: "Software\Classes\IT26.ModuleXM"; ValueType: string; ValueName: ""; ValueData: "FastTracker II Module"; Flags: uninsdeletekey; Tasks: associatefiles
Root: HKA; Subkey: "Software\Classes\IT26.ModuleXM\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"; Tasks: associatefiles
Root: HKA; Subkey: "Software\Classes\IT26.ModuleXM\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""; Tasks: associatefiles

; .s3m files
Root: HKA; Subkey: "Software\Classes\.s3m"; ValueType: string; ValueName: ""; ValueData: "IT26.ModuleS3M"; Flags: uninsdeletevalue; Tasks: associatefiles
Root: HKA; Subkey: "Software\Classes\IT26.ModuleS3M"; ValueType: string; ValueName: ""; ValueData: "Scream Tracker 3 Module"; Flags: uninsdeletekey; Tasks: associatefiles
Root: HKA; Subkey: "Software\Classes\IT26.ModuleS3M\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"; Tasks: associatefiles
Root: HKA; Subkey: "Software\Classes\IT26.ModuleS3M\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""; Tasks: associatefiles

; .mod files
Root: HKA; Subkey: "Software\Classes\.mod"; ValueType: string; ValueName: ""; ValueData: "IT26.ModuleMOD"; Flags: uninsdeletevalue; Tasks: associatefiles
Root: HKA; Subkey: "Software\Classes\IT26.ModuleMOD"; ValueType: string; ValueName: ""; ValueData: "ProTracker Module"; Flags: uninsdeletekey; Tasks: associatefiles
Root: HKA; Subkey: "Software\Classes\IT26.ModuleMOD\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"; Tasks: associatefiles
Root: HKA; Subkey: "Software\Classes\IT26.ModuleMOD\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""; Tasks: associatefiles

[Run]
; Auto-trust the certificate into the Trusted Publishers store so SmartScreen does not flag IT26
Filename: "certutil.exe"; Parameters: "-addstore -f ""TrustedPublisher"" ""{app}\MartonVoros_C41.cer"""; Flags: runhidden; Tasks: trustcert
; Also attempt Root store
Filename: "certutil.exe"; Parameters: "-addstore -f ""ROOT"" ""{app}\MartonVoros_C41.cer"""; Flags: runhidden; Tasks: trustcert
; Launch option
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

; CleanShareInstaller.iss

#define MyAppName       "CleanShare"
#define MyAppVersion    "1.0.0"
#define MyAppPublisher  "CleanShare Team"
#define MyAppURL        "https://cisc320.caslab.queensu.ca/davidbalann/cleanshare"
#define MyAppExeName    "CleanShare.exe"
#define MyBuildDir      "build\bin\Release"

[Setup]
; --- Basic app info ---
AppId={{F5A0C2F7-7F74-4E3B-9B2E-4B4C54CBE1A1}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}

; --- Install location ---
DefaultDirName={pf64}\{#MyAppName}
DefaultGroupName={#MyAppName}

; --- Output installer exe ---
OutputBaseFilename={#MyAppName}-Setup
OutputDir=output

; --- Look & feel ---
WizardStyle=modern
Compression=lzma2
SolidCompression=yes
SetupLogging=yes

; --- Platform ---
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

; Require admin (Program Files); change to "lowest" + different DefaultDirName if you want per-user install
PrivilegesRequired=admin

; Uncomment this if you later add a license.txt beside this .iss
; LicenseFile=license.txt

; Optional: small tweaks
DisableWelcomePage=no
DisableReadyPage=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop shortcut"; GroupDescription: "Additional icons:"; Flags: unchecked

[Files]
; Grab EVERYTHING you need from the Release output folder.
; This should include:
;   - CleanShare.exe
;   - Qt DLLs (windeployqt)
;   - opencv_worldXXXX.dll
;   - onnxruntime.dll
;   - models\alcohol-detector-v1.onnx
Source: "{#MyBuildDir}\*"; DestDir: "{app}"; Flags: recursesubdirs ignoreversion

[Icons]
; Start menu shortcut
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"

; Desktop shortcut (optional task)
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
; Offer to launch app after install
Filename: "{app}\{#MyAppExeName}"; Description: "Launch {#MyAppName} now"; Flags: nowait postinstall skipifsilent

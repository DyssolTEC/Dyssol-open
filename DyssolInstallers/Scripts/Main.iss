; Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. 

#define MyAppName "Dyssol"
#define MyAppExeName "Dyssol.exe"
#define MyAppPublisher "DyssolTEC"
#define MyAppURL "https://github.com/DyssolTEC/Dyssol-open"
#define MyAppPublisherURL "https://www.dyssoltec.com/"
#define MyAppContact "info@dyssoltec.com"
#ifdef IsIncludeX64
#define MyAppVersion GetStringFileInfo(SolutionDir+'\x64\Release\Dyssol.exe', 'ProductVersion')
#else
#define MyAppVersion GetStringFileInfo(SolutionDir+'\Win32\Release\Dyssol.exe', 'ProductVersion')
#endif

; all come as parameters from the running script
; #define MyAppBranch
; #define SolutionDir
; #define QtPath
; #define IsDocs
; #define IsIncludeX32
; #define IsIncludeX64
; #define IsWithSrc
; #define IsWithSDK

#include "ExtLibs.iss"
#include "QtLibs.iss"
#include "FlowsheetsExamples.iss"
#include "SolversExamples.iss"
#include "UnitsExamples.iss"
#include "HelpFiles.iss"
#include "Solvers.iss"
#include "Units.iss"
#ifdef IsWithSDK
#include "ModelsCreator.iss"
#endif

[Setup]
AppId={{F12AB44E-589E-413D-A6CA-6A2EE5620776}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppPublisherURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={commonpf}\{#MyAppName}
DefaultGroupName={#MyAppName}
UninstallDisplayIcon={app}\{#MyAppExeName}
AppContact={#MyAppContact}
#ifdef MyAppBranch
OutputBaseFilename={#MyAppName} {#MyAppVersion} {#MyAppBranch} setup
#else
OutputBaseFilename={#MyAppName} {#MyAppVersion} setup
#endif
OutputDir={#SolutionDir}\DyssolInstallers\Installers
LicenseFile={#SolutionDir}\LICENSE
SetupIconFile={#SolutionDir}\DyssolMainWindow\Resources\Icon.ico
WizardImageFile={#SolutionDir}\DyssolInstallers\Data\WizardImageFile.bmp
WizardSmallImageFile={#SolutionDir}\DyssolInstallers\Data\WizardSmallImageFile.bmp
DisableStartupPrompt=False
DisableWelcomePage=False
SolidCompression=yes
Compression=lzma
InternalCompressLevel=max
AllowNoIcons=yes
ChangesAssociations=True
ShowLanguageDialog=auto
PrivilegesRequired=poweruser
UsedUserAreasWarning=no
#ifdef IsIncludeX64
  #ifdef IsIncludeX32
ArchitecturesAllowed=x86 x64
ArchitecturesInstallIn64BitMode=x64
  #else
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
  #endif
#else
ArchitecturesAllowed=x86 x64
#endif

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
#ifdef IsIncludeX32
Source: "..\..\Win32\Release\{#MyAppExeName}"      ; DestDir: "{app}"                                            ; Flags: ignoreversion; Check: not Is64BitInstallMode
  #ifdef IsWithSDK                                                                                               
Source: "..\..\Win32\Debug\{#MyAppExeName}"        ; DestDir: "{app}\{code:DirModelsCreator}\{code:DirDebugExe}" ; Flags: ignoreversion; Check: not Is64BitInstallMode
  #endif                                                                                                         
Source: "..\..\Win32\Release\DyssolC.exe"          ; DestDir: "{app}"                                            ; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "..\..\ExternalLibraries\graphviz\bin32\*" ; DestDir: "{app}"                                            ; Flags: ignoreversion; Check: not Is64BitInstallMode
#endif                                                                                                           
#ifdef IsIncludeX64                                                                                              
Source: "..\..\x64\Release\{#MyAppExeName}"        ; DestDir: "{app}"                                            ; Flags: ignoreversion; Check: Is64BitInstallMode
  #ifdef IsWithSDK                                                                                              
Source: "..\..\x64\Debug\{#MyAppExeName}"          ; DestDir: "{app}\{code:DirModelsCreator}\{code:DirDebugExe}" ; Flags: ignoreversion; Check: Is64BitInstallMode
  #endif                                                                                                        
Source: "..\..\x64\Release\DyssolC.exe"            ; DestDir: "{app}"                                            ; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "..\..\ExternalLibraries\graphviz\bin64\*" ; DestDir: "{app}"                                            ; Flags: ignoreversion; Check: Is64BitInstallMode
#endif
Source: "..\..\LICENSE";          DestDir: "{app}";          Flags: ignoreversion
Source: "..\..\Materials.dmdb";   DestDir: "{app}";          Flags: ignoreversion
Source: "..\Data\Licenses\*.txt"; DestDir: "{app}\Licenses"; Flags: ignoreversion

[Dirs]
Name: "{app}\Licenses";                                  Flags: uninsalwaysuninstall
Name: "{userappdata}\{#MyAppName}";                      Flags: uninsalwaysuninstall
Name: "{userappdata}\{#MyAppName}\{code:DirCache}";      Flags: uninsalwaysuninstall
Name: "{userappdata}\{#MyAppName}\{code:DirCacheDebug}"; Flags: uninsalwaysuninstall

[Icons]
Name: "{group}\{#MyAppName}";                        Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:ProgramOnTheWeb,{#MyAppName}}";   Filename: "{#MyAppURL}"
Name: "{group}\{cm:UninstallProgram, {#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{group}\{#MyAppName} Uninstall";              Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}";                Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Registry]
Root: "HKLM"; Subkey: "Software\Classes\.dflw";                              ValueType: string;                   ValueData: "DyssolFlowsheet";                 Flags: uninsdeletevalue
Root: "HKLM"; Subkey: "Software\Classes\DyssolFlowsheet";                    ValueType: string;                   ValueData: "Dyssol Flowsheet";                Flags: uninsdeletekey
Root: "HKLM"; Subkey: "Software\Classes\DyssolFlowsheet\DefaultIcon";        ValueType: string;                   ValueData: "{app}\{#MyAppExeName},0"
Root: "HKLM"; Subkey: "Software\Classes\DyssolFlowsheet\shell\open\command"; ValueType: string;                   ValueData: """{app}\{#MyAppExeName}"" ""%1"""
Root: "HKLM"; Subkey: "Software\{#MyAppName}";                               ValueType: string; ValueName: "key"; ValueData: "{code:VarRegKey}";                Flags: uninsdeletekey

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: dirifempty;     Name: "{app}"
Type: filesandordirs; Name: "{userappdata}\{#MyAppName}\{code:DirCache}"
Type: filesandordirs; Name: "{userappdata}\{#MyAppName}\{code:DirCacheDebug}"

[INI]
Filename: "{userappdata}\{#MyAppName}\{code:FileConfigIni}"; Section: "General"; Key: "modelsFolders";         String: "{code:VarIniModelsFolders}";                   Flags: createkeyifdoesntexist
Filename: "{userappdata}\{#MyAppName}\{code:FileConfigIni}"; Section: "General"; Key: "modelsFoldersActivity"; String: "{code:VarIniFoldersActivity}";                 Flags: createkeyifdoesntexist
Filename: "{userappdata}\{#MyAppName}\{code:FileConfigIni}"; Section: "General"; Key: "materialsDBPath";       String: "{code:MakeRightSlashes|{app}\Materials.dmdb}"; Flags: createkeyifdoesntexist
Filename: "{userappdata}\{#MyAppName}\{code:FileConfigIni}"; Section: "General"; Key: "cachePath";             String: "{userappdata}\{#MyAppName}";                   Flags: createkeyifdoesntexist
Filename: "{userappdata}\{#MyAppName}\{code:FileConfigIni}"; Section: "General"; Key: "loadLast";              String: "false";                                        Flags: createkeyifdoesntexist

[Code]
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usUninstall then
  begin
    if MsgBox('Delete all configuration files?'+#13#10+'This includes a list of recently opened files, paths to models and materials database, etc.', mbConfirmation, MB_YESNO or MB_DEFBUTTON2) = IDYES then 
    begin
      DeleteFile(ExpandConstant('{userappdata}\{#MyAppName}\{code:FileConfigIni}'));
    end;
  end;
end;

procedure UpdateConfigIni();
var
  oldPaths: string;
  newPaths: string;
  iniPath: string;
begin
  iniPath := ExpandConstant('{userappdata}') + '\' + '{#MyAppName}' + '\' + FileConfigIni('');
  oldPaths := GetIniString('General', 'modelsFolders', '', iniPath);
  if oldPaths <> VarIniModelsFolders('') then
  begin
    if oldPaths = '' then
      newPaths := VarIniModelsFolders('')
    else
      newPaths := oldPaths + ', ' + VarIniModelsFolders('');
    SetIniString('General', 'modelsFolders', newPaths, iniPath);
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    UpdateConfigIni();
#ifdef IsWithSDK
    UpdateProjFiles();
#endif
  end;
end;

function MakeRightSlashes(Value: string): string;
begin
  Result := Value;
  StringChangeEx(Result, '\', '/', True);
end;

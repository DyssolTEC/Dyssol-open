; Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. 

#include "CommonConstants.iss"

#dim CppProjects[7]
#define CppProjects[0] "BaseSolvers"
#define CppProjects[1] "CacheHandler"
#define CppProjects[2] "EquationSolvers"
#define CppProjects[3] "HDF5Handler"
#define CppProjects[4] "MaterialsDatabase"
#define CppProjects[5] "ModelsAPI"
#define CppProjects[6] "Utilities"

#dim SolverTemplates[1]
#define SolverTemplates[0] "AgglomerationTemplate"
  
#dim UnitTemplates[4]
#define UnitTemplates[0] "Dynamic"
#define UnitTemplates[1] "DynamicWithDAESolver"
#define UnitTemplates[2] "SteadyState"
#define UnitTemplates[3] "SteadyStateWithNLSolver"

#define I

[Files]
; Projects
#sub CppProjectsFileEntry
  #ifdef IsWithSrc
Source: "..\..\{#CppProjects[I]}\*.vcxproj";          DestDir: "{app}\{code:DirModelsCreator}\{#CppProjects[I]}";                            Flags: ignoreversion
Source: "..\..\{#CppProjects[I]}\*.h";                DestDir: "{app}\{code:DirModelsCreator}\{#CppProjects[I]}";                            Flags: ignoreversion
Source: "..\..\{#CppProjects[I]}\*.cpp";              DestDir: "{app}\{code:DirModelsCreator}\{#CppProjects[I]}";                            Flags: ignoreversion
  #else                                                                                                                                    
Source: "..\..\{#CppProjects[I]}\*.h";                DestDir: "{app}\{code:DirModelsCreator}\{code:DirNoSrcInclude}";                       Flags: ignoreversion
    #ifdef IsIncludeX32                                                                                                                    
Source: "..\..\Win32\Debug\{#CppProjects[I]}*.lib";   DestDir: "{app}\{code:DirModelsCreator}\{code:DirNoSrcLib}\{code:DirNoSrcLibDebug}";   Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "..\..\Win32\Release\{#CppProjects[I]}*.lib"; DestDir: "{app}\{code:DirModelsCreator}\{code:DirNoSrcLib}\{code:DirNoSrcLibRelease}"; Flags: ignoreversion; Check: not Is64BitInstallMode
    #endif
    #ifdef IsIncludeX64
Source: "..\..\x64\Debug\{#CppProjects[I]}*.lib";     DestDir: "{app}\{code:DirModelsCreator}\{code:DirNoSrcLib}\{code:DirNoSrcLibDebug}";   Flags: ignoreversion; Check: Is64BitInstallMode
Source: "..\..\x64\Release\{#CppProjects[I]}*.lib";   DestDir: "{app}\{code:DirModelsCreator}\{code:DirNoSrcLib}\{code:DirNoSrcLibRelease}"; Flags: ignoreversion; Check: Is64BitInstallMode
    #endif
  #endif
#endsub
#for {I = 0; I < DimOf(CppProjects); I++} CppProjectsFileEntry

; Solvers templates
#sub SolverTemplatesFileEntry
Source: "..\..\Solvers\Templates\{#SolverTemplates[I]}\*.vcxproj"; DestDir: "{app}\{code:DirModelsCreator}\{code:DirSolverTemplates}\{#SolverTemplates[I]}"; Flags: ignoreversion
Source: "..\..\Solvers\Templates\{#SolverTemplates[I]}\*.h";       DestDir: "{app}\{code:DirModelsCreator}\{code:DirSolverTemplates}\{#SolverTemplates[I]}"; Flags: ignoreversion
Source: "..\..\Solvers\Templates\{#SolverTemplates[I]}\*.cpp";     DestDir: "{app}\{code:DirModelsCreator}\{code:DirSolverTemplates}\{#SolverTemplates[I]}"; Flags: ignoreversion
#endsub
#for {I = 0; I < DimOf(SolverTemplates); I++} SolverTemplatesFileEntry

; Units templates
#sub UnitTemplatesFileEntry
Source: "..\..\Units\Templates\{#UnitTemplates[I]}\*.vcxproj"; DestDir: "{app}\{code:DirModelsCreator}\{code:DirUnitTemplates}\{#UnitTemplates[I]}"; Flags: ignoreversion
Source: "..\..\Units\Templates\{#UnitTemplates[I]}\*.h";       DestDir: "{app}\{code:DirModelsCreator}\{code:DirUnitTemplates}\{#UnitTemplates[I]}"; Flags: ignoreversion
Source: "..\..\Units\Templates\{#UnitTemplates[I]}\*.cpp";     DestDir: "{app}\{code:DirModelsCreator}\{code:DirUnitTemplates}\{#UnitTemplates[I]}"; Flags: ignoreversion
#endsub
#for {I = 0; I < DimOf(UnitTemplates); I++} UnitTemplatesFileEntry

; Property sheets
Source: "..\..\PropertySheets\*";                                         DestDir: "{app}\{code:DirModelsCreator}\{code:DirPropertySheets}"; Flags: ignoreversion createallsubdirs recursesubdirs
#ifndef IsWithSrc
Source: "..\Data\ModelsCreatorSDK\PropertySheets\CommonDebugSDK.props";   DestDir: "{app}\{code:DirModelsCreator}\{code:DirPropertySheets}"; Flags: ignoreversion
Source: "..\Data\ModelsCreatorSDK\PropertySheets\CommonReleaseSDK.props"; DestDir: "{app}\{code:DirModelsCreator}\{code:DirPropertySheets}"; Flags: ignoreversion
#endif

; Solution files
Source: "..\Data\ModelsCreatorSDK\ModelsAPI.vcxproj.user"; DestDir: "{app}\{code:DirModelsCreator}\{code:DirMainProj}";               Flags: ignoreversion
#ifdef IsWithSrc
Source: "..\Data\ModelsCreatorSDK\Dyssol_Src.sln";         DestDir: "{app}\{code:DirModelsCreator}"; DestName: "{code:FileSolution}"; Flags: ignoreversion
#else                                               
Source: "..\..\ModelsAPI\*.vcxproj";                       DestDir: "{app}\{code:DirModelsCreator}\{code:DirMainProj}";               Flags: ignoreversion
Source: "..\Data\ModelsCreatorSDK\Dyssol_Lib.sln";         DestDir: "{app}\{code:DirModelsCreator}"; DestName: "{code:FileSolution}"; Flags: ignoreversion
#endif

; Binaries
#ifdef IsIncludeX32
Source: "..\..\ExternalLibraries\graphviz\bin32\*"; DestDir: "{app}\{code:DirModelsCreator}\{code:DirDebugExe}"; Flags: ignoreversion; Check: not Is64BitInstallMode
#endif
#ifdef IsIncludeX64
Source: "..\..\ExternalLibraries\graphviz\bin64\*"; DestDir: "{app}\{code:DirModelsCreator}\{code:DirDebugExe}"; Flags: ignoreversion; Check: Is64BitInstallMode
#endif

[Dirs]
Name: "{app}\{code:DirModelsCreator}";                                              Flags: uninsalwaysuninstall
Name: "{app}\{code:DirModelsCreator}\{code:DirExternalLibs}";                       Flags: uninsalwaysuninstall
Name: "{app}\{code:DirModelsCreator}\{code:DirMainProj}";                           Flags: uninsalwaysuninstall
Name: "{app}\{code:DirModelsCreator}\{code:DirSolversProjects}";                    Flags: uninsalwaysuninstall
Name: "{app}\{code:DirModelsCreator}\{code:DirUnitsProjects}";                      Flags: uninsalwaysuninstall
#ifndef IsWithSrc                                                                 
Name: "{app}\{code:DirModelsCreator}\{code:DirNoSrcInclude}";                       Flags: uninsalwaysuninstall
Name: "{app}\{code:DirModelsCreator}\{code:DirNoSrcLib}";                           Flags: uninsalwaysuninstall
Name: "{app}\{code:DirModelsCreator}\{code:DirNoSrcLib}\{code:DirNoSrcLibDebug}";   Flags: uninsalwaysuninstall
Name: "{app}\{code:DirModelsCreator}\{code:DirNoSrcLib}\{code:DirNoSrcLibRelease}"; Flags: uninsalwaysuninstall
#endif

[Code]
// Removes all lines that contain Tag
procedure RemoveLines(const FileName, Tag: string);
var
  I: Integer;
  FileLines: TStringList;
begin
  FileLines := TStringList.Create;
  try
    FileLines.LoadFromFile(FileName);
    I := 0;
    while I < FileLines.Count do 
    begin
      if Pos(Tag, FileLines[I]) = 0 then
        I := I + 1
      else
        FileLines.Delete(I);
    end;
  finally
    FileLines.SaveToFile(FileName);
    FileLines.Free;
  end;
end;

// Removes all Nodes, which attribute contains AttributeText
procedure RemoveNodeFromXML(const FileName, Node, AttributeText: string);
var
  XMLDocument: Variant;
  XMLNodes: Variant;
  I: Integer;
begin
  XMLDocument := CreateOleObject('Msxml2.DOMDocument.6.0');
  try
    XMLDocument.async := False;
    XMLDocument.load(FileName);
    if (XMLDocument.parseError.errorCode = 0) then
    begin
      XMLDocument.setProperty('SelectionLanguage', 'XPath');
      XMLNodes := XMLDocument.selectNodes('//*[local-name()=''' + Node + ''' and contains(@*,''' + AttributeText + ''')]');
      for I := 0 to XMLNodes.length - 1 do
      begin
        XMLNodes.item[I].ParentNode.RemoveChild(XMLNodes.item[I]);
      end;
      XMLDocument.save(FileName);
    end;
  except  
  end;
end;

// Writes new Value to the ChildNode, which is a child of Node with Attribute
procedure WriteValueToXML(const FileName, Node, Attribute, ChildNode, Value: string);
var
  XMLDocument: Variant;
  XMLNodes: Variant;
  I: Integer;
begin
  XMLDocument := CreateOleObject('Msxml2.DOMDocument.6.0');
  try
    XMLDocument.async := False;
    XMLDocument.load(FileName);
    if (XMLDocument.parseError.errorCode = 0) then
    begin
      XMLDocument.setProperty('SelectionLanguage', 'XPath');
      XMLNodes := XMLDocument.selectNodes('//*[local-name()=''' + Node + ''' and contains(@*,''' + Attribute + ''')]/node()');
      for I := 0 to XMLNodes.length - 1 do
      begin
        if XMLNodes.item[I].nodeName = ChildNode then
          XMLNodes.item[I].text := Value;
      end;
      XMLDocument.save(FileName);
    end;
  except
  end;
end;

// Remove Win32/x64 configuration from file
procedure UpdateVcprojFile(FilePath : string);
begin
  if Is64BitInstallMode then
  begin    
    RemoveNodeFromXML(FilePath, 'ProjectConfiguration', '|Win32');
    RemoveNodeFromXML(FilePath, 'PropertyGroup',        '|Win32');
    RemoveNodeFromXML(FilePath, 'ImportGroup',          '|Win32');
    RemoveNodeFromXML(FilePath, 'ItemDefinitionGroup',  '|Win32');
  end
  else
  begin
    RemoveNodeFromXML(FilePath, 'ProjectConfiguration', '|x64');
    RemoveNodeFromXML(FilePath, 'PropertyGroup',        '|x64');
    RemoveNodeFromXML(FilePath, 'ImportGroup',          '|x64');
    RemoveNodeFromXML(FilePath, 'ItemDefinitionGroup',  '|x64');
  end;
end;

// Remove Win32/x64 configuration from all *.vcxproj file
procedure UpdateVcprojFiles();
var
  FilePath: string;
begin
#sub UpdateProjFilesEntry
  FilePath := ExpandConstant('{app}\') + DirModelsCreator('') + '\' + ExpandConstant('{#CppProjects[I]}') + '\' + ExpandConstant('{#CppProjects[I]}') + '.vcxproj';
  UpdateVcprojFile(FilePath);
#endsub
#for {I = 0; I < DimOf(CppProjects); I++} UpdateProjFilesEntry
#sub UpdateProjUnitTemplatesFileEntry
  FilePath := ExpandConstant('{app}\') + DirModelsCreator('') + '\' + DirUnitTemplates('') + '\' + ExpandConstant('{#UnitTemplates[I]}') + '\' + ExpandConstant('{#UnitTemplates[I]}') + '.vcxproj';
  UpdateVcprojFile(FilePath);
#endsub
#for {I = 0; I < DimOf(UnitTemplates); I++} UpdateProjUnitTemplatesFileEntry
#sub UpdateProjSolverTemplatesFileEntry
  FilePath := ExpandConstant('{app}\') + DirModelsCreator('') + '\' + DirSolverTemplates('') + '\' + ExpandConstant('{#SolverTemplates[I]}') + '\' + ExpandConstant('{#SolverTemplates[I]}') + '.vcxproj';
  UpdateVcprojFile(FilePath);
#endsub
#for {I = 0; I < DimOf(SolverTemplates); I++} UpdateProjSolverTemplatesFileEntry
end;

// Remove Win32/x64 configuration from all ModelsAPI.vcxproj.user file
procedure UpdateVcprojUserFile();
var
  FilePath: string;
begin
  // remove unnecessary configurations
  FilePath := ExpandConstant('{app}\') + DirModelsCreator('') + '\' + DirMainProj('') + '\' + DirMainProj('') + '.vcxproj.user';
  if Is64BitInstallMode then
    RemoveNodeFromXML(FilePath, 'PropertyGroup', '|Win32')
  else
    RemoveNodeFromXML(FilePath, 'PropertyGroup', '|x64');
  // set proper path to exe
  WriteValueToXML(FilePath, 'PropertyGroup', 'Release', 'LocalDebuggerCommand', ExpandConstant('{app}\{#MyAppExeName}'));
end;

// Update ModelsAPI.vcxproj file for no-src case
procedure UpdateMainVcprojFile();
var
  FilePath: string;
begin
  FilePath := ExpandConstant('{app}\') + DirModelsCreator('') + '\' + DirMainProj('') + '\' + DirMainProj('') + '.vcxproj';  
  RemoveNodeFromXML(FilePath, 'ClInclude', '.h');
  RemoveNodeFromXML(FilePath, 'ClCompile', '.cpp');
  RemoveNodeFromXML(FilePath, 'ProjectReference', '.vcxproj');
  WriteValueToXML(FilePath, 'PropertyGroup', 'Configuration', 'ConfigurationType', 'Application');
end;

// Remove Win32/x64 configuration from Dyssol.sln file
procedure UpdateSlnFile();
var
  FilePath: string;
begin
  FilePath := ExpandConstant('{app}\') + DirModelsCreator('') + '\' +  FileSolution('');
  if Is64BitInstallMode then
  begin
    RemoveLines(FilePath, 'Debug|Win32');
    RemoveLines(FilePath, 'Release|Win32');
  end
  else
  begin
    RemoveLines(FilePath, 'Debug|x64');
    RemoveLines(FilePath, 'Release|x64');
  end;
end;

procedure UpdateProjFiles();
begin
  UpdateVcprojFiles();
  UpdateVcprojUserFile();
#ifndef IsWithSrc
  UpdateMainVcprojFile();
#endif
  UpdateSlnFile();
end;

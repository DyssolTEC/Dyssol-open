; Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. 

#ifndef CommonConstantsIss
#define CommonConstantsIss

[Code]
function DirCache(Param: String): String;               begin  Result := 'cache';               end;
function DirCacheDebug(Param: String): String;          begin  Result := 'cache_debug';         end;
function DirExampleFlowsheets(Param: String): String;   begin  Result := 'Example Flowsheets';  end;
function DirHelp(Param: String): String;                begin  Result := 'Help';                end;
function DirQtPlatforms(Param: String): String;         begin  Result := 'platforms';           end;
function DirUnitsDll(Param: String): String;            begin  Result := 'Units';               end;
function DirSolversDll(Param: String): String;          begin  Result := 'Solvers';             end;
function DirExampleUnits(Param: String): String;        begin  Result := 'Example Units';       end;
function DirExampleSolvers(Param: String): String;      begin  Result := 'Example Solvers';     end;
function DirCppTemplate(Param: String): String;         begin  Result := 'VCProject';           end;
function DirDebugExe(Param: String): String;            begin  Result := 'ExecutableDebug';     end;
function DirSolversDebugDll(Param: String): String;     begin  Result := 'SolversDebugLibs';    end;
function DirUnitsDebugDll(Param: String): String;       begin  Result := 'UnitsDebugLibs';      end;
function DirExternalLibs(Param: String): String;        begin  Result := 'ExternalLibraries';   end;
function DirPropertySheets(Param: String): String;      begin  Result := 'PropertySheets';      end;
function DirMainProj(Param: String): String;            begin  Result := 'ModelsAPI';           end;
function DirSolversProjects(Param: String): String;     begin  Result := 'Solvers';             end;
function DirUnitsProjects(Param: String): String;       begin  Result := 'Units';               end;
function DirSolverTemplates(Param: String): String;     begin  Result := 'SolversTemplates';    end;
function DirUnitTemplates(Param: String): String;       begin  Result := 'UnitsTemplates';      end;
function DirNoSrcInclude(Param: String): String;        begin  Result := 'include';             end;
function DirNoSrcLib(Param: String): String;            begin  Result := 'lib';                 end;
function DirNoSrcLibDebug(Param: String): String;       begin  Result := 'Debug';               end;
function DirNoSrcLibRelease(Param: String): String;     begin  Result := 'Release';             end;

function FileConfigIni(Param: String): String;          begin  Result := 'config.ini';          end;
function FileSolution(Param: String): String;           begin  Result := 'Dyssol.sln';          end;

function VarIniModelsFolders(Param: String): String;    begin  Result := 'Units, Solvers, ../UnitsDebugLibs, ../SolversDebugLibs, ../Win32/Debug, ../Win32/Release, ../x64/Debug, ../x64/Release'; end;
function VarIniFoldersActivity(Param: String): String;  begin  Result := '@Variant(\0\0\0\x7f\0\0\0\fQList<bool>\0\0\0\0\b\x1\x1\x1\x1\x1\x1\x1\x1)';                                              end;

function VarRegKey(Param: String): String;              begin  Result := '{9E9E04A4-2EE2-4146-96C2-18FF59F7B0B5}';  end;

#endif

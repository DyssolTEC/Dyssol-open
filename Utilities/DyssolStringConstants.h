/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "StringFunctions.h"

namespace StrConst
{
//////////////////////////////////////////////////////////////////////////
/// Global
//////////////////////////////////////////////////////////////////////////
	const char* const G_RegPath         = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Dyssol";
	const char* const G_RegKey          = "key";
	const char* const G_TUHHRegPath     = "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters";
	const char* const G_TUHHRegKeyName  = "NV Domain";
	const char* const G_TUHHRegKeyValue = "tu-harburg.de";

	const std::string COMMENT_SYMBOL    = "$";

//////////////////////////////////////////////////////////////////////////
/// Dyssol
//////////////////////////////////////////////////////////////////////////
	const char* const Dyssol_ApplicationName       = "Dyssol";
	const char* const Dyssol_ConfigApp             = "application";
	const char* const Dyssol_ConfigFileName        = "config.ini";
	const char* const Dyssol_ConfigLastParamName   = "lastFile";
	const char* const Dyssol_ConfigRecentParamName = "recentFiles";
	const char* const Dyssol_ConfigLoadLastFlag    = "loadLast";
	const char* const Dyssol_ConfigDMDBPath        = "materialsDBPath";
	const char* const Dyssol_ConfigCachePath       = "cachePath";
	const char* const Dyssol_CacheDirRelease       = "/cache";
	const char* const Dyssol_CacheDirDebug         = "/cache_debug";
	const char* const Dyssol_HelpDir			   = "Help";
	const char* const Dyssol_HelpFileExt           = ".pdf";
	const char* const Dyssol_MainWindowName        = "Dyssol";
	const char* const Dyssol_MDBWindowName		   = "Materials Database";
	const char* const Dyssol_FlowsheetTabName      = "Flowsheet";
	const char* const Dyssol_SimulatorTabName      = "Simulator";
	const char* const Dyssol_StreamsTabName        = "Streams";
	const char* const Dyssol_UnitsTabName          = "Units";
	const char* const Dyssol_SaveMessageBoxText    = "Save current flowsheet?";
	const char* const Dyssol_SaveMDBMessageBoxText = "Save current materials database?";
	const char* const Dyssol_AbortMessage          = "This will abort the current simulation and close Dyssol. Proceed?";
	const char* const Dyssol_DialogDflwFilter      = "DYSSOL files (*.dflw);;All files (*.*);;";
	const char* const Dyssol_DialogTxtFilter       = "Text files (*.txt);;All files (*.*);;";
	const char* const Dyssol_DialogSaveName        = "Save flowsheet";
	const char* const Dyssol_DialogOpenName        = "Open flowsheet";
	const char* const Dyssol_DialogSaveConfigName  = "Save configuration file";
	const char* const Dyssol_StatusLoadingTitle    = "Loading";
	const char* const Dyssol_StatusLoadingText     = "Flowsheet is loading. <br/>Please wait";
	const char* const Dyssol_StatusLoadingQuestion = "Terminate loading of the flowsheet?";
	const char* const Dyssol_StatusSavingTitle     = "Saving";
	const char* const Dyssol_StatusSavingText      = "Flowsheet is saving. <br/>Please wait";
	const char* const Dyssol_StatusSavingQuestion  = "Terminate saving of the flowsheet?";
	const char* const Dyssol_AboutDyssolDescr      = "Dyssol: Dynamic Simulation of Solids Processes";
	const char* const Dyssol_AppFolderPathLink     = "AppFolder.lnk";


//////////////////////////////////////////////////////////////////////////
/// CBaseModel
//////////////////////////////////////////////////////////////////////////
	const char* const BModel_H5ModelName	   = "ModelName";
	const char* const BModel_H5ModelKey		   = "ModelKey";
	const char* const BModel_H5GroupUnit	   = "Unit";
	const char* const BModel_H5AttrSaveVersion = "SaveVersion";


//////////////////////////////////////////////////////////////////////////
/// CUnitContainer
//////////////////////////////////////////////////////////////////////////
	const char* const BCont_H5UnitName   = "UnitName";
	const char* const BCont_H5UnitKey    = "UnitKey";
	const char* const BCont_H5ModelKey   = "ModelKey";
	const char* const BCont_H5GroupModel = "Unit";


//////////////////////////////////////////////////////////////////////////
/// CUnitPort
//////////////////////////////////////////////////////////////////////////
	const char* const UPort_H5AttrPortsNum  = "PortsNumber";
	const char* const UPort_H5GroupPortName = "Port";
	const char* const UPort_H5PortsNames    = "UnitPortsNames";
	const char* const UPort_H5PortsKeys     = "UnitPortsKeys";
	const char* const UPort_H5Name          = "Name";
	const char* const UPort_H5Type          = "Type";
	const char* const UPort_H5StreamKey     = "StreamKey";


//////////////////////////////////////////////////////////////////////////
/// CStateVariable
//////////////////////////////////////////////////////////////////////////
	const char* const SVar_H5Name    = "Name";
	const char* const SVar_H5Value   = "Value";
	const char* const SVar_H5History = "History";


//////////////////////////////////////////////////////////////////////////
/// CStreamManager
//////////////////////////////////////////////////////////////////////////
	const char* const StrMngr_H5AttrFeedsInitNum   = "FeedsInitNumber";
	const char* const StrMngr_H5AttrFeedsWorkNum   = "FeedsWorkNumber";
	const char* const StrMngr_H5AttrHoldupsInitNum = "HoldupsInitNumber";
	const char* const StrMngr_H5AttrHoldupsWorkNum = "HoldupsWorkNumber";
	const char* const StrMngr_H5AttrStreamsWorkNum = "StreamsWorkNumber";
	const char* const StrMngr_H5GroupFeedsInit     = "FeedsInit";
	const char* const StrMngr_H5GroupFeedsWork     = "FeedsWork";
	const char* const StrMngr_H5GroupHoldupsInit   = "HoldupsInit";
	const char* const StrMngr_H5GroupHoldupsWork   = "HoldupsWork";
	const char* const StrMngr_H5GroupStreamsWork   = "StreamsWork";
	const char* const StrMngr_H5GroupFeedName      = "Feed";
	const char* const StrMngr_H5GroupHoldupName    = "Holdup";
	const char* const StrMngr_H5GroupStreamName    = "Stream";
	const char* const StrMngr_H5Names              = "Names";


//////////////////////////////////////////////////////////////////////////
/// CPlotManager
//////////////////////////////////////////////////////////////////////////
	const char* const PlotMngr_H5AttrPlotsNum   = "PlotsNumber";
	const char* const PlotMngr_H5GroupPlotName  = "Plot";
	const char* const PlotMngr_H5PlotName       = "PlotName";
	const char* const PlotMngr_H5PlotXAxis      = "XAxis";
	const char* const PlotMngr_H5PlotYAxis      = "YAxis";
	const char* const PlotMngr_H5PlotZAxis      = "ZAxis";
	const char* const PlotMngr_H5AttrCurvesNum  = "CurvesNumber";
	const char* const PlotMngr_H5GroupCurves    = "Curves";
	const char* const PlotMngr_H5GroupCurveName = "Curve";
	const char* const PlotMngr_H5CurveName      = "CurveName";
	const char* const PlotMngr_H5CurveZValue    = "ZValue";
	const char* const PlotMngr_H5CurveData      = "Data";


//////////////////////////////////////////////////////////////////////////
/// CStateVariablesManager
//////////////////////////////////////////////////////////////////////////
	const char* const SVMngr_H5AttrStateVarsNum  = "StateVarsNumber";
	const char* const SVMngr_H5GroupStateVarName = "StateVariable";


//////////////////////////////////////////////////////////////////////////
/// CBaseUnit
//////////////////////////////////////////////////////////////////////////
	const char* const BUnit_DefaultPortName   = "Port";

	const char* const BUnit_UnspecValue	   = "Unspecified";
	const char* const BUnit_NoWarnings	   = "";
	const char* const BUnit_NoErrors	   = "";
	const char* const BUnit_NoInfos		   = "";
	const char* const BUnit_UnknownWarning = "Unknown unit's warning";
	const char* const BUnit_UnknownError   = "Unknown unit's error";

	const char* const BUnit_H5UnitKey			      = "UnitKey";
	const char* const BUnit_H5GroupPorts			  = "Ports";
	const char* const BUnit_H5GroupPortName			  = "Port";
	const char* const BUnit_H5UnitPorts			      = "UnitPorts";
	const char* const BUnit_H5UnitPortsKeys		      = "UnitPortsKeys";
	const char* const BUnit_H5UnitPortsNames		  = "UnitPortsNames";
	const char* const BUnit_H5GroupInternalMaterials  = "InternalMaterials";
	const char* const BUnit_H5GroupHoldups	          = "Holdups";
	const char* const BUnit_H5HoldupsNames	          = "HoldupsNames";
	const char* const BUnit_H5GroupHoldupName	      = "Holdup";
	const char* const BUnit_H5GroupHoldupsWork	      = "WorkHoldups";
	const char* const BUnit_H5GroupHoldupWorkName     = "WorkHoldup";
	const char* const BUnit_H5WorkHoldupsNames		  = "WorkHoldupsNames";
	const char* const BUnit_H5GroupStreamsWork	      = "WorkStreams";
	const char* const BUnit_H5GroupStreamWorkName     = "WorkStream";
	const char* const BUnit_H5WorkStreamsNames		  = "WorkStreamsNames";
	const char* const BUnit_H5GroupParams		      = "UnitParameters";
	const char* const BUnit_H5GroupStateVars          = "StateVariables";
	const char* const BUnit_H5GroupStateVarName	      = "StateVariable";
	const char* const BUnit_H5StateVarName            = "Name";
	const char* const BUnit_H5StateVarValue		      = "Value";
	const char* const BUnit_H5StateVarSavedVal	      = "SavedValue";
	const char* const BUnit_H5StateVarIsSaved	      = "IsSaved";
	const char* const BUnit_H5StateVarTimes		      = "Times";
	const char* const BUnit_H5StateVarValues	      = "Values";
	const char* const BUnit_H5AttrPortsNum			  = "PortsNumber";
	const char* const BUnit_H5AttrHoldupsNum	      = "HoldupsNumber";
	const char* const BUnit_H5AttrHoldupsWorkNum      = "WorkHoldupsNumber";
	const char* const BUnit_H5AttrStreamsWorkNum      = "WorkStreamsNumber";
	const char* const BUnit_H5AttrStateVarsNum	      = "StateVarsNumber";
	const char* const BUnit_H5AttrSaveVersion	      = "SaveVersion";

	const char* const BUnit_H5AttrPlotsNum		  = "PlotsNumber";
	const char* const BUnit_H5GroupPlots	      = "Plots";
	const char* const BUnit_H5GroupPlotName	      = "Plot";
	const char* const BUnit_H5PlotName            = "PlotName";
	const char* const BUnit_H5PlotXAxis           = "XAxis";
	const char* const BUnit_H5PlotYAxis           = "YAxis";
	const char* const BUnit_H5PlotZAxis           = "ZAxis";
	const char* const BUnit_H5PlotIs2D            = "Is2D";
	const char* const BUnit_H5AttrCurvesNum       = "CurvesNumber";
	const char* const BUnit_H5GroupCurves         = "Curves";
	const char* const BUnit_H5GroupCurveName      = "Curve";
	const char* const BUnit_H5CurveName           = "Name";
	const char* const BUnit_H5CurveX              = "XVector";
	const char* const BUnit_H5CurveY              = "YVector";
	const char* const BUnit_H5CurveZ              = "ZValue";

	// TODO: delete unused
	inline std::string BUnit_Err0(const std::string& unit, const std::string& fun) {
		return std::string("Error in unit '" + unit + "', function call " + fun + "(): "); }
	inline std::string BUnit_Err1(const std::string& unit, const std::string& fun, const std::string& param1) {
		return std::string("Error in unit '" + unit + "', function call " + fun + "('" + param1 + "'): "); }
	inline std::string BUnit_Err2(const std::string& unit, const std::string& fun, const std::string& param1, const std::string& param2) {
		return std::string("Error in unit '" + unit + "', function call " + fun + "('" + param1 + "', '" + param2 + "'): "); }
	inline std::string BUnit_Err3(const std::string& unit, const std::string& fun, const std::string& param1, const std::string& param2, const std::string& param3) {
		return std::string("Error in unit '" + unit + "', function call " + fun + "('" + param1 + "', '" + param2 + "', '" + param3 + "'): "); }
	inline std::string BUnit_ErrAddPort(const std::string& s1, const std::string& s2, const std::string& s3) {
		return BUnit_Err1(s1, s3, s2) + "Port '" + s2 + "' has duplicates. Ports names must be unique within the unit."; }
	inline std::string BUnit_ErrGetPort(const std::string& s1, const std::string& s2, const std::string& s3) {
		return BUnit_Err1(s1, s3, s2) + "A port with such name does not exist in this unit."; }
	inline std::string BUnit_ErrGetPortEmpty(const std::string& s1, const std::string& s2, const std::string& s3) {
		return BUnit_Err1(s1, s3, s2) + "A port is not connected to the stream."; }
	inline std::string BUnit_ErrAddFeed(const std::string& s1, const std::string& s2, const std::string& s3) {
		return BUnit_Err1(s1, s3, s2) + "Feed '" + s2 + "' has duplicates. Feeds names must be unique within the unit."; }
	inline std::string BUnit_ErrAddHoldup(const std::string& s1, const std::string& s2, const std::string& s3) {
		return BUnit_Err1(s1, s3, s2) + "Holdup '" + s2 + "' has duplicates. Holdups names must be unique within the unit."; }
	inline std::string BUnit_ErrAddStream(const std::string& s1, const std::string& s2, const std::string& s3) {
		return BUnit_Err1(s1, s3, s2) + "Stream '" + s2 + "' has duplicates. Streams names must be unique within the unit."; }
	inline std::string BUnit_ErrGetFeed(const std::string& s1, const std::string& s2, const std::string& s3) {
		return BUnit_Err1(s1, s3, s2) + "A feed with such name does not exist in this unit."; }
	inline std::string BUnit_ErrGetHoldup(const std::string& s1, const std::string& s2, const std::string& s3) {
		return BUnit_Err1(s1, s3, s2) + "A holdup with such name does not exist in this unit."; }
	inline std::string BUnit_ErrGetStream(const std::string& s1, const std::string& s2, const std::string& s3) {
		return BUnit_Err1(s1, s3, s2) + "A stream with such name does not exist in this unit."; }
	inline std::string BUnit_ErrGetParam(const std::string& s1, const std::string& s2, const std::string& s3) {
		return std::string("Error in unit '" + s1 + "', function call " + s3 + "('" + s2 + "'): unit parameter with such combination of name and type does not exist in this unit.");	}
	inline std::string BUnit_ErrAddParam(const std::string& s1, const std::string& s2, const std::string& s3) {
		return BUnit_Err1(s1, s3, s2) + "Unit parameter '" + s2 + "' has duplicates. Unit parameters names must be unique within the unit."; }
	inline std::string BUnit_ErrAddComboParam(const std::string& s1, const std::string& s2, const std::string& s3) {
		return BUnit_Err1(s1, s3, s2) + "Unit parameter '" + s2 + "' cannot be added. The number of items must be equal to the number of names."; }
	inline std::string BUnit_ErrGroupParamBlock(const std::string& s1, const std::string& s2, const std::string& s3, const std::string& s4) {
		return BUnit_Err3(s1, s4, s2, s3, "{}") + "A group unit parameter with name '" + s2 + "' does not exist in this unit.";	}
	inline std::string BUnit_ErrGroupParamGroup(const std::string& s1, const std::string& s2, const std::string& s3, const std::string& s4) {
		return BUnit_Err3(s1, s4, s2, s3, "{}") + "A group with name '" + s3 + "' does not exist in this group unit parameter."; }
	inline std::string BUnit_ErrGroupParamParam(const std::string& s1, const std::string& s2, const std::string& s3, const std::string& s4, const std::string& s5) {
		return BUnit_Err3(s1, s5, s2, s3, s4) + std::string("A unit parameter with name  '" + s4 + "' does not exist in this unit."); }
	inline std::string BUnit_ErrAddSV(const std::string& s1, const std::string& s2, const std::string& s3) {
		return BUnit_Err1(s1, s3, s2) + "State variable '" + s2 + "' has duplicates. State variables names must be unique within the unit."; }
	inline std::string BUnit_ErrGetSV(const std::string& s1, const std::string& s2, const std::string& s3) {
		return BUnit_Err1(s1, s3, s2) + "A state variable with such name does not exist in this unit.";}
	inline std::string BUnit_ErrAddPlot(const std::string& s1, const std::string& s2, const std::string& s3) {
		return BUnit_Err1(s1, s3, s2) + ("Plot '" + s2 + "' has duplicates. Plots names must be unique within the unit."); }
	inline std::string BUnit_ErrAddCurve2D(const std::string& s1, const std::string& s2, const std::string& s3, const std::string& s4) {
		return BUnit_Err2(s1, s4, s3, s2) + "Curve '" + s3 + "' has duplicates in plot '" + s2 + "'. Curves names must be unique within the plot."; }
	inline std::string BUnit_ErrAddCurve3D(const std::string& s1, const std::string& s2, double s3, const std::string& s4) {
		return BUnit_Err2(s1, s4, StringFunctions::Double2String(s3), s2) + ("A curve with Z-value '" + StringFunctions::Double2String(s3) + "' has duplicates in plot '" + s2 + "'. Z-values must be unique within the plot."); }
	inline std::string BUnit_ErrGetPlot(const std::string& s1, const std::string& s2, const std::string& s3) {
		return BUnit_Err1(s1, s3, s2) + "A plot with such name does not exist in this unit"; }
	inline std::string BUnit_ErrGetCurve2D(const std::string& s1, const std::string& s2, const std::string& s3, const std::string& s4) {
		return BUnit_Err2(s1, s4, s3, s2) + "A curve with such name does not exist in plot '" + s2 + "'."; }
	inline std::string BUnit_ErrGetCurve3D(const std::string& s1, const std::string& s2, double s3, const std::string& s4) {
		return BUnit_Err2(s1, s4, StringFunctions::Double2String(s3), s2) + "A curve with such Z-value does not exist in plot '" + s2 + "'."; }
	inline std::string BUnit_ErrCopyToPort(const std::string& s1, const std::string& s2, const std::string& s3, const std::string& s4) {
		return BUnit_Err2(s1, s4, s2, s3) + "Cannot copy to input port with name '" + s3 + "'."; }
	inline std::string BUnit_ErrCopyFromPort(const std::string& s1, const std::string& s2, const std::string& s3, const std::string& s4) {
		return BUnit_Err2(s1, s4, s2, s3) + "Cannot copy from output port with name '" + s2 + "'."; }

//////////////////////////////////////////////////////////////////////////
/// CUnitParameters
//////////////////////////////////////////////////////////////////////////
	const char* const UParam_H5Times            = "Times";
	const char* const UParam_H5Values           = "Values";
	const char* const UParam_H5StrValue         = "StrValue";
	const char* const UParam_H5AttrReactionsNum = "ReactionsNumber";
	const char* const UParam_H5Reaction         = "Reaction";
	const char* const UParam_H5Names            = "ParamsNames";
	const char* const UParam_H5GroupParamName   = "UnitParameter";


//////////////////////////////////////////////////////////////////////////
/// CDenseDistr2D
//////////////////////////////////////////////////////////////////////////
	const char* const Distr2D_H5AttrDimsNum		= "DimensionsNumber";
	const char* const Distr2D_H5TimePoints		= "TimePoints";
	const char* const Distr2D_H5Data			= "Data";
	const char* const Distr2D_H5AttrSaveVersion	= "SaveVersion";


//////////////////////////////////////////////////////////////////////////
/// CDistributionsGrid
//////////////////////////////////////////////////////////////////////////
	const char* const DGrid_H5AttrGridsNum	  = "GridsNumber";
	const char* const DGrid_H5GroupName	      = "Grid";
	const char* const DGrid_H5GridType	      = "States";
	const char* const DGrid_H5DistrType		  = "Types";
	const char* const DGrid_H5GridFun		  = "Function";
	const char* const DGrid_H5Classes	      = "Classes";
	const char* const DGrid_H5Units			  = "Units";
	const char* const DGrid_H5NumGrid	      = "NumGrid";
	const char* const DGrid_H5StrGrid         = "StrGrid";
	const char* const DGrid_H5AttrSaveVersion = "SaveVersion";


//////////////////////////////////////////////////////////////////////////
/// CSimulator
//////////////////////////////////////////////////////////////////////////
	const char* const	Sim_ErrMinTWLength           = "The minimum length of the time window is reached. Simulation stopped.";
	const char* const	Sim_ErrMaxTWIterations       = "Maximum number of iterations has been reached. Simulation will be stopped.";
	const char* const	Sim_InfoSaveInitTearStreams  = "Saving new initial values of tear streams...";
	const char* const	Sim_InfoFalseInitTearStreams = "Cannot converge using previous results as initial values. Resetting to defaults and restarting.";
	inline std::string  Sim_InfoRecycleStreamCalculating(unsigned iWin, unsigned iIter, double t1, double t2) {
		return std::string("Recycle stream. Time window #" + std::to_string(iWin) + ". Iteration #" + std::to_string(iIter) + " [" + StringFunctions::Double2String(t1) + ", " + StringFunctions::Double2String(t2) + "]..."); }
	inline std::string  Sim_InfoRecycleStreamFinishing(unsigned iWin, unsigned iIter, double t1, double t2) {
		return std::string("Finishing recycle stream. Time window #" + std::to_string(iWin) + ". Iteration #" + std::to_string(iIter) + " [" + StringFunctions::Double2String(t1) + ", " + StringFunctions::Double2String(t2) + "]..."); }
	inline std::string  Sim_InfoUnitInitialization(const std::string& unit, const std::string& model) {
		return std::string("Initialization of " + unit + " (" + model + ")..."); }
	inline std::string  Sim_InfoUnitSimulation(const std::string& unit, const std::string& model, double t1, double t2) {
		return std::string("Simulation of " + unit + " (" + model + "): [" + StringFunctions::Double2String(t1) + ", " + StringFunctions::Double2String(t2) + "]..."); }
	inline std::string  Sim_InfoUnitFinalization(const std::string& unit, const std::string& model) {
		return std::string("Finalization of " + unit + " (" + model + ")..."); }
	inline std::string  Sim_WarningParamOutOfRange(const std::string& unit, const std::string& model, const std::string& param) {
		return std::string("In unit '" + unit + "' (" + model + "), parameter '" + param + "': value is out of range."); }


//////////////////////////////////////////////////////////////////////////
/// CFlowsheet
//////////////////////////////////////////////////////////////////////////
	const char* const Flow_H5AttrUnitsNum		     = "ModelsNumber";
	const char* const Flow_H5GroupUnits		         = "Models";
	const char* const Flow_H5GroupUnitName		     = "Model";
	const char* const Flow_H5ModelKey			     = "UnitKey";
	const char* const Flow_H5AttrStreamsNum		     = "StreamsNumber";
	const char* const Flow_H5GroupStreams		     = "Streams";
	const char* const Flow_H5GroupStreamName	     = "Stream";
	const char* const Flow_H5GroupCalcSeq			 = "CalculationSequence";
	const char* const Flow_H5GroupInitTearStreams	 = "InitTearStreams";
	const char* const Flow_H5GroupPartitionName	     = "Partition";
	const char* const Flow_H5GroupInitTearStreamName = "InitTearStream";
	const char* const Flow_H5Compounds			     = "Compounds";
	const char* const Flow_H5AttrOverallsNum         = "OverallsNumber";
	const char* const Flow_H5GroupOveralls           = "OverallProperties";
	const char* const Flow_H5GroupOverallName        = "OverallProperty";
	const char* const Flow_H5OverallType             = "Type";
	const char* const Flow_H5OverallName             = "Name";
	const char* const Flow_H5OverallUnits            = "Units";
	const char* const Flow_H5AttrPhasesNum           = "PhasesNumber";
	const char* const Flow_H5GroupPhases		     = "Phases";
	const char* const Flow_H5GroupPhaseName          = "Phase";
	const char* const Flow_H5PhaseType               = "Type";
	const char* const Flow_H5PhaseName               = "Name";
	const char* const Flow_H5PhasesNames		     = "PhaseNames";
	const char* const Flow_H5PhasesSOA			     = "PhaseAggregationStates";
	const char* const Flow_H5GroupOptions		     = "Options";
	const char* const Flow_H5OptionSimTime		     = "SimulationTime";
	const char* const Flow_H5AttrSaveVersion	     = "SaveVersion";

	inline std::string  Flow_ErrEmptyHoldup(const std::string& s1, const std::string& s2) {
		return std::string("Holdup '" + s2 + "' in unit '" + s1 + "' is empty."); }
	inline std::string  Flow_ErrEmptyUnit(const std::string& s1) {
		return std::string("No model is assigned to unit '" + s1 + "'."); }
	inline std::string  Flow_ErrPhaseFractions(const std::string& u, const std::string& h, double t) {
		return std::string("Phase fractions of holdup '" + h + "' in unit '" + u + "' at time point " + std::to_string(t) + " s do not sum up to 1."); }
	inline std::string  Flow_ErrCompoundFractions(const std::string& u, const std::string& h, const std::string& p, double t) {
		return std::string("Compound fractions of holdup '" + h + "' of phase '" + p + "' in unit '" + u + "' at time point " + std::to_string(t) + " s do not sum up to 1."); }
	const char* const	Flow_ErrEmptyMDB = "Materials database file has not been loaded or empty.";
	const char* const	Flow_ErrEmptySequence = "Calculation sequence is empty.";
	const char* const	Flow_ErrWrongSequence = "Wrong calculation sequence.";
	inline std::string  Flow_ErrIncompSequence(const std::string& s1) { return std::string("Unit '" + s1 + "' is not in calculation sequence."); }
	const char* const	Flow_ErrMultSolids = "More than 1 solid phase has been defined.";
	const char* const	Flow_ErrMultVapors = "More than 1 vapor phase has been defined.";
	const char* const	Flow_ErrNoCompounds = "No compounds specified.";
	inline std::string	Flow_ErrWrongCompound(const std::string& s1) {
		return std::string("Cannot find compound '" + s1 + "' in the loaded materials database.");	}
	inline std::string	Flow_ErrWrongCompoundParam(const std::string& s1, const std::string& s2) {
		return std::string("Cannot find compound '" + s2 + "', defined in unit parameter '" + s1 + "', in the loaded materials database."); }
	const char* const	Flow_ErrNoPhases = "No phases specified.";
	inline std::string  Flow_ErrUnconnectedPorts(const std::string& s1, const std::string& s2) {
		return "Port '" + s2 + "' in unit '" + s1 + "' is unconnected."; }
	inline std::string	Flow_ErrWrongStreams(const std::string& s1) {
		return std::string("Stream '" + s1 + "' is not correctly connected. Each stream must be connected to one input and to one output port."); }
	inline std::string  Flow_ErrSolverKeyEmpty(const std::string& s1, const std::string& s2) { return std::string("Solver '" + s1 + "' in unit '" + s2 + "' has not been chosen in unit parameters."); }
	inline std::string  Flow_ErrCannotLoadSolverLib(const std::string& s1, const std::string& s2) { return std::string("Solver '" + s1 + "' in unit '" + s2 + "' can not be found or loaded."); }


//////////////////////////////////////////////////////////////////////////
/// CCalculationSequence
//////////////////////////////////////////////////////////////////////////

	const char* const Seq_H5AttrPartitionsNum  = "PartitionsNumber";
	const char* const Seq_H5GroupPartitionName = "Partition";
	const char* const Seq_H5ModelsKeys         = "ModelsKeys";
	const char* const Seq_H5TearStreamsKeys    = "TearStreamsKeys";
	const char* const Seq_H5AttrSaveVersion    = "SaveVersion";
	const char* const Seq_H5GroupInitTears     = "InitialTearStreams";
	const char* const Seq_H5GroupInitTearName  = "Stream";

	const char* const  Seq_ErrEmptySequence = "Calculation sequence is empty.";
	const char* const  Seq_ErrEmptyModel    = "Some model in calculation sequence is empty.";
	const char* const  Seq_ErrEmptyStream   = "Some tear stream in calculation sequence is empty.";
	inline std::string Seq_ErrMissingUnit(const std::string& s1) { return std::string("Unit '" + s1 + "' is not in calculation sequence."); }


//////////////////////////////////////////////////////////////////////////
/// CParametersHolder
//////////////////////////////////////////////////////////////////////////
	const char* const FlPar_H5ATol		              = "AbsTolerance";
	const char* const FlPar_H5RTol		              = "RelTolerance";
	const char* const FlPar_H5MinFrac	              = "MinimalFraction";
	const char* const FlPar_H5SimTime                 = "SimulationTime";
	const char* const FlPar_H5InitTimeWin             = "InitialTimeWindow";
	const char* const FlPar_H5MinTimeWin	          = "MinTimeWindow";
	const char* const FlPar_H5MaxTimeWin	          = "MaxTimeWindow";
	const char* const FlPar_H5MaxStepsNum	          = "MaxStepsNum";
	const char* const FlPar_H5UpperLimit	          = "ItersUpperLimit";
	const char* const FlPar_H5LowerLimit	          = "ItersLowerLimit";
	const char* const FlPar_H51stUpperLimit           = "Iters1stUpperLimit";
	const char* const FlPar_H5MagnificRatio           = "TimeWindowRatio";
	const char* const FlPar_H5ConvMethod	          = "ConvergenceMethod";
	const char* const FlPar_H5WegsteinParam           = "WegsteinAccelParam";
	const char* const FlPar_H5RelaxParam	          = "RelaxationParam";
	const char* const FlPar_H5ExtrapMethod	          = "ExtrapolationMethod";
	const char* const FlPar_H5SaveTimeStep	          = "SaveFileStep";
	const char* const FlPar_H5SaveTimeStepFlagHoldups = "SaveTimeStepFlagHoldups";
	const char* const FlPar_H5CacheFlagStreams        = "CacheFlag";
	const char* const FlPar_H5CacheFlagHoldups        = "CacheFlagHoldups";
	const char* const FlPar_H5CacheFlagInternal       = "CacheFlagInternal";
	const char* const FlPar_H5CacheWindow	          = "CacheWindow";
	const char* const FlPar_H5FileSingleFlag	      = "FileSingleFlag";
	const char* const FlPar_H5InitTearStreamsFlag	  = "InitTearStreamsFlag";
	const char* const FlPar_H5EnthalpyMinT            = "EnthalpyMinTemperature";
	const char* const FlPar_H5EnthalpyMaxT            = "EnthalpyMaxTemperature";
	const char* const FlPar_H5EnthalpyIntervals       = "EnthalpyIntervals";
	const char* const FlPar_H5AttrSaveVersion         = "SaveVersion";


//////////////////////////////////////////////////////////////////////////
/// CMDMatrix
//////////////////////////////////////////////////////////////////////////
	const char* const MDM_H5Dimensions		= "Dimensions";
	const char* const MDM_H5Classes			= "Classes";
	const char* const MDM_H5TimePoints		= "TimePoints";
	const char* const MDM_H5Data			= "Data";
	const char* const MDM_H5AttrBlocksNum	= "BlocksNumber";
	const char* const MDM_H5AttrSaveVersion	= "SaveVersion";


//////////////////////////////////////////////////////////////////////////
/// CStream
//////////////////////////////////////////////////////////////////////////
	const char* const Stream_UnspecStreamName	= "Unspecified";
	const char* const Stream_FileExt			= ".dat";
	const char* const Stream_FileNameStream		= "stream";
	const char* const Stream_FileNameMTP		= "mtp";
	const char* const Stream_FileNamePhaseFrac	= "phs";
	const char* const Stream_FileNamePhaseDistr	= "phase_distribution";

	const char* const Stream_H5StreamName		= "StreamName";
	const char* const Stream_H5StreamKey		= "StreamKey";
	const char* const Stream_H5TimePoints		= "TimePoints";
	const char* const Stream_H5AttrPhasesNum	= "PhasesNumber";
	const char* const Stream_H5GroupPhases		= "Phases";
	const char* const Stream_H5GroupPhaseName	= "Phase";
	const char* const Stream_H5Attr2DDistrNum	= "2DDistrNumber";
	const char* const Stream_H5Group2DDistrs	= "Distributions";
	const char* const Stream_H5Group2DDistrName	= "Distribution";
	const char* const Stream_H5OverallKeys      = "OverallKeys";
	const char* const Stream_H5GroupOveralls    = "Overalls";
	const char* const Stream_H5GroupOverallName = "Overall";
	const char* const Stream_H5PhaseKeys        = "PhaseKeys";
	const char* const Stream_H5PhaseName		= "PhaseName";
	const char* const Stream_H5PhaseSOA			= "PhaseSOA";
	const char* const Stream_H5AttrSaveVersion	= "SaveVersion";

//////////////////////////////////////////////////////////////////////////
/// CTimeDependentValue
//////////////////////////////////////////////////////////////////////////
	const char* const TDV_H5Name  = "Name";
	const char* const TDV_H5Units = "Units";
	const char* const TDV_H5Data  = "Data";

//////////////////////////////////////////////////////////////////////////
/// CPhase
//////////////////////////////////////////////////////////////////////////
	const char* const Phase_H5Name         = "Name";
	const char* const Phase_H5State        = "State";
	const char* const Phase_H5Fractions    = "Fractions";
	const char* const Phase_H5Distribution = "Distribution";

//////////////////////////////////////////////////////////////////////////
/// CChemicalReaction
//////////////////////////////////////////////////////////////////////////
	const char* const Reac_H5Name             = "Name";
	const char* const Reac_H5Base             = "BaseSubstance";
	const char* const Reac_H5SubstancesNumber = "SubstancesNumber";
	const char* const Reac_H5Substance        = "Substance";
	const char* const Reac_H5SubstanceKey     = "Key";
	const char* const Reac_H5SubstanceNu      = "Nu";
	const char* const Reac_H5SubstanceOrder   = "Order";
	const char* const Reac_H5SubstancePhase   = "Phase";

//////////////////////////////////////////////////////////////////////////
/// HDF5 saving/loading
//////////////////////////////////////////////////////////////////////////
	const char* const H5AttrSaveVersion = "SaveVersion";
	const char* const H5GroupDistrGrid  = "DistrGrid";

//////////////////////////////////////////////////////////////////////////
/// CH5Handler
//////////////////////////////////////////////////////////////////////////
	const std::wstring HDF5H_FileExt		   = L"dflw";
	const std::wstring HDF5H_DotFileExt		   = L"." + HDF5H_FileExt;
	const std::wstring HDF5H_FileExtSpec	   = L"%d";
	const std::wstring HDF5H_FileExtInitRegex  = LR"(\[\[([0-9]+)\]\])";
	const std::wstring HDF5H_FileExtFinalRegex = LR"(\[\[(%d)\]\])";
	const std::wstring HDF5H_FileExtMult	   = L"[[" + HDF5H_FileExtSpec + L"]]";
	const std::wstring HDF5H_DotFileExtMult	   = L"." + HDF5H_FileExtMult;


//////////////////////////////////////////////////////////////////////////
/// CFlowsheetEditor
//////////////////////////////////////////////////////////////////////////
	const char* const FE_PortTypeInput     = "Input";
	const char* const FE_PortTypeOutput    = "Output";
	const char* const FE_UnitParamMinVal   = "Min = ";
	const char* const FE_UnitParamMaxVal   = "Max = ";
	const char* const FE_UnitDefaultName   = "Unit";
	const char* const FE_StreamDefaultName = "Stream";
	const char* const FE_TDParamMessage    = "Use the table below to set values";
	const char* const FE_TDRemoveLast      = "At least one time point must remain";


//////////////////////////////////////////////////////////////////////////
/// CGridEditor
//////////////////////////////////////////////////////////////////////////
	const char* const GE_GridTypeManual		    = "Manual";
	const char* const GE_GridTypeEquidistant    = "Equidistant";
	const char* const GE_GridTypeGeometricS2L	= "Geometric inc";
	const char* const GE_GridTypeLogarithmicS2L = "Logarithmic inc";
	const char* const GE_GridTypeGeometricL2S	= "Geometric dec";
	const char* const GE_GridTypeLogarithmicL2S = "Logarithmic dec";
	const char* const GE_GridEntryNumeric       = "Numeric";
	const char* const GE_GridEntrySymbolic	    = "Symbolic";
	const char* const GE_ErrorDuplicates        = "Unable to apply: duplicated distributions.";
	const char* const GE_ErrorUndefined         = "Unable to apply: undefined distribution.";
	inline std::string GE_ErrorEmpty(const std::string& s) {
		return std::string("Unable to apply: distribution grid for '" + s + "' is empty."); }
	inline std::string GE_ErrorNegative(const std::string& s) {
		return std::string("Unable to apply: distribution grid for '" + s + "' contains negative values."); }
	inline std::string GE_ErrorSequence(const std::string& s) {
		return std::string("Unable to apply: distribution grid for '" + s + "' does not form an ascending sequence."); }
	inline std::string GE_ErrorGaps(const std::string& s) {
		return std::string("Unable to apply: distribution grid for '" + s + "' contains empty values."); }

//////////////////////////////////////////////////////////////////////////
/// CModelsManager
//////////////////////////////////////////////////////////////////////////
	const char* const MM_ConfigModelsParamName	     = "modelsFolders";
	const char* const MM_ConfigModelsFlagsParamName  = "modelsFoldersActivity";
#ifdef _MSC_VER
	const wchar_t* const MM_LibraryFileExtension     = L".dll";
#else
	const wchar_t* const MM_LibraryFileExtension     = L".so";
#endif


//////////////////////////////////////////////////////////////////////////
/// CSimulatorTab
//////////////////////////////////////////////////////////////////////////
	const char* const  ST_ButtonRunTextRun       = "Start simulation";
	const char* const  ST_ButtonRunTextStop      = "Stop simulation";

	const char* const  ST_LogSimStopRequest      = "Simulation stop requested...";
	const char* const  ST_LogSimUserStop         = "Simulation was stopped by user!";
	inline std::string ST_LogSimStart(const std::string& s1, const std::string& s2) {
		return std::string("Simulation started at " + s1 + " on " + s2); }
	inline std::string ST_LogSimFinishedTime(const std::string& s1, const std::string& s2, const std::string& s3)	{
		return std::string("\nSimulation finished at " + s1 + " on " + s2 + " in " + s3 + " s"); }
	const char* const  ST_LogInitStart           = "Initialization of flowsheet is started";
	const char* const  ST_LogInitFinish          = "Initialization of flowsheet is finished\n";

	const char* const  ST_TitleClearResults      = "Clear simulation results";
	const char* const  ST_QuestionClearResults   = "All simulation results will be removed. Proceed?";
	const char* const  ST_TitleClearRecycles     = "Clear recycle streams";
	const char* const  ST_QuestionClearRecycles  = "Initial values of all recycle streams will be removed. Proceed?";
	const char* const  ST_TitleClearAll          = "Clear all";
	const char* const  ST_QuestionClearAll       = "All simulation results and initial values of all recycle streams will be removed. Proceed?";

//////////////////////////////////////////////////////////////////////////
/// CBasicStreamsViewer
//////////////////////////////////////////////////////////////////////////
	const char* const BSV_TabNameTable          = "Table view";
	const char* const BSV_TabNamePlot           = "Plot view";
	const char* const BSV_PropMass              = "Mass";
	const char* const BSV_PropMassFlow          = "Mass flow";
	const char* const BSV_PropTemperature       = "Temperature";
	const char* const BSV_PropPressure          = "Pressure";
	const char* const BSV_PropPhaseFractions    = "Phase fractions";
	const char* const BSV_PropSolidDistribution = "Solid distribution";
	const char* const BSV_PropSauterDiameter    = "Sauter diameter";
	const char* const BSV_ComboBoxNoDimension	= " ";
	const char* const BSV_ComboBoxAllCompounds  = "All compounds";
	const char* const BSV_LabelDistribution     = "Distribution:";
	const char* const BSV_LabelRows             = "        Rows:";
	const char* const BSV_TableHeaderTime       = "Time [s]";
	const char* const BSV_TableHeaderNumber     = "Number";
	const char* const BSV_TableHeaderq3         = "q3";
	const char* const BSV_TableHeaderQ3         = "Q3";
	const char* const BSV_TableHeaderq2         = "q2";
	const char* const BSV_TableHeaderQ2         = "Q2";
	const char* const BSV_TableHeaderq0         = "q0";
	const char* const BSV_TableHeaderQ0         = "Q0";
	const char* const BSV_TableHeaderMassFrac   = "Mass fraction";
	const char* const BSV_TableHeaderSauter     = "Sauter diameter [m]";
	const char* const BSV_HeaderDiameter		= "Diameter";
	const char* const BSV_HeaderVolume			= "Volume";
	const char* const BSV_HeaderPartPorosity    = "Particle porosity [-]";
	const char* const BSV_HeaderFormFactor		= "Form factor [-]";

//////////////////////////////////////////////////////////////////////////
/// CBasicStreamsEditor
//////////////////////////////////////////////////////////////////////////
	const char* const  SDE_CompoundsMixture = "Total mixture";
	const char* const  SDE_TotalMixture     = "Total mixture";

	const char* const  FUN_EmptyUnits	    = "[-]";
	const char* const  FUN_DiameterUnits    = "[m]";
	const char* const  FUN_VolumeUnits	    = "[m3]";
	const char* const  FUN_DiameterUnitsInv = "[1/m]";
	const char* const  FUN_VolumeUnitsInv   = "[1/m3]";
	const char* const  FUN_NormalName       = "Normal";
	const char* const  FUN_RRSBName         = "RRSB";
	const char* const  FUN_GGSName          = "GGS";
	const char* const  FUN_LogNormalName    = "LogNormal";
	const char* const  FUN_NormalParam1	    = "D50";
	const char* const  FUN_NormalParam2	    = "Standard deviation";
	const char* const  FUN_RRSBParam1	    = "D63";
	const char* const  FUN_RRSBParam2	    = "Dispersion";
	const char* const  FUN_GGSParam1	    = "Dmax";
	const char* const  FUN_GGSParam2	    = "Dispersion";
	const char* const  FUN_LogNormalParam1  = "D50";
	const char* const  FUN_LogNormalParam2  = "Geometric mean deviation";
	const char* const  FUN_UndefinedParam   = "Undefined";
	const char* const  FUN_ErrorName        = "Wrong parameters";
	inline std::string FUN_ErrorZeroParameter(const std::string& s1) {
		return std::string(s1 + " can not be equal to zero!"); }

//////////////////////////////////////////////////////////////////////////
/// CTearStreamsEditor
//////////////////////////////////////////////////////////////////////////
	const char* const TSE_PartitionName     = "Partition";
	const char* const TSE_WindowName        = "Tear Streams Editor";
	const char* const TSE_InfoWrongMode     = "You cannot change initial data in Auto mode.";
	const char* const TSE_QuestionRemoveAll = "Do you really want to remove all initial data from all tear streams?";

//////////////////////////////////////////////////////////////////////////
/// CMaterialsDatabaseTab
//////////////////////////////////////////////////////////////////////////
	const char* const  MDT_DialogSaveName      = "Save database";
	const char* const  MDT_DialogLoadName      = "Load database";
	const char* const  MDT_DialogDMDBFilter    = "Materials database (*.dmdb);;All files (*.*);;";
	const char* const  MDT_RemoveCompoundTitle = "Remove compound";
	inline std::string MDT_RemoveCompoundConfirm(const std::string& s1) { return std::string("Do you really want to remove " + s1 + "?"); }
	const char* const  MDT_RemoveCorrelationsTitle = "Remove correlations";
	inline std::string MDT_RemoveCorrelationsConfirm(const std::string& s1) { return std::string("Do you really want to remove all correlations of " + s1 + "?"); }
	const char* const  MDT_WindowTitle = "Compounds editor";

//////////////////////////////////////////////////////////////////////////
/// CModulesManagerTab
//////////////////////////////////////////////////////////////////////////
	const char* const MMT_Dynamic     = " Dynamic Unit";
	const char* const MMT_SteadyState = " Steady-state Unit";
	const char* const MMT_Solver      = " Solver";

//////////////////////////////////////////////////////////////////////////
/// CCalculationSequenceEditor
//////////////////////////////////////////////////////////////////////////
	const char* const CSE_Partition = "Partition ";
	const char* const CSE_Models    = "Models";
	const char* const CSE_Streams   = "Tear streams";
}

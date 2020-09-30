/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitParameters.h"
#include "AgglomerationSolver.h"
#include "PBMSolver.h"
#include "PlotManager.h"
#include "UnitPorts.h"
#include "StateVariable.h"
#include "StreamManager.h"

// TODO: properly set solvers to solver parameters from BaseModel (UnitContainer).

class CStream;

//Basic class for dynamic and steady-state units.
class CBaseUnit2
{
	static const unsigned m_saveVersion{ 3 }; // Current version of the saving procedure.

protected:
	////////////////////////////////////////////////////////////////////////////////
	// Basic unit information
	//

	std::string m_unitName;		// Name of the unit.
	std::string m_uniqueID;		// Unique identifier of the unit.
	std::string m_authorName;	// Name of the unit's author.
	size_t m_version{ 0 };		// Version of the unit.

private:
	////////////////////////////////////////////////////////////////////////////////
	// References to flowsheet structural data and settings
	//

	const CMaterialsDatabase& m_materialsDB;			// Reference to a database of materials.
	const CDistributionsGrid& m_grid;					// Reference to a distribution grid.
	const std::vector<std::string>& m_compounds;		// Reference to compounds.
	const std::vector<SOverallDescriptor>& m_overall;	// Reference to overall properties.
	const std::vector<SPhaseDescriptor>& m_phases;		// Reference to phases.
	const SCacheSettings& m_cache;						// Reference to cache settings.
	double& m_minFraction;								// Reference to minimal fraction.
	double& m_toleranceAbs;								// Reference to absolute tolerance.
	double& m_toleranceRel;								// Reference to relative tolerance.

	////////////////////////////////////////////////////////////////////////////////
	// Structural unit data
	//

	CPortsManager m_ports;																							// Ports of the unit.
	CUnitParametersManager m_unitParameters;																		// Handler of unit parameters.
	CStateVariablesManager m_stateVariables;																		// State variables of the unit.
	CStreamManager m_streams{ m_materialsDB, m_grid, m_compounds, m_overall, m_phases, m_cache, m_minFraction };	// Feeds, holdups and internal streams.
	CPlotManager m_plots;																							// Plots.

	CUnitLookupTables m_lookupTables{ &m_materialsDB, &m_compounds };												// Lookup tables to calculate TP-dependent properties.

	////////////////////////////////////////////////////////////////////////////////
	// Generated messages
	//

	bool m_hasError{ false };		// Contains true if an error was detected.
	bool m_hasWarning{ false };		// Contains true if a warning was detected.
	bool m_hasInfo{ false };		// Contains true if an info tag was detected.
	std::string m_errorMessage;		// Description of the last detected error.
	std::string m_warningMessage;	// Description of the last detected warning.
	std::string m_infoMessage;		// Description of the last info.

public:
	CBaseUnit2(const CMaterialsDatabase& _materialsDB, const CDistributionsGrid& _grid, const std::vector<std::string>& _compounds, const std::vector<SOverallDescriptor>& _overall,
		const std::vector<SPhaseDescriptor>& _phases, const SCacheSettings& _cache, double& _minFraction, double& _toleranceAbs, double& _toleranceRel);
	CBaseUnit2(const CBaseUnit2& _other)             = delete;
	CBaseUnit2(CBaseUnit2 && _other)                 = delete;
	CBaseUnit2& operator=(const CBaseUnit2 & _other) = delete;
	CBaseUnit2& operator=(CBaseUnit2 && _other)      = delete;
	virtual ~CBaseUnit2()                            = default;

	////////////////////////////////////////////////////////////////////////////////
	// Basic unit information
	//

	// Returns the name of the unit.
	std::string UnitName() const;
	// Returns the name of unit's author.
	std::string AuthorName() const;
	// Returns the version of the unit.
	size_t Version() const;
	// Returns the unique identifier of the unit.
	std::string UniqueID() const;

	// Returns the name of the unit.
	void SetUnitName(const std::string& _name);
	// Returns the name of unit's author.
	void SetAuthorName(const std::string& _author);
	// Returns the version of the unit.
	void SetVersion(size_t _version);
	// Returns the unique identifier of the unit.
	void SetUniqueID(const std::string& _id);

	////////////////////////////////////////////////////////////////////////////////
	// Ports
	//

	// Adds a port to the unit and returns a pointer to it. If the unit already has a port with the same name, a logic_error exception is thrown.
	CUnitPort* AddPort(const std::string& _portName, CUnitPort::EPortType2 _type);
	// Returns a const reference to the specified port of the unit.
	const CUnitPort* GetPort(const std::string& _portName) const;
	// Returns a reference to the specified port of the unit.
	CUnitPort* GetPort(const std::string& _portName);
	// Returns a pointer to the stream connected to this port. If such port does not exist or empty, a logic_error exception is thrown.
	CStream* GetPortStream(const std::string& _portName) const;

	////////////////////////////////////////////////////////////////////////////////
	// Internal feeds, holdups and streams
	//

	// Returns a const reference to streams manager.
	const CStreamManager& GetStreamsManager() const;
	// Returns a reference to streams manager.
	CStreamManager& GetStreamsManager();

	// Adds a new feed to the unit with proper structure (MD dimensions, phases, materials, etc.), and returns a pointer to it. If a feed with the given name already exists, a logic_error exception is thrown.
	CStream* AddFeed(const std::string& _name);
	// Returns a feed with the specified name. If such feed does not exist, a logic_error exception is thrown.
	CStream* GetFeed(const std::string& _name);
	// Removes the feed with the specified name from the unit. If such feed does not exist, a logic_error exception is thrown.
	void RemoveFeed(const std::string& _name);

	// Adds a new holdup to the unit with proper structure (MD dimensions, phases, materials, etc.), and returns a pointer to it. If a holdup with the given name already exists, a logic_error exception is thrown.
	CHoldup* AddHoldup(const std::string& _name);
	// Returns a holdup with the specified name. If such holdup does not exist, a logic_error exception is thrown.
	CHoldup* GetHoldup(const std::string& _name);
	// Removes the holdup with the specified name from the unit. If such holdup does not exist, a logic_error exception is thrown.
	void RemoveHoldup(const std::string& _name);

	// Adds a new stream to the unit with proper structure (MD dimensions, phases, materials, etc.), and returns a pointer to it. If a stream with the given name already exists, a logic_error exception is thrown.
	CStream* AddStream(const std::string& _name);
	// Returns a stream with the specified name. If such stream does not exist, a logic_error exception is thrown.
	CStream* GetStream(const std::string& _name);
	// Removes the stream with the specified name from the unit. If such stream does not exist, a logic_error exception is thrown.
	void RemoveStream(const std::string& _name);

	// Sets up the stream structure (MD dimensions, phases, materials, etc.) the same as it is configured in the flowsheet. Removes all existing data.
	void SetupStream(CBaseStream* _stream) const;

	////////////////////////////////////////////////////////////////////////////////
	// Unit parameters
	//

	// Returns a const reference to unit parameters manager.
	const CUnitParametersManager& GetUnitParametersManager() const;
	// Returns a reference to unit parameters manager.
	CUnitParametersManager& GetUnitParametersManager();

	// Adds a new real constant unit parameter and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	CConstRealUnitParameter* AddConstRealParameter(const std::string& _name, double _initValue, const std::string& _units, const std::string& _description, double _minValue = std::numeric_limits<double>::lowest(), double _maxValue = std::numeric_limits<double>::max());
	// Adds a new signed integer constant unit parameter and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	CConstIntUnitParameter* AddConstIntParameter(const std::string& _name, int64_t _initValue, const std::string& _units, const std::string& _description, int64_t _minValue = std::numeric_limits<int64_t>::lowest(), int64_t _maxValue = std::numeric_limits<int64_t>::max());
	// Adds a new unsigned integer constant unit parameter and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	CConstUIntUnitParameter* AddConstUIntParameter(const std::string& _name, uint64_t _initValue, const std::string& _units, const std::string& _description, uint64_t _minValue = std::numeric_limits<uint64_t>::lowest(), uint64_t _maxValue = std::numeric_limits<uint64_t>::max());
	// Adds a new time-dependent unit parameter and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	CTDUnitParameter* AddTDParameter(const std::string& _name, double _initValue, const std::string& _units, const std::string& _description, double _minValue = std::numeric_limits<double>::lowest(), double _maxValue = std::numeric_limits<double>::max());
	// Adds a new string unit parameter and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	CStringUnitParameter* AddStringParameter(const std::string& _name, const std::string& _initValue, const std::string& _description);
	// Adds a new two-positional check box unit parameter and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	CCheckBoxUnitParameter* AddCheckBoxParameter(const std::string& _name, bool _initValue, const std::string& _description);
	// Adds a new combo-box unit parameter and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	CComboUnitParameter* AddComboParameter(const std::string& _name, size_t _initValue, const std::vector<size_t>& _items, const std::vector<std::string>& _itemsNames, const std::string& _description);
	// Adds a new compound combo-box unit parameter, allowing to choose one of the defined compounds, and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	CCompoundUnitParameter* AddCompoundParameter(const std::string& _name, const std::string& _description);
	// Adds a new solver unit parameter with type SOLVER_AGGLOMERATION_1, allowing to choose one of the available solvers of this type, and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	CAgglomerationSolver* AddSolverAgglomeration(const std::string& _name, const std::string& _description);
	// Adds a new solver unit parameter with type SOLVER_PBM_1, allowing to choose one of the available solvers of this type, and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	CPBMSolver* AddSolverPBM(const std::string& _name, const std::string& _description);

	// Groups the specified unit parameters and associates them with the given value of the selected unit parameter. The parameter, its value and all the adding parameters must already exist. If something does not exist, logic_error exception is thrown.
	void AddParametersToGroup(const std::string& _unitParamName, const std::string& _unitParamValueName, const std::vector<std::string>& _groupedParamNames);

	// Returns value of the real constant unit parameter. Throws logic_error exception if a real constant unit parameter with the given name does not exist.
	double GetConstRealParameterValue(const std::string& _name) const;
	// Returns value of the signed integer constant unit parameter. Throws logic_error exception if a signed integer constant unit parameter with the given name does not exist.
	int64_t GetConstIntParameterValue(const std::string& _name) const;
	// Returns value of the unsigned integer constant unit parameter. Throws logic_error exception if an unsigned integer constant unit parameter with the given name does not exist.
	uint64_t GetConstUIntParameterValue(const std::string& _name) const;
	// Returns value of the time-dependent unit parameter at the specified time point. Throws logic_error exception if a time-dependent unit parameter with the given name does not exist.
	double GetTDParameterValue(const std::string& _name, double _time) const;
	// Returns value of the string unit parameter. Throws logic_error exception if a string unit parameter with the given name does not exist.
	std::string GetStringParameterValue(const std::string& _name) const;
	// Returns value of the check box unit parameter. Throws logic_error exception if a check box unit parameter with the given name does not exist.
	bool GetCheckboxParameterValue(const std::string& _name) const;
	// Returns value of the combo box unit parameter. Throws logic_error exception if combo box unit parameter with the given name does not exist.
	size_t GetComboParameterValue(const std::string& _name) const;
	// Returns value of the compound unit parameter. Throws logic_error exception if a compound unit parameter with the given name does not exist.
	std::string GetCompoundParameterValue(const std::string& _name) const;
	// Returns a pointer to an agglomeration solver of a type SOLVER_AGGLOMERATION_1 with the specified name. Throws logic_error exception if a solver unit parameter with the given name does not exist.
	CAgglomerationSolver* GetSolverAgglomeration(const std::string& _name) const;
	// Returns a pointer to a PBM solver of a type SOLVER_PBM_1 with the specified name. Throws logic_error exception if a solver unit parameter with the given name does not exist.
	CPBMSolver* GetSolverPBM(const std::string& _name) const;

	////////////////////////////////////////////////////////////////////////////////
	// State variables
	//

	// Adds a new time-dependent state variable to the unit and returns a pointer to it. If the unit already has a state variable with the same name, logic_error exception is thrown.
	CStateVariable* AddStateVariable(const std::string& _name, double _initValue);
	// Returns current value of a state variable. If a state variable with the given name does not exist in this unit, logic_error exception is thrown.
	double GetStateVariable(const std::string& _name) const;
	// Sets new value of a state variable. If a state variable with the given name does not exist in this unit, logic_error exception is thrown.
	void SetStateVariable(const std::string& _name, double _value);
	// Sets new value of a state variable and adds its value to the history of time-dependent changes. If a state variable with the given name does not exist in this unit, logic_error exception is thrown.
	void SetStateVariable(const std::string& _name, double _time, double _value);

	////////////////////////////////////////////////////////////////////////////////
	// Plots
	//

	// Returns a const reference to plots manager.
	const CPlotManager& GetPlotsManager() const;
	// Returns a reference to plots manager.
	CPlotManager& GetPlotsManager();

	// Adds a new 2-dimensional plot with the specified name and axes labels to the units and returns a pointer to it. If the unit already has a plot with the same name, logic_error exception is thrown.
	CPlot* AddPlot(const std::string& _plotName, const std::string& _labelX, const std::string& _labelY);
	// Adds a new 3-dimensional plot with the specified name and axes labels to the units and returns a pointer to it. If the unit already has a plot with the same name, logic_error exception is thrown.
	CPlot* AddPlot(const std::string& _plotName, const std::string& _labelX, const std::string& _labelY, const std::string& _labelZ);
	// Adds a new curve with the specified name on the 2-dimensional plot with the given name and returns a pointer to it. If a plot with this name does not exist or it already contains a curve with the given name, logic_error exception is thrown.
	CCurve* AddCurveOnPlot(const std::string& _plotName, const std::string& _curveName);
	// Adds a new curve with the specified Z-value on the 3-dimensional plot with the given name and returns a pointer to it. If a plot with this name does not exist or it already contains a curve with the given Z-value, logic_error exception is thrown.
	CCurve* AddCurveOnPlot(const std::string& _plotName, double _valueZ);
	// Adds a new point to the specified curve of the 2-dimensional plot. If a plot or a curve with these names do not exist, logic_error exception is thrown.
	void AddPointOnCurve(const std::string& _plotName, const std::string& _curveName, double _x, double _y);
	// Adds a new point to the specified curve of the 3-dimensional plot. If a plot with this name or a curve with this Z-value do not exist, logic_error exception is thrown.
	void AddPointOnCurve(const std::string& _plotName, double _valueZ, double _x, double _y);
	// Adds new points to the specified curve of the 2-dimensional plot. If a plot or a curve with these names do not exist, logic_error exception is thrown.
	void AddPointsOnCurve(const std::string& _plotName, const std::string& _curveName, const std::vector<double>& _x, const std::vector<double>& _y);
	// Adds new points to the specified curve of the 3-dimensional plot. If a plot with this name or a curve with this Z-value do not exist, logic_error exception is thrown.
	void AddPointsOnCurve(const std::string& _plotName, double _valueZ, const std::vector<double>& _x, const std::vector<double>& _y);

	////////////////////////////////////////////////////////////////////////////////
	// Stream manipulations
	//

	// Copies all stream data from one stream to another at the given time point. All data after the time point are removed from the destination stream.
	static void CopyStreamToStream(double _time, const CStream* _srcStream, CStream* _dstStream);
	// Copies all stream data from one stream to another at the given time interval. All data after the end time point are removed from the destination stream.
	static void CopyStreamToStream(double _timeBeg, double _timeEnd, const CStream* _srcStream, CStream* _dstStream);
	// Copies all stream data from the given stream to the stream connected to the selected output port at the given time point. All data after the time point are removed from the destination stream. Throws logic_error exception on error.
	void CopyStreamToPort(double _time, const CStream* _stream, CUnitPort* _port) const;
	// Copies all stream data from the given stream to the stream connected to the selected output port at the given time point. All data after the time point are removed from the destination stream. Throws logic_error exception on error.
	void CopyStreamToPort(double _time, const CStream* _stream, const std::string& _portName);
	// Copies all stream data from the given stream to the stream connected to the selected output port at the given time interval. All data after the end time point are removed from the destination stream. Throws logic_error exception on error.
	void CopyStreamToPort(double _timeBeg, double _timeEnd, const CStream* _stream, CUnitPort* _port) const;
	// Copies all stream data from the given stream to the stream connected to the selected output port at the given time interval. All data after the end time point are removed from the destination stream. Throws logic_error exception on error.
	void CopyStreamToPort(double _timeBeg, double _timeEnd, const CStream* _stream, const std::string& _portName);
	// Copies all stream data from the stream connected to the selected input port to the given stream at the given time point. All data after the time point are removed from the destination stream. Throws logic_error exception on error.
	void CopyPortToStream(double _time, const CUnitPort* _port, CStream* _stream) const;
	// Copies all stream data from the stream connected to the selected input port to the given stream at the given time point. All data after the time point are removed from the destination stream. Throws logic_error exception on error.
	void CopyPortToStream(double _time, const std::string& _portName, CStream* _stream);
	// Copies all stream data from the stream connected to the selected input port to the given stream at the given time interval. All data after the end time point are removed from the destination stream. Throws logic_error exception on error.
	void CopyPortToStream(double _timeBeg, double _timeEnd, const CUnitPort* _port, CStream* _stream) const;
	// Copies all stream data from the stream connected to the selected input port to the given stream at the given time interval. All data after the end time point are removed from the destination stream. Throws logic_error exception on error.
	void CopyPortToStream(double _timeBeg, double _timeEnd, const std::string& _portName, CStream* _stream);

	////////////////////////////////////////////////////////////////////////////////
	// Time points
	//

	// Returns all time points in the specified time interval at which input streams and time-dependent parameters are defined.
	std::vector<double> GetAllTimePoints(double _timeBeg, double _timeEnd) const;
	// Returns all time points in the specified time interval at which input streams and time-dependent parameters are defined. Boundary time points are unconditionally included into result.
	std::vector<double> GetAllTimePointsClosed(double _timeBeg, double _timeEnd) const;
	// Returns all time points in the specified time interval at which input streams are defined.
	std::vector<double> GetInputTimePoints(double _timeBeg, double _timeEnd) const;
	// Returns all time points in the specified time interval at which input streams are defined. Boundary time points are unconditionally included into result.
	std::vector<double> GetInputTimePointsClosed(double _timeBeg, double _timeEnd) const;
	// Returns all time points in the specified time interval at which given streams are defined.
	std::vector<double> GetStreamsTimePoints(double _timeBeg, double _timeEnd, const std::vector<CStream*>& _srteams) const;
	// Returns all time points in the specified time interval at which given streams are defined. Boundary time points are unconditionally included into result.
	std::vector<double> GetStreamsTimePointsClosed(double _timeBeg, double _timeEnd, const std::vector<CStream*>& _srteams) const;

	// Removes time points within the specified interval [timeBeg; timeEnd) that are closer together than step.
	void ReduceTimePoints(double _timeBeg, double _timeEnd, double _step);

	////////////////////////////////////////////////////////////////////////////////
	// Compounds
	//

	// Adds a compound with the specified unique key to all feeds, holdups and streams in the unit.
	void AddCompound(const std::string& _compoundKey);
	// Removes a compound with the specified unique key from all feeds, holdups and streams in the unit.
	void RemoveCompound(const std::string& _compoundKey);
	// Returns the name of the compound with the specified unique key.
	std::string GetCompoundName(const std::string& _compoundKey) const;
	// Returns the unique key of the compound with the specified name.
	std::string GetCompoundKey(const std::string& _compoundName) const;
	// Returns unique keys of all defined materials.
	std::vector<std::string> GetAllCompounds() const;
	// Returns names of all defined materials.
	std::vector<std::string> GetAllCompoundsNames() const;
	// Returns the number of defined compounds.
	size_t GetCompoundsNumber() const;
	// Checks if a compound with the specified unique key is defined.
	bool IsCompoundDefined(const std::string& _compoundKey) const;
	// Checks if a compound with the specified name is defined.
	bool IsCompoundNameDefined(const std::string& _compoundName) const;

	////////////////////////////////////////////////////////////////////////////////
	// Phases
	//

	// Adds the specified phase to all feeds, holdups and streams in the unit, if it does not exist yet.
	void AddPhase(EPhase _phase, const std::string& _name);
	// Removes the specified phase from all feeds, holdups and streams in the unit.
	void RemovePhase(EPhase _phase);
	// Returns the name of the specified phase.
	std::string GetPhaseName(EPhase _phase) const;
	// Returns the number of defined phases.
	size_t GetPhasesNumber() const;
	// Checks if a specified phase is defined.
	bool IsPhaseDefined(EPhase _phase) const;

	////////////////////////////////////////////////////////////////////////////////
	// Distributed properties
	//

	// Updates grids of distributed parameters.
	void UpdateDistributionsGrid();

	// Returns the number of defined distributed parameters.
	size_t GetDistributionsNumber() const;
	// Returns all defined distributed parameters.
	std::vector<EDistrTypes> GetDistributionsTypes() const;
	// Returns numbers of classes of all defined distributed parameters.
	std::vector<size_t> GetDistributionsClasses() const;
	// Returns grid type (GRID_NUMERIC, GRID_SYMBOLIC) of the given distributed parameter. Returns GRID_UNDEFINED if the given distributed parameter is not defined.
	EGridEntry GetDistributionGridType(EDistrTypes _distribution) const;

	// Returns the number of classes defined for the specified distributed parameter.
	size_t GetClassesNumber(EDistrTypes _distribution) const;
	// Returns a continuous or discrete numeric grid defined for the specified distributed parameter.
	std::vector<double> GetNumericGrid(EDistrTypes _distribution) const;
	// Returns a symbolic discrete grid defined for the specified distributed parameter.
	std::vector<std::string> GetSymbolicGrid(EDistrTypes _distribution) const;
	// Returns the sizes of classes defined in the grid of the specified distributed parameter.
	std::vector<double> GetClassesSizes(EDistrTypes _distribution) const;
	// Returns the mean values of classes defined in the grid of the specified distributed parameter.
	std::vector<double> GetClassesMeans(EDistrTypes _distribution) const;

	// Returns a grid defined for the particle size distribution in terms of particle diameters.
	std::vector<double> GetPSDGridDiameters() const;
	// Returns a grid defined for the particle size distribution in terms of particle surfaces.
	std::vector<double> GetPSDGridSurfaces() const;
	// Returns a grid defined for the particle size distribution in terms of particle volumes.
	std::vector<double> GetPSDGridVolumes() const;
	// Returns the mean values of classes defined in the grid of the particle size distribution in terms of particle diameters.
	std::vector<double> GetPSDMeanDiameters() const;
	// Returns the mean values of classes defined in the grid of the particle size distribution in terms of particle surfaces.
	std::vector<double> GetPSDMeanSurfaces() const;
	// Returns the mean values of classes defined in the grid of the particle size distribution in terms of particle volumes.
	std::vector<double> GetPSDMeanVolumes() const;

	// Checks if the specified distributed parameter is defined.
	bool IsDistributionDefined(EDistrTypes _distribution) const;

	// TODO: return by value.
	// Calculates a transformation matrix needed to obtain the output distribution from the input one.
	void CalculateTM(EDistrTypes _distribution, const std::vector<double>& _inValue, const std::vector<double>& _outValue, CTransformMatrix& _matrix) const;

	////////////////////////////////////////////////////////////////////////////////
	// Global flowsheet data and settings
	//

	// Returns a pointer to the current materials database.
	const CMaterialsDatabase* GetMaterialsDatabase() const;

	// Returns a pointer to the current grid od distributed parameters.
	const CDistributionsGrid* GetGrid() const;

	// Returns the global absolute tolerance.
	double GetAbsTolerance() const;
	// Returns the global relative tolerance.
	double GetRelTolerance() const;

	// Updates minimum fraction to be considered in MDMatrix of all streams.
	void UpdateMinimumFraction();

	// Updates cache settings in all streams.
	void UpdateCacheSettings();

	////////////////////////////////////////////////////////////////////////////////
	// Materials database
	//

	// TODO: rename, make the same
	// Returns the value of the constant physical property (CRITICAL_TEMPERATURE, MOLAR_MASS, etc) of the specified compound.
	double GetCompoundConstProperty(const std::string& _compoundKey, ECompoundConstProperties _property) const;
	// Returns the value of the temperature/pressure-dependent physical property (DENSITY, ENTHALPY, etc) of the specified compound with the given temperature [K] and pressure [Pa].
	double GetCompoundTPProperty(const std::string& _compoundKey, ECompoundTPProperties _property, double _temperature = STANDARD_CONDITION_T, double _pressure = STANDARD_CONDITION_P) const;
	// Returns the value of the interaction physical property (INTERFACE_TENSION, etc) between the specified compounds with the given specified temperature [K] and pressure [Pa].
	double GetCompoundInteractionProperty(const std::string& _compoundKey1, const std::string& _compoundKey2, EInteractionProperties _property, double _temperature = STANDARD_CONDITION_T, double _pressure = STANDARD_CONDITION_P) const;

	// Checks if a constant physical property with the specified key is present in the materials database.
	bool IsPropertyDefined(ECompoundConstProperties _property) const;
	// Checks if a temperature/pressure-dependent physical property with the specified key is present in the materials database.
	bool IsPropertyDefined(ECompoundTPProperties _property) const;
	// Checks if an interaction physical property with the specified key is present in the materials database.
	bool IsPropertyDefined(EInteractionProperties _property) const;

	////////////////////////////////////////////////////////////////////////////////
	// Thermodynamics
	//

	// Returns a pointer to lookup tables.
	CUnitLookupTables* GetLookupTables();

	// Performs a heat exchange between two streams at the specified time point with a specified efficiency (0..1].
	void HeatExchange(double _time, CStream* _stream1, CStream* _stream2, double _efficiency) const;

	////////////////////////////////////////////////////////////////////////////////
	// Errors and warnings
	//

	// Sets an error state of the unit, prints the message to the simulation log, and requests to stop simulation.
	void RaiseError(const std::string& _message = "");
	// Sets a warning state of the unit, and prints the message to the simulation log.
	void RaiseWarning(const std::string& _message = "");
	// Prints the message to the simulation log.
	void ShowInfo(const std::string& _message);

	// Checks if an error state is requested.
	bool HasError() const;
	// Checks if a warning state is requested.
	bool HasWarning() const;
	// Checks if an info output is requested.
	bool HasInfo() const;

	// Returns a textual description of the last error.
	std::string GetErrorMessage() const;
	// Returns a textual description of the last warning.
	std::string GetWarningMessage() const;
	// Returns a textual description of the last info.
	std::string GetInfoMessage() const;

	// Clears an error state.
	void ClearError();
	// Clears a warning state.
	void ClearWarning();
	// Clears an info state.
	void ClearInfo();

	////////////////////////////////////////////////////////////////////////////////
	// Simulation-time operations
	//

	// Performs initialization of unit before starting the simulation.
	void InitializeUnit();
	// Performs finalization of unit after the simulation is finished.
	void FinalizeUnit();
	// Stores current state of the unit on specified time interval.
	void SaveStateUnit(double _timeBeg, double _timeEnd);
	// Restores previously saved state of the unit.
	void LoadStateUnit();

	// Removes all data produced during simulation.
	void ClearSimulationResults();

	////////////////////////////////////////////////////////////////////////////////
	// User-overloaded functions
	//

	// Setups the structure of the unit and its basic parameters.
	virtual void Configure() {}
	// Calculates the unit on the given time point.
	virtual void Simulate(double _dTime) {}
	// Calculates the unit on the given time interval.
	virtual void Simulate(double _timeBeg, double _timeEnd) {}
	// Initializes unit for at time point 0 before starting the simulation.
	virtual void Initialize() {}
	// Is called once at the end of the simulation to finalize the unit.
	virtual void Finalize() {}
	// Saves the current state of all time-dependent parameters for a possible restart of the simulation from this time point.
	virtual void SaveState() {}
	// Loads the stored state of all time-dependent parameters before a restart of the simulation from that time point.
	virtual void LoadState() {}

	////////////////////////////////////////////////////////////////////////////////
	// Saving/loading
	//

	// Saves all parameters to HDF5 file.
	void SaveToFile(CH5Handler& _h5File, const std::string& _path);
	// Loads all parameters from HDF5 file.
	void LoadFromFile(CH5Handler& _h5File, const std::string& _path);
	// Loads all parameters from HDF5 file. A compatibility version.
	void LoadFromFile_v2(const CH5Handler& _h5File, const std::string& _path);
	// Loads all parameters from HDF5 file. A compatibility version.
	void LoadFromFile_v1(const CH5Handler& _h5File, const std::string& _path);
};


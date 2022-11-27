/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitParameters.h"
#include "AgglomerationSolver.h"
#include "PBMSolver.h"
#include "PlotManager.h"
#include "UnitPorts.h"
#include "StateVariable.h"
#include "StreamManager.h"
#include <mutex>

#ifdef _DEBUG
#define DYSSOL_CREATE_MODEL_FUN CreateDYSSOLUnitV4_DEBUG
#else
#define DYSSOL_CREATE_MODEL_FUN CreateDYSSOLUnitV4
#endif
#define DYSSOL_CREATE_MODEL_FUN_NAME MACRO_TOSTRING(DYSSOL_CREATE_MODEL_FUN)

// TODO: move it somewhere
////////////////////////////////////////////////////////////////////////////////
/// Deprecated defines for compatibility
#define m_sUnitName		GetNameRef()
#define m_sAuthorName	GetAuthorRef()
#define m_dUnitVersion	GetVersionRef()
#define m_sUniqueID		GetKeyRef()
#define m_pMaterialsDB	GetMaterialsDBRef()

class CStream;

// TODO: rename to CBaseModel and create a type alias to CBaseUnit.

/**
 * \brief Basic class for dynamic and steady-state units.
 * \details Basically, it describes a mathematical Model. A bit misleading name of the class is kept for compatibility reasons.
 */
class CBaseUnit
{
	static const unsigned m_saveVersion{ 4 };	// Current version of the saving procedure.

protected:
	////////////////////////////////////////////////////////////////////////////////
	// Basic unit information
	//

	std::string m_unitName{};	// Name of the unit.
	std::string m_uniqueID{};	// Unique identifier of the unit.
	std::string m_authorName{};	// Name of the unit's author.
	size_t m_version{ 0 };		// Version of the unit.

private:
	////////////////////////////////////////////////////////////////////////////////
	// References to flowsheet structural data and settings
	//

	CMultidimensionalGrid m_grid;	// Defined distribution grid.

	// TODO: gather them in some global structure.
	const CMaterialsDatabase* m_materialsDB{ nullptr };				// Reference to a database of materials.
	const std::vector<SOverallDescriptor>* m_overall{ nullptr };	// Reference to overall properties.
	const std::vector<SPhaseDescriptor>* m_phases{ nullptr };		// Reference to phases.
	const SCacheSettings* m_cache{ nullptr };						// Reference to cache settings.
	const SToleranceSettings* m_tolerances{ nullptr };				// Reference to tolerance settings.
	const SThermodynamicsSettings* m_thermodynamics{ nullptr };		// Reference to thermodynamics settings.

	////////////////////////////////////////////////////////////////////////////////
	// Structural unit data
	//

	CPortsManager m_ports;						// Ports of the unit.
	CUnitParametersManager m_unitParameters;	// Handler of unit parameters.
	CStateVariablesManager m_stateVariables;	// State variables of the unit.
	CStreamManager m_streams;					// Feeds, holdups and internal streams.
	CPlotManager m_plots;						// Plots.

	mutable std::unique_ptr<CMixtureEnthalpyLookup> m_enthalpyCalculator;	// Lookup table to calculate temperature<->enthalpy.

	////////////////////////////////////////////////////////////////////////////////
	// Generated messages
	//

	bool m_hasError{ false };		// Contains true if an error was detected.
	bool m_hasWarning{ false };		// Contains true if a warning was detected.
	bool m_hasInfo{ false };		// Contains true if an info tag was detected.
	std::string m_errorMessage;		// Description of the last detected error.
	std::string m_warningMessage;	// Description of the last detected warning.
	std::string m_infoMessage;		// Description of the last info.

	mutable std::mutex m_messageMutex;		// Mutex for thread-safe work with messages.

public:
	// TODO: initialize all pointers in constructor and make them references.
	CBaseUnit()                                    = default;
	CBaseUnit(const CBaseUnit& _other)             = delete;
	CBaseUnit(CBaseUnit && _other)                 = delete;
	CBaseUnit& operator=(const CBaseUnit & _other) = delete;
	CBaseUnit& operator=(CBaseUnit && _other)      = delete;
	virtual ~CBaseUnit()                           = default;

	/**
	 * \internal
	 * \brief Sets pointers and values of all required data.
	 * \param _materialsDB Pointer to global materials database.
	 * \param _grid Const reference to global multidimensional grid.
	 * \param _overall Pointer to a vector of global overall stream parameters.
	 * \param _phases Pointer to a vector of global phases.
	 * \param _cache Pointer to global cache settings.
	 * \param _tolerance Pointer to global tolerance settings.
	 * \param _thermodynamics Pointer to global thermodynamics settings.
	 */
	void SetSettings(const CMaterialsDatabase* _materialsDB, const CMultidimensionalGrid& _grid, const std::vector<SOverallDescriptor>* _overall,
					const std::vector<SPhaseDescriptor>* _phases, const SCacheSettings* _cache, const SToleranceSettings* _tolerance, const SThermodynamicsSettings* _thermodynamics);

	////////////////////////////////////////////////////////////////////////////////
	/// Basic unit information
	///

	// TODO: rename to ModelName() and add an alias to GetUnitName().
	/**
	 * \brief Returns the name of the unit.
	 */
	std::string GetUnitName() const;
	/**
	 * \brief Returns the name of unit's author.
	 */
	std::string GetAuthorName() const;
	/**
	 * \brief Returns the version of the unit.
	 */
	size_t GetVersion() const;
	/**
	 * \brief Returns the unique identifier of the unit.
	 */
	std::string GetUniqueID() const;

	// TODO: rename to SetModelName() and add an alias to SetUnitName().
	/**
	 * \brief Sets the name of the unit.
	 * \details Should be used in CreateBasicInfo function only.
	 * \param _name Name of the unit.
	 */
	void SetUnitName(const std::string& _name);
	/**
	 * \brief Sets the name of unit's author.
	 * \details Should be used in CreateBasicInfo function only.
	 * \param _author Author's name
	 */
	void SetAuthorName(const std::string& _author);
	/**
	 * \brief Sets the version of the unit.
	 * \details Should be used in CreateBasicInfo function only.
	 * \param _version Unit's version.
	 */
	void SetVersion(size_t _version);
	/**
	 * \brief Sets the unique identifier of the unit.
	 * \details Should be used in CreateBasicInfo function only.
	 * \param _id Identifier of the unit. Must be unique among all units in the library.
	 */
	void SetUniqueID(const std::string& _id);

	////////////////////////////////////////////////////////////////////////////////
	// Ports
	//

	/**
	 * Returns a const reference to ports manager.
	 */
	const CPortsManager& GetPortsManager() const;
	// Returns a reference to ports manager.
	CPortsManager& GetPortsManager();

	/**
	 * \brief Adds a port to the unit.
	 * \details Should be used in CreateStructure function only. If the unit already has a port with the same name, a logic_error exception is thrown.
	 * \param _portName Name of the port. Must be unique within the unit.
	 * \param _type Type of the port.
	 * \return Pointer to the created port.
	 * */
	CUnitPort* AddPort(const std::string& _portName, EUnitPort _type);
	/**
	 * \brief Returns a const pointer to the specified port of the unit.
	 * \details Should be used in CreateStructure function only. If no such port exists, a logic_error exception is thrown.
	 * \param _portName Name of the port.
	 * \return Const pointer to the port.
	 */
	const CUnitPort* GetPort(const std::string& _portName) const;
	/**
	 * \brief Returns a pointer to the specified port of the unit.
	 * \details If no such port exists, a logic_error exception is thrown.
	 * \param _portName Name of the port.
	 * \return Pointer to the port.
	 */
	CUnitPort* GetPort(const std::string& _portName);
	/**
	 * \brief Returns a pointer to the stream connected to this port.
	 * \details If such port does not exist or empty, a logic_error exception is thrown.
	 * \param _portName Name of the port.
	 * \return Pointer to the connected stream.
	 */
	CStream* GetPortStream(const std::string& _portName) const;

	////////////////////////////////////////////////////////////////////////////////
	// Internal feeds, holdups and streams
	//

	// Returns a const reference to streams manager.
	const CStreamManager& GetStreamsManager() const;
	// Returns a reference to streams manager.
	CStreamManager& GetStreamsManager();

	/**
	 * Adds a new feed to the unit with proper structure (MD dimensions, phases, materials, etc.), and returns a pointer to it. If a feed with the given name already exists, a logic_error exception is thrown.
	 */
	CStream* AddFeed(const std::string& _name);
	/**
	 * Returns a const feed with the specified name. If such feed does not exist, a logic_error exception is thrown.
	 */
	const CStream* GetFeed(const std::string & _name) const;
	/**
	 * Returns a feed with the specified name. If such feed does not exist, a logic_error exception is thrown.
	 */
	CStream* GetFeed(const std::string& _name);
	/**
	 * Removes the feed with the specified name from the unit. If such feed does not exist, a logic_error exception is thrown.
	 */
	void RemoveFeed(const std::string& _name);

	/**
	 * Adds a new holdup to the unit with proper structure (MD dimensions, phases, materials, etc.), and returns a pointer to it. If a holdup with the given name already exists, a logic_error exception is thrown.
	 */
	CHoldup* AddHoldup(const std::string& _name);
	/**
	 * Returns a const holdup with the specified name. If such holdup does not exist, a logic_error exception is thrown.
	 */
	const CHoldup* GetHoldup(const std::string & _name) const;
	/**
	 * Returns a holdup with the specified name. If such holdup does not exist, a logic_error exception is thrown.
	 */
	CHoldup* GetHoldup(const std::string& _name);
	/**
	 * Removes the holdup with the specified name from the unit. If such holdup does not exist, a logic_error exception is thrown.
	 */
	void RemoveHoldup(const std::string& _name);

	/**
	 * Adds a new stream to the unit with proper structure (MD dimensions, phases, materials, etc.), and returns a pointer to it. If a stream with the given name already exists, a logic_error exception is thrown.
	 */
	CStream* AddStream(const std::string& _name);
	/**
	 * Returns a const stream with the specified name. If such stream does not exist, a logic_error exception is thrown.
	 */
	const CStream* GetStream(const std::string & _name) const;
	/**
	 * Returns a stream with the specified name. If such stream does not exist, a logic_error exception is thrown.
	 */
	CStream* GetStream(const std::string& _name);
	/**
	 * Removes the stream with the specified name from the unit. If such stream does not exist, a logic_error exception is thrown.
	 */
	void RemoveStream(const std::string& _name);

	// Sets up the stream structure (MD dimensions, phases, materials, etc.) the same as it is configured in the unit. Removes all existing data.
	void SetupStream(CBaseStream* _stream) const;

	////////////////////////////////////////////////////////////////////////////////
	// Unit parameters
	//

	// Returns a const reference to unit parameters manager.
	const CUnitParametersManager& GetUnitParametersManager() const;
	// Returns a reference to unit parameters manager.
	CUnitParametersManager& GetUnitParametersManager();

	/**
	 * Adds a new real constant unit parameter and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	 */
	CConstRealUnitParameter* AddConstRealParameter(const std::string& _name, double _initValue, const std::string& _units, const std::string& _description, double _minValue = std::numeric_limits<double>::lowest(), double _maxValue = std::numeric_limits<double>::max());
	/**
	 * Adds a new signed integer constant unit parameter and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	 */
	CConstIntUnitParameter* AddConstIntParameter(const std::string& _name, int64_t _initValue, const std::string& _units, const std::string& _description, int64_t _minValue = std::numeric_limits<int64_t>::lowest(), int64_t _maxValue = std::numeric_limits<int64_t>::max());
	/**
	 * Adds a new unsigned integer constant unit parameter and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	 */
	CConstUIntUnitParameter* AddConstUIntParameter(const std::string& _name, uint64_t _initValue, const std::string& _units, const std::string& _description, uint64_t _minValue = std::numeric_limits<uint64_t>::lowest(), uint64_t _maxValue = std::numeric_limits<uint64_t>::max());
	/**
	 * Adds a new time-dependent unit parameter and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	 */
	CTDUnitParameter* AddTDParameter(const std::string& _name, double _initValue, const std::string& _units, const std::string& _description, double _minValue = std::numeric_limits<double>::lowest(), double _maxValue = std::numeric_limits<double>::max());
	/**
	 * Adds a new string unit parameter and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	 */
	CStringUnitParameter* AddStringParameter(const std::string& _name, const std::string& _initValue, const std::string& _description);
	/**
	 * Adds a new two-positional check box unit parameter and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	 */
	CCheckBoxUnitParameter* AddCheckBoxParameter(const std::string& _name, bool _initValue, const std::string& _description);
	/**
	 * Adds a new combo-box unit parameter and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	 */
	CComboUnitParameter* AddComboParameter(const std::string& _name, size_t _initValue, const std::vector<size_t>& _items, const std::vector<std::string>& _itemsNames, const std::string& _description);
	/**
	 * Adds a new compound combo-box unit parameter, allowing to choose one of the defined compounds, and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	 */
	CCompoundUnitParameter* AddCompoundParameter(const std::string& _name, const std::string& _description);
	/**
	 * Adds a new MDB compound combo-box unit parameter, allowing to choose one of the MDB compounds, and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	 */
	CMDBCompoundUnitParameter* AddMDBCompoundParameter(const std::string& _name, const std::string& _description);
	/**
	 * Adds a new reaction unit parameter and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	 */
	CReactionUnitParameter* AddReactionParameter(const std::string& _name, const std::string& _description);
	/**
	 * Adds a new real list unit parameter and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	 */
	CListRealUnitParameter* AddListRealParameter(const std::string& _name, double _initValue, const std::string& _units, const std::string& _description, double _minValue = std::numeric_limits<double>::lowest(), double _maxValue = std::numeric_limits<double>::max());
	/**
	 * Adds a new signed integer list unit parameter and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	 */
	CListIntUnitParameter* AddListIntParameter(const std::string& _name, int64_t _initValue, const std::string& _units, const std::string& _description, int64_t _minValue = std::numeric_limits<int64_t>::lowest(), int64_t _maxValue = std::numeric_limits<int64_t>::max());
	/**
	 * Adds a new unsigned integer list unit parameter and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	 */
	CListUIntUnitParameter* AddListUIntParameter(const std::string& _name, uint64_t _initValue, const std::string& _units, const std::string& _description, uint64_t _minValue = std::numeric_limits<uint64_t>::lowest(), uint64_t _maxValue = std::numeric_limits<uint64_t>::max());
	/**
	 * Adds a new solver unit parameter with type SOLVER_AGGLOMERATION_1, allowing to choose one of the available solvers of this type, and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	 */
	CSolverUnitParameter* AddSolverAgglomeration(const std::string& _name, const std::string& _description);
	/**
	 * Adds a new solver unit parameter with type SOLVER_PBM_1, allowing to choose one of the available solvers of this type, and returns a pointer to it. If the unit already has a parameter with the same name, logic_error exception is thrown.
	 */
	CSolverUnitParameter* AddSolverPBM(const std::string& _name, const std::string& _description);

	/**
	 * Groups the specified unit parameters and associates them with the given value of the selected unit parameter. The parameter, its value and all the adding parameters must already exist. If something does not exist, logic_error exception is thrown.
	 */
	void AddParametersToGroup(const std::string& _unitParamName, const std::string& _unitParamValueName, const std::vector<std::string>& _groupedParamNames);

	/**
	 * Returns value of the real constant unit parameter. Throws logic_error exception if a real constant unit parameter with the given name does not exist./
	 */
	double GetConstRealParameterValue(const std::string& _name) const;
	/**
	 * Returns value of the signed integer constant unit parameter. Throws logic_error exception if a signed integer constant unit parameter with the given name does not exist.
	 */
	int64_t GetConstIntParameterValue(const std::string& _name) const;
	/**
	 * Returns value of the unsigned integer constant unit parameter. Throws logic_error exception if an unsigned integer constant unit parameter with the given name does not exist.
	 */
	uint64_t GetConstUIntParameterValue(const std::string& _name) const;
	/**
	 * Returns value of the time-dependent unit parameter at the specified time point. Throws logic_error exception if a time-dependent unit parameter with the given name does not exist.
	 */
	double GetTDParameterValue(const std::string& _name, double _time) const;
	/**
	 * Returns value of the string unit parameter. Throws logic_error exception if a string unit parameter with the given name does not exist.
	 */
	std::string GetStringParameterValue(const std::string& _name) const;
	/**
	 * Returns value of the check box unit parameter. Throws logic_error exception if a check box unit parameter with the given name does not exist.
	 */
	bool GetCheckboxParameterValue(const std::string& _name) const;
	/**
	 * Returns value of the combo box unit parameter. Throws logic_error exception if combo box unit parameter with the given name does not exist.
	 */
	size_t GetComboParameterValue(const std::string& _name) const;
	/**
	 * Returns value of the compound unit parameter. Throws logic_error exception if a compound unit parameter with the given name does not exist.
	 */
	std::string GetCompoundParameterValue(const std::string& _name) const;
	/**
	 * Returns value of the MDB compound unit parameter. Throws logic_error exception if a compound unit parameter with the given name does not exist.
	 */
	std::string GetMDBCompoundParameterValue(const std::string& _name) const;
	/**
	 * Returns value of the reaction unit parameter. Throws logic_error exception if a reaction unit parameter with the given name does not exist.
	 */
	std::vector<CChemicalReaction> GetReactionParameterValue(const std::string& _name) const;
	/**
	 * Returns value of the real list unit parameter. Throws logic_error exception if a real list unit parameter with the given name does not exist.
	 */
	double GetListRealParameterValue(const std::string& _name, size_t _index) const;
	/**
	 * Returns value of the signed integer list unit parameter. Throws logic_error exception if a signed integer list unit parameter with the given name does not exist.
	 */
	int64_t GetListIntParameterValue(const std::string& _name, size_t _index) const;
	/**
	 * Returns value of the unsigned integer list unit parameter. Throws logic_error exception if an unsigned integer list unit parameter with the given name does not exist.
	 */
	uint64_t GetListUIntParameterValue(const std::string& _name, size_t _index) const;
	/**
	 * Returns a pointer to an agglomeration solver of a type SOLVER_AGGLOMERATION_1 with the specified name. Throws logic_error exception if a solver unit parameter with the given name does not exist.
	 */
	CAgglomerationSolver* GetSolverAgglomeration(const std::string& _name) const;
	/**
	 * Returns a pointer to an agglomeration solver of a type SOLVER_AGGLOMERATION_1 from the specified unit parameter. Throws logic_error exception if a solver unit parameter with the given type does not exist.
	 */
	CAgglomerationSolver* GetSolverAgglomeration(const CSolverUnitParameter* _param) const;
	/**
	 * Returns a pointer to a PBM solver of a type SOLVER_PBM_1 with the specified name. Throws logic_error exception if a solver unit parameter with the given name does not exist.
	 */
	CPBMSolver* GetSolverPBM(const std::string& _name) const;
	/**
	 * Returns a pointer to a PBM solver of a type SOLVER_PBM_1 from the specified unit parameter. Throws logic_error exception if a solver unit parameter with the given type does not exist.
	 */
	CPBMSolver* GetSolverPBM(const CSolverUnitParameter* _param) const;

	////////////////////////////////////////////////////////////////////////////////
	// State variables
	//

	// Returns a const reference to state variables manager.
	const CStateVariablesManager& GetStateVariablesManager() const;
	// Returns a reference to state variables manager.
	CStateVariablesManager& GetStateVariablesManager();

	/**
	 * Adds a new time-dependent state variable to the unit and returns a pointer to it. If the unit already has a state variable with the same name, logic_error exception is thrown.
	 */
	CStateVariable* AddStateVariable(const std::string& _name, double _initValue);
	/**
	 * Returns current value of a state variable. If a state variable with the given name does not exist in this unit, logic_error exception is thrown.
	 */
	double GetStateVariable(const std::string& _name) const;
	/**
	 * Sets new value of a state variable. If a state variable with the given name does not exist in this unit, logic_error exception is thrown.
	 */
	void SetStateVariable(const std::string& _name, double _value);
	/**
	 * Sets new value of a state variable and adds its value to the history of time-dependent changes. If a state variable with the given name does not exist in this unit, logic_error exception is thrown.
	 */
	void SetStateVariable(const std::string& _name, double _value, double _time);

	////////////////////////////////////////////////////////////////////////////////
	// Plots
	//

	// Returns a const reference to plots manager.
	const CPlotManager& GetPlotsManager() const;
	// Returns a reference to plots manager.
	CPlotManager& GetPlotsManager();

	/**
	 * Adds a new 2-dimensional plot with the specified name and axes labels to the units and returns a pointer to it. If the unit already has a plot with the same name, logic_error exception is thrown.
	 */
	CPlot* AddPlot(const std::string& _plotName, const std::string& _labelX, const std::string& _labelY);
	/**
	 * Adds a new 3-dimensional plot with the specified name and axes labels to the units and returns a pointer to it. If the unit already has a plot with the same name, logic_error exception is thrown.
	 */
	CPlot* AddPlot(const std::string& _plotName, const std::string& _labelX, const std::string& _labelY, const std::string& _labelZ);
	/**
	 * Adds a new curve with the specified name on the 2-dimensional plot with the given name and returns a pointer to it. If a plot with this name does not exist or it already contains a curve with the given name, logic_error exception is thrown.
	 */
	CCurve* AddCurveOnPlot(const std::string& _plotName, const std::string& _curveName);
	/**
	 * Adds a new curve with the specified Z-value on the 3-dimensional plot with the given name and returns a pointer to it. If a plot with this name does not exist or it already contains a curve with the given Z-value, logic_error exception is thrown.
	 */
	CCurve* AddCurveOnPlot(const std::string& _plotName, double _valueZ);
	/**
	 * Adds a new point to the specified curve of the 2-dimensional plot. If a plot or a curve with these names do not exist, logic_error exception is thrown.
	 */
	void AddPointOnCurve(const std::string& _plotName, const std::string& _curveName, double _x, double _y);
	/**
	 * Adds a new point to the specified curve of the 3-dimensional plot. If a plot with this name or a curve with this Z-value do not exist, logic_error exception is thrown.
	 */
	void AddPointOnCurve(const std::string& _plotName, double _valueZ, double _x, double _y);
	/**
	 * Adds new points to the specified curve of the 2-dimensional plot. If a plot or a curve with these names do not exist, logic_error exception is thrown.
	 */
	void AddPointsOnCurve(const std::string& _plotName, const std::string& _curveName, const std::vector<double>& _x, const std::vector<double>& _y);
	/**
	 * Adds new points to the specified curve of the 3-dimensional plot. If a plot with this name or a curve with this Z-value do not exist, logic_error exception is thrown.
	 */
	void AddPointsOnCurve(const std::string& _plotName, double _valueZ, const std::vector<double>& _x, const std::vector<double>& _y);

	////////////////////////////////////////////////////////////////////////////////
	// Stream manipulations
	//

	/**
	 * Copies all stream data from one stream to another at the given time point. All data after the time point are removed from the destination stream.
	 */
	static void CopyStreamToStream(double _time, const CStream* _srcStream, CStream* _dstStream);
	/**
	 * Copies all stream data from one stream to another at the given time interval. All data after the end time point are removed from the destination stream.
	 */
	static void CopyStreamToStream(double _timeBeg, double _timeEnd, const CStream* _srcStream, CStream* _dstStream);
	/**
	 * Copies all stream data from the given stream to the stream connected to the selected output port at the given time point. All data after the time point are removed from the destination stream. Throws logic_error exception on error.
	 */
	void CopyStreamToPort(double _time, const CStream* _stream, CUnitPort* _port) const;
	/**
	 * Copies all stream data from the given stream to the stream connected to the selected output port at the given time point. All data after the time point are removed from the destination stream. Throws logic_error exception on error.
	 */
	void CopyStreamToPort(double _time, const CStream* _stream, const std::string& _portName);
	/**
	 * Copies all stream data from the given stream to the stream connected to the selected output port at the given time interval. All data after the end time point are removed from the destination stream. Throws logic_error exception on error.
	 */
	void CopyStreamToPort(double _timeBeg, double _timeEnd, const CStream* _stream, CUnitPort* _port) const;
	/**
	 * Copies all stream data from the given stream to the stream connected to the selected output port at the given time interval. All data after the end time point are removed from the destination stream. Throws logic_error exception on error.
	 */
	void CopyStreamToPort(double _timeBeg, double _timeEnd, const CStream* _stream, const std::string& _portName);
	/**
	 * Copies all stream data from the stream connected to the selected input port to the given stream at the given time point. All data after the time point are removed from the destination stream. Throws logic_error exception on error.
	 */
	void CopyPortToStream(double _time, const CUnitPort* _port, CStream* _stream) const;
	/**
	 * Copies all stream data from the stream connected to the selected input port to the given stream at the given time point. All data after the time point are removed from the destination stream. Throws logic_error exception on error.
	 */
	void CopyPortToStream(double _time, const std::string& _portName, CStream* _stream);
	/**
	 * Copies all stream data from the stream connected to the selected input port to the given stream at the given time interval. All data after the end time point are removed from the destination stream. Throws logic_error exception on error.
	 */
	void CopyPortToStream(double _timeBeg, double _timeEnd, const CUnitPort* _port, CStream* _stream) const;
	/**
	 * Copies all stream data from the stream connected to the selected input port to the given stream at the given time interval. All data after the end time point are removed from the destination stream. Throws logic_error exception on error.
	 */
	void CopyPortToStream(double _timeBeg, double _timeEnd, const std::string& _portName, CStream* _stream);

	////////////////////////////////////////////////////////////////////////////////
	// Time points
	//

	/**
	 * Returns all time points in the specified time interval at which input streams and time-dependent parameters are defined.
	 */
	std::vector<double> GetAllTimePoints(double _timeBeg, double _timeEnd) const;
	/**
	 * Returns all time points in the specified time interval at which input streams and time-dependent parameters are defined. Boundary time points are unconditionally included into result.
	 */
	std::vector<double> GetAllTimePointsClosed(double _timeBeg, double _timeEnd) const;
	/**
	 * Returns all time points in the specified time interval at which input streams are defined.
	 */
	std::vector<double> GetInputTimePoints(double _timeBeg, double _timeEnd) const;
	/**
	 * Returns all time points in the specified time interval at which input streams are defined. Boundary time points are unconditionally included into result.
	 */
	std::vector<double> GetInputTimePointsClosed(double _timeBeg, double _timeEnd) const;
	/**
	 * Returns all time points in the specified time interval at which given streams are defined.
	 */
	std::vector<double> GetStreamsTimePoints(double _timeBeg, double _timeEnd, const std::vector<CStream*>& _srteams) const;
	/**
	 * Returns all time points in the specified time interval at which given streams are defined. Boundary time points are unconditionally included into result.
	 */
	std::vector<double> GetStreamsTimePointsClosed(double _timeBeg, double _timeEnd, const std::vector<CStream*>& _srteams) const;

	/**
	 * Removes time points within the specified interval [timeBeg; timeEnd) that are closer together than step.
	 */
	void ReduceTimePoints(double _timeBeg, double _timeEnd, double _step);

	////////////////////////////////////////////////////////////////////////////////
	// Compounds
	//

	// Adds a compound with the specified unique key to all feeds, holdups and streams in the unit, if it does not exist yet.
	void AddCompound(const std::string& _compoundKey);
	// Removes a compound with the specified unique key from all feeds, holdups and streams in the unit.
	void RemoveCompound(const std::string& _compoundKey);

	/**
	 * Returns the name of the compound with the specified unique key. Returns empty string if such compound does not exist.
	 */
	std::string GetCompoundName(const std::string& _compoundKey) const;
	/**
	 * Returns the name of the compound with the specified index. Returns empty string if such compound does not exist.
	 */
	std::string GetCompoundName(size_t _index) const;
	/**
	 * Returns the unique key of the compound with the specified name. Returns empty string if such compound does not exist.
	 */
	std::string GetCompoundKey(const std::string& _compoundName) const;
	/**
	 * Returns the unique key of the compound with the specified index. Returns empty string if such compound does not exist.
	 */
	std::string GetCompoundKey(size_t _index) const;
	/**
	 * Returns index of the compound with the specified key. Returns -1 if such compound does not exist.
	 */
	size_t GetCompoundIndex(const std::string& _compoundKey) const;
	/**
	 * Returns index of the compound with the specified name. Returns -1 if such compound does not exist.
	 */
	size_t GetCompoundIndexByName(const std::string& _compoundName) const;
	/**
	 * Returns unique keys of all defined materials.
	 */
	std::vector<std::string> GetAllCompounds() const;
	/**
	 * Returns names of all defined materials.
	 */
	std::vector<std::string> GetAllCompoundsNames() const;
	/**
	 * Returns the number of defined compounds.
	 */
	size_t GetCompoundsNumber() const;
	/**
	 * Checks if a compound with the specified unique key is defined.
	 */
	bool IsCompoundDefined(const std::string& _compoundKey) const;
	/**
	 * Checks if a compound with the specified name is defined.
	 */
	bool IsCompoundNameDefined(const std::string& _compoundName) const;

	////////////////////////////////////////////////////////////////////////////////
	// Overall properties
	//

	// Adds new overall property to all feeds, holdups and streams in the unit, if it does not exist yet.
	void AddOverallProperty(EOverall _property, const std::string& _name, const std::string& _units);
	// Removes an overall property from all feeds, holdups and streams in the unit.
	void RemoveOverallProperty(EOverall _property);

	////////////////////////////////////////////////////////////////////////////////
	// Phases
	//

	// Adds the specified phase to all feeds, holdups and streams in the unit, if it does not exist yet.
	void AddPhase(EPhase _phase, const std::string& _name);
	// Removes the specified phase from all feeds, holdups and streams in the unit.
	void RemovePhase(EPhase _phase);

	/**
	 * Returns the name of the specified phase.
	 */
	std::string GetPhaseName(EPhase _phase) const;
	/**
	 * Returns the type of the phase with the specified index.
	 */
	EPhase GetPhaseType(size_t _index) const;
	/**
	 * Returns types of all defined phases.
	 */
	std::vector<EPhase> GetAllPhases() const;
	/**
	 * Returns the number of defined phases.
	 */
	size_t GetPhasesNumber() const;
	/**
	 * Checks if a specified phase is defined.
	 */
	bool IsPhaseDefined(EPhase _phase) const;

	////////////////////////////////////////////////////////////////////////////////
	// Distributed properties
	//

	// Updates grids of distributed parameters and compounds.
	void SetGrid(const CMultidimensionalGrid& _grid);

	/**
	 * Returns the number of defined distributed parameters.
	 */
	size_t GetDistributionsNumber() const;
	/**
	 * Returns all defined distributed parameters.
	 */
	std::vector<EDistrTypes> GetDistributionsTypes() const;
	/**
	 * Returns numbers of classes of all defined distributed parameters.
	 */
	std::vector<size_t> GetDistributionsClasses() const;
	/**
	 * Returns grid type (GRID_NUMERIC, GRID_SYMBOLIC) of the given distributed parameter. Returns -1 if the given distributed parameter is not defined.
	 */
	EGridEntry GetDistributionGridType(EDistrTypes _distribution) const;

	/**
	 * Returns the number of classes defined for the specified distributed parameter.
	 */
	size_t GetClassesNumber(EDistrTypes _distribution) const;
	/**
	 * Returns a continuous or discrete numeric grid defined for the specified distributed parameter.
	 */
	std::vector<double> GetNumericGrid(EDistrTypes _distribution) const;
	/**
	 * Returns a symbolic discrete grid defined for the specified distributed parameter.
	 */
	std::vector<std::string> GetSymbolicGrid(EDistrTypes _distribution) const;
	/**
	 * Returns the sizes of classes defined in the grid of the specified distributed parameter.
	 */
	std::vector<double> GetClassesSizes(EDistrTypes _distribution) const;
	/**
	 * Returns the mean values of classes defined in the grid of the specified distributed parameter.
	 */
	std::vector<double> GetClassesMeans(EDistrTypes _distribution) const;

	/**
	 * Returns a grid defined for the particle size distribution in terms of particle diameters.
	 */
	std::vector<double> GetPSDGridDiameters() const;
	/**
	 * Returns a grid defined for the particle size distribution in terms of particle surfaces.
	 */
	std::vector<double> GetPSDGridSurfaces() const;
	/**
	 * Returns a grid defined for the particle size distribution in terms of particle volumes.
	 */
	std::vector<double> GetPSDGridVolumes() const;
	/**
	 * Returns the mean values of classes defined in the grid of the particle size distribution in terms of particle diameters.
	 */
	std::vector<double> GetPSDMeanDiameters() const;
	/**
	 * Returns the mean values of classes defined in the grid of the particle size distribution in terms of particle surfaces.
	 */
	std::vector<double> GetPSDMeanSurfaces() const;
	/**
	 * Returns the mean values of classes defined in the grid of the particle size distribution in terms of particle volumes.
	 */
	std::vector<double> GetPSDMeanVolumes() const;

	/**
	 * Checks if the specified distributed parameter is defined.
	 */
	bool IsDistributionDefined(EDistrTypes _distribution) const;

	// TODO: return by value.
	/**
	 * Calculates a transformation matrix needed to obtain the output distribution from the input one.
	 */
	static void CalculateTM(EDistrTypes _distribution, const std::vector<double>& _inValue, const std::vector<double>& _outValue, CTransformMatrix& _matrix);

	////////////////////////////////////////////////////////////////////////////////
	// Global flowsheet data and settings
	//

	// Returns a pointer to the current materials database.
	const CMaterialsDatabase* GetMaterialsDatabase() const;

	// Returns current grid of distributed parameters.
	const CMultidimensionalGrid& GetGrid() const;

	// Returns current tolerance settings.
	SToleranceSettings GetToleranceSettings() const;
	/**
	 * Returns the global absolute tolerance.
	 */
	double GetAbsTolerance() const;
	/**
	 * Returns the global relative tolerance.
	 */
	double GetRelTolerance() const;
	// Returns current thermodynamics settings.
	SThermodynamicsSettings GetThermodynamicsSettings() const;

	// Updates tolerance settings in all units and streams.
	void UpdateToleranceSettings();

	// Updates cache settings in all streams.
	void UpdateCacheSettings();

	// Updates thermodynamics settings in all streams.
	void UpdateThermodynamicsSettings();

	////////////////////////////////////////////////////////////////////////////////
	// Materials database
	//

	/**
	 * Returns the value of the constant physical property (CRITICAL_TEMPERATURE, MOLAR_MASS, etc) of the specified compound.
	 */
	double GetCompoundProperty(const std::string& _compoundKey, ECompoundConstProperties _property) const;
	/**
	 * Returns the value of the temperature/pressure-dependent physical property (DENSITY, ENTHALPY, etc) of the specified compound with the given temperature [K] and pressure [Pa].
	 */
	double GetCompoundProperty(const std::string& _compoundKey, ECompoundTPProperties _property, double _temperature = STANDARD_CONDITION_T, double _pressure = STANDARD_CONDITION_P) const;
	/**
	 * Returns the value of the interaction physical property (INTERFACE_TENSION, etc) between the specified compounds with the given specified temperature [K] and pressure [Pa].
	 */
	double GetCompoundProperty(const std::string& _compoundKey1, const std::string& _compoundKey2, EInteractionProperties _property, double _temperature = STANDARD_CONDITION_T, double _pressure = STANDARD_CONDITION_P) const;

	/**
	 * Checks if a constant physical property with the specified key is present in the materials database.
	 */
	bool IsPropertyDefined(ECompoundConstProperties _property) const;
	/**
	 * Checks if a temperature/pressure-dependent physical property with the specified key is present in the materials database.
	 */
	bool IsPropertyDefined(ECompoundTPProperties _property) const;
	/**
	 * Checks if an interaction physical property with the specified key is present in the materials database.
	 */
	bool IsPropertyDefined(EInteractionProperties _property) const;

	////////////////////////////////////////////////////////////////////////////////
	// Thermodynamics
	//

	// Returns a pointer to enthalpy calculator.
	[[nodiscard]] CMixtureEnthalpyLookup* GetEnthalpyCalculator() const;

	/**
	 * Calculates enthalpy of the mixture of all defined compounds for the given temperature and compound fractions using a lookup table.
	 */
	[[nodiscard]] double CalculateEnthalpyFromTemperature(double _temperature, const std::vector<double>& _fractions) const;
	/**
	 * Calculates temperature of the mixture of all defined compounds for the given enthalpy and compound fractions using a lookup table.
	 */
	[[nodiscard]] double CalculateTemperatureFromEnthalpy(double _enthalpy, const std::vector<double>& _fractions) const;

	/**
	 * Performs a heat exchange between two streams at the specified time point with a specified efficiency (0..1].
	 */
	void HeatExchange(double _time, CBaseStream* _stream1, CBaseStream* _stream2, double _efficiency) const;

	////////////////////////////////////////////////////////////////////////////////
	// Errors and warnings
	//

	/**
	 * Sets an error state of the unit, prints the message to the simulation log, and requests to stop simulation.
	 */
	void RaiseError(const std::string& _message = "");
	/**
	 * Sets a warning state of the unit, and prints the message to the simulation log.
	 */
	void RaiseWarning(const std::string& _message = "");
	/**
	 * Prints the message to the simulation log.
	 */
	void ShowInfo(const std::string& _message);

	/**
	 * Checks if an error state is requested.
	 */
	bool HasError() const;
	/**
	 * Checks if a warning state is requested.
	 */
	bool HasWarning() const;
	/**
	 * Checks if an info output is requested.
	 */
	bool HasInfo() const;

	/**
	 * Returns a textual description of the last error.
	 */
	std::string GetErrorMessage() const;
	/**
	 * Returns a textual description of the last warning.
	 */
	std::string GetWarningMessage() const;
	/**
	 * Returns a textual description of the last info.
	 */
	std::string GetInfoMessage() const;

	// Clears an error state.
	void ClearError();
	// Clears a warning state.
	void ClearWarning();
	// Clears an info state.
	void ClearInfo();

	// Returns a textual description of the last error if any and clears the error state.
	std::string PopErrorMessage();
	// Returns a textual description of the last warning if any and clears the warning state.
	std::string PopWarningMessage();
	// Returns a textual description of the last info if any and clears the info state.
	std::string PopInfoMessage();

	////////////////////////////////////////////////////////////////////////////////
	// Simulation-time operations
	//

	// Setup the structure of the unit (ports, unit parameters, holdups, internal streams).
	void DoCreateStructure();
	// Performs initialization of unit before starting the simulation.
	void DoInitializeUnit();
	// Performs finalization of unit after the simulation is finished.
	void DoFinalizeUnit();
	// Stores current state of the unit on specified time interval.
	void DoSaveStateUnit(double _timeBeg, double _timeEnd);
	// Restores previously saved state of the unit.
	void DoLoadStateUnit();

	// Removes all data produced during simulation.
	void ClearSimulationResults();

	////////////////////////////////////////////////////////////////////////////////
	// User-overloaded functions
	//

	/**
	 * Setup basic parameters of the unit (name, author, key, version).
	 */
	virtual void CreateBasicInfo() = 0;
	/**
	 * Setup the structure of the unit (ports, unit parameters, holdups, internal streams).
	 */
	virtual void CreateStructure() = 0;
	/**
	 * Initialize unit for at time point 0 before starting the simulation.
	 */
	virtual void Initialize(double _time) {}
	/**
	 * Calculate the unit on the given time point.
	 */
	virtual void Simulate(double _time) {}
	/**
	 * Calculate the unit on the given time interval.
	 */
	virtual void Simulate(double _timeBeg, double _timeEnd) {}
	/**
	 * Is called once at the end of the simulation to finalize the unit.
	 */
	virtual void Finalize() {}
	/**
	 * Save the current state of all time-dependent parameters for a possible restart of the simulation from this time point.
	 */
	virtual void SaveState() {}
	/**
	 * Load the stored state of all time-dependent parameters before a restart of the simulation from that time point.
	 */
	virtual void LoadState() {}

	////////////////////////////////////////////////////////////////////////////////
	// Saving/loading
	//

	// Saves unit to HDF5 file.
	void SaveToFile(CH5Handler& _h5File, const std::string& _path);
	// Loads unit from HDF5 file.
	void LoadFromFile(CH5Handler& _h5File, const std::string& _path);
	// Loads unit from HDF5 file. A compatibility version.
	void LoadFromFile_v2(const CH5Handler& _h5File, const std::string& _path);
	// Loads unit from HDF5 file. A compatibility version.
	void LoadFromFile_v1(const CH5Handler& _h5File, const std::string& _path);

private:
	// Clears enthalpy calculator.
	void ClearEnthalpyCalculator() const;

public:
	// TODO: move it somewhere
	////////////////////////////////////////////////////////////////////////////////
	// Deprecated functions
	[[deprecated("WARNING! m_sUnitName is deprecated. Use SetUnitName(const std::string&) instead.")]]
	std::string& GetNameRef();
	[[deprecated("WARNING! m_sAuthorName is deprecated. Use SetAuthorName(const std::string&) instead.")]]
	std::string& GetAuthorRef();
	[[deprecated("WARNING! m_dUnitVersion is deprecated. Use SetVersion(size_t) instead.")]]
	size_t& GetVersionRef();
	[[deprecated("WARNING! m_sUniqueID is deprecated. Use SetUniqueID(const std::string&) instead.")]]
	std::string& GetKeyRef();
	[[deprecated("WARNING! m_pMaterialsDB is deprecated. Use GetMaterialsDatabase() instead.")]]
	const CMaterialsDatabase* GetMaterialsDBRef() const;
	[[deprecated("WARNING! SetDynamicUnit(bool) is deprecated and not needed anymore.")]]
	static void SetDynamicUnit(bool _flag) {}
	[[deprecated("WARNING! AddPort(const std::string&, EPortType) is deprecated. Use AddPort(const std::string&, EUnitPort) instead.")]]
	unsigned AddPort(const std::string& _name, unsigned _type);
	[[deprecated("WARNING! GetPortStream(unsigned) is deprecated. Use GetPortStream(const std::string&) or CUnitPort::GetStream() instead.")]]
	CStream* GetPortStream(unsigned _index) const;
	[[deprecated("WARNING! AddFeed(const std::string&, const std::string&) is deprecated. Use AddFeed(const std::string&) instead.")]]
	CStream* AddFeed(const std::string& _name, const std::string& _key);
	[[deprecated("WARNING! AddHoldup(const std::string&, const std::string&) is deprecated. Use AddHoldup(const std::string&) instead.")]]
	CHoldup* AddHoldup(const std::string& _name, const std::string& _key);
	[[deprecated("WARNING! AddMaterialStream(const std::string&, const std::string&) is deprecated. Use AddStream(const std::string&) instead.")]]
	CStream* AddMaterialStream(const std::string& _name, const std::string& key = "");
	[[deprecated("WARNING! GetMaterialStream(const std::string&) is deprecated. Use GetStream(const std::string&) instead.")]]
	CStream* GetMaterialStream(const std::string& _name);
	[[deprecated("WARNING! AddConstParameter(const std::string&, double, double, double, const std::string&, const std::string&) is deprecated. Use AddConstRealParameter(const std::string&, double, const std::string&, const std::string& _description, double, double), AddConstIntParameter(const std::string&, int64_t, const std::string&, const std::string& _description, int64_t, int64_t), or AddConstUIntParameter(const std::string&, uint64_t, const std::string&, const std::string& _description, uint64_t, uint64_t) instead.")]]
	void AddConstParameter(const std::string& _name, double _minValue, double _maxValue, double _initValue, const std::string& _units, const std::string& _description);
	[[deprecated("WARNING! AddTDParameter(const std::string&, double, double, double, const std::string&, const std::string&) is deprecated. Use AddTDParameter(const std::string&, double, const std::string&, const std::string&, double, double) instead.")]]
	void AddTDParameter(const std::string& _name, double _minValue, double _maxValue, double _initValue, const std::string& _units, const std::string& _description);
	[[deprecated("WARNING! AddGroupParameter(const std::string&, size_t, sonct std::vector<size_t>&, const std::vector<std::string>&, const std::string&) is deprecated. Use AddComboParameter(const std::string&, size_t, const std::vector<size_t>&, const std::vector<std::string>&, const std::string&) instead.")]]
	void AddGroupParameter(const std::string& _name, size_t _initValue, const std::vector<size_t>& _values, const std::vector<std::string>& _valuesNames, const std::string& _description);
	[[deprecated("WARNING! GetConstParameterValue(const std::string&) is deprecated. Use GetConstRealParameterValue(const std::string&), GetConstIntParameterValue(const std::string&), or GetConstUIntParameterValue(const std::string&) instead.")]]
	double GetConstParameterValue(const std::string& _name) const;
	[[deprecated("WARNING! GetGroupParameterValue(const std::string&) is deprecated. Use GetComboParameterValue(const std::string&) instead.")]]
	size_t GetGroupParameterValue(const std::string& _name) const;
	[[deprecated("WARNING! AddStateVariable(const std::string&, double, bool) is deprecated. Use AddStateVariable(const std::string&, double) instead.")]]
	unsigned AddStateVariable(const std::string& _name, double _initValue, bool _saveHistory);
	[[deprecated("WARNING! SetStateVariable(unsigned, double) is deprecated. Use SetStateVariable(const std::string&, double, double) or SetStateVariable(const std::string&, double) instead.")]]
	void SetStateVariable(unsigned _index, double _value);
	[[deprecated("WARNING! SaveStateVariables(double) is deprecated. Use SetStateVariable(const std::string&, double, double) instead when setting a value.")]]
	void SaveStateVariables(double _time);
	[[deprecated("WARNING! AddPointOnCurve(const std::string&, const std::string&, const std::vector<double>&, const std::vector<double>&) is deprecated. Use AddPointsOnCurve(const std::string&, const std::string&, const std::vector<double>&, const std::vector<double>&) instead.")]]
	void AddPointOnCurve(const std::string& _plotName, const std::string& _curveName, const std::vector<double>& _x, const std::vector<double>& _y);
	[[deprecated("WARNING! AddPointOnCurve(const std::string&, _valueZ, const std::vector<double>&, const std::vector<double>&) is deprecated. Use AddPointsOnCurve(const std::string&, _valueZ, const std::vector<double>&, const std::vector<double>&) instead.")]]
	void AddPointOnCurve(const std::string& _plotName, double _valueZ, const std::vector<double>& _x, const std::vector<double>& _y);
	[[deprecated("WARNING! GetPlotsNumber() is deprecated. Use GetPlotsManager().GetPlotsNumber() instead.")]]
	unsigned GetPlotsNumber() const;
	[[deprecated("WARNING! GetCurvesNumber(size_t) is deprecated. Use GetPlotsManager().GetPlot(const std::string&)->GetCurvesNumber() instead.")]]
	unsigned GetCurvesNumber(size_t _iPlot) const;
	[[deprecated("WARNING! GetPlotName(size_t) is deprecated. Use GetPlotsManager().GetPlot(const std::string&)->GetName() instead.")]]
	std::string GetPlotName(size_t _iPlot) const;
	[[deprecated("WARNING! GetPlotXAxisName(unsigned) is deprecated. Use GetPlotsManager().GetPlot(const std::string&)->GetLabelX() instead.")]]
	std::string GetPlotXAxisName(unsigned _iPlot) const;
	[[deprecated("WARNING! GetPlotYAxisName(unsigned) is deprecated. Use GetPlotsManager().GetPlot(const std::string&)->GetLabelY() instead.")]]
	std::string GetPlotYAxisName(unsigned _iPlot) const;
	[[deprecated("WARNING! GetPlotZAxisName(unsigned) is deprecated. Use GetPlotsManager().GetPlot(const std::string&)->GetLabelZ() instead.")]]
	std::string GetPlotZAxisName(unsigned _iPlot) const;
	[[deprecated("WARNING! GetCurveName(unsigned, unsigned) is deprecated. Use GetPlotsManager().GetPlot(const std::string&)->GetCurve(const std::string&)->GetName() or GetPlotsManager().GetPlot(const std::string&)->GetCurve(double)->GetName() instead.")]]
	std::string GetCurveName(unsigned _iPlot, unsigned _iCurve) const;
	[[deprecated("WARNING! GetCurveX(unsigned, unsigned) is deprecated. Use GetPlotsManager().GetPlot(const std::string&)->GetCurve(const std::string&)->GetXValues() or GetPlotsManager().GetPlot(const std::string&)->GetCurve(double)->GetXValues() instead.")]]
	std::vector<double> GetCurveX(unsigned _iPlot, unsigned _iCurve) const;
	[[deprecated("WARNING! GetCurveY(unsigned, unsigned) is deprecated. Use GetPlotsManager().GetPlot(const std::string&)->GetCurve(const std::string&)->GetYValues() or GetPlotsManager().GetPlot(const std::string&)->GetCurve(double)->GetYValues() instead.")]]
	std::vector<double> GetCurveY(unsigned _iPlot, unsigned _iCurve) const;
	[[deprecated("WARNING! GetCurveZ(size_t, size_t) is deprecated. Use GetPlotsManager().GetPlot(const std::string&)->GetCurve(const std::string&)->GetZValue() or GetPlotsManager().GetPlot(const std::string&)->GetCurve(double)->GetZValue() instead.")]]
	double GetCurveZ(size_t _iPlot, size_t _iCurve) const;
	[[deprecated("WARNING! IsPlot2D(unsigned) is deprecated. Use GetPlotsManager().GetPlot(const std::string&)->Is2D() instead.")]]
	bool IsPlot2D(unsigned _iPlot);
	[[deprecated("WARNING! GetAllDefinedTimePoints(double, double, bool, bool) is deprecated. Use GetAllTimePoints(double, double) or GetAllTimePointsClosed(double, double) instead.")]]
	std::vector<double> GetAllDefinedTimePoints(double _timeBeg, double _timeEnd, bool _forceStartBoundary = false, bool _forceEndBoundary = false) const;
	[[deprecated("WARNING! GetAllInputTimePoints(double, double, bool, bool) is deprecated. Use GetInputTimePoints(double, double) or GetInputTimePointsClosed(double, double) instead.")]]
	std::vector<double> GetAllInputTimePoints(double _timeBeg, double _timeEnd, bool _forceStartBoundary = false, bool _forceEndBoundary = false) const;
	[[deprecated("WARNING! GetAllStreamsTimePoints(const std::vector<CStream*>&, double, double) is deprecated. Use GetStreamsTimePoints(double, double, const std::vector<CStream*>&) or GetStreamsTimePointsClosed(double, double, const std::vector<CStream*>&) instead.")]]
	std::vector<double> GetAllStreamsTimePoints(const std::vector<CStream*>& _srteams, double _timeBeg, double _timeEnd) const;
	[[deprecated("WARNING! GetCompoundsList() is deprecated. Use GetAllCompounds() instead.")]]
	std::vector<std::string> GetCompoundsList() const;
	[[deprecated("WARNING! GetCompoundsNames() is deprecated. Use GetAllCompoundsNames(), GetCompoundName(const std::string&) or GetCompoundName(size_t) instead.")]]
	std::vector<std::string> GetCompoundsNames() const;
	[[deprecated("WARNING! IsCompoundKeyDefined(const std::string&) is deprecated. Use IsCompoundDefined(const std::string&) instead.")]]
	bool IsCompoundKeyDefined(const std::string& _compoundKey) const;
	[[deprecated("WARNING! GetPhaseIndex(EPhaseTypes) is deprecated. Access phases by their type.")]]
	unsigned GetPhaseIndex(unsigned _soa) const;
	[[deprecated("WARNING! GetPhaseSOA(unsigned) is deprecated. Use GetPhaseType(size_t) instead.")]]
	unsigned GetPhaseSOA(unsigned _index) const;
	[[deprecated("WARNING! IsPhaseDefined(EPhaseTypes) is deprecated. Use IsPhaseDefined(EPhase) instead.")]]
	bool IsPhaseDefined(unsigned _soa) const;
	[[deprecated("WARNING! GetCompoundConstant(const std::string&, ECompoundConstProperties) is deprecated. Use GetCompoundProperty(const std::string&, ECompoundConstProperties) instead.")]]
	double GetCompoundConstant(const std::string& _compoundKey, unsigned _property) const;
	[[deprecated("WARNING! GetCompoundTPDProp(const std::string&, ECompoundTPProperties) is deprecated. Use GetCompoundProperty(const std::string&, ECompoundTPProperties, double, double) instead.")]]
	double GetCompoundTPDProp(const std::string& _compoundKey, unsigned _property, double _temperature, double _pressure) const;
	[[deprecated("WARNING! GetCompoundsInteractionProp(const std::string&, const std::string&, EInteractionProperties, double, double) is deprecated. Use GetCompoundProperty(const std::string&, const std::string&, EInteractionProperties, double, double) instead.")]]
	double GetCompoundsInteractionProp(const std::string& _compoundKey1, const std::string& _compoundKey2, unsigned _property, double _temperature = STANDARD_CONDITION_T, double _pressure = STANDARD_CONDITION_P) const;
	[[deprecated("WARNING! CalcTemperatureFromProperty(ECompoundTPProperties, const std::vector<double>&, double) is deprecated. Use CalculateTemperatureFromEnthalpy(double, const std::vector<double>&) or CMixtureLookup class instead.")]]
	double CalcTemperatureFromProperty(ECompoundTPProperties _property, const std::vector<double>& _fractions, double _value) const;
	[[deprecated("WARNING! CalcPressureFromProperty(ECompoundTPProperties, const std::vector<double>&, double) is deprecated. Use CMixtureLookup class instead.")]]
	double CalcPressureFromProperty(ECompoundTPProperties _property, const std::vector<double>& _fractions, double _value) const;
	[[deprecated("WARNING! HeatExchange(CMaterialStream*, CMaterialStream*, double, double) is deprecated. Use HeatExchange(double, CBaseStream*, CBaseStream*, double) instead.")]]
	void HeatExchange(CStream* _stream1, CStream* _stream2, double _time, double _efficiency) const;
	[[deprecated("WARNING! CheckError() is deprecated. Use HasError() instead.")]]
	bool CheckError() const;
private:
	static EPhase SOA2EPhase(unsigned _soa);
	static unsigned EPhase2SOA(EPhase _phase);
};

typedef DECLDIR CBaseUnit* (*CreateUnit2)();

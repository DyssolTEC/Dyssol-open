/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "CalculationSequence.h"
#include "UnitContainer.h"
#include "DyssolTypes.h"
#include "ParametersHolder.h"
#include "Phase.h"
#include "MultidimensionalGrid.h"

/*
 * Stores the whole information about the flowsheet.
 * To support different distribution grids for different units, each stream consists of two parts:
 * output (connected to an output port of a unit) and input (connected to an input port of a unit).
 * Data must be copied between them during the simulation.
 * It is inefficient if there is no grid conversion needed, so the split between input and output is rather virtual.
 * m_streams is the main list of streams, which users communicate with: they have a name, can be tear streams, are saved/loaded and visualized.
 * m_streamsI are input streams and either point to main stream (no conversion needed) or to their own objects (conversion needed).
 * Thus, additional streams are only created if they are needed, and unnecessary copy operations are omitted.
 * Unique key for each input-output pair is kept the same.
 */
class CFlowsheet
{
	static const unsigned m_saveVersion{ 5 }; // Current version of the saving procedure.

	////////////////////////////////////////////////////////////////////////////////
	// Global structural data and settings
	//
	const CMaterialsDatabase& m_materialsDB;	// Reference to a global database of materials.
	CModelsManager& m_modelsManager;			// Reference to a global models manager.
	// TODO: move it out of the flowsheet and save/load independently.
	CParametersHolder m_parameters;				// Holder of different flowsheet settings.
	CMultidimensionalGrid m_mainGrid;			// Global version of distribution grids.
	std::vector<SOverallDescriptor> m_overall;	// List of all defined overall properties.
	std::vector<SPhaseDescriptor> m_phases;		// List of all defined phases.
	SCacheSettings m_cacheStreams{ m_parameters.cacheFlagStreams, m_parameters.cacheWindow, m_parameters.cachePath };				// Global cache settings for streams.
	SCacheSettings m_cacheHoldups{ m_parameters.cacheFlagHoldups, m_parameters.cacheWindow, m_parameters.cachePath };				// Global cache settings for holdups in models.
	SToleranceSettings m_tolerance{ m_parameters.absTol, m_parameters.relTol, m_parameters.minFraction };							// Global tolerance settings.
	SThermodynamicsSettings m_thermodynamics{ { m_parameters.enthalpyMinT, m_parameters.enthalpyMaxT }, m_parameters.enthalpyInt };	// Global thermodynamics settings.

	////////////////////////////////////////////////////////////////////////////////
	// Flowsheet structure
	//
	std::vector<std::unique_ptr<CUnitContainer>> m_units;	// All defined units.
	std::vector<std::shared_ptr<CStream>> m_streams;		// All defined streams.

	std::vector<std::shared_ptr<CStream>> m_streamsI;		// Input streams. Either point to streams from m_streams or to own objects.

	////////////////////////////////////////////////////////////////////////////////
	// Topology
	//
	CCalculationSequence m_calculationSequence{ &m_units, &m_streams }; // Unit simulation sequence.
	// TODO: perform the corresponding check in CalculationSequence.
	bool m_topologyModified{ false };	// Indicates whether the flowsheet structure has changed since the last run of the calculation sequence analysis.

public:
	// Basic constructor
	CFlowsheet(CModelsManager& _modelsManager, const CMaterialsDatabase& _materialsDB);

	// Makes initial preparations of the flowsheet structure.
	void Create();
	// Removes all existing data and sets parameters to their default values.
	void Clear();
	// Checks if the flowsheet is empty.
	bool IsEmpty() const;

	////////////////////////////////////////////////////////////////////////////////
	// Units
	//

	// Returns the number of defined units.
	size_t GetUnitsNumber() const;

	// Adds a new unit to the flowsheet and returns a pointer to it.
	CUnitContainer* AddUnit(const std::string& _key = "");
	// Removes a unit with the specified unique key.
	void DeleteUnit(const std::string& _key);
	// Moves a unit with the specified unique key upwards/downwards in the list of units.
	void ShiftUnit(const std::string& _key, EDirection _direction);

	// Returns a unit by its index. If no such unit has been defined, returns nullptr.
	[[nodiscard]] const CUnitContainer* GetUnit(size_t _index) const;
	// Returns a unit by its index. If no such unit has been defined, returns nullptr.
	CUnitContainer* GetUnit(size_t _index);
	// Returns a unit with the specified unique key. If no such unit has been defined, returns nullptr.
	[[nodiscard]] const CUnitContainer* GetUnit(const std::string& _key) const;
	// Returns a unit with the specified unique key. If no such unit has been defined, returns nullptr.
	CUnitContainer* GetUnit(const std::string& _key) ;
	// Returns a unit with the specified name. If no such unit has been defined, returns nullptr. If several units have the same name, returns an arbitrary one.
	[[nodiscard]] const CUnitContainer* GetUnitByName(const std::string& _name) const;
	// Returns a unit with the specified name. If no such unit has been defined, returns nullptr. If several units have the same name, returns an arbitrary one.
	CUnitContainer* GetUnitByName(const std::string& _name);
	// Returns const pointers to all defined units.
	[[nodiscard]] std::vector<const CUnitContainer*> GetAllUnits() const;
	// Returns pointers to all defined units.
	std::vector<CUnitContainer*> GetAllUnits();

	// Copies output streams to input streams if necessary.
	void PrepareInputStreams(const CUnitContainer* _unit, double _timeBeg, double _timeEnd) const;

	////////////////////////////////////////////////////////////////////////////////
	// Streams
	//

	// Returns the number of defined streams.
	size_t GetStreamsNumber() const;

	// Adds a new stream to the flowsheet and returns a pointer to it.
	CStream* AddStream(const std::string& _key = "");
	// Removes a stream with the specified unique key.
	void DeleteStream(const std::string& _key);
	// Moves a stream with the specified unique key upwards/downwards in the list of streams.
	void ShiftStream(const std::string& _key, EDirection _direction);

	// Returns a stream by its index. If no such stream has been defined, returns nullptr.
	[[nodiscard]] const CStream* GetStream(size_t _index) const;
	// Returns a stream by its index. If no such stream has been defined, returns nullptr.
	CStream* GetStream(size_t _index);
	// Returns a stream with the specified unique key. If no such stream has been defined, returns nullptr.
	const CStream* GetStream(const std::string& _key) const;
	// Returns a stream with the specified unique key. If no such stream has been defined, returns nullptr.
	CStream* GetStream(const std::string& _key);
	// Returns a stream with the specified name. If no such stream has been defined, returns nullptr. If several streams have the same name, returns an arbitrary one.
	[[nodiscard]] const CStream* GetStreamByName(const std::string& _name) const;
	// Returns a stream with the specified name. If no such stream has been defined, returns nullptr. If several streams have the same name, returns an arbitrary one.
	CStream* GetStreamByName(const std::string& _name);

	// Returns const pointers to all defined streams.
	std::vector<const CStream*> GetAllStreams() const;
	// Returns pointers to all defined streams.
	std::vector<CStream*> GetAllStreams();

	////////////////////////////////////////////////////////////////////////////////
	// Topology
	//

	// Determines the calculation sequence. Returns true if the analysis is successful.
	bool DetermineCalculationSequence();
	// Sets the flag indicating whether the flowsheet structure has changed since the last run of the calculation sequence analysis.
	void SetTopologyModified(bool _flag);
	// Returns a const pointer to calculation sequence.
	const CCalculationSequence* GetCalculationSequence() const;
	// Returns a pointer to calculation sequence.
	CCalculationSequence* GetCalculationSequence();

	// Generates a graph-like representation of the flowsheet.
	std::vector<SFlowsheetConnection> GenerateConnectionsDescription() const;
	// Generates a flowsheet representation in DOT format.
	std::string GenerateDOTFile();

	////////////////////////////////////////////////////////////////////////////////
	// Compounds
	//

	// Returns the number of defined compounds.
	size_t GetCompoundsNumber() const;
	// Adds a new compound with the specified unique key to the flowsheet, if it does not exist yet.
	void AddCompound(const std::string& _key);
	// Remove a compound with the specified unique key from the flowsheet.
	void RemoveCompound(const std::string& _key);
	// Sets new set of compounds.
	void SetCompounds(const std::vector<std::string>& _keys);
	// Returns unique keys of all defined compounds.
	std::vector<std::string> GetCompounds() const;

	////////////////////////////////////////////////////////////////////////////////
	// Overall properties
	//

	// Returns the number of defined overall properties.
	size_t GetOverallPropertiesNumber() const;
	// Adds a new overall property with the specified parameters to the flowsheet, if it does not exist yet.
	void AddOverallProperty(EOverall _property, const std::string& _name, const std::string& _units);
	// Remove an overall property with the specified type from the flowsheet.
	void RemoveOverallProperty(EOverall _property);
	// Returns descriptors of all defined overall properties.
	std::vector<SOverallDescriptor> GetOveralProperties() const;
	// Checks whether the specified overall property is defined.
	bool HasOverallProperty(EOverall _property) const;

	////////////////////////////////////////////////////////////////////////////////
	// Phases
	//

	// Returns the number of defined phases.
	size_t GetPhasesNumber() const;
	// Adds a new phase with the specified parameters to the flowsheet, if it does not exist yet.
	void AddPhase(EPhase _phase, const std::string& _name);
	// Remove a phase with the specified type from the flowsheet.
	void RemovePhase(EPhase _phase);
	// Sets new set of phases.
	void SetPhases(const std::vector<SPhaseDescriptor>& _phases);
	// Returns descriptors of all defined phases.
	std::vector<SPhaseDescriptor> GetPhases() const;
	// Checks if a phase of the specified type exists in the flowsheet.
	bool HasPhase(EPhase _phase);

	////////////////////////////////////////////////////////////////////////////////
	// Related to simulation
	//

	// Initializes flowsheet before simulation and checks for errors. Returns an error description on error, or an empty string otherwise.
	std::string Initialize();
	// Sets pointers to streams into units' ports.
	void SetStreamsToPorts();
	// Checks that all units' ports are properly connected. Returns an error description on error, or an empty string otherwise.
	std::string CheckPortsConnections();

	// Clears all simulation results.
	void ClearSimulationResults();

	////////////////////////////////////////////////////////////////////////////////
	// Other
	//

	// Sets new main grid of distributed parameters to all stream and units that use it.
	void SetMainGrid(const CMultidimensionalGrid& _grid);
	// Updates the grids of distributed parameters in all units and connected streams. Must be called to notify the flowsheet that units' grids have changed.
	void UpdateGrids();
	// Updates cache settings in all units and streams.
	void UpdateCacheSettings();
	// Updates tolerance settings in all units and streams.
	void UpdateToleranceSettings();
	// Updates thermodynamics settings in all units and streams.
	void UpdateThermodynamicsSettings();

	// Returns current grid of distributed parameters.
	const CMultidimensionalGrid& GetGrid() const;

	// Returns a const pointer to parameters settings.
	const CParametersHolder* GetParameters() const;
	// Returns a pointer to parameters settings.
	CParametersHolder* GetParameters();

	////////////////////////////////////////////////////////////////////////////////
	// File I/O
	//

	// Saves the flowsheet into the HDF5 file.
	bool SaveToFile(CH5Handler& _h5File, const std::filesystem::path& _fileName);
	// Loads the flowsheet from the HDF5 file.
	bool LoadFromFile(CH5Handler& _h5File, const std::filesystem::path& _fileName);
	// Loads the flowsheet from the HDF5 file. A compatibility version.
	bool LoadFromFile_v3(CH5Handler& _h5File, const std::string& _path);

private:
	// Returns a pointer to a stream with the specified unique key from the given vector. If no such stream defined, returns nullptr.
	static CStream* DoGetStream(const std::string& _key, const std::vector<std::shared_ptr<CStream>>& _streams);

	// Returns unique keys of all defined units.
	std::vector<std::string> GetAllUnitsKeys() const;
	// Returns unique keys of all defined streams.
	std::vector<std::string> GetAllStreamsKeys() const;
};


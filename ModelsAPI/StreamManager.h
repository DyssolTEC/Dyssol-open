/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "Stream.h"
#include "Holdup.h"
#include "Phase.h"

/* Manages streams in each unit.
   There are 3 types of holdups and streams:
   1. Fixed - added when constructing unit. Their number is always constant. Are shown in holdups editor.
   2. Variable - added during initialization of unit. Their number can vary depending on e.g. unit parameters. Are shown in simulation results.
   3. Temporary - added during simulation. Are removed right after the simulation is finished. */
class CStreamManager
{
	static const unsigned m_saveVersion{ 1 };	// Current version of the saving procedure.

	std::vector<std::unique_ptr<CStream>> m_feedsInit;		// Initial values of feeds defined in this unit. These will be displayed in holdups editor UI. Will not be changed during the simulation.
	std::vector<std::unique_ptr<CStream>> m_feedsWork;		// Feeds defined in this unit that take part in the simulation, and are displayed in the results.

	std::vector<std::unique_ptr<CHoldup>> m_holdupsInit;	// Initial values of holdups defined in this unit. These will be displayed in holdups editor UI. Will not be changed during the simulation.
	std::vector<std::unique_ptr<CHoldup>> m_holdupsWork;	// Holdups defined in this unit that take part in the simulation, and are displayed in the results.
	std::vector<std::unique_ptr<CHoldup>> m_holdupsStored;	// Copies of holdups used to store data from holdups during cyclic recalculations.

	std::vector<std::unique_ptr<CStream>> m_streamsWork;	// Internal streams.
	std::vector<std::unique_ptr<CStream>> m_streamsStored;	// Copies of streams used to store data from streams during cyclic recalculations.

	size_t m_nFixHoldups{ 0 };	// Number of holdups always present in the unit.
	size_t m_nFixStreams{ 0 };	// Number of streams always present in the unit.
	size_t m_nVarHoldups{ 0 };	// Number of holdups added during initialization of the unit.
	size_t m_nVarStreams{ 0 };	// Number of streams added during initialization of the unit.

	double m_timeBegStored{ 0.0 };	// Begin of the time window that is currently stored in store streams.
	double m_timeEndStored{ 0.0 };	// End of the time window that is currently stored in store streams.

	const CMaterialsDatabase* m_materialsDB{ nullptr };				// Pointer to a database of materials.
	const CMultidimensionalGrid* m_grid{ nullptr };					// Pointer to a distribution grid.
	const std::vector<SOverallDescriptor>* m_overall{ nullptr };	// Pointer to overall properties.
	const std::vector<SPhaseDescriptor>* m_phases{ nullptr };		// Pointer to phases.
	const SCacheSettings* m_cache{ nullptr };						// Pointer to cache settings.
	const SToleranceSettings* m_tolerances{ nullptr };				// Pointer to tolerance settings.
	const SThermodynamicsSettings* m_thermodynamics{ nullptr };		// Pointer to thermodynamics settings.

	// All streams of all types, to simplify massive operations.
	std::vector<std::vector<std::unique_ptr<CStream>>*> m_allStreams{ &m_feedsInit, &m_feedsWork, &m_streamsWork, &m_streamsStored };
	// All holdups of all types, to simplify massive operations.
	std::vector<std::vector<std::unique_ptr<CHoldup>>*> m_allHoldups{ &m_holdupsInit, &m_holdupsWork, &m_holdupsStored };

public:
	// TODO: set it all in constructor and make them references when the same is done in BaseUnit.
	// Sets pointers to all required data.
	void SetPointers(const CMaterialsDatabase* _materialsDB, const CMultidimensionalGrid* _grid, const std::vector<SOverallDescriptor>* _overall,
		const std::vector<SPhaseDescriptor>* _phases, const SCacheSettings* _cache, const SToleranceSettings* _tolerances, const SThermodynamicsSettings* _thermodynamics);

	/**
	 * \internal
	 * \brief Sets pointer to a global materials database.
	 * \param _materialsDB Pointer to materials database.
	 */
	void SetMaterialsDatabase(const CMaterialsDatabase* _materialsDB);
	/**
	 * \internal
	 * \brief Copies user-defined data from _streams.
	 * \details Copies information about configured and simulated feeds and holdups. Assumes the corresponding streams and holdups structure is the same.
	 * \param _streams Reference to source streams manager.
	 */
	void CopyUserData(const CStreamManager& _streams) const;

	// Is called when initial structure of the unit is configured.
	void CreateStructure();
	// Initializes all defines feeds, holdups and streams before starting initialization of the unit.
	void Initialize();
	// Performs additional initialization actions after initialization of the unit, but before starting the simulation.
	void PostInitialize();
	// Removes temporary holdups and streams.
	void RemoveTemporary();
	// Removes variable holdups and streams.
	void RemoveVariable();
	// Removes all simulation results.
	void ClearResults();
	// Removes all defined streams.
	void Clear();

	// Returns the number of defined feeds.
	size_t GetFeedsNumber() const;
	// Adds new feed, setups its structure (MD dimensions, phases, materials, etc.), and returns a pointer to it. If a feed with the given name already exists, does nothing, and returns nullptr.
	CStream* AddFeed(const std::string& _name);
	// Returns a feed with the specified name. If such feed does not exist, returns nullptr.
	const CStream* GetFeed(const std::string& _name) const;
	// Returns a feed with the specified name. If such feed does not exist, returns nullptr.
	CStream* GetFeed(const std::string& _name);
	// Returns all defined initial feeds.
	std::vector<const CStream*> GetFeedsInit() const;
	// Returns all defined initial feeds.
	std::vector<CStream*> GetFeedsInit();
	// Returns all defined feeds.
	std::vector<const CStream*> GetFeeds() const;
	// Returns all defined feeds.
	std::vector<CStream*> GetFeeds();
	// Removes the specified feed.
	void RemoveFeed(const std::string& _name);

	// Returns the number of defined holdups.
	size_t GetHoldupsNumber() const;
	// Adds new holdup, setups its structure (MD dimensions, phases, materials, etc.), and returns a pointer to it. If a holdup with the given name already exists, does nothing, and returns nullptr.
	CHoldup* AddHoldup(const std::string& _name);
	// Returns a holdup with the specified name. If such holdup does not exist, returns nullptr.
	const CHoldup* GetHoldup(const std::string& _name) const;
	// Returns a holdup with the specified name. If such holdup does not exist, returns nullptr.
	CHoldup* GetHoldup(const std::string& _name);
	// Returns all defined initial holdups for fixed holdups.
	std::vector<const CHoldup*> GetHoldupsInit() const;
	// Returns all defined initial holdups for fixed holdups.
	std::vector<CHoldup*> GetHoldupsInit();
	// Returns all defined holdups.
	std::vector<const CHoldup*> GetHoldups() const;
	// Returns all defined holdups.
	std::vector<CHoldup*> GetHoldups();
	// Removes the specified holdup.
	void RemoveHoldup(const std::string& _name);

	// Returns the number of defined streams.
	size_t GetStreamsNumber() const;
	// Adds new stream, setups its structure (MD dimensions, phases, materials, etc.), and returns a pointer to it. If a stream with the given name already exists, does nothing, and returns nullptr.
	CStream* AddStream(const std::string& _name);
	// Returns a stream with the specified name. If such stream does not exist, returns nullptr.
	const CStream* GetStream(const std::string& _name) const;
	// Returns a stream with the specified name. If such stream does not exist, returns nullptr.
	CStream* GetStream(const std::string& _name);
	// Returns all defined streams.
	std::vector<const CStream*> GetStreams() const;
	// Returns all defined streams.
	std::vector<CStream*> GetStreams();
	// Removes the specified stream.
	void RemoveStream(const std::string& _name);

	// Returns initial feed or holdup by its index. If such object does not exist, returns nullptr.
	[[nodiscard]] const CBaseStream* GetObjectInit(size_t _index) const;
	// Returns initial feed or holdup by its index. If such object does not exist, returns nullptr.
	CBaseStream* GetObjectInit(size_t _index);
	// Returns initial feed or holdup by its name. If such object does not exist, returns nullptr.
	[[nodiscard]] const CBaseStream* GetObjectInit(const std::string& _name) const;
	// Returns initial feed or holdup by its name. If such object does not exist, returns nullptr.
	CBaseStream* GetObjectInit(const std::string& _name);

	// Returns working feed holdup or stream by its index. If such object does not exist, returns nullptr.
	[[nodiscard]] const CBaseStream* GetObjectWork(size_t _index) const;
	// Returns working feed holdup or stream by its index. If such object does not exist, returns nullptr.
	CBaseStream* GetObjectWork(size_t _index);
	// Returns working feed holdup or stream by its name. If such object does not exist, returns nullptr.
	[[nodiscard]] const CBaseStream* GetObjectWork(const std::string& _name) const;
	// Returns working feed holdup or stream by its name. If such object does not exist, returns nullptr.
	CBaseStream* GetObjectWork(const std::string& _name);

	// Returns all defined initial feeds and holdups.
	std::vector<const CBaseStream*> GetAllInit() const;
	// Returns all defined initial feeds and holdups.
	std::vector<CBaseStream*> GetAllInit();

	// Returns all defined work feeds, holdups and streams.
	std::vector<const CBaseStream*> GetAllWork() const;
	// Returns all defined work feeds, holdups and streams.
	std::vector<CBaseStream*> GetAllWork();

	// Stores the current state of all data at the given time interval.
	void SaveState(double _timeBeg, double _timeEnd);
	// Restores previously stored state of all data.
	void LoadState();

	// Adds a compound with the specified unique key to all streams.
	void AddCompound(const std::string& _compoundKey);
	// Removes a compound with the specified unique key from all streams.
	void RemoveCompound(const std::string& _compoundKey);

	// Adds an overall property to all streams.
	void AddOverallProperty(EOverall _property, const std::string& _name, const std::string& _units);
	// Removes an overall property from all streams.
	void RemoveOverallProperty(EOverall _property);

	// Adds the specified phase to all streams.
	void AddPhase(EPhase _phase, const std::string& _name);
	// Removes the specified phase from all streams.
	void RemovePhase(EPhase _phase);

	// Updates grids of distributed parameters and compounds in all streams.
	void UpdateGrid();
	// Updates tolerance settings in all units and streams.
	void UpdateToleranceSettings();
	// Updates cache settings in all streams.
	void UpdateCacheSettings();
	// Updates thermodynamics settings in all streams.
	void UpdateThermodynamicsSettings();

	// Removes time points within the specified interval [timeBeg; timeEnd) that are closer together than step.
	void ReduceTimePoints(double _timeBeg, double _timeEnd, double _step);

	// Saves data to file.
	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	// Loads data from file.
	void LoadFromFile(const CH5Handler& _h5File, const std::string& _path);
	// Loads data from file. A compatibility version.
	void LoadFromFile_v0(const CH5Handler& _h5File, const std::string& _path);
	// Loads data from file. A compatibility version.
	void LoadFromFile_v00(const CH5Handler& _h5File, const std::string& _path);

private:
	// Creates a new stream with proper structure (MD dimensions, phases, materials, etc.).
	template<typename T>
	T* CreateObject(const std::string& _key, const std::string& _name) const;
	// Looks for a stream with the specified name in the given list of streams, and returns a pointer to it. If such stream does not exist, returns nullptr.
	template<typename T>
	const T* GetObject(const std::vector<std::unique_ptr<T>>& _streams, const std::string& _name) const;
	// Looks for a stream with the specified name in the given list of streams, and returns a pointer to it. If such stream does not exist, returns nullptr.
	template<typename T>
	T* GetObject(const std::vector<std::unique_ptr<T>>& _streams, const std::string& _name);
	// Returns const pointers to all defined streams from the given list of streams.
	template<typename T>
	std::vector<const T*> GetObjects(const std::vector<std::unique_ptr<T>>& _streams) const;
	// Returns pointers to all defined streams from the given list of streams.
	template<typename T>
	std::vector<T*> GetObjects(const std::vector<std::unique_ptr<T>>& _streams);
	// Removes a stream with the specified name from the given list of streams.
	template<typename T>
	void RemoveObjects(std::vector<std::unique_ptr<T>>& _streams, const std::string& _name);

	// Returns index of the stream with the specified name in the given list of streams.
	template<typename T>
	size_t GetObjectIndex(std::vector<std::unique_ptr<T>>& _streams, const std::string& _name);

	// Saves all streams from the given list.
	template<typename T>
	void SaveObjects(CH5Handler& _h5File, const std::string& _path, const std::vector<std::unique_ptr<T>>& _streams, const std::string& _attribute, const std::string& _group, const std::string& _subgroup, const std::string& _namespath) const;
	// Loads all streams from the given list. Adds new variable objects if necessary during loading.
	template<typename T>
	using AddObjectFun = T* (CStreamManager::*)(const std::string&);
	template<typename T>
	void LoadObjects(const CH5Handler& _h5File, const std::string& _path, const std::vector<std::unique_ptr<T>>& _streams, const std::string& _attribute, const std::string& _group, const std::string& _subgroup, const std::string& _namespath, AddObjectFun<T> _addObjectFun);

	// Returns keys of all the streams from the list.
	template<typename T>
	std::vector<std::string> GetAllKeys(const std::vector<std::unique_ptr<T>>& _streams) const;

	// Returns names of all the streams from the list.
	template<typename T>
	std::vector<std::string> GetAllNames(const std::vector<std::unique_ptr<T>>& _streams) const;

	// Returns pointers to all feeds, streams and holdups.
	std::vector<CBaseStream*> AllObjects();

	/**
	 * \brief Ensure that all initial holdups and feeds have at least one time point.
	 * \details If no time point is defined, a time point 0 is forcefully added with default values of overall parameters.
	 */
	void InitializeInitStreams() const;
};


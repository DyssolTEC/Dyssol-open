/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "Stream.h"
#include "Holdup.h"

// Manages streams in each unit.
class CStreamManager
{
	std::vector<std::unique_ptr<CStream>> m_feedsInit;		// Initial values of feeds defined in this unit. These will be displayed in holdups editor UI. Will not be changed during the simulation.
	std::vector<std::unique_ptr<CStream>> m_feedsWork;		// Feeds defined in this unit that take part in the simulation, and are displayed in the results.

	std::vector<std::unique_ptr<CHoldup>> m_holdupsInit;	// Initial values of holdups defined in this unit. These will be displayed in holdups editor UI. Will not be changed during the simulation.
	std::vector<std::unique_ptr<CHoldup>> m_holdupsWork;	// Holdups defined in this unit that take part in the simulation, and are displayed in the results.
	std::vector<std::unique_ptr<CHoldup>> m_holdupsStored;	// Copies of holdups used to store data from holdups during cyclic recalculations.

	std::vector<std::unique_ptr<CStream>> m_streamsWork;	// Internal streams.
	std::vector<std::unique_ptr<CStream>> m_streamsStored;	// Copies of streams used to store data from streams during cyclic recalculations.

	size_t m_tempHoldups{ 0 };		// Is used to distinguish between permanent (shown in GUI) and temporary holdups (added during the simulation).
	size_t m_tempStreams{ 0 };		// Is used to distinguish between permanent (shown in GUI) and temporary streams (added during the simulation).

	double m_timeBegStored{ 0.0 };	// Begin of the time window that is currently stored in store streams.
	double m_timeEndStored{ 0.0 };	// End of the time window that is currently stored in store streams.

	const CMaterialsDatabase& m_materialsDB;			// Reference to a database of materials.
	const CDistributionsGrid& m_grid;					// Reference to a distribution grid.
	const std::vector<std::string>& m_compounds;		// Reference to compounds.
	const std::vector<SOverallDescriptor>& m_overall;	// Reference to overall properties.
	const std::vector<SPhaseDescriptor>& m_phases;		// Reference to phases.
	const SCacheSettings& m_cache;						// Reference to cache settings.
	double& m_minFraction;								// Reference to minimal fraction.

	// All streams of all types, to simplify massive operations.
	std::vector<std::vector<std::unique_ptr<CStream>>*> m_allStreams{ &m_feedsInit, &m_feedsWork, &m_streamsWork, &m_streamsStored };
	// All holdups of all types, to simplify massive operations.
	std::vector<std::vector<std::unique_ptr<CHoldup>>*> m_allHoldups{ &m_holdupsInit, &m_holdupsWork, &m_holdupsStored };

public:
	CStreamManager(const CMaterialsDatabase& _materialsDB, const CDistributionsGrid& _grid, const std::vector<std::string>& _compounds, const std::vector<SOverallDescriptor>& _overall,
		const std::vector<SPhaseDescriptor>& _phases, const SCacheSettings& _cache, double& _minFraction);

	// Initializes all defines feeds, holdups and streams before starting the simulation.
	void Initialize();
	// Removes temporary holdups and streams.
	void RemoveTemporary();
	// Removes all simulation results.
	void ClearResults();
	// Removes all defined streams.
	void Clear();

	// Adds new feed, setups its structure (MD dimensions, phases, materials, etc.), and returns a pointer to it. If a feed with the given name already exists, does nothing, and returns nullptr.
	CStream* AddFeed(const std::string& _name);
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

	// Adds new holdup, setups its structure (MD dimensions, phases, materials, etc.), and returns a pointer to it. If a holdup with the given name already exists, does nothing, and returns nullptr.
	CHoldup* AddHoldup(const std::string& _name);
	// Returns a holdup with the specified name. If such holdup does not exist, returns nullptr.
	CHoldup* GetHoldup(const std::string& _name);
	// Returns all defined initial holdups.
	std::vector<const CHoldup*> GetHoldupInit() const;
	// Returns all defined initial holdups.
	std::vector<CHoldup*> GetHoldupInit();
	// Returns all defined holdups.
	std::vector<const CHoldup*> GetHoldups() const;
	// Returns all defined holdups.
	std::vector<CHoldup*> GetHoldups();
	// Removes the specified holdup.
	void RemoveHoldup(const std::string& _name);

	// Adds new stream, setups its structure (MD dimensions, phases, materials, etc.), and returns a pointer to it. If a stream with the given name already exists, does nothing, and returns nullptr.
	CStream* AddStream(const std::string& _name);
	// Returns a stream with the specified name. If such stream does not exist, returns nullptr.
	CStream* GetStream(const std::string& _name);
	// Returns all defined streams.
	std::vector<const CStream*> GetStreams() const;
	// Returns all defined streams.
	std::vector<CStream*> GetStreams();
	// Removes the specified stream.
	void RemoveStream(const std::string& _name);

	// Stores the current state of all data at the given time interval.
	void SaveState(double _timeBeg, double _timeEnd);
	// Restores previously stored state of all data.
	void LoadState();

	// Adds a compound with the specified unique key to all streams.
	void AddCompound(const std::string& _compoundKey);
	// Removes a compound with the specified unique key from all streams.
	void RemoveCompound(const std::string& _compoundKey);

	// Adds the specified phase to all streams.
	void AddPhase(EPhase _phase, const std::string& _name);
	// Removes the specified phase from all streams.
	void RemovePhase(EPhase _phase);

	// Updates grids of distributed parameters in all streams.
	void UpdateDistributionsGrid();
	// Updates minimum fraction to be considered in MDMatrix of all streams.
	void UpdateMinimumFraction();
	// Updates cache settings in all streams.
	void UpdateCacheSettings();

	// Removes time points within the specified interval [timeBeg; timeEnd) that are closer together than step.
	void ReduceTimePoints(double _timeBeg, double _timeEnd, double _step);

	// Sets up the stream structure (MD dimensions, phases, materials, etc.) the same as it is configured in the flowsheet.
	void SetupStreamStructure(CBaseStream& _stream) const;

private:
	// Creates a new stream with proper structure (MD dimensions, phases, materials, etc.).
	template<typename T>
	T* CreateObject(const std::string& _key, const std::string& _name) const;
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
	void RemoveObjects(const std::vector<std::unique_ptr<T>>& _streams, const std::string& _name);

	// Returns keys of all the streams from the list.
	template<typename T>
	std::vector<std::string> GetAllKeys(const std::vector<std::unique_ptr<T>>& _streams);

	// Returns pointers to all feeds, streams and holdups.
	std::vector<CBaseStream*> AllObjects();
};


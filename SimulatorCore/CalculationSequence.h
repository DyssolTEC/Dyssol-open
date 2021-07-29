/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolDefines.h"
#include <string>
#include <vector>
#include <memory>

#include "DyssolTypes.h"

class CUnitContainer;
class CStream;
class CH5Handler;
struct SCacheSettings;
struct SToleranceSettings;

/** This class contains information about the way how calculations of models should be performed.
 *	Each Partition is the list of units in separate recycle loop. They should be calculated together.
 *	Afterwards, the convergence in the list of corresponding tear streams has to be analyzed.
 *	Additionally, for each partition, it maintains a list a streams, which are used as initials for tear streams.
 */
class CCalculationSequence
{
public:
	// TODO: get rid of this struct and use only keys.
	struct SPartition
	{
		std::vector<CUnitContainer*> models;	// List of pointers to models belonging to this partition.
		std::vector<CStream*> tearStreams;	// List of pointers to tear streams belonging to this partition.
	};

private:
	struct SPartitionKeys
	{
		std::vector<std::string> models;      // List of models' keys for each partition.
		std::vector<std::string> tearStreams; // List of tear streams' keys for each partition.
	};

	static const unsigned m_saveVersion{ 2 };	// Current version of the saving procedure.


	const std::vector<std::unique_ptr<CUnitContainer>>* m_models{};	// Pointer to a vector of available models.
	const std::vector<std::shared_ptr<CStream>>* m_streams{};		// Pointer to a vector of available streams.

	std::vector<SPartitionKeys> m_partitions;       // List of defined partitions.

	std::vector<std::vector<std::unique_ptr<CStream>>> m_initialTearStreams;	// Streams needed for initialization of tear streams.

public:
	// Sets pointers to all existing models and streams.
	CCalculationSequence(const std::vector<std::unique_ptr<CUnitContainer>>* _allModels, const std::vector<std::shared_ptr<CStream>>* _allStreams);

	// Clear calculation sequence and all initial tear streams.
	void Clear();
	// Sets partitions using indices of models and streams for each partition.
	void SetSequence(const std::vector<std::vector<std::string>>& _modelsKeys, const std::vector<std::vector<std::string>>& _streamsKeys);
	// Sets new model key to existing model.
	void SetModel(size_t _iPartition, size_t _iModel, const std::string& _modelKey);
	// Sets new stream key to existing tear stream.
	void SetStream(size_t _iPartition, size_t _iStream, const std::string& _streamKey);

	// Add new partition to the sequence.
	void AddPartition(const std::vector<std::string>& _modelsKeys, const std::vector<std::string>& _tearStreamsKeys);
	// Add new model to a partition.
	void AddModel(size_t _iPartition, const std::string& _modelKey);
	// Add new tear stream to a partition.
	void AddStream(size_t _iPartition, const std::string& _streamKey);
	// Remove a partition from the sequence.
	void DeletePartition(size_t _iPartition);
	// Remove a model with the key _modelKey from the sequence.
	void DeleteModel(const std::string& _modelKey);
	// Remove the model _iModel from the partition _iPartition.
	void DeleteModel(size_t _iPartition, size_t _iModel);
	// Remove a stream with the key _streamKey from the sequence.
	void DeleteStream(const std::string& _streamKey);
	// Remove the stream _iStream from the partition _iPartition.
	void DeleteStream(size_t _iPartition, size_t _iStream);

	// Shift selected partition upwards in partitions list.
	void ShiftPartitionUp(size_t _iPartition);
	// Shift selected partition downwards in partitions list.
	void ShiftPartitionDown(size_t _iPartition);
	// Shift selected model in partition upwards in models list.
	void ShiftModelUp(size_t _iPartition, size_t _iModel);
	// Shift selected model in partition downwards in models list.
	void ShiftModelDown(size_t _iPartition, size_t _iModel);
	// Shift selected tear stream in partition upwards in tear streams list.
	void ShiftStreamUp(size_t _iPartition, size_t _iStream);
	// Shift selected tear stream in partition downwards in tear streams list.
	void ShiftStreamDown(size_t _iPartition, size_t _iStream);

	// Get pointers to models from the specified partition.
	std::vector<CUnitContainer*> PartitionModels(size_t _iPartition) const;
	// Get pointers to tear streams from the specified partition.
	std::vector<CStream*> PartitionTearStreams(size_t _iPartition) const;
	// Get the specified partitions.
	SPartition Partition(size_t _iPartition) const;
	// Get all defined partitions.
	std::vector<SPartition> Partitions() const;

	// Number of defined partitions.
	size_t PartitionsNumber() const;
	// Number of models in a partition with index _iPartition.
	size_t ModelsNumber(size_t _iPartition) const;
	// Number of tear streams in a partition with index _iPartition.
	size_t TearStreamsNumber(size_t _iPartition) const;

	// Check if any partition is defined.
	bool IsEmpty() const;
	// Check for errors in the sequence.
	std::string Check() const;

	// Creates the required number of initial streams according to a current calculation sequence and configures their structure, preparing for simulation start.
	void CreateInitialStreams();
	// Clears all time points in initial tear streams.
	void ClearInitialStreamsData();
	// Copies initial streams to tear streams to initialize them.
	void InitializeTearStreams(double _timeWindow);
	// Copies current data of tear streams to initial streams.
	void UpdateInitialStreams(double _timeWindow);

	// Returns all initial tear streams.
	std::vector<std::vector<const CStream*>> GetAllInitialStreams() const;
	// Returns all initial tear streams.
	std::vector<std::vector<CStream*>> GetAllInitialStreams();

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

	// Updates grids of distributed parameters in all streams.
	void UpdateDistributionsGrid();
	// Updates cache settings in all streams.
	void UpdateCacheSettings(const SCacheSettings& _cache);
	// Updates tolerance settings in all streams.
	void UpdateToleranceSettings(const SToleranceSettings& _tolerance);
	// Updates thermodynamics settings in all streams.
	void UpdateThermodynamicsSettings(const SThermodynamicsSettings& _settings);

	void SaveToFile(CH5Handler& _h5Saver, const std::string& _path);
	void LoadFromFile(CH5Handler& _h5Loader, const std::string& _path);
	void LoadFromFile_v1(CH5Handler& _h5Loader, const std::string& _path);
	void LoadFromFile_v0(CH5Handler& _h5Loader, const std::string& _path);

private:
	// Check whether a model with the key _modelKey is presented in the sequence.
	bool IsModelInSequence(const std::string& _modelKey) const;
};


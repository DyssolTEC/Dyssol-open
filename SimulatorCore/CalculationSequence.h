/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BaseModel.h"

class CH5Handler;

/** This class contains information about the way how calculations of models should be performed.
 *	Each Partition is the list of units in separate recycle loop. They should be calculated together.
 *	Afterwards, the convergence in the list of corresponding tear streams has to be analyzed. */
class CCalculationSequence
{
public:
	struct SPartition
	{
		std::vector<CBaseModel*> models;           // List of pointers to models of this partition.
		std::vector<CMaterialStream*> tearStreams; // List of pointers to tear streams of this partition.
	};

private:
	struct SPartitionKeys
	{
		std::vector<std::string> models;      // List of models' keys for each partition.
		std::vector<std::string> tearStreams; // List of tear streams' keys for each partition.
	};

	static const int m_cnSaveVersion;

	std::vector<SPartitionKeys> m_partitions;       // List of defined partitions.

	const std::vector<CBaseModel*>* m_models{};       // Pointer to a vector of available models.
	const std::vector<CMaterialStream*>* m_streams{}; // Pointer to a vector of available streams.

public:
	// Sets pointers to all existing models and streams.
	CCalculationSequence(const std::vector<CBaseModel*>* _allModels, const std::vector<CMaterialStream*>* _allStreams);

	// Clear calculation sequence.
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
	std::vector<CBaseModel*> PartitionModels(size_t _iPartition) const;
	// Get pointers to tear streams from the specified partition.
	std::vector<CMaterialStream*> PartitionTearStreams(size_t _iPartition) const;
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

	void SaveToFile(CH5Handler& _h5Saver, const std::string& _path);
	void LoadFromFile(CH5Handler& _h5Loader, const std::string& _path);
	void LoadFromFileOld(CH5Handler& _h5Loader, const std::string& _path);

private:
	// Check whether a model with the key _modelKey is presented in the sequence.
	bool IsModelInSequence(const std::string& _modelKey) const;
};


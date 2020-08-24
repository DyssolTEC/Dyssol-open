/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ContainerFunctions.h"
#include "MaterialStream.h"
#include "DyssolStringConstants.h"
#include "CalculationSequence.h"

const int CCalculationSequence::m_cnSaveVersion = 1;

CCalculationSequence::CCalculationSequence(const std::vector<CBaseModel*>* _allModels, const std::vector<CMaterialStream*>* _allStreams)
{
	m_models = _allModels;
	m_streams = _allStreams;
}

void CCalculationSequence::Clear()
{
	m_partitions.clear();
}

void CCalculationSequence::SetSequence(const std::vector<std::vector<std::string>>& _modelsKeys, const std::vector<std::vector<std::string>>& _streamsKeys)
{
	m_partitions.clear();
	if (_modelsKeys.size() != _streamsKeys.size()) return;
	m_partitions.resize(_modelsKeys.size());
	for (size_t iPartition = 0; iPartition < _modelsKeys.size(); ++iPartition)
	{
		for (const auto& modelKey : _modelsKeys[iPartition])
			m_partitions[iPartition].models.push_back(modelKey);
		for (const auto& streamKey : _streamsKeys[iPartition])
			m_partitions[iPartition].tearStreams.push_back(streamKey);
	}
}

void CCalculationSequence::SetModel(size_t _iPartition, size_t _iModel, const std::string& _modelKey)
{
	if (_iPartition >= m_partitions.size()) return;
	if (_iModel >= m_partitions[_iPartition].models.size()) return;
	m_partitions[_iPartition].models[_iModel] = _modelKey;
}

void CCalculationSequence::SetStream(size_t _iPartition, size_t _iStream, const std::string& _streamKey)
{
	if (_iPartition >= m_partitions.size()) return;
	if (_iStream >= m_partitions[_iPartition].tearStreams.size()) return;
	m_partitions[_iPartition].tearStreams[_iStream] = _streamKey;
}

void CCalculationSequence::AddPartition(const std::vector<std::string>& _modelsKeys, const std::vector<std::string>& _tearStreamsKeys)
{
	m_partitions.emplace_back(SPartitionKeys{ _modelsKeys , _tearStreamsKeys });
}

void CCalculationSequence::AddModel(size_t _iPartition, const std::string& _modelKey)
{
	if (_iPartition >= m_partitions.size()) return;
	m_partitions[_iPartition].models.push_back(_modelKey);
}

void CCalculationSequence::AddStream(size_t _iPartition, const std::string& _streamKey)
{
	if (_iPartition >= m_partitions.size()) return;
	m_partitions[_iPartition].tearStreams.push_back(_streamKey);
}

void CCalculationSequence::DeletePartition(size_t _iPartition)
{
	VectorDelete(m_partitions, _iPartition);
}

void CCalculationSequence::DeleteModel(const std::string& _modelKey)
{
	// remove model from the calculation sequence
	for (auto& partition : m_partitions)
		VectorDelete(partition.models, [&](const std::string& k) { return k == _modelKey; });

	// remove empty partitions
	VectorDelete(m_partitions, [&](const SPartitionKeys& p) { return p.models.empty(); });
}

void CCalculationSequence::DeleteModel(size_t _iPartition, size_t _iModel)
{
	if (_iPartition >= m_partitions.size()) return;
	VectorDelete(m_partitions[_iPartition].models, _iModel);
}

void CCalculationSequence::DeleteStream(const std::string& _streamKey)
{
	for (auto& partition : m_partitions)
		VectorDelete(partition.tearStreams, [&](const std::string& k) { return k == _streamKey; });
}

void CCalculationSequence::DeleteStream(size_t _iPartition, size_t _iStream)
{
	if (_iPartition >= m_partitions.size()) return;
	VectorDelete(m_partitions[_iPartition].tearStreams, _iStream);
}

void CCalculationSequence::ShiftPartitionUp(size_t _iPartition)
{
	if (_iPartition == 0 || _iPartition >= m_partitions.size()) return;
	std::iter_swap(m_partitions.begin() + _iPartition, m_partitions.begin() + _iPartition - 1);
}

void CCalculationSequence::ShiftPartitionDown(size_t _iPartition)
{
	if (_iPartition == m_partitions.size() - 1 || _iPartition >= m_partitions.size()) return;
	std::iter_swap(m_partitions.begin() + _iPartition, m_partitions.begin() + _iPartition + 1);
}

void CCalculationSequence::ShiftModelUp(size_t _iPartition, size_t _iModel)
{
	if (_iPartition >= m_partitions.size()) return;
	if (_iModel == 0 || _iModel >= m_partitions[_iPartition].models.size()) return;
	std::iter_swap(m_partitions[_iPartition].models.begin() + _iModel, m_partitions[_iPartition].models.begin() + _iModel - 1);
}

void CCalculationSequence::ShiftModelDown(size_t _iPartition, size_t _iModel)
{
	if (_iPartition >= m_partitions.size()) return;
	if (_iModel == m_partitions[_iPartition].models.size() - 1 || _iModel >= m_partitions[_iPartition].models.size()) return;
	std::iter_swap(m_partitions[_iPartition].models.begin() + _iModel, m_partitions[_iPartition].models.begin() + _iModel + 1);
}

void CCalculationSequence::ShiftStreamUp(size_t _iPartition, size_t _iStream)
{
	if (_iPartition >= m_partitions.size()) return;
	if (_iStream == 0 || _iStream >= m_partitions[_iPartition].tearStreams.size()) return;
	std::iter_swap(m_partitions[_iPartition].tearStreams.begin() + _iStream, m_partitions[_iPartition].tearStreams.begin() + _iStream - 1);
}

void CCalculationSequence::ShiftStreamDown(size_t _iPartition, size_t _iStream)
{
	if (_iPartition >= m_partitions.size()) return;
	if (_iStream == m_partitions[_iPartition].tearStreams.size() - 1 || _iStream >= m_partitions[_iPartition].tearStreams.size()) return;
	std::iter_swap(m_partitions[_iPartition].tearStreams.begin() + _iStream, m_partitions[_iPartition].tearStreams.begin() + _iStream + 1);
}

std::vector<CBaseModel*> CCalculationSequence::PartitionModels(size_t _iPartition) const
{
	if (_iPartition >= m_partitions.size()) return {};
	std::vector<CBaseModel*> res{ m_partitions[_iPartition].models.size(), nullptr };
	for (size_t i = 0; i < m_partitions[_iPartition].models.size(); ++i)
		for (const auto& model : *m_models)
			if (model && model->GetModelKey() == m_partitions[_iPartition].models[i])
				res[i] = model;
	return res;
}

std::vector<CMaterialStream*> CCalculationSequence::PartitionTearStreams(size_t _iPartition) const
{
	if (_iPartition >= m_partitions.size()) return {};
	std::vector<CMaterialStream*> res{ m_partitions[_iPartition].tearStreams.size(), nullptr };;
	for (size_t i = 0; i < m_partitions[_iPartition].tearStreams.size(); ++i)
		for (CMaterialStream* stream : *m_streams)
			if (stream && stream->GetStreamKey() == m_partitions[_iPartition].tearStreams[i])
				res[i] = stream;
	return res;
}

CCalculationSequence::SPartition CCalculationSequence::Partition(size_t _iPartition) const
{
	if (_iPartition >= m_partitions.size()) return {};
	return SPartition{ PartitionModels(_iPartition), PartitionTearStreams(_iPartition) };
}

std::vector<CCalculationSequence::SPartition> CCalculationSequence::Partitions() const
{
	std::vector<SPartition> res(m_partitions.size());
	for (size_t i = 0; i < m_partitions.size(); ++i)
		res[i] = Partition(i);
	return res;
}

size_t CCalculationSequence::PartitionsNumber() const
{
	return m_partitions.size();
}

size_t CCalculationSequence::ModelsNumber(size_t _iPartition) const
{
	if (_iPartition >= m_partitions.size()) return 0;
	return m_partitions[_iPartition].models.size();
}

size_t CCalculationSequence::TearStreamsNumber(size_t _iPartition) const
{
	if (_iPartition >= m_partitions.size()) return 0;
	return m_partitions[_iPartition].tearStreams.size();
}

bool CCalculationSequence::IsEmpty() const
{
	return m_partitions.empty();
}

std::string CCalculationSequence::Check() const
{
	// sequence is not empty
	if (m_partitions.empty())
		return StrConst::Seq_ErrEmptySequence;
	// no empty entries
	for (const auto& partition : m_partitions)
	{
		// no empty models
		for (const auto& model : partition.models)
			if (model.empty())
				return StrConst::Seq_ErrEmptyModel;
		// no empty tear streams
		for (const auto& stream : partition.tearStreams)
			if (stream.empty())
				return StrConst::Seq_ErrEmptyStream;
	}

	for (const auto& model : *m_models)
		if (!IsModelInSequence(model->GetModelKey()))
			return StrConst::Seq_ErrMissingUnit(model->GetModelName());

	return {};
}

void CCalculationSequence::SaveToFile(CH5Handler& _h5Saver, const std::string& _path)
{
	if (!_h5Saver.IsValid()) return;

	// current version of save procedure
	_h5Saver.WriteAttribute(_path, StrConst::Seq_H5AttrSaveVersion, m_cnSaveVersion);

	_h5Saver.WriteAttribute(_path, StrConst::Seq_H5AttrPartitionsNum, static_cast<int>(m_partitions.size()));

	for (size_t i = 0; i < m_partitions.size(); ++i)
	{
		const std::string sPath = _h5Saver.CreateGroup(_path, StrConst::Seq_H5GroupPartitionName + std::to_string(i));
		_h5Saver.WriteData(sPath, StrConst::Seq_H5ModelsKeys, m_partitions[i].models);
		_h5Saver.WriteData(sPath, StrConst::Seq_H5TearStreamsKeys, m_partitions[i].tearStreams);
	}
}

void CCalculationSequence::LoadFromFile(CH5Handler& _h5Loader, const std::string& _path)
{
	m_partitions.clear();

	if (!_h5Loader.IsValid()) return;

	// load version of save procedure
	const int version = _h5Loader.ReadAttribute(_path, StrConst::Seq_H5AttrSaveVersion);
	if (version < m_cnSaveVersion) // old version
	{
		LoadFromFileOld(_h5Loader, _path);
		return;
	}

	const int partitionsNumber = _h5Loader.ReadAttribute(_path, StrConst::Seq_H5AttrPartitionsNum);
	m_partitions.resize(partitionsNumber);
	for (int i = 0; i < partitionsNumber; ++i)
	{
		const std::string sPath = _path + "/" + StrConst::Seq_H5GroupPartitionName + std::to_string(i);
		_h5Loader.ReadData(sPath, StrConst::Seq_H5ModelsKeys, m_partitions[i].models);
		_h5Loader.ReadData(sPath, StrConst::Seq_H5TearStreamsKeys, m_partitions[i].tearStreams);
	}
}

void CCalculationSequence::LoadFromFileOld(CH5Handler& _h5Loader, const std::string& _path)
{
	const std::string Flow_H5AttrStepsNumber = "CalcStepsNumber";
	const std::string Flow_H5GroupSteps      = "CalcSteps";
	const std::string Flow_H5GroupStepName   = "CalcStep";
	const std::string Flow_H5ModelsKeys      = "ModelsKeys";
	const std::string Flow_H5StreamsKeys     = "StreamsKeys";

	const int version = _h5Loader.ReadAttribute(_path, StrConst::Seq_H5AttrSaveVersion);
	if (version != 0) return; // wrong version

	const int partitionsNumber = _h5Loader.ReadAttribute("/", Flow_H5AttrStepsNumber);
	if (partitionsNumber == -1) return;

	m_partitions.resize(partitionsNumber);
	for (int i = 0; i < partitionsNumber; ++i)
	{
		const std::string sPath = "/" + Flow_H5GroupSteps + "/" + Flow_H5GroupStepName + std::to_string(i);
		_h5Loader.ReadData(sPath, Flow_H5ModelsKeys, m_partitions[i].models);
		_h5Loader.ReadData(sPath, Flow_H5StreamsKeys, m_partitions[i].tearStreams);
	}
}

bool CCalculationSequence::IsModelInSequence(const std::string& _modelKey) const
{
	for (const auto& partition : m_partitions)
		for (const auto& model : partition.models)
			if (model == _modelKey)
				return true;
	return false;
}

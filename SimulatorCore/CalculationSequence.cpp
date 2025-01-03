/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2025, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "CalculationSequence.h"
#include "UnitContainer.h"
#include "Stream.h"
#include "Phase.h"
#include "ContainerFunctions.h"
#include "DyssolStringConstants.h"

CCalculationSequence::CCalculationSequence(const std::vector<std::unique_ptr<CUnitContainer>>* _allModels, const std::vector<std::shared_ptr<CStream>>* _allStreams)
{
	m_models = _allModels;
	m_streams = _allStreams;
}

CCalculationSequence::CCalculationSequence(const CCalculationSequence& _other)
	: m_partitions{ _other.m_partitions }
{
	m_initialTearStreams.reserve(_other.m_initialTearStreams.size());
	for (const auto& vec : _other.m_initialTearStreams)
		m_initialTearStreams.push_back(DeepCopy(vec));
}

CCalculationSequence::CCalculationSequence(CCalculationSequence&& _other) noexcept
{
	swap(*this, _other);
}

CCalculationSequence& CCalculationSequence::operator=(CCalculationSequence _other)
{
	swap(*this, _other);
	return *this;
}

CCalculationSequence& CCalculationSequence::operator=(CCalculationSequence&& _other) noexcept
{
	CCalculationSequence tmp{ std::move(_other) };
	swap(*this, tmp);
	return *this;
}

void swap(CCalculationSequence& _first, CCalculationSequence& _second) noexcept
{
	using std::swap;
	// these two are set by the flowsheet
	//swap(_first.m_models            , _second.m_models);
	//swap(_first.m_streams           , _second.m_streams);
	swap(_first.m_partitions        , _second.m_partitions);
	swap(_first.m_initialTearStreams, _second.m_initialTearStreams);
}

void CCalculationSequence::SetPointers(const std::vector<std::unique_ptr<CUnitContainer>>* _allModels, const std::vector<std::shared_ptr<CStream>>* _allStreams)
{
	m_models = _allModels;
	m_streams = _allStreams;
}

void CCalculationSequence::Clear()
{
	m_partitions.clear();
	m_initialTearStreams.clear();
}

std::vector<std::vector<std::string>> CCalculationSequence::GetModelsKeys() const
{
	auto res = ReservedVector<std::vector<std::string>>(m_partitions.size());
	for (const auto& p : m_partitions)
		res.push_back(p.models);
	return res;
}

std::vector<std::vector<std::string>> CCalculationSequence::GetStreamsKeys() const
{
	auto res = ReservedVector<std::vector<std::string>>(m_partitions.size());
	for (const auto& p : m_partitions)
		res.push_back(p.tearStreams);
	return res;
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
	UpdateInitialStreams();
}

void CCalculationSequence::SetModel(size_t _iPartition, size_t _iModel, const std::string& _modelKey)
{
	if (_iPartition >= m_partitions.size()) return;
	if (_iModel >= m_partitions[_iPartition].models.size()) return;
	m_partitions[_iPartition].models[_iModel] = _modelKey;
	UpdateInitialStreams();
}

void CCalculationSequence::SetStream(size_t _iPartition, size_t _iStream, const std::string& _streamKey)
{
	if (_iPartition >= m_partitions.size()) return;
	if (_iStream >= m_partitions[_iPartition].tearStreams.size()) return;
	m_partitions[_iPartition].tearStreams[_iStream] = _streamKey;
	UpdateInitialStreams();
}

void CCalculationSequence::AddPartition(const std::vector<std::string>& _modelsKeys, const std::vector<std::string>& _tearStreamsKeys)
{
	m_partitions.emplace_back(SPartitionKeys{ _modelsKeys , _tearStreamsKeys });
	UpdateInitialStreams();
}

void CCalculationSequence::AddModel(size_t _iPartition, const std::string& _modelKey)
{
	if (_iPartition >= m_partitions.size()) return;
	m_partitions[_iPartition].models.push_back(_modelKey);
	UpdateInitialStreams();
}

void CCalculationSequence::AddStream(size_t _iPartition, const std::string& _streamKey)
{
	if (_iPartition >= m_partitions.size()) return;
	m_partitions[_iPartition].tearStreams.push_back(_streamKey);
	UpdateInitialStreams();
}

void CCalculationSequence::DeletePartition(size_t _iPartition)
{
	VectorDelete(m_partitions, _iPartition);
	UpdateInitialStreams();
}

void CCalculationSequence::DeleteModel(const std::string& _modelKey)
{
	// remove model from the calculation sequence
	for (auto& partition : m_partitions)
		VectorDelete(partition.models, [&](const std::string& k) { return k == _modelKey; });

	// remove empty partitions
	VectorDelete(m_partitions, [&](const SPartitionKeys& p) { return p.models.empty(); });

	UpdateInitialStreams();
}

void CCalculationSequence::DeleteModel(size_t _iPartition, size_t _iModel)
{
	if (_iPartition >= m_partitions.size()) return;
	VectorDelete(m_partitions[_iPartition].models, _iModel);
	UpdateInitialStreams();
}

void CCalculationSequence::DeleteStream(const std::string& _streamKey)
{
	for (auto& partition : m_partitions)
		VectorDelete(partition.tearStreams, [&](const std::string& k) { return k == _streamKey; });
	UpdateInitialStreams();
}

void CCalculationSequence::DeleteStream(size_t _iPartition, size_t _iStream)
{
	if (_iPartition >= m_partitions.size()) return;
	VectorDelete(m_partitions[_iPartition].tearStreams, _iStream);
	UpdateInitialStreams();
}

void CCalculationSequence::ShiftPartitionUp(size_t _iPartition)
{
	if (_iPartition == 0 || _iPartition >= m_partitions.size()) return;
	std::iter_swap(m_partitions.begin() + _iPartition, m_partitions.begin() + _iPartition - 1);
	UpdateInitialStreams();
}

void CCalculationSequence::ShiftPartitionDown(size_t _iPartition)
{
	if (_iPartition == m_partitions.size() - 1 || _iPartition >= m_partitions.size()) return;
	std::iter_swap(m_partitions.begin() + _iPartition, m_partitions.begin() + _iPartition + 1);
	UpdateInitialStreams();
}

void CCalculationSequence::ShiftModelUp(size_t _iPartition, size_t _iModel)
{
	if (_iPartition >= m_partitions.size()) return;
	if (_iModel == 0 || _iModel >= m_partitions[_iPartition].models.size()) return;
	std::iter_swap(m_partitions[_iPartition].models.begin() + _iModel, m_partitions[_iPartition].models.begin() + _iModel - 1);
	UpdateInitialStreams();
}

void CCalculationSequence::ShiftModelDown(size_t _iPartition, size_t _iModel)
{
	if (_iPartition >= m_partitions.size()) return;
	if (_iModel == m_partitions[_iPartition].models.size() - 1 || _iModel >= m_partitions[_iPartition].models.size()) return;
	std::iter_swap(m_partitions[_iPartition].models.begin() + _iModel, m_partitions[_iPartition].models.begin() + _iModel + 1);
	UpdateInitialStreams();
}

void CCalculationSequence::ShiftStreamUp(size_t _iPartition, size_t _iStream)
{
	if (_iPartition >= m_partitions.size()) return;
	if (_iStream == 0 || _iStream >= m_partitions[_iPartition].tearStreams.size()) return;
	std::iter_swap(m_partitions[_iPartition].tearStreams.begin() + _iStream, m_partitions[_iPartition].tearStreams.begin() + _iStream - 1);
	UpdateInitialStreams();
}

void CCalculationSequence::ShiftStreamDown(size_t _iPartition, size_t _iStream)
{
	if (_iPartition >= m_partitions.size()) return;
	if (_iStream == m_partitions[_iPartition].tearStreams.size() - 1 || _iStream >= m_partitions[_iPartition].tearStreams.size()) return;
	std::iter_swap(m_partitions[_iPartition].tearStreams.begin() + _iStream, m_partitions[_iPartition].tearStreams.begin() + _iStream + 1);
	UpdateInitialStreams();
}

std::vector<CUnitContainer*> CCalculationSequence::PartitionModels(size_t _iPartition) const
{
	if (_iPartition >= m_partitions.size()) return {};
	std::vector<CUnitContainer*> res{ m_partitions[_iPartition].models.size(), nullptr };
	for (size_t i = 0; i < m_partitions[_iPartition].models.size(); ++i)
		for (const auto& model : *m_models)
			if (model && model->GetKey() == m_partitions[_iPartition].models[i])
				res[i] = model.get();
	return res;
}

std::vector<CStream*> CCalculationSequence::PartitionTearStreams(size_t _iPartition) const
{
	if (_iPartition >= m_partitions.size()) return {};
	std::vector<CStream*> res{ m_partitions[_iPartition].tearStreams.size(), nullptr };
	for (size_t i = 0; i < m_partitions[_iPartition].tearStreams.size(); ++i)
		for (const auto& stream : *m_streams)
			if (stream && stream->GetKey() == m_partitions[_iPartition].tearStreams[i])
				res[i] = stream.get();
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
		if (!IsModelInSequence(model->GetKey()))
			return StrConst::Seq_ErrMissingUnit(model->GetName());

	return {};
}

void CCalculationSequence::UpdateInitialStreams()
{
	// create or remove initial tear streams
	m_initialTearStreams.resize(PartitionsNumber());
	for (size_t iPart = 0; iPart < PartitionsNumber(); ++iPart)
		// just remove excessive
		if (m_initialTearStreams[iPart].size() > TearStreamsNumber(iPart))
			m_initialTearStreams[iPart].resize(TearStreamsNumber(iPart));
		// create new as copies of real streams
		else
			for (size_t iStr = m_initialTearStreams[iPart].size(); iStr < TearStreamsNumber(iPart); ++iStr)
				m_initialTearStreams[iPart].emplace_back(std::make_unique<CStream>(*PartitionTearStreams(iPart)[iStr]));
	// update structures of the streams to set overall, phases, grids, etc.
	for (size_t iPart = 0; iPart < m_initialTearStreams.size(); ++iPart)
		for (size_t iStr = 0; iStr < m_initialTearStreams[iPart].size(); ++iStr)
			if (!CBaseStream::HaveSameStructure(*m_initialTearStreams[iPart][iStr], *PartitionTearStreams(iPart)[iStr]))
				m_initialTearStreams[iPart][iStr]->SetupStructure(PartitionTearStreams(iPart)[iStr]);
}

void CCalculationSequence::ClearInitialStreamsData()
{
	for (auto& partition : m_initialTearStreams)
		for (auto& stream : partition)
			stream->RemoveAllTimePoints();
}

void CCalculationSequence::CopyInitToTearStreams(double _timeWindow)
{
	for (size_t i = 0; i < PartitionsNumber(); ++i)
		for (size_t j = 0; j < TearStreamsNumber(i); ++j)
		{
			PartitionTearStreams(i)[j]->CopyFromStream(0.0, _timeWindow, m_initialTearStreams[i][j].get());
			if (PartitionTearStreams(i)[j]->GetAllTimePoints().empty()) // make sure, there is at least one time point in the stream
				PartitionTearStreams(i)[j]->AddTimePoint(0.0);
		}
}

void CCalculationSequence::CopyTearToInitStreams(double _timeWindow)
{
	for (size_t i = 0; i < PartitionsNumber(); ++i)
		for (size_t j = 0; j < TearStreamsNumber(i); ++j)
			m_initialTearStreams[i][j]->CopyFromStream(0.0, _timeWindow, PartitionTearStreams(i)[j]);
}

std::vector<std::vector<const CStream*>> CCalculationSequence::GetAllInitialStreams() const
{
	std::vector<std::vector<const CStream*>> res(PartitionsNumber());
	for (size_t i = 0; i < PartitionsNumber(); ++i)
		for (size_t j = 0; j < TearStreamsNumber(i); ++j)
			res[i].push_back(m_initialTearStreams[i][j].get());
	return res;
}

std::vector<std::vector<CStream*>> CCalculationSequence::GetAllInitialStreams()
{
	std::vector<std::vector<CStream*>> res(PartitionsNumber());
	for (size_t i = 0; i < PartitionsNumber(); ++i)
		for (size_t j = 0; j < TearStreamsNumber(i); ++j)
			res[i].push_back(m_initialTearStreams[i][j].get());
	return res;
}

void CCalculationSequence::UpdateCacheSettings(const SCacheSettings& _cache)
{
	for (auto& part : m_initialTearStreams)
		for (auto& str : part)
			str->SetCacheSettings(_cache);
}

void CCalculationSequence::UpdateToleranceSettings(const SToleranceSettings& _tolerance)
{
	for (auto& part : m_initialTearStreams)
		for (auto& str : part)
			str->SetToleranceSettings(_tolerance);
}

void CCalculationSequence::UpdateThermodynamicsSettings(const SThermodynamicsSettings& _settings)
{
	for (auto& part : m_initialTearStreams)
		for (auto& str : part)
			str->SetThermodynamicsSettings(_settings);
}

void CCalculationSequence::SaveToFile(CH5Handler& _h5Saver, const std::string& _path)
{
	if (!_h5Saver.IsValid()) return;

	// current version of save procedure
	_h5Saver.WriteAttribute(_path, StrConst::Seq_H5AttrSaveVersion, m_saveVersion);

	_h5Saver.WriteAttribute(_path, StrConst::Seq_H5AttrPartitionsNum, static_cast<int>(m_partitions.size()));

	for (size_t i = 0; i < m_partitions.size(); ++i)
	{
		const std::string sPath = _h5Saver.CreateGroup(_path, StrConst::Seq_H5GroupPartitionName + std::to_string(i));
		_h5Saver.WriteData(sPath, StrConst::Seq_H5ModelsKeys, m_partitions[i].models);
		_h5Saver.WriteData(sPath, StrConst::Seq_H5TearStreamsKeys, m_partitions[i].tearStreams);

		if (!m_partitions[i].tearStreams.empty())
		{
			const std::string groupInitTears = _h5Saver.CreateGroup(sPath, StrConst::Seq_H5GroupInitTears);
			for (size_t j = 0; j < m_partitions[i].tearStreams.size(); ++j)
			{
				m_initialTearStreams[i][j]->SaveToFile(_h5Saver, _h5Saver.CreateGroup(groupInitTears, StrConst::Seq_H5GroupInitTearName + std::to_string(j)));
			}
		}
	}
}

void CCalculationSequence::LoadFromFile(CH5Handler& _h5Loader, const std::string& _path)
{
	Clear();

	if (!_h5Loader.IsValid()) return;

	// load version of save procedure
	const int version = _h5Loader.ReadAttribute(_path, StrConst::Seq_H5AttrSaveVersion);
	if (version == 0) // old version
	{
		LoadFromFile_v0(_h5Loader, _path);
		return;
	}
	if (version == 1) // old version
	{
		LoadFromFile_v1(_h5Loader, _path);
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

	UpdateInitialStreams();

	for (int i = 0; i < partitionsNumber; ++i)
	{
		const std::string sPath = _path + "/" + StrConst::Seq_H5GroupPartitionName + std::to_string(i);
		if (!m_partitions[i].tearStreams.empty())
		{
			const std::string groupInitTears = sPath + "/" + StrConst::Seq_H5GroupInitTears + "/" + StrConst::Seq_H5GroupInitTearName;
			for (size_t j = 0; j < m_partitions[i].tearStreams.size(); ++j)
			{
				m_initialTearStreams[i][j]->LoadFromFile(_h5Loader, groupInitTears + std::to_string(j));
			}
		}
	}
}

void CCalculationSequence::LoadFromFile_v1(CH5Handler& _h5Loader, const std::string& _path)
{
	// TODO: move old StrConst names here.
	const int partitionsNumber = _h5Loader.ReadAttribute(_path, StrConst::Seq_H5AttrPartitionsNum);
	m_partitions.resize(partitionsNumber);
	for (int i = 0; i < partitionsNumber; ++i)
	{
		const std::string sPath = _path + "/" + StrConst::Seq_H5GroupPartitionName + std::to_string(i);
		_h5Loader.ReadData(sPath, StrConst::Seq_H5ModelsKeys, m_partitions[i].models);
		_h5Loader.ReadData(sPath, StrConst::Seq_H5TearStreamsKeys, m_partitions[i].tearStreams);
	}

	UpdateInitialStreams();

	const std::string initTearStreamsPath = "/" + std::string(StrConst::Flow_H5GroupInitTearStreams);
	for (size_t i = 0; i < m_initialTearStreams.size(); ++i)
	{
		const std::string partitionPath = initTearStreamsPath + "/" + StrConst::Flow_H5GroupPartitionName + std::to_string(i);
		for (size_t j = 0; j < m_initialTearStreams[i].size(); ++j)
		{
			const std::string streamPath = partitionPath + "/" + StrConst::Flow_H5GroupInitTearStreamName + std::to_string(j);
			m_initialTearStreams[i][j]->LoadFromFile(_h5Loader, streamPath);
		}
	}
}

void CCalculationSequence::LoadFromFile_v0(CH5Handler& _h5Loader, const std::string& _path)
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

	// initial streams are not available - just create them
	UpdateInitialStreams();
}

bool CCalculationSequence::IsModelInSequence(const std::string& _modelKey) const
{
	for (const auto& partition : m_partitions)
		for (const auto& model : partition.models)
			if (model == _modelKey)
				return true;
	return false;
}

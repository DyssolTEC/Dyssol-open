/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "StreamManager.h"
#include "DyssolStringConstants.h"

CStreamManager::CStreamManager(const CMaterialsDatabase& _materialsDB, const CDistributionsGrid& _grid, const std::vector<std::string>& _compounds, const std::vector<SOverallDescriptor>& _overall,
	const std::vector<SPhaseDescriptor>& _phases, const SCacheSettings& _cache, double& _minFraction) :
	m_materialsDB{ _materialsDB },
	m_grid{ _grid },
	m_compounds{ _compounds },
	m_overall{ _overall },
	m_phases{ _phases },
	m_cache{ _cache },
	m_minFraction{ _minFraction }
{
}

void CStreamManager::Initialize()
{
	// copy all time points for feeds
	for (size_t i = 0; i < m_feedsInit.size(); ++i)
		m_feedsWork[i]->CopyFromStream(0.0, m_feedsInit[i]->GetLastTimePoint(), m_feedsInit[i].get());
	// copy initial time point for holdups
	for (size_t i = 0; i < m_holdupsInit.size(); ++i)
		m_holdupsWork[i]->CopyFromHoldup(0.0, m_holdupsInit[i].get());
	// clear stored streams
	for (auto& stream : m_holdupsStored)
		stream->RemoveAllTimePoints();
	for (auto& stream : m_streamsStored)
		stream->RemoveAllTimePoints();
	// reset number of temporary objects
	m_tempHoldups = 0;
	m_tempStreams = 0;
	// reset stored time window
	m_timeBegStored = 0.0;
	m_timeEndStored = 0.0;
}

void CStreamManager::RemoveTemporary()
{
	for (size_t i = 0; i < m_tempHoldups; ++i)
		RemoveHoldup(m_holdupsWork.back()->GetName());
	for (size_t i = 0; i < m_tempStreams; ++i)
		RemoveStream(m_streamsWork.back()->GetName());
	m_tempHoldups = 0;
	m_tempStreams = 0;
}

void CStreamManager::ClearResults()
{
	RemoveTemporary();
	for (auto& s : m_feedsWork)		s->RemoveAllTimePoints();
	for (auto& s : m_holdupsWork)	s->RemoveAllTimePoints();
	for (auto& s : m_streamsWork)	s->RemoveAllTimePoints();
	for (auto& s : m_holdupsStored)	s->RemoveAllTimePoints();
	for (auto& s : m_streamsStored)	s->RemoveAllTimePoints();
}

void CStreamManager::Clear()
{
	for (auto* streams : m_allStreams)
		streams->clear();
	for (auto* holdups : m_allHoldups)
		holdups->clear();
}

CStream* CStreamManager::AddFeed(const std::string& _name)
{
	if (GetFeed(_name)) return nullptr; // already exists
	const std::string key = StringFunctions::GenerateUniqueKey(GetAllKeys(m_feedsWork));
	m_feedsInit.emplace_back(CreateObject<CStream>(key, _name));
	m_feedsWork.emplace_back(CreateObject<CStream>(key, _name));
	return m_feedsWork.back().get();
}

CStream* CStreamManager::GetFeed(const std::string& _name)
{
	return GetObject(m_feedsWork, _name);
}

std::vector<const CStream*> CStreamManager::GetFeedsInit() const
{
	return GetObjects(m_feedsInit);
}

std::vector<CStream*> CStreamManager::GetFeedsInit()
{
	return GetObjects(m_feedsInit);
}

std::vector<const CStream*> CStreamManager::GetFeeds() const
{
	return GetObjects(m_feedsWork);
}

std::vector<CStream*> CStreamManager::GetFeeds()
{
	return GetObjects(m_feedsWork);
}

void CStreamManager::RemoveFeed(const std::string& _name)
{
	RemoveObjects(m_feedsInit, _name);
	RemoveObjects(m_feedsWork, _name);
}

CHoldup* CStreamManager::AddHoldup(const std::string& _name)
{
	if (GetHoldup(_name)) return nullptr; // already exists
	const std::string key = StringFunctions::GenerateUniqueKey(GetAllKeys(m_holdupsWork));
	m_holdupsInit.emplace_back(CreateObject<CHoldup>(key, _name));
	m_holdupsWork.emplace_back(CreateObject<CHoldup>(key, _name));
	m_holdupsStored.emplace_back(CreateObject<CHoldup>(key, _name));
	return m_holdupsWork.back().get();
}

CHoldup* CStreamManager::GetHoldup(const std::string& _name)
{
	return GetObject(m_holdupsWork, _name);
}

std::vector<const CHoldup*> CStreamManager::GetHoldupInit() const
{
	return GetObjects(m_holdupsInit);
}

std::vector<CHoldup*> CStreamManager::GetHoldupInit()
{
	return GetObjects(m_holdupsInit);
}

std::vector<const CHoldup*> CStreamManager::GetHoldups() const
{
	return GetObjects(m_holdupsWork);
}

std::vector<CHoldup*> CStreamManager::GetHoldups()
{
	return GetObjects(m_holdupsWork);
}

void CStreamManager::RemoveHoldup(const std::string& _name)
{
	RemoveObjects(m_holdupsInit, _name);
	RemoveObjects(m_holdupsWork, _name);
	RemoveObjects(m_holdupsStored, _name);
}

CStream* CStreamManager::AddStream(const std::string& _name)
{
	if (GetStream(_name)) return nullptr; // already exists
	const std::string key = StringFunctions::GenerateUniqueKey(GetAllKeys(m_streamsWork));
	m_streamsWork.emplace_back(CreateObject<CHoldup>(key, _name));
	m_streamsStored.emplace_back(CreateObject<CHoldup>(key, _name));
	return m_streamsWork.back().get();
}

CStream* CStreamManager::GetStream(const std::string& _name)
{
	return GetObject(m_streamsWork, _name);
}

std::vector<const CStream*> CStreamManager::GetStreams() const
{
	return GetObjects(m_streamsWork);
}

std::vector<CStream*> CStreamManager::GetStreams()
{
	return GetObjects(m_streamsWork);
}

void CStreamManager::RemoveStream(const std::string& _name)
{
	RemoveObjects(m_streamsWork, _name);
	RemoveObjects(m_streamsStored, _name);
}

void CStreamManager::SaveState(double _timeBeg, double _timeEnd)
{
	for (size_t i = 0; i < m_holdupsWork.size(); ++i)
	{
		m_holdupsStored[i]->RemoveAllTimePoints();
		m_holdupsStored[i]->CopyFromHoldup(_timeBeg, _timeEnd, m_holdupsWork[i].get());
	}
	for (size_t i = 0; i < m_streamsWork.size(); ++i)
	{
		m_streamsStored[i]->RemoveAllTimePoints();
		m_streamsStored[i]->CopyFromStream(_timeBeg, _timeEnd, m_streamsWork[i].get());
	}
	m_timeBegStored = _timeBeg;
	m_timeEndStored = _timeEnd;
}

void CStreamManager::LoadState()
{
	for (size_t i = 0; i < m_holdupsWork.size(); ++i)
		m_holdupsWork[i]->CopyFromHoldup(m_timeBegStored, m_timeEndStored, m_holdupsStored[i].get());
	for (size_t i = 0; i < m_streamsWork.size(); ++i)
		m_streamsWork[i]->CopyFromStream(m_timeBegStored, m_timeEndStored, m_streamsStored[i].get());
}

void CStreamManager::AddCompound(const std::string& _compoundKey)
{
	for (auto& stream : AllObjects())
		stream->AddCompound(_compoundKey);
}

void CStreamManager::RemoveCompound(const std::string& _compoundKey)
{
	for (auto& stream : AllObjects())
		stream->RemoveCompound(_compoundKey);
}

void CStreamManager::AddPhase(EPhase _phase, const std::string& _name)
{
	for (auto& stream : AllObjects())
		stream->AddPhase(_phase, _name);
}

void CStreamManager::RemovePhase(EPhase _phase)
{
	for (auto& stream : AllObjects())
		stream->RemovePhase(_phase);
}

void CStreamManager::UpdateDistributionsGrid()
{
	for (auto& stream : AllObjects())
		stream->UpdateDistributionsGrid();
}

void CStreamManager::UpdateMinimumFraction()
{
	for (auto& stream : AllObjects())
		stream->SetMinimumFraction(m_minFraction);
}

void CStreamManager::UpdateCacheSettings()
{
	for (auto& stream : AllObjects())
		stream->SetCacheSettings(m_cache);
}

void CStreamManager::ReduceTimePoints(double _timeBeg, double _timeEnd, double _step)
{
	for (auto& s : m_feedsWork)		s->ReduceTimePoints(_timeBeg, _timeEnd, _step);
	for (auto& s : m_holdupsWork)	s->ReduceTimePoints(_timeBeg, _timeEnd, _step);
	for (auto& s : m_holdupsStored)	s->ReduceTimePoints(_timeBeg, _timeEnd, _step);
	for (auto& s : m_streamsWork)	s->ReduceTimePoints(_timeBeg, _timeEnd, _step);
	for (auto& s : m_streamsStored)	s->ReduceTimePoints(_timeBeg, _timeEnd, _step);
}

void CStreamManager::SetupStreamStructure(CBaseStream& _stream) const
{
	for (const auto& key : m_compounds)
		_stream.AddCompound(key);
	for (const auto& overall : m_overall)
		_stream.AddOverallProperty(overall.type, overall.name, overall.units);
	for (const auto& phase : m_phases)
		_stream.AddPhase(phase.state, phase.name);
	_stream.SetCacheSettings(m_cache);
	_stream.SetMinimumFraction(m_minFraction);
}

template <typename T>
T* CStreamManager::CreateObject(const std::string& _key, const std::string& _name) const
{
	auto* stream = new T{ _key, &m_materialsDB, &m_grid };
	stream.SetName(_name);
	SetupStreamStructure(*stream);
	return stream;
}

template <typename T>
T* CStreamManager::GetObject(const std::vector<std::unique_ptr<T>>& _streams, const std::string& _name)
{
	for (auto& stream : _streams)
		if (stream->GetName() == _name)
			return stream.get();
	return nullptr;
}

template <typename T>
std::vector<const T*> CStreamManager::GetObjects(const std::vector<std::unique_ptr<T>>& _streams) const
{
	std::vector<const T*> res;
	res.reserve(_streams.size());
	for (const auto& stream : _streams)
		res.push_back(stream.get());
	return res;
}

template <typename T>
std::vector<T*> CStreamManager::GetObjects(const std::vector<std::unique_ptr<T>>& _streams)
{
	std::vector<T*> res;
	res.reserve(_streams.size());
	for (const auto& stream : _streams)
		res.push_back(stream.get());
	return res;
}

template <typename T>
void CStreamManager::RemoveObjects(const std::vector<std::unique_ptr<T>>& _streams, const std::string& _name)
{
	_streams.erase(std::remove_if(_streams.begin(), _streams.end(), [&](std::unique_ptr<T> s) { return s->GetName() == _name; }), _streams.end());
}

template <typename T>
std::vector<std::string> CStreamManager::GetAllKeys(const std::vector<std::unique_ptr<T>>& _streams)
{
	std::vector<std::string> res;
	for (const auto& stream : _streams)
		res.push_back(stream->GetKey());
	return res;
}

std::vector<CBaseStream*> CStreamManager::AllObjects()
{
	std::vector<CBaseStream*> res;
	res.reserve(m_allStreams.size() + m_allHoldups.size());
	for (auto* streams : m_allStreams)
		for (auto& stream : *streams)
			res.push_back(stream.get());
	for (auto* holdups : m_allHoldups)
		for (auto& holdup : *holdups)
			res.push_back(holdup.get());
	return res;
}
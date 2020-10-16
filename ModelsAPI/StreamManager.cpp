/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "StreamManager.h"
#include "H5Handler.h"
#include "ContainerFunctions.h"
#include "DyssolStringConstants.h"

void CStreamManager::SetPointers(const CMaterialsDatabase* _materialsDB, const CDistributionsGrid* _grid, const std::vector<std::string>* _compounds, const std::vector<SOverallDescriptor>* _overall, const std::vector<SPhaseDescriptor>* _phases, const SCacheSettings* _cache, const SToleranceSettings* _tolerances)
{
	m_materialsDB = _materialsDB;
	m_grid        = _grid;
	m_compounds   = _compounds;
	m_overall     = _overall;
	m_phases      = _phases;
	m_cache       = _cache;
	m_tolerances  = _tolerances;
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

std::vector<const CHoldup*> CStreamManager::GetHoldupsInit() const
{
	return GetObjects(m_holdupsInit);
}

std::vector<CHoldup*> CStreamManager::GetHoldupsInit()
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
	m_streamsWork.emplace_back(CreateObject<CStream>(key, _name));
	m_streamsStored.emplace_back(CreateObject<CStream>(key, _name));
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

std::vector<const CBaseStream*> CStreamManager::GetAllInit() const
{
	const auto& feeds = GetObjects(m_feedsInit);
	const auto& holdups = GetObjects(m_holdupsInit);
	std::vector<const CBaseStream*> res;
	res.reserve(feeds.size() + holdups.size());
	for (const auto& feed : feeds)
		res.push_back(feed);
	for (const auto& holdup : holdups)
		res.push_back(holdup);
	return res;
}

std::vector<CBaseStream*> CStreamManager::GetAllInit()
{
	const auto& feeds = GetObjects(m_feedsInit);
	const auto& holdups = GetObjects(m_holdupsInit);
	std::vector<CBaseStream*> res;
	res.reserve(feeds.size() + holdups.size());
	for (const auto& feed : feeds)
		res.push_back(feed);
	for (const auto& holdup : holdups)
		res.push_back(holdup);
	return res;
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

void CStreamManager::AddOverallProperty(EOverall _property, const std::string& _name, const std::string& _units)
{
	for (auto& stream : AllObjects())
		stream->AddOverallProperty(_property, _name, _units);
}

void CStreamManager::RemoveOverallProperty(EOverall _property)
{
	for (auto& stream : AllObjects())
		stream->RemoveOverallProperty(_property);
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

void CStreamManager::UpdateToleranceSettings()
{
	for (auto& stream : AllObjects())
		stream->SetToleranceSettings(*m_tolerances);
}

void CStreamManager::UpdateCacheSettings()
{
	for (auto& stream : AllObjects())
		stream->SetCacheSettings(*m_cache);
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
	for (const auto& key : *m_compounds)
		_stream.AddCompound(key);
	for (const auto& overall : *m_overall)
		_stream.AddOverallProperty(overall.type, overall.name, overall.units);
	for (const auto& phase : *m_phases)
		_stream.AddPhase(phase.state, phase.name);
	_stream.SetCacheSettings(*m_cache);
	_stream.SetToleranceSettings(*m_tolerances);
}

void CStreamManager::SaveToFile(CH5Handler& _h5File, const std::string& _path) const
{
	if (!_h5File.IsValid()) return;

	// current version of save procedure
	_h5File.WriteAttribute(_path, StrConst::H5AttrSaveVersion, m_saveVersion);

	// save init and working streams
	SaveObjects(_h5File, _path, m_feedsInit,   StrConst::StrMngr_H5AttrFeedsInitNum,   StrConst::StrMngr_H5GroupFeedsInit,   StrConst::StrMngr_H5GroupFeedName,   StrConst::StrMngr_H5Names);
	SaveObjects(_h5File, _path, m_feedsWork,   StrConst::StrMngr_H5AttrFeedsWorkNum,   StrConst::StrMngr_H5GroupFeedsWork,   StrConst::StrMngr_H5GroupFeedName,   StrConst::StrMngr_H5Names);
	SaveObjects(_h5File, _path, m_holdupsInit, StrConst::StrMngr_H5AttrHoldupsInitNum, StrConst::StrMngr_H5GroupHoldupsInit, StrConst::StrMngr_H5GroupHoldupName, StrConst::StrMngr_H5Names);
	SaveObjects(_h5File, _path, m_holdupsWork, StrConst::StrMngr_H5AttrHoldupsWorkNum, StrConst::StrMngr_H5GroupHoldupsWork, StrConst::StrMngr_H5GroupHoldupName, StrConst::StrMngr_H5Names);
	SaveObjects(_h5File, _path, m_streamsWork, StrConst::StrMngr_H5AttrStreamsWorkNum, StrConst::StrMngr_H5GroupStreamsWork, StrConst::StrMngr_H5GroupStreamName, StrConst::StrMngr_H5Names);
}

void CStreamManager::LoadFromFile(const CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;

	// current version of save procedure
	//const int version = _h5File.ReadAttribute(_path, StrConst::H5AttrSaveVersion);

	// load init and working streams
	LoadObjects(_h5File, _path, m_feedsInit,   StrConst::StrMngr_H5AttrFeedsInitNum,   StrConst::StrMngr_H5GroupFeedsInit,   StrConst::StrMngr_H5GroupFeedName,   StrConst::StrMngr_H5Names);
	LoadObjects(_h5File, _path, m_feedsWork,   StrConst::StrMngr_H5AttrFeedsWorkNum,   StrConst::StrMngr_H5GroupFeedsWork,   StrConst::StrMngr_H5GroupFeedName,   StrConst::StrMngr_H5Names);
	LoadObjects(_h5File, _path, m_holdupsInit, StrConst::StrMngr_H5AttrHoldupsInitNum, StrConst::StrMngr_H5GroupHoldupsInit, StrConst::StrMngr_H5GroupHoldupName, StrConst::StrMngr_H5Names);
	LoadObjects(_h5File, _path, m_holdupsWork, StrConst::StrMngr_H5AttrHoldupsWorkNum, StrConst::StrMngr_H5GroupHoldupsWork, StrConst::StrMngr_H5GroupHoldupName, StrConst::StrMngr_H5Names);
	LoadObjects(_h5File, _path, m_streamsWork, StrConst::StrMngr_H5AttrStreamsWorkNum, StrConst::StrMngr_H5GroupStreamsWork, StrConst::StrMngr_H5GroupStreamName, StrConst::StrMngr_H5Names);

	// properly configure store streams
	for (size_t i = 0; i < m_holdupsStored.size(); ++i)
		m_holdupsStored[i]->SetupStructure(*m_holdupsWork[i]);
	for (size_t i = 0; i < m_streamsStored.size(); ++i)
		m_streamsStored[i]->SetupStructure(*m_streamsWork[i]);
}

void CStreamManager::LoadFromFile_v0(const CH5Handler& _h5File, const std::string& _path)
{
	const auto& Load = [&](const std::vector<std::unique_ptr<CHoldup>>& _holdups, const std::vector<std::unique_ptr<CStream>>& _feeds, const std::string& _group, const std::string& _subgroup, const std::string& _namespath)
	{
		std::vector<std::string> names;
		_h5File.ReadData(_path + "/" + _group, _namespath, names);
		std::vector<bool> holdupsReaded(names.size(), false);
		std::vector<bool> holdupsLoaded(_holdups.size(), false);
		std::vector<bool> feedsLoaded(_holdups.size(), false);
		const std::string holdupPath = _path + "/" + _group + "/" + _subgroup;
		// try to load holdups by names
		for (size_t iExist = 0; iExist < _holdups.size(); ++iExist)
			for (size_t iSaved = 0; iSaved < names.size(); ++iSaved)
				if (_holdups[iExist]->GetName() == names[iSaved])
				{
					_holdups[iExist]->LoadFromFile(_h5File, holdupPath + std::to_string(iSaved));
					holdupsReaded[iSaved] = true;
					holdupsLoaded[iExist] = true;
					break;
				}
		// try to load feeds by names
		for (size_t iExist = 0; iExist < _feeds.size(); ++iExist)
			for (size_t iSaved = 0; iSaved < names.size(); ++iSaved)
				if (_feeds[iExist]->GetName() == names[iSaved])
				{
					_feeds[iExist]->LoadFromFile(_h5File, holdupPath + std::to_string(iSaved));
					holdupsReaded[iSaved] = true;
					feedsLoaded[iExist] = true;
					break;
				}
		// load rest by positions
		for (size_t i = 0; i < _holdups.size(); ++i)
			if (!holdupsLoaded[i] && i < holdupsReaded.size() && !holdupsReaded[i])
			{
				const std::string name = _holdups[i]->GetName();
				_holdups[i]->LoadFromFile(_h5File, holdupPath + std::to_string(i));
				_holdups[i]->SetName(name);
			}
		for (size_t i = 0; i < _feeds.size(); ++i)
			if (!feedsLoaded[i] && i < holdupsReaded.size() && !holdupsReaded[i])
			{
				const std::string name = _feeds[i]->GetName();
				_feeds[i]->LoadFromFile(_h5File, holdupPath + std::to_string(i));
				_feeds[i]->SetName(name);
			}
	};

	if (!_h5File.IsValid()) return;

	// load init holdups and feeds
	Load(m_holdupsInit, m_feedsInit, StrConst::BUnit_H5GroupHoldups, StrConst::BUnit_H5GroupHoldupName, StrConst::BUnit_H5HoldupsNames);
	// load working holdups and feeds
	Load(m_holdupsWork, m_feedsWork, StrConst::BUnit_H5GroupHoldupsWork, StrConst::BUnit_H5GroupHoldupWorkName, StrConst::BUnit_H5WorkHoldupsNames);
	// load working material streams
	LoadObjects(_h5File, _path, m_streamsWork, StrConst::StrMngr_H5AttrStreamsWorkNum, StrConst::BUnit_H5GroupStreamsWork, StrConst::BUnit_H5GroupStreamWorkName, StrConst::BUnit_H5WorkStreamsNames);

	// properly configure store streams
	for (size_t i = 0; i < m_holdupsStored.size(); ++i)
		m_holdupsStored[i]->SetupStructure(*m_holdupsWork[i]);
	for (size_t i = 0; i < m_streamsStored.size(); ++i)
		m_streamsStored[i]->SetupStructure(*m_streamsWork[i]);
}

void CStreamManager::LoadFromFile_v00(const CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;

	// load holdups
	for (size_t i = 0; i < m_holdupsInit.size(); ++i)
		m_holdupsInit[i]->LoadFromFile(_h5File, _path + "/" + StrConst::BUnit_H5GroupHoldups + "/" + StrConst::BUnit_H5GroupHoldupName + std::to_string(i));
	for (size_t i = 0; i < m_feedsInit.size(); ++i)
		m_feedsInit[i]->LoadFromFile(_h5File, _path + "/" + StrConst::BUnit_H5GroupHoldups + "/" + StrConst::BUnit_H5GroupHoldupName + std::to_string(i));

	// load working holdups
	const size_t nWorkHoldups = _h5File.ReadAttribute(_path, StrConst::BUnit_H5AttrHoldupsWorkNum);
	if (nWorkHoldups != static_cast<size_t>(-1))
	{
		for (size_t i = 0; i < m_holdupsWork.size(); ++i)
			m_holdupsWork[i]->LoadFromFile(_h5File, _path + "/" + StrConst::BUnit_H5GroupHoldupsWork + "/" + StrConst::BUnit_H5GroupHoldupWorkName + std::to_string(i));
		for (size_t i = 0; i < m_feedsWork.size(); ++i)
			m_feedsWork[i]->LoadFromFile(_h5File, _path + "/" + StrConst::BUnit_H5GroupHoldupsWork + "/" + StrConst::BUnit_H5GroupHoldupWorkName + std::to_string(i));
	}

	// load working material streams
	for (const auto& name : GetAllNames(m_streamsWork))
		RemoveStream(name);
	const size_t nStreams = _h5File.ReadAttribute(_path, StrConst::BUnit_H5AttrStreamsWorkNum);
	if (nStreams != static_cast<size_t>(-1))
		for (size_t i = 0; i < nStreams; ++i)
			AddStream("TempName")->LoadFromFile(_h5File, _path + "/" + StrConst::BUnit_H5GroupStreamsWork + "/" + StrConst::BUnit_H5GroupStreamWorkName + std::to_string(i));

	// properly configure store streams
	for (size_t i = 0; i < m_holdupsStored.size(); ++i)
		m_holdupsStored[i]->SetupStructure(*m_holdupsWork[i]);
	for (size_t i = 0; i < m_streamsStored.size(); ++i)
		m_streamsStored[i]->SetupStructure(*m_streamsWork[i]);
}

template <typename T>
T* CStreamManager::CreateObject(const std::string& _key, const std::string& _name) const
{
	auto* stream = new T{ _key, &m_materialsDB, &m_grid, &m_compounds, &m_overall, &m_phases, &m_cache, &m_tolerances };
	stream->SetName(_name);
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
void CStreamManager::RemoveObjects(std::vector<std::unique_ptr<T>>& _streams, const std::string& _name)
{
	VectorDelete(_streams, [&](const std::unique_ptr<T>& s) { return s->GetName() == _name; });
}

template <typename T>
void CStreamManager::SaveObjects(CH5Handler& _h5File, const std::string& _path, const std::vector<std::unique_ptr<T>>& _streams, const std::string& _attribute, const std::string& _group, const std::string& _subgroup, const std::string& _namespath) const
{
	_h5File.WriteAttribute(_path, _attribute, static_cast<int>(_streams.size()));
	const std::string blockGroup = _h5File.CreateGroup(_path, _group);
	_h5File.WriteData(blockGroup, _namespath, GetAllNames(_streams));
	for (size_t i = 0; i < _streams.size(); ++i)
	{
		const std::string streamPath = _h5File.CreateGroup(blockGroup, _subgroup + std::to_string(i));
		_streams[i]->SaveToFile(_h5File, streamPath);
	}
}

template <typename T>
void CStreamManager::LoadObjects(const CH5Handler& _h5File, const std::string& _path, const std::vector<std::unique_ptr<T>>& _streams, const std::string& _attribute, const std::string& _group, const std::string& _subgroup, const std::string& _namespath)
{
	/* complex loading procedure with names, while users can change feeds/holdups/streams during the development of a unit.
	 * this approach allows to properly load even if the order or names of streams are changed by a developer. */
	std::vector<std::string> names;
	_h5File.ReadData(_path + "/" + _group, StrConst::StrMngr_H5Names, names);
	std::vector<bool> streamLoaded(_streams.size(), false);	// whether an existing stream is already loaded
	std::vector<bool> streamReaded(names.size(), false);	// whether a saved stream is already used to load an existing stream
	const std::string streamPath = _path + "/" + _group + "/" + _subgroup;
	// try to load by names
	for (size_t iExist = 0; iExist < _streams.size(); ++iExist)
		for (size_t iSaved = 0; iSaved < names.size(); ++iSaved)
			if (_streams[iExist]->GetName() == names[iSaved])
			{
				_streams[iExist]->LoadFromFile(_h5File, streamPath + std::to_string(iSaved));
				streamReaded[iSaved] = true;
				streamLoaded[iExist] = true;
				break;
			}
	// load rest by positions
	for (size_t i = 0; i < _streams.size(); ++i)
		if (!streamLoaded[i] && i < streamReaded.size() && !streamReaded[i])
		{
			const std::string name = _streams[i]->GetName();
			_streams[i]->LoadFromFile(_h5File, streamPath + std::to_string(i));
			_streams[i]->SetName(name);
		}
}

template <typename T>
std::vector<std::string> CStreamManager::GetAllKeys(const std::vector<std::unique_ptr<T>>& _streams) const
{
	std::vector<std::string> res;
	for (const auto& stream : _streams)
		res.push_back(stream->GetKey());
	return res;
}

template <typename T>
std::vector<std::string> CStreamManager::GetAllNames(const std::vector<std::unique_ptr<T>>& _streams) const
{
	std::vector<std::string> res;
	for (const auto& stream : _streams)
		res.push_back(stream->GetName());
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

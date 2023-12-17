/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "StreamManager.h"
#include "MaterialsDatabase.h"
#include "H5Handler.h"
#include "ContainerFunctions.h"
#include "DyssolStringConstants.h"

void CStreamManager::SetPointers(const CMaterialsDatabase* _materialsDB, const CMultidimensionalGrid* _grid,
	const std::vector<SOverallDescriptor>* _overall, const std::vector<SPhaseDescriptor>* _phases,
	const SCacheSettings* _cache, const SToleranceSettings* _tolerances, const SThermodynamicsSettings* _thermodynamics)
{
	m_materialsDB    = _materialsDB;
	m_grid           = _grid;
	m_overall        = _overall;
	m_phases         = _phases;
	m_cache          = _cache;
	m_tolerances     = _tolerances;
	m_thermodynamics = _thermodynamics;
}

void CStreamManager::SetMaterialsDatabase(const CMaterialsDatabase* _materialsDB)
{
	m_materialsDB = _materialsDB;
}

void CStreamManager::CopyUserData(const CStreamManager& _streams) const
{
	if (m_feedsInit.size() > _streams.m_feedsInit.size()) return;
	if (m_feedsWork.size() > _streams.m_feedsWork.size()) return;
	if (m_holdupsInit.size() > _streams.m_holdupsInit.size()) return;
	if (m_holdupsWork.size() > _streams.m_holdupsWork.size()) return;
	for (size_t i = 0; i < m_feedsInit.size(); ++i)
		m_feedsInit[i]->CopyFromStream(0.0, _streams.m_feedsInit[i]->GetLastTimePoint(), _streams.m_feedsInit[i].get());
	for (size_t i = 0; i < m_feedsWork.size(); ++i)
		m_feedsWork[i]->CopyFromStream(0.0, _streams.m_feedsWork[i]->GetLastTimePoint(), _streams.m_feedsWork[i].get());
	for (size_t i = 0; i < m_holdupsInit.size(); ++i)
		m_holdupsInit[i]->CopyFromHoldup(0.0, _streams.m_holdupsInit[i].get());
	for (size_t i = 0; i < m_holdupsWork.size(); ++i)
		m_holdupsWork[i]->CopyFromHoldup(0.0, _streams.m_holdupsWork[i].get());
}

void CStreamManager::CreateStructure()
{
	// store number of fixed objects
	m_nFixHoldups = m_holdupsWork.size();
	m_nFixStreams = m_streamsWork.size();
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
	// reset number of variable objects
	m_nVarHoldups = 0;
	m_nVarStreams = 0;
	// reset stored time window
	m_timeBegStored = 0.0;
	m_timeEndStored = 0.0;
}

void CStreamManager::PostInitialize()
{
	// store number of variable objects
	m_nVarHoldups = m_holdupsWork.size() - m_nFixHoldups;
	m_nVarStreams = m_streamsWork.size() - m_nFixStreams;
}

void CStreamManager::RemoveTemporary()
{
	const size_t nTempHoldups = m_holdupsWork.size() - m_nFixHoldups - m_nVarHoldups; // number of temporary holdups
	for (size_t i = 0; i < nTempHoldups; ++i)
		RemoveHoldup(m_holdupsWork[m_nFixHoldups + m_nVarHoldups]->GetName());
	const size_t nTempStreams = m_streamsWork.size() - m_nFixStreams - m_nVarStreams; // number of temporary streams
	for (size_t i = 0; i < nTempStreams; ++i)
		RemoveStream(m_streamsWork[m_nFixStreams + m_nVarStreams]->GetName());
}

void CStreamManager::RemoveVariable()
{
	while (m_nVarHoldups > 0)
		RemoveHoldup(m_holdupsWork[m_nFixHoldups]->GetName());
	while (m_nVarStreams > 0)
		RemoveStream(m_streamsWork[m_nFixStreams]->GetName());
}

void CStreamManager::ClearResults()
{
	RemoveTemporary();
	RemoveVariable();
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

size_t CStreamManager::GetFeedsNumber() const
{
	return m_feedsWork.size();
}

CStream* CStreamManager::AddFeed(const std::string& _name)
{
	if (GetFeed(_name)) return nullptr; // already exists
	const std::string key = StringFunctions::GenerateUniqueKey(GetAllKeys(m_feedsWork));
	m_feedsInit.emplace_back(CreateObject<CStream>(key, _name));
	m_feedsWork.emplace_back(CreateObject<CStream>(key, _name));
	return m_feedsWork.back().get();
}

const CStream* CStreamManager::GetFeed(const std::string& _name) const
{
	return GetObject(m_feedsWork, _name);
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

size_t CStreamManager::GetHoldupsNumber() const
{
	return m_holdupsWork.size();
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

const CHoldup* CStreamManager::GetHoldup(const std::string& _name) const
{
	return GetObject(m_holdupsWork, _name);
}

CHoldup* CStreamManager::GetHoldup(const std::string& _name)
{
	return GetObject(m_holdupsWork, _name);
}

std::vector<const CHoldup*> CStreamManager::GetHoldupsInit() const
{
	auto res = GetObjects(m_holdupsInit);	// get all init holdups
	res.resize(m_nFixHoldups);				// leave only fixed ones
	return res;
}

std::vector<CHoldup*> CStreamManager::GetHoldupsInit()
{
	auto res = GetObjects(m_holdupsInit);	// get all init holdups
	res.resize(m_nFixHoldups);				// leave only fixed ones
	return res;
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
	if (m_nFixHoldups > 0 && GetObjectIndex(m_holdupsWork, _name) < m_nFixHoldups                ) m_nFixHoldups--;
	if (m_nVarHoldups > 0 && GetObjectIndex(m_holdupsWork, _name) < m_nFixHoldups + m_nVarHoldups) m_nVarHoldups--;
	RemoveObjects(m_holdupsInit, _name);
	RemoveObjects(m_holdupsWork, _name);
	RemoveObjects(m_holdupsStored, _name);
}

size_t CStreamManager::GetStreamsNumber() const
{
	return m_streamsWork.size();
}

CStream* CStreamManager::AddStream(const std::string& _name)
{
	if (GetStream(_name)) return nullptr; // already exists
	const std::string key = StringFunctions::GenerateUniqueKey(GetAllKeys(m_streamsWork));
	m_streamsWork.emplace_back(CreateObject<CStream>(key, _name));
	m_streamsStored.emplace_back(CreateObject<CStream>(key, _name));
	return m_streamsWork.back().get();
}

const CStream* CStreamManager::GetStream(const std::string& _name) const
{
	return GetObject(m_streamsWork, _name);
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
	if (m_nFixStreams > 0 && GetObjectIndex(m_streamsWork, _name) < m_nFixStreams                ) m_nFixStreams--;
	if (m_nVarStreams > 0 && GetObjectIndex(m_streamsWork, _name) < m_nFixStreams + m_nVarStreams) m_nVarStreams--;
	RemoveObjects(m_streamsWork, _name);
	RemoveObjects(m_streamsStored, _name);
}

const CBaseStream* CStreamManager::GetObjectInit(size_t _index) const
{
	const auto& objects = GetAllInit();
	if (_index >= objects.size()) return nullptr;
	return objects[_index];
}

CBaseStream* CStreamManager::GetObjectInit(size_t _index)
{
	return const_cast<CBaseStream*>(static_cast<const CStreamManager&>(*this).GetObjectInit(_index));
}

const CBaseStream* CStreamManager::GetObjectInit(const std::string& _name) const
{
	for (const auto* object : GetAllInit())
		if (object->GetName() == _name)
			return object;
	return nullptr;
}

CBaseStream* CStreamManager::GetObjectInit(const std::string& _name)
{
	return const_cast<CBaseStream*>(static_cast<const CStreamManager&>(*this).GetObjectInit(_name));
}

const CBaseStream* CStreamManager::GetObjectWork(size_t _index) const
{
	const auto& objects = GetAllWork();
	if (_index >= objects.size()) return nullptr;
	return objects[_index];
}

CBaseStream* CStreamManager::GetObjectWork(size_t _index)
{
	return const_cast<CBaseStream*>(static_cast<const CStreamManager&>(*this).GetObjectWork(_index));
}

const CBaseStream* CStreamManager::GetObjectWork(const std::string& _name) const
{
	for (const auto* object : GetAllWork())
		if (object->GetName() == _name)
			return object;
	return nullptr;
}

CBaseStream* CStreamManager::GetObjectWork(const std::string& _name)
{
	return const_cast<CBaseStream*>(static_cast<const CStreamManager&>(*this).GetObjectWork(_name));
}

std::vector<const CBaseStream*> CStreamManager::GetAllInit() const
{
	const auto& feeds   = GetFeedsInit();
	const auto& holdups = GetHoldupsInit();
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
	const auto& feeds   = GetFeedsInit();
	const auto& holdups = GetHoldupsInit();
	std::vector<CBaseStream*> res;
	res.reserve(feeds.size() + holdups.size());
	for (const auto& feed : feeds)
		res.push_back(feed);
	for (const auto& holdup : holdups)
		res.push_back(holdup);
	return res;
}

std::vector<const CBaseStream*> CStreamManager::GetAllWork() const
{
	const auto& feeds   = GetObjects(m_feedsWork);
	const auto& holdups = GetObjects(m_holdupsWork);
	const auto& streams = GetObjects(m_streamsWork);
	std::vector<const CBaseStream*> res;
	res.reserve(feeds.size() + holdups.size() + streams.size());
	for (const auto& feed : feeds)
		res.push_back(feed);
	for (const auto& holdup : holdups)
		res.push_back(holdup);
	for (const auto& stream : streams)
		res.push_back(stream);
	return res;
}

std::vector<CBaseStream*> CStreamManager::GetAllWork()
{
	const auto& feeds   = GetObjects(m_feedsWork);
	const auto& holdups = GetObjects(m_holdupsWork);
	const auto& streams = GetObjects(m_streamsWork);
	std::vector<CBaseStream*> res;
	res.reserve(feeds.size() + holdups.size() + streams.size());
	for (const auto& feed : feeds)
		res.push_back(feed);
	for (const auto& holdup : holdups)
		res.push_back(holdup);
	for (const auto& stream : streams)
		res.push_back(stream);
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

void CStreamManager::UpdateGrid()
{
	for (auto& stream : AllObjects())
		stream->SetGrid(*m_grid);
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

void CStreamManager::UpdateThermodynamicsSettings()
{
	for (auto& stream : AllObjects())
		stream->SetThermodynamicsSettings(*m_thermodynamics);
}

void CStreamManager::ReduceTimePoints(double _timeBeg, double _timeEnd, double _step)
{
	for (auto& s : m_feedsWork)		s->ReduceTimePoints(_timeBeg, _timeEnd, _step);
	for (auto& s : m_holdupsWork)	s->ReduceTimePoints(_timeBeg, _timeEnd, _step);
	for (auto& s : m_holdupsStored)	s->ReduceTimePoints(_timeBeg, _timeEnd, _step);
	for (auto& s : m_streamsWork)	s->ReduceTimePoints(_timeBeg, _timeEnd, _step);
	for (auto& s : m_streamsStored)	s->ReduceTimePoints(_timeBeg, _timeEnd, _step);
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
	LoadObjects(_h5File, _path, m_feedsInit,   StrConst::StrMngr_H5AttrFeedsInitNum,   StrConst::StrMngr_H5GroupFeedsInit,   StrConst::StrMngr_H5GroupFeedName,   StrConst::StrMngr_H5Names, &CStreamManager::AddFeed);
	LoadObjects(_h5File, _path, m_feedsWork,   StrConst::StrMngr_H5AttrFeedsWorkNum,   StrConst::StrMngr_H5GroupFeedsWork,   StrConst::StrMngr_H5GroupFeedName,   StrConst::StrMngr_H5Names, &CStreamManager::AddFeed);
	LoadObjects(_h5File, _path, m_holdupsInit, StrConst::StrMngr_H5AttrHoldupsInitNum, StrConst::StrMngr_H5GroupHoldupsInit, StrConst::StrMngr_H5GroupHoldupName, StrConst::StrMngr_H5Names, &CStreamManager::AddHoldup);
	LoadObjects(_h5File, _path, m_holdupsWork, StrConst::StrMngr_H5AttrHoldupsWorkNum, StrConst::StrMngr_H5GroupHoldupsWork, StrConst::StrMngr_H5GroupHoldupName, StrConst::StrMngr_H5Names, &CStreamManager::AddHoldup);
	LoadObjects(_h5File, _path, m_streamsWork, StrConst::StrMngr_H5AttrStreamsWorkNum, StrConst::StrMngr_H5GroupStreamsWork, StrConst::StrMngr_H5GroupStreamName, StrConst::StrMngr_H5Names, &CStreamManager::AddStream);

	// properly configure store streams
	for (size_t i = 0; i < m_holdupsStored.size(); ++i)
		m_holdupsStored[i]->SetupStructure(m_holdupsWork[i].get());
	for (size_t i = 0; i < m_streamsStored.size(); ++i)
		m_streamsStored[i]->SetupStructure(m_streamsWork[i].get());

	// store number of variable objects
	m_nVarHoldups = m_holdupsWork.size() - m_nFixHoldups;
	m_nVarStreams = m_streamsWork.size() - m_nFixStreams;
}

void CStreamManager::LoadFromFile_v0(const CH5Handler& _h5File, const std::string& _path)
{
	const auto& Load = [&](const std::vector<std::unique_ptr<CHoldup>>& _holdups, const std::vector<std::unique_ptr<CStream>>& _feeds, const std::string& _group, const std::string& _subgroup, const std::string& _namespath)
	{
		std::vector<std::string> names;
		_h5File.ReadData(_path + "/" + _group, _namespath, names);
		std::vector<bool> holdupsReaded(names.size(), false);
		std::vector<bool> holdupsLoaded(_holdups.size(), false);
		std::vector<bool> feedsLoaded(_feeds.size(), false);
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
		// load the rest, if any, as variable holdups
		for (size_t i = 0; i < names.size(); ++i)
		{
			if (!holdupsReaded[i])
			{
				AddHoldup(names[i]);
				_holdups.back()->LoadFromFile(_h5File, holdupPath + std::to_string(i));
			}
		}
	};

	if (!_h5File.IsValid()) return;

	// load init holdups and feeds
	Load(m_holdupsInit, m_feedsInit, StrConst::BUnit_H5GroupHoldups, StrConst::BUnit_H5GroupHoldupName, StrConst::BUnit_H5HoldupsNames);
	// load working holdups and feeds
	Load(m_holdupsWork, m_feedsWork, StrConst::BUnit_H5GroupHoldupsWork, StrConst::BUnit_H5GroupHoldupWorkName, StrConst::BUnit_H5WorkHoldupsNames);
	// load working material streams
	LoadObjects(_h5File, _path, m_streamsWork, StrConst::StrMngr_H5AttrStreamsWorkNum, StrConst::BUnit_H5GroupStreamsWork, StrConst::BUnit_H5GroupStreamWorkName, StrConst::BUnit_H5WorkStreamsNames, &CStreamManager::AddStream);

	// properly configure store streams
	for (size_t i = 0; i < m_holdupsStored.size(); ++i)
		m_holdupsStored[i]->SetupStructure(m_holdupsWork[i].get());
	for (size_t i = 0; i < m_streamsStored.size(); ++i)
		m_streamsStored[i]->SetupStructure(m_streamsWork[i].get());

	// store number of variable objects
	m_nVarHoldups = m_holdupsWork.size() - m_nFixHoldups;
	m_nVarStreams = m_streamsWork.size() - m_nFixStreams;
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
		m_holdupsStored[i]->SetupStructure(m_holdupsWork[i].get());
	for (size_t i = 0; i < m_streamsStored.size(); ++i)
		m_streamsStored[i]->SetupStructure(m_streamsWork[i].get());

	// store number of variable objects
	m_nVarHoldups = m_holdupsWork.size() - m_nFixHoldups;
	m_nVarStreams = m_streamsWork.size() - m_nFixStreams;
}

template <typename T>
T* CStreamManager::CreateObject(const std::string& _key, const std::string& _name) const
{
	T* stream;
	if (m_materialsDB && m_grid && m_overall && m_phases && m_cache && m_tolerances && m_thermodynamics) // should be usually the case
		stream = new T{ _key, m_materialsDB, *m_grid, m_overall, m_phases, m_cache, m_tolerances, m_thermodynamics };
	else // for the case of old models, which add holdups in constructor - just create a placeholder, since the loading of the model will be discarded
		stream = new T{};
	stream->SetName(_name);
	return stream;
}

template <typename T>
const T* CStreamManager::GetObject(const std::vector<std::unique_ptr<T>>& _streams, const std::string& _name) const
{
	for (auto& stream : _streams)
		if (stream->GetName() == _name)
			return stream.get();
	return nullptr;
}

template <typename T>
T* CStreamManager::GetObject(const std::vector<std::unique_ptr<T>>& _streams, const std::string& _name)
{
	return const_cast<T*>(static_cast<const CStreamManager&>(*this).GetObject(_streams, _name));
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
size_t CStreamManager::GetObjectIndex(std::vector<std::unique_ptr<T>>& _streams, const std::string& _name)
{
	return VectorFind(_streams, [&](const std::unique_ptr<T>& s) { return s->GetName() == _name; });
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
void CStreamManager::LoadObjects(const CH5Handler& _h5File, const std::string& _path, const std::vector<std::unique_ptr<T>>& _streams, const std::string& _attribute, const std::string& _group, const std::string& _subgroup, const std::string& _namespath, AddObjectFun<T> _addObjectFun)
{
	/* complex loading procedure with names, while users can change feeds/holdups/streams during the development of a unit.
	 * this approach allows to properly load even if the order or names of streams are changed by a developer. */
	std::vector<std::string> names;
	_h5File.ReadData(_path + "/" + _group, StrConst::StrMngr_H5Names, names);
	std::vector<bool> streamLoaded(_streams.size(), false);	// whether an existing stream is already loaded
	std::vector<bool> streamReaded(names.size(), false);	// whether a saved stream is already used to load an existing stream
	const std::string streamPath = _path + "/" + _group + "/" + _subgroup;
	// try to load fixed by names
	for (size_t iExist = 0; iExist < _streams.size(); ++iExist)
		for (size_t iSaved = 0; iSaved < names.size(); ++iSaved)
			if (_streams[iExist]->GetName() == names[iSaved])
			{
				_streams[iExist]->LoadFromFile(_h5File, streamPath + std::to_string(iSaved));
				streamReaded[iSaved] = true;
				streamLoaded[iExist] = true;
				break;
			}
	// load rest fixed by positions
	for (size_t i = 0; i < _streams.size(); ++i)
		if (!streamLoaded[i] && i < streamReaded.size() && !streamReaded[i])
		{
			const std::string name = _streams[i]->GetName();
			_streams[i]->LoadFromFile(_h5File, streamPath + std::to_string(i));
			_streams[i]->SetName(name);
		}
	// load the rest, if any, as variable objects
	for (size_t i = 0; i < names.size(); ++i)
	{
		if (!streamReaded[i])
		{
			(this->*_addObjectFun)(names[i]); // add a corresponding object as variable
			_streams.back()->LoadFromFile(_h5File, streamPath + std::to_string(i));
		}
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

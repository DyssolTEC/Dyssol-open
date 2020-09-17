/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "Flowsheet.h"
#include "FlowsheetParameters.h"
#include "Topology.h"
#include "Stream.h"
#include "DyssolStringConstants.h"
#include "FileSystem.h"
#include "DyssolUtilities.h"
#include "DistributionsGrid.h"
#include "Phase.h"
#include <fstream>

const unsigned CFlowsheet::m_cnSaveVersion = 3;

CFlowsheet::CFlowsheet() :
	m_pParams{ new CFlowsheetParameters() },
	m_pMaterialsDatabase{ nullptr },
	m_pModelsManager{ nullptr },
	m_topologyModified{ false },
	m_pDistributionsGrid{ new CDistributionsGrid() },
	m_dSimulationTime{ 0 }
{
	InitializeFlowsheet();
}

CFlowsheet::~CFlowsheet()
{
	for (size_t i = 0; i < m_vpModels.size(); ++i)
		if (m_vpModels[i])
			delete m_vpModels[i];
	for (size_t i = 0; i < m_vpStreams.size(); ++i)
		if (m_vpStreams[i])
			delete m_vpStreams[i];

	delete m_pDistributionsGrid;
	m_pDistributionsGrid = nullptr;

	delete m_pParams;
}

void CFlowsheet::InitializeFlowsheet()
{
	m_pDistributionsGrid->AddDimension(DISTR_COMPOUNDS, EGridEntry::GRID_SYMBOLIC, std::vector<double>(), std::vector<std::string>());
	m_dSimulationTime = DEFAULT_SIMULATION_TIME;
}

void CFlowsheet::Clear()
{
	// remove all models
	for (unsigned i = 0; i < m_vpModels.size(); i++)
		delete m_vpModels[i];
	m_vpModels.clear();

	// remove all streams
	for (unsigned i = 0; i < m_vpStreams.size(); i++)
		delete m_vpStreams[i];
	m_vpStreams.clear();

	// remove all initial tear streams
	m_vvInitTearStreams.clear();

	// clear calculation sequence
	m_calculationSequence.Clear();

	// clear compounds
	m_vCompoundsKeys.clear();

	// clear phases
	m_vPhasesNames.clear();
	m_vPhasesSOA.clear();

	// clear distributions grid
	m_pDistributionsGrid->Clear();

	// set parameters to default
	m_pParams->Initialize();

	// initialize flowsheet with default values
	InitializeFlowsheet();

	// set the topology is modified
	SetTopologyModified(true);
}

std::string CFlowsheet::Initialize()
{
	// set pointers to the streams to all unit ports
	SetStreamsToPorts();

	// check that ports of all units are connected to some streams
	std::string err = CheckConnections();
	if (!err.empty())
		return err;

	// determine and check calculation sequence
	err = m_calculationSequence.Check();
	if (!err.empty() || m_topologyModified)
	{
		AnalyzeTopology();
		err = m_calculationSequence.Check();
		if (!err.empty())
			return err;
	}
	SetTopologyModified(false);

	// create streams needed to initialize tear streams
	CreateInitTearStreams();

	// check that all models has assigned units
	for (auto& model : m_vpModels)
		if (model->GetUnitKey().empty())
			return StrConst::Flow_ErrEmptyUnit(model->GetModelName());

	// load and check external solvers in units
	for (auto& model : m_vpModels)
	{
		err = model->InitializeExternalSolvers();
		if (!err.empty())
			return err;
	}

	// check compounds
	if (m_pMaterialsDatabase->CompoundsNumber() == 0)
		return StrConst::Flow_ErrEmptyMDB;
	if (m_vCompoundsKeys.empty())
		return StrConst::Flow_ErrNoCompounds;
	for (const auto& key : m_vCompoundsKeys)
		if (!m_pMaterialsDatabase->GetCompound(key))
			return StrConst::Flow_ErrWrongCompound(key);

	// check phases
	if (m_vPhasesNames.empty())
		return StrConst::Flow_ErrNoPhases;

	size_t solidCounter = 0;
	for (unsigned i : m_vPhasesSOA)
		if (i == SOA_SOLID)
			solidCounter++;
	if (solidCounter > 1)
		return StrConst::Flow_ErrMultSolids;

	size_t vaporCounter = 0;
	for (unsigned i : m_vPhasesSOA)
		if (i == SOA_VAPOR)
			vaporCounter++;
	if (vaporCounter > 1)
		return StrConst::Flow_ErrMultVapors;

	// check if this is an empty feed
	for (const auto& model : m_vpModels)
	{
		bool bNoInPorts = true;
		for (const auto& port : model->GetUnitPorts())
			if (port.nType == INPUT_PORT)
			{
				bNoInPorts = false;
				break;
			}
		if (bNoInPorts && !model->GetUnitPorts().empty()) // feed
			for (size_t j = 0; j < model->GetHoldupsCount(); ++j)
				if (model->GetHoldupInit(j)->GetAllTimePoints().empty())
					return StrConst::Flow_ErrEmptyFeed(model->GetModelName());
	}

	// set parameters of models
	for (auto& m : m_vpModels)
	{
		m->SetAbsTolerance(m_pParams->absTol);
		m->SetRelTolerance(m_pParams->relTol);
		m->SetMinimalFraction(m_pParams->minFraction);
	}

	return "";
}

std::string CFlowsheet::CheckConnections()
{
	for (const auto& model : m_vpModels)
		for (const auto& port : model->GetUnitPorts())
			if (!port.pStream)
				return StrConst::Flow_ErrUnconnectedPorts(model->GetModelName());

	for (const auto& stream : m_vpStreams)
	{
		unsigned cIn = 0, cOut = 0;
		for (const auto& model : m_vpModels)
			for (const auto& port : model->GetUnitPorts())
			{
				if (port.nType == OUTPUT_PORT && port.sStreamKey == stream->GetKey())
					cOut++;
				if (port.nType == INPUT_PORT && port.sStreamKey == stream->GetKey())
					cIn++;
			}
		if (cIn != cOut || cIn != 1 && cIn != 0)
			return StrConst::Flow_ErrWrongStreams(stream->GetName());
	}

	return {};
}

void CFlowsheet::SetStreamsToPorts()
{
	for (unsigned i = 0; i < m_vpModels.size(); i++)
	{
		std::vector<sPortStruct> vUnitPorts = m_vpModels[i]->GetUnitPorts();
		for (unsigned j = 0; j < vUnitPorts.size(); j++)
			m_vpModels[i]->SetPortStream(j, GetStream(vUnitPorts[j].sStreamKey));
	}
}

bool CFlowsheet::AnalyzeTopology()
{
	CTopology top(m_vpModels.size());
	for (size_t iModelFr = 0; iModelFr < m_vpModels.size(); ++iModelFr)
		for (const auto& srcPort : m_vpModels[iModelFr]->GetUnitPorts())
			if (srcPort.nType == OUTPUT_PORT)
				for (size_t iModelTo = 0; iModelTo < m_vpModels.size(); ++iModelTo)
					for (const auto& dstPort : m_vpModels[iModelTo]->GetUnitPorts())
						if (dstPort.nType == INPUT_PORT && dstPort.sStreamKey == srcPort.sStreamKey)
							top.AddEdge(iModelFr, iModelTo);

	std::vector<std::vector<size_t>> iModels;                               // indices of models for each partition
	std::vector<std::vector<std::pair<size_t, size_t>>> iModelsConnections; // indices of models connected by tear streams for each partition
	const bool bRes = top.Analyse(iModels, iModelsConnections);

	// returns index of a stream connecting two models
	const auto ConnectionStreamKey = [&](size_t _iSrcModel, size_t _iDstModel) -> std::string
	{
		for (const auto& srcPort : m_vpModels[_iSrcModel]->GetUnitPorts())
			for (const auto& dstPort : m_vpModels[_iDstModel]->GetUnitPorts())
				if (srcPort.sStreamKey == dstPort.sStreamKey)
					return srcPort.pStream->GetKey();
		return "";
	};

	// gather keys of models
	std::vector<std::vector<std::string>> modelsKeys; // keys of models for each partition
	for (auto& partition : iModels)
	{
		modelsKeys.emplace_back();
		for (size_t i : partition)
			modelsKeys.back().push_back(m_vpModels[i]->GetModelKey());
	}

	// gather keys of tear streams
	std::vector<std::vector<std::string>> streamsKeys; // keys of tear streams for each partition
	for (auto& partition : iModelsConnections)
	{
		streamsKeys.emplace_back();
		for (auto& iSrc_iDst : partition)
			streamsKeys.back().push_back(ConnectionStreamKey(iSrc_iDst.first, iSrc_iDst.second));
	}

	m_calculationSequence.SetSequence(modelsKeys, streamsKeys);

	return bRes;
}

void CFlowsheet::CreateInitTearStreams()
{
	// create initial tear streams
	m_vvInitTearStreams.resize(m_calculationSequence.PartitionsNumber());
	for (size_t i = 0; i < m_calculationSequence.PartitionsNumber(); ++i)
		if (m_calculationSequence.TearStreamsNumber(i))
			m_vvInitTearStreams[i].resize(m_calculationSequence.TearStreamsNumber(i), CStream(*m_calculationSequence.PartitionTearStreams(i).front()));
	for (auto& partition : m_vvInitTearStreams)
		for (auto& stream : partition)
			if (!CBaseStream::HaveSameStructure(stream, *m_vpStreams.front()))	// TODO: check whether it is needed
				stream.SetupStructure(*m_vpStreams.front());
}

void CFlowsheet::SetTopologyModified(bool _modified)
{
	m_topologyModified = _modified;
}

void CFlowsheet::ClearSimulationResults()
{
	for (unsigned i = 0; i < m_vpStreams.size(); ++i)
		m_vpStreams[i]->RemoveAllTimePoints();
	for (unsigned i = 0; i < m_vpModels.size(); ++i)
		m_vpModels[i]->ClearSimulationResults();
}

void CFlowsheet::AddPhase(const std::string& _sName, unsigned _nAggregationState)
{
	// add to local array
	m_vPhasesNames.push_back(_sName);
	m_vPhasesSOA.push_back(_nAggregationState);

	// add to streams
	for (unsigned i = 0; i < m_vpStreams.size(); ++i)
		m_vpStreams[i]->AddPhase(PhaseSOA2EPhase(_nAggregationState), _sName);
	// add to models
	for (unsigned i = 0; i < m_vpModels.size(); ++i)
		m_vpModels[i]->AddPhase(_sName, _nAggregationState);
	// add to initial tear streams
	for (auto& part : m_vvInitTearStreams)
		for (auto& str : part)
			str.AddPhase(PhaseSOA2EPhase(_nAggregationState), _sName);
}

void CFlowsheet::RemovePhase(unsigned _nIndex)
{
	if (_nIndex < m_vPhasesNames.size())
	{
		// remove from local array
		m_vPhasesNames.erase(m_vPhasesNames.begin() + _nIndex);
		m_vPhasesSOA.erase(m_vPhasesSOA.begin() + _nIndex);

		// remove from streams
		for (unsigned i = 0; i < m_vpStreams.size(); ++i)
			m_vpStreams[i]->RemovePhase(PhaseSOA2EPhase(m_vPhasesSOA[_nIndex]));
		// remove from models
		for (unsigned i = 0; i < m_vpModels.size(); ++i)
			m_vpModels[i]->RemovePhase(_nIndex);
		// remove from  initial tear streams
		for (auto& part : m_vvInitTearStreams)
			for (auto& str : part)
				str.RemovePhase(PhaseSOA2EPhase(m_vPhasesSOA[_nIndex]));
	}
}

void CFlowsheet::ChangePhase(unsigned _nIndex, const std::string& _sName, unsigned _nAggregationState)
{
	if (_nIndex < m_vPhasesNames.size())
	{
		// change in local array
		m_vPhasesNames[_nIndex] = _sName;
		m_vPhasesSOA[_nIndex] = _nAggregationState;

		// change in streams
		for (unsigned i = 0; i < m_vpStreams.size(); ++i)
		{
			m_vpStreams[i]->RemovePhase(PhaseSOA2EPhase(m_vPhasesSOA[_nIndex]));
			m_vpStreams[i]->AddPhase(PhaseSOA2EPhase(m_vPhasesSOA[_nIndex]), _sName);
		}
		// change in models
		for (unsigned i = 0; i < m_vpModels.size(); ++i)
			m_vpModels[i]->ChangePhase(_nIndex, _sName, _nAggregationState);
		// change in initial tear streams
		for (auto& part : m_vvInitTearStreams)
			for (auto& str : part)
			{
				str.RemovePhase(PhaseSOA2EPhase(m_vPhasesSOA[_nIndex]));
				str.AddPhase(PhaseSOA2EPhase(m_vPhasesSOA[_nIndex]), _sName);
			}
	}
}

unsigned CFlowsheet::GetPhasesNumber()
{
	return (unsigned)m_vPhasesNames.size();
}

const std::vector<std::string>& CFlowsheet::GetPhasesNames() const
{
	return m_vPhasesNames;
}

std::string CFlowsheet::GetPhaseName(unsigned _nIndex)
{
	if (_nIndex < m_vPhasesNames.size())
		return m_vPhasesNames[_nIndex];
	return "";
}

std::vector<unsigned>* CFlowsheet::GetPhasesAggregationStates()
{
	return &m_vPhasesSOA;
}

int CFlowsheet::GetPhaseAggregationState(unsigned _nIndex)
{
	if (_nIndex < m_vPhasesSOA.size())
		return m_vPhasesSOA[_nIndex];
	return -1;
}

void CFlowsheet::ClearPhases()
{
	while (GetPhasesNumber() > 0)
		RemovePhase(0);
}

bool CFlowsheet::IsPhaseDefined(unsigned _nSOA)
{
	for (unsigned i = 0; i < m_vPhasesSOA.size(); ++i)
		if (m_vPhasesSOA[i] == _nSOA)
			return true;
	return false;
}

int CFlowsheet::GetPhaseIndex(unsigned _nSOA)
{
	for (unsigned i = 0; i < m_vPhasesSOA.size(); ++i)
		if (m_vPhasesSOA[i] == _nSOA)
			return i;
	return -1;
}

void CFlowsheet::SetMaterialsDatabase(CMaterialsDatabase* _pNewDatabase)
{
	for (unsigned i = 0; i < m_vpStreams.size(); ++i)
		m_vpStreams[i]->SetMaterialsDatabase(_pNewDatabase);
	for (unsigned i = 0; i < m_vpModels.size(); ++i)
		m_vpModels[i]->SetMaterialsDatabase(_pNewDatabase);
	for (auto& part : m_vvInitTearStreams)
		for (auto& str : part)
			str.SetMaterialsDatabase(_pNewDatabase);

	m_pMaterialsDatabase = _pNewDatabase;
}

const CMaterialsDatabase* CFlowsheet::GetMaterialsDatabase() const
{
	return m_pMaterialsDatabase;
}

void CFlowsheet::SetDistributionsGrid()
{
	// set to all material streams
	for (unsigned i = 0; i < m_vpStreams.size(); ++i)
	{
		m_vpStreams[i]->SetGrid(m_pDistributionsGrid);
		m_vpStreams[i]->UpdateDistributionsGrid();
	}
	// set to all holdups
	for (unsigned i = 0; i < m_vpModels.size(); ++i)
		m_vpModels[i]->SetDistributionsGrid(m_pDistributionsGrid);
	// set to all initial tear streams
	for (auto& part : m_vvInitTearStreams)
		for (auto& str : part)
		{
			str.SetGrid(m_pDistributionsGrid);
			str.UpdateDistributionsGrid();
		}
}

CModelsManager* CFlowsheet::GetModelsManager() const
{
	return m_pModelsManager;
}

void CFlowsheet::SetModelsManager(CModelsManager* _pModelsManager)
{
	m_pModelsManager = _pModelsManager;
}

const CCalculationSequence* CFlowsheet::GetCalculationSequence() const
{
	return &m_calculationSequence;
}

CCalculationSequence* CFlowsheet::GetCalculationSequence()
{
	return &m_calculationSequence;
}

void CFlowsheet::SetSimulationTime(double _dSimulationTime)
{
	m_dSimulationTime = _dSimulationTime;
}

double CFlowsheet::GetSimulationTime()
{
	return m_dSimulationTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

size_t CFlowsheet::GetCompoundsNumber() const
{
	return m_vCompoundsKeys.size();
}

void CFlowsheet::AddCompound(const std::string& _sCompoundKey)
{
	const size_t iCompound = GetCompoundIndex(_sCompoundKey);
	if (iCompound < m_vCompoundsKeys.size()) return; // such compound already exists in the list

	m_vCompoundsKeys.push_back(_sCompoundKey);

	// add new compound to all streams in the flowsheet
	for (auto& stream : m_vpStreams)
		stream->AddCompound(_sCompoundKey);

	// adds new compound to all models in the flowsheet
	for (auto& model : m_vpModels)
		model->AddCompound(_sCompoundKey);

	// add new compound to all initial tear streams
	for (auto& partition : m_vvInitTearStreams)
		for (auto& stream : partition)
			stream.AddCompound(_sCompoundKey);

	if (!m_pDistributionsGrid->IsDistrTypePresent(DISTR_COMPOUNDS))
		m_pDistributionsGrid->AddDimension(DISTR_COMPOUNDS, EGridEntry::GRID_SYMBOLIC, std::vector<double>(), std::vector<std::string>());
	m_pDistributionsGrid->AddNamedClass(DISTR_COMPOUNDS, m_pMaterialsDatabase->GetCompound(_sCompoundKey)->GetName());
}

void CFlowsheet::RemoveCompound(const std::string& _sCompoundKey)
{
	const size_t iCompound = GetCompoundIndex(_sCompoundKey);
	if (iCompound >= m_vCompoundsKeys.size()) return;

	m_vCompoundsKeys.erase(m_vCompoundsKeys.begin() + iCompound);

	// remove compound from all streams
	for (auto& stream : m_vpStreams)
		stream->RemoveCompound(_sCompoundKey);

	// remove compound from all models in the flowsheet
	for (auto& model : m_vpModels)
		model->RemoveCompound(_sCompoundKey);

	// remove compound from all initial tear streams
	for (auto& partition : m_vvInitTearStreams)
		for (auto& stream : partition)
			stream.RemoveCompound(_sCompoundKey);

	m_pDistributionsGrid->RemoveNamedClass(DISTR_COMPOUNDS, iCompound);
}

std::vector<std::string> CFlowsheet::GetCompounds() const
{
	return m_vCompoundsKeys;
}

std::vector<std::string> CFlowsheet::GetCompoundsNames() const
{
	std::vector<std::string> vNames;
	for (const auto& key : m_vCompoundsKeys)
		if (const CCompound* pComp = m_pMaterialsDatabase->GetCompound(key))
			vNames.push_back(pComp->GetName());
		else
			vNames.emplace_back();
	return vNames;
}

std::string CFlowsheet::GetCompoundName(size_t _iCompound) const
{
	if (_iCompound < m_vCompoundsKeys.size())
		if (const CCompound* pComp = m_pMaterialsDatabase->GetCompound(m_vCompoundsKeys[_iCompound]))
			return pComp->GetName();
	return "";
}

std::string CFlowsheet::GetCompoundKey(size_t _iCompound) const
{
	if (_iCompound < m_vCompoundsKeys.size())
		return m_vCompoundsKeys[_iCompound];
	return "";
}

size_t CFlowsheet::GetCompoundIndex(const std::string& _sCompoundKey) const
{
	for (size_t i = 0; i < m_vCompoundsKeys.size(); ++i)
		if (m_vCompoundsKeys[i] == _sCompoundKey)
			return i;
	return -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

size_t CFlowsheet::GetModelsCount() const
{
	return m_vpModels.size();
}

CBaseModel* CFlowsheet::AddModel(const std::string& _modelKey /*= ""*/)
{
	const std::string uniqueKey = GenerateUniqueModelKey(_modelKey);
	auto* pModel = new CBaseModel(m_pModelsManager, uniqueKey);
	pModel->SetCacheParams(m_pParams->cacheFlagHoldups, m_pParams->cacheWindow);
	pModel->SetCachePath(m_pParams->cachePath);
	m_vpModels.push_back(pModel);
	SetTopologyModified(true);
	return pModel;
}

void CFlowsheet::DeleteModel(const std::string& _sModelKey)
{
	const size_t iModel = GetModelIndex(_sModelKey);
	if (iModel >= m_vpModels.size()) return;

	// remove model from the calculation sequence
	m_calculationSequence.DeleteModel(_sModelKey);
	SetTopologyModified(true);

	// delete model
	delete m_vpModels[iModel];
	m_vpModels.erase(m_vpModels.begin() + iModel);
}

const CBaseModel* CFlowsheet::GetModel(size_t _index) const
{
	if (_index < m_vpModels.size())
		return m_vpModels[_index];
	return nullptr;
}

CBaseModel* CFlowsheet::GetModel(size_t _index)
{
	return const_cast<CBaseModel*>(static_cast<const CFlowsheet&>(*this).GetModel(_index));
}

const CBaseModel* CFlowsheet::GetModel(const std::string& _sModelKey) const
{
	return GetModel(GetModelIndex(_sModelKey));
}

CBaseModel* CFlowsheet::GetModel(const std::string& _sModelKey)
{
	return const_cast<CBaseModel*>(static_cast<const CFlowsheet&>(*this).GetModel(_sModelKey));
}

void CFlowsheet::ShiftModelUp(const std::string& _sModelKey)
{
	const size_t iModel = GetModelIndex(_sModelKey);
	if (iModel >= m_vpModels.size() || iModel == 0) return;
	std::iter_swap(m_vpModels.begin() + iModel, m_vpModels.begin() + iModel - 1);
	SetTopologyModified(true);
}

void CFlowsheet::ShiftModelDown(const std::string& _sModelKey)
{
	const size_t iModel = GetModelIndex(_sModelKey);
	if (iModel >= m_vpModels.size() || iModel == m_vpModels.size() - 1) return;
	std::iter_swap(m_vpModels.begin() + iModel, m_vpModels.begin() + iModel + 1);
	SetTopologyModified(true);
}

size_t CFlowsheet::GetModelIndex(const std::string& _sModelKey) const
{
	for (size_t i = 0; i < m_vpModels.size(); ++i)
		if (m_vpModels[i]->GetModelKey() == _sModelKey)
			return i;
	return -1;
}

void CFlowsheet::InitializeModel(const std::string& _sModelKey)
{
	CBaseModel* pModel = GetModel(_sModelKey);
	if (!pModel) return;

	pModel->SetDistributionsGrid(m_pDistributionsGrid);
	pModel->SetMaterialsDatabase(m_pMaterialsDatabase);
	pModel->SetCompounds(&m_vCompoundsKeys);
	pModel->SetPhases(&m_vPhasesNames, &m_vPhasesSOA);
	pModel->SetAbsTolerance(m_pParams->absTol);
	pModel->SetRelTolerance(m_pParams->relTol);
	pModel->SetMinimalFraction(m_pParams->minFraction);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

size_t CFlowsheet::GetStreamsCount() const
{
	return m_vpStreams.size();
}

CStream* CFlowsheet::AddStream(const std::string& _streamKey /*= ""*/)
{
	const std::string uniqueKey = GenerateUniqueStreamKey(_streamKey);
	auto* pStream = new CStream(uniqueKey);
	pStream->SetGrid(m_pDistributionsGrid);
	pStream->UpdateDistributionsGrid();
	pStream->SetMaterialsDatabase(m_pMaterialsDatabase);
	for (const auto& c : m_vCompoundsKeys)
		pStream->AddCompound(c);
	for (size_t i = 0; i < m_vPhasesNames.size(); ++i)
		pStream->AddPhase(PhaseSOA2EPhase(m_vPhasesSOA[i]), m_vPhasesNames[i]);
	pStream->SetCacheSettings({ m_pParams->cacheFlagStreams, m_pParams->cacheWindow, m_pParams->cachePath});
	m_vpStreams.push_back(pStream);
	SetTopologyModified(true);
	return pStream;
}

void CFlowsheet::DeleteStream(const std::string& _sStreamKey)
{
	const size_t iStream = GetStreamIndex(_sStreamKey);
	if (iStream >= m_vpStreams.size()) return;

	// remove stream from the calculation sequence
	m_calculationSequence.DeleteStream(_sStreamKey);
	SetTopologyModified(true);

	// delete stream
	delete m_vpStreams[iStream];
	m_vpStreams.erase(m_vpStreams.begin() + iStream);
}

const CStream* CFlowsheet::GetStream(size_t _index) const
{
	if (_index < m_vpStreams.size())
		return m_vpStreams[_index];
	return nullptr;
}

CStream* CFlowsheet::GetStream(size_t _index)
{
	return const_cast<CStream*>(static_cast<const CFlowsheet&>(*this).GetStream(_index));
}

const CStream* CFlowsheet::GetStream(const std::string& _sStreamKey) const
{
	return GetStream(GetStreamIndex(_sStreamKey));
}

CStream* CFlowsheet::GetStream(const std::string& _sStreamKey)
{
	return const_cast<CStream*>(static_cast<const CFlowsheet&>(*this).GetStream(_sStreamKey));
}

void CFlowsheet::ShiftStreamUp(const std::string& _sStreamKey)
{
	const size_t iStream = GetStreamIndex(_sStreamKey);
	if (iStream >= m_vpStreams.size() || iStream == 0) return;
	std::iter_swap(m_vpStreams.begin() + iStream, m_vpStreams.begin() + iStream - 1);
	SetTopologyModified(true);
}

void CFlowsheet::ShiftStreamDown(const std::string& _sStreamKey)
{
	const size_t iStream = GetStreamIndex(_sStreamKey);
	if (iStream >= m_vpStreams.size() || iStream == m_vpStreams.size() - 1) return;
	std::iter_swap(m_vpStreams.begin() + iStream, m_vpStreams.begin() + iStream + 1);
	SetTopologyModified(true);
}

size_t CFlowsheet::GetStreamIndex(const std::string& _sStreamKey) const
{
	for (size_t i = 0; i < m_vpStreams.size(); ++i)
		if (m_vpStreams[i]->GetKey() == _sStreamKey)
			return i;
	return -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool CFlowsheet::SaveToFile(CH5Handler& _h5Saver, const std::wstring& _sFileName)
{
	if (_sFileName.empty()) return false;

	_h5Saver.Create(_sFileName, m_pParams->fileSingleFlag);

	if (!_h5Saver.IsValid())
		return false;

	// current version of save procedure
	_h5Saver.WriteAttribute("/", StrConst::Flow_H5AttrSaveVersion, m_cnSaveVersion);

	// save models
	_h5Saver.WriteAttribute("/", StrConst::Flow_H5AttrModelsNum, (int)m_vpModels.size());
	_h5Saver.CreateGroup("/", StrConst::Flow_H5GroupModels);
	for (size_t i = 0; i < m_vpModels.size(); ++i)
	{
		const std::string sPath = _h5Saver.CreateGroup("/" + std::string(StrConst::Flow_H5GroupModels), StrConst::Flow_H5GroupModelName + std::to_string(i));
		_h5Saver.WriteData(sPath, StrConst::Flow_H5UnitKey, m_vpModels[i]->GetUnitKey());
		m_vpModels[i]->SaveToFile(_h5Saver, sPath);
	}

	// save streams
	_h5Saver.WriteAttribute("/", StrConst::Flow_H5AttrStreamsNum, (int)m_vpStreams.size());
	_h5Saver.CreateGroup("/", StrConst::Flow_H5GroupStreams);
	for (size_t i = 0; i < m_vpStreams.size(); ++i)
	{
		const std::string sPath = _h5Saver.CreateGroup("/" + std::string(StrConst::Flow_H5GroupStreams), StrConst::Flow_H5GroupStreamName + std::to_string(i));
		m_vpStreams[i]->SaveToFile(_h5Saver, sPath);
	}

	// save calculation sequence
	const std::string calcSeqPath = _h5Saver.CreateGroup("/", StrConst::Flow_H5GroupCalcSeq);
	m_calculationSequence.SaveToFile(_h5Saver, calcSeqPath);

	// save initial tear streams
	const std::string initTearStreamsPath = _h5Saver.CreateGroup("/", StrConst::Flow_H5GroupInitTearStreams);
	for (size_t i = 0; i < m_vvInitTearStreams.size(); ++i)
	{
		const std::string partitionPath = _h5Saver.CreateGroup(initTearStreamsPath, StrConst::Flow_H5GroupPartitionName + std::to_string(i));
		for (size_t j = 0; j < m_vvInitTearStreams[i].size(); ++j)
		{
			const std::string streamPath = _h5Saver.CreateGroup(partitionPath, StrConst::Flow_H5GroupInitTearStreamName + std::to_string(j));
			m_vvInitTearStreams[i][j].SaveToFile(_h5Saver, streamPath);
		}
	}

	// save compounds
	_h5Saver.WriteData("/", StrConst::Flow_H5Compounds, m_vCompoundsKeys);

	// save distributions grid
	_h5Saver.CreateGroup("/", StrConst::Flow_H5GroupDistrGrid);
	m_pDistributionsGrid->SaveToFile(_h5Saver, "/" + std::string(StrConst::Flow_H5GroupDistrGrid));

	// save phases
	_h5Saver.CreateGroup("/", StrConst::Flow_H5GroupPhases);
	_h5Saver.WriteData("/" + std::string(StrConst::Flow_H5GroupPhases), StrConst::Flow_H5PhasesNames, m_vPhasesNames);
	_h5Saver.WriteData("/" + std::string(StrConst::Flow_H5GroupPhases), StrConst::Flow_H5PhasesSOA, m_vPhasesSOA);

	// save simulation time
	const std::string sParamsPath = _h5Saver.CreateGroup("/", StrConst::Flow_H5GroupOptions);
	_h5Saver.WriteData(sParamsPath, StrConst::Flow_H5OptionSimTime, m_dSimulationTime);

	// save parameters
	m_pParams->SaveToFile(_h5Saver, sParamsPath);

	_h5Saver.Close();

	return true;
}

bool CFlowsheet::LoadFromFile(CH5Handler& _h5Loader, const std::wstring& _sFileName)
{
	if (_sFileName.empty()) return false;

	_h5Loader.Open(_sFileName);

	if (!_h5Loader.IsValid())
		return false;

	Clear();

	// load version of save procedure
	const int nVer = _h5Loader.ReadAttribute("/", StrConst::Flow_H5AttrSaveVersion);

	// load parameters
	m_pParams->LoadFromFile(_h5Loader, "/" + std::string(StrConst::Flow_H5GroupOptions));

	// load compounds
	_h5Loader.ReadData("/", StrConst::Flow_H5Compounds, m_vCompoundsKeys);

	// load distributions grid
	m_pDistributionsGrid->LoadFromFile(_h5Loader, "/" + std::string(StrConst::Flow_H5GroupDistrGrid));

	// load phases
	_h5Loader.ReadData("/" + std::string(StrConst::Flow_H5GroupPhases), StrConst::Flow_H5PhasesNames, m_vPhasesNames);
	_h5Loader.ReadData("/" + std::string(StrConst::Flow_H5GroupPhases), StrConst::Flow_H5PhasesSOA, m_vPhasesSOA);

	// load simulation time
	_h5Loader.ReadData("/" + std::string(StrConst::Flow_H5GroupOptions), StrConst::Flow_H5OptionSimTime, m_dSimulationTime);

	// load streams
	const int nStreamsNum = _h5Loader.ReadAttribute("/", StrConst::Flow_H5AttrStreamsNum);
	if (nStreamsNum != -1)
	{
		for (size_t i = 0; i < static_cast<size_t>(nStreamsNum); i++)
		{
			AddStream("TempKey");
			const std::string sPath = "/" + std::string(StrConst::Flow_H5GroupStreams) + "/" + std::string(StrConst::Flow_H5GroupStreamName) + std::to_string(i);
			m_vpStreams[i]->SetMaterialsDatabase(m_pMaterialsDatabase);
			m_vpStreams[i]->SetGrid(m_pDistributionsGrid);
			m_vpStreams[i]->LoadFromFile(_h5Loader, sPath);
			m_vpStreams[i]->SetCacheSettings({ m_pParams->cacheFlagStreams, m_pParams->cacheWindow, m_pParams->cachePath });
			m_vpStreams[i]->SetMinimumFraction(m_pParams->minFraction);
		}
	}
	EnsureUniqueStreamsKeys();

	// load models
	const int nModelsNum = _h5Loader.ReadAttribute("/", StrConst::Flow_H5AttrModelsNum);
	if (nModelsNum != -1)
	{
		for (size_t i = 0; i < static_cast<size_t>(nModelsNum); ++i)
		{
			AddModel("TempKey");
			const std::string sPath = "/" + std::string(StrConst::Flow_H5GroupModels) + "/" + std::string(StrConst::Flow_H5GroupModelName) + std::to_string(i);
			std::string sUnitKey;
			_h5Loader.ReadData(sPath, StrConst::Flow_H5UnitKey, sUnitKey);
			m_vpModels[i]->SetUnit(sUnitKey);
			m_vpModels[i]->SetMaterialsDatabase(m_pMaterialsDatabase);
			m_vpModels[i]->SetDistributionsGrid(m_pDistributionsGrid);
			m_vpModels[i]->LoadFromFile(_h5Loader, sPath);

			m_vpModels[i]->SetCompoundsPtr(&m_vCompoundsKeys);
			m_vpModels[i]->SetPhasesPtr(&m_vPhasesNames, &m_vPhasesSOA);
			m_vpModels[i]->SetAbsTolerance(m_pParams->absTol);
			m_vpModels[i]->SetRelTolerance(m_pParams->relTol);
			m_vpModels[i]->SetMinimalFraction(m_pParams->minFraction);
			m_vpModels[i]->SetCacheParams(m_pParams->cacheFlagHoldups, m_pParams->cacheWindow);
		}
	}
	EnsureUniqueModelsKeys();

	// load calculation sequence
	const std::string calcSeqPath = "/" + std::string(StrConst::Flow_H5GroupCalcSeq);
	m_calculationSequence.LoadFromFile(_h5Loader, calcSeqPath);

	// load initial tear streams
	if (nVer < 3)
		LoadInitTearStreamsOld(_h5Loader);
	else
	{
		const std::string initTearStreamsPath = "/" + std::string(StrConst::Flow_H5GroupInitTearStreams);
		m_vvInitTearStreams.resize(m_calculationSequence.PartitionsNumber());
		for (size_t i = 0; i < m_vvInitTearStreams.size(); ++i)
		{
			const std::string partitionPath = initTearStreamsPath + "/" + StrConst::Flow_H5GroupPartitionName + std::to_string(i);
			m_vvInitTearStreams[i].resize(m_calculationSequence.TearStreamsNumber(i));
			for (size_t j = 0; j < m_vvInitTearStreams[i].size(); ++j)
			{
				const std::string streamPath = partitionPath + "/" + StrConst::Flow_H5GroupInitTearStreamName + std::to_string(j);
				m_vvInitTearStreams[i][j].SetMaterialsDatabase(m_pMaterialsDatabase);
				m_vvInitTearStreams[i][j].SetGrid(m_pDistributionsGrid);
				m_vvInitTearStreams[i][j].LoadFromFile(_h5Loader, streamPath);
			}
		}
	}

	_h5Loader.Close();

	SetDistributionsGrid();
	SetMaterialsDatabase(m_pMaterialsDatabase);

	SetTopologyModified(false);

	return true;
}

void CFlowsheet::LoadInitTearStreamsOld(CH5Handler& _h5Loader)
{
	const std::string Flow_H5GroupSteps          = "CalcSteps";
	const std::string Flow_H5GroupStepName       = "CalcStep";
	const std::string Flow_H5GroupInitStreams    = "InitStreams";
	const std::string Flow_H5GroupInitStreamName = "InitStream";

	// load version of save procedure
	const int saveVersion = _h5Loader.ReadAttribute("/", StrConst::Flow_H5AttrSaveVersion);

	for (size_t i = 0; i < m_calculationSequence.PartitionsNumber(); ++i)
	{
		const auto& tearStreams = m_calculationSequence.PartitionTearStreams(i);

		const std::string path = "/" + Flow_H5GroupSteps + "/" + Flow_H5GroupStepName + std::to_string(i);

		// load initialization streams
		if (saveVersion > 1)
		{
			m_vvInitTearStreams.emplace_back(tearStreams.size());
			for (size_t j = 0; j < m_vvInitTearStreams.back().size(); ++j)
			{
				m_vvInitTearStreams.back()[j].SetMaterialsDatabase(m_pMaterialsDatabase);
				m_vvInitTearStreams.back()[j].SetGrid(m_pDistributionsGrid);
				m_vvInitTearStreams.back()[j].LoadFromFile(_h5Loader, path + "/" + Flow_H5GroupInitStreams + "/" + Flow_H5GroupInitStreamName + std::to_string(j));
			}
		}
		else // just initialize the structure of initial stream
		{
			if (tearStreams.empty()) // no recycle streams
				m_vvInitTearStreams.emplace_back();
			else                     // initialize with the structure of corresponding material stream
				m_vvInitTearStreams.emplace_back(tearStreams.size(), CStream(*tearStreams.front()));
		}
	}
}

void CFlowsheet::SaveConfigFile(const std::wstring& _fileName, const std::wstring& _flowsheetFile) const
{
	std::ofstream file(StringFunctions::UnicodePath(_fileName));
	if (file.fail()) return;

	file << TO_ARG_STR(EArguments::SOURCE_FILE) << " " << StringFunctions::WString2String(_flowsheetFile) << std::endl;
	file << TO_ARG_STR(EArguments::RESULT_FILE) << " " << StringFunctions::WString2String(FileSystem::FilePath(_flowsheetFile) + L"/" + FileSystem::FileName(_flowsheetFile) + L"_res." + FileSystem::FileExtension(_flowsheetFile)) << std::endl;
	for (size_t i = 0; i < m_pModelsManager->DirsNumber(); ++i)
		if (m_pModelsManager->GetDirActivity(i))
			file << TO_ARG_STR(EArguments::MODELS_PATH) << " " << StringFunctions::WString2String(m_pModelsManager->GetDirPath(i)) << std::endl;
	file << TO_ARG_STR(EArguments::MATERIALS_DATABASE) << " " << StringFunctions::WString2String(m_pMaterialsDatabase->GetFileName()) << std::endl;
	file << std::endl;

	file << TO_ARG_STR(EArguments::SIMULATION_TIME)    << " " << m_dSimulationTime << std::endl;
	file << std::endl;

	file << TO_ARG_STR(EArguments::RELATIVE_TOLERANCE) << " " << m_pParams->relTol << std::endl;
	file << TO_ARG_STR(EArguments::ABSOLUTE_TOLERANCE) << " " << m_pParams->absTol << std::endl;
	file << TO_ARG_STR(EArguments::MINIMAL_FRACTION)   << " " << m_pParams->minFraction << std::endl;
	file << TO_ARG_STR(EArguments::INIT_TIME_WINDOW)   << " " << m_pParams->initTimeWindow << std::endl;
	file << TO_ARG_STR(EArguments::MIN_TIME_WINDOW)    << " " << m_pParams->minTimeWindow << std::endl;
	file << TO_ARG_STR(EArguments::MAX_TIME_WINDOW)    << " " << m_pParams->maxTimeWindow << std::endl;
	file << TO_ARG_STR(EArguments::MAX_ITERATIONS_NUM) << " " << m_pParams->maxItersNumber << std::endl;
	file << TO_ARG_STR(EArguments::WINDOW_CHANGE_RATE) << " " << m_pParams->magnificationRatio << std::endl;
	file << TO_ARG_STR(EArguments::ITER_UPPER_LIMIT)   << " " << m_pParams->itersUpperLimit << std::endl;
	file << TO_ARG_STR(EArguments::ITER_LOWER_LIMIT)   << " " << m_pParams->itersLowerLimit << std::endl;
	file << TO_ARG_STR(EArguments::ITER_UPPER_LIMIT_1) << " " << m_pParams->iters1stUpperLimit << std::endl;
	file << TO_ARG_STR(EArguments::CONVERGENCE_METHOD) << " " << m_pParams->convergenceMethod << std::endl;
	file << TO_ARG_STR(EArguments::ACCEL_PARAMETER)    << " " << m_pParams->wegsteinAccelParam << std::endl;
	file << TO_ARG_STR(EArguments::RELAX_PARAMETER)    << " " << m_pParams->relaxationParam << std::endl;
	file << TO_ARG_STR(EArguments::EXTRAPOL_METHOD)    << " " << E2I(static_cast<EExtrapMethod>(m_pParams->extrapolationMethod)) << std::endl;
	file << std::endl;

	for (size_t i = 0; i < m_pDistributionsGrid->GetDistributionsNumber(); ++i)
	{
		const EGridEntry type = m_pDistributionsGrid->GetGridEntryByIndex(i);
		file << TO_ARG_STR(EArguments::DISTRIBUTION_GRID) << " " << i + 1 << " " << E2I(type) << " " << m_pDistributionsGrid->GetClassesByIndex(i) << " ";
		switch (type)
		{
		case EGridEntry::GRID_NUMERIC:
			file << E2I(EGridFunction::GRID_FUN_MANUAL) << " ";
			for (double v : m_pDistributionsGrid->GetNumericGridByIndex(i))
				file << " " << v;
			break;
		case EGridEntry::GRID_SYMBOLIC:
			for (const std::string& v : m_pDistributionsGrid->GetSymbolicGridByIndex(i))
				file << " " << v;
			break;
		case EGridEntry::GRID_UNDEFINED:
			break;
		}
		file << "\t" << StrConst::COMMENT_SYMBOL << " " << std::vector<std::string>{ DISTR_NAMES }[GetDistributionTypeIndex(m_pDistributionsGrid->GetDistrType(i))] << std::endl;
	}
	file << std::endl;

	for (size_t iUnit = 0; iUnit < m_vpModels.size(); ++iUnit)
		for (size_t iParam = 0; iParam < m_vpModels[iUnit]->GetUnitParametersManager()->ParametersNumber(); ++iParam)
		{
			const CBaseUnitParameter* param = m_vpModels[iUnit]->GetUnitParametersManager()->GetParameter(iParam);
			file << TO_ARG_STR(EArguments::UNIT_PARAMETER) << " " << iUnit + 1 << " " << iParam + 1;
			switch (param->GetType())
			{
			case EUnitParameter::CONSTANT:       file << " " << dynamic_cast<const CConstRealUnitParameter*>(param)->GetValue();		break;
			case EUnitParameter::TIME_DEPENDENT: file << " " << dynamic_cast<const CTDUnitParameter*>(param)->GetTDData();			break;
			case EUnitParameter::STRING:         file << " " << dynamic_cast<const CStringUnitParameter*>(param)->GetValue();		break;
			case EUnitParameter::COMBO:			 file << " " << dynamic_cast<const CComboUnitParameter*>(param)->GetValue();		break;
			case EUnitParameter::GROUP:			 file << " " << dynamic_cast<const CComboUnitParameter*>(param)->GetValue();		break;
			case EUnitParameter::CHECKBOX:		 file << " " << dynamic_cast<const CCheckBoxUnitParameter*>(param)->IsChecked();	break;
			case EUnitParameter::COMPOUND:		 file << " " << dynamic_cast<const CCompoundUnitParameter*>(param)->GetCompound();	break;
			case EUnitParameter::SOLVER:         file << " " << StringFunctions::WString2String(m_pModelsManager->GetSolverLibName(dynamic_cast<const CSolverUnitParameter*>(param)->GetKey()));	break;
			case EUnitParameter::UNKNOWN:        break;
			default: ;
			}
			file << "\t" << StrConst::COMMENT_SYMBOL << " " << m_vpModels[iUnit]->GetModelName() << " - " << param->GetName() << " - <Values>" << std::endl;
		}
	file << std::endl;

	for (size_t iUnit = 0; iUnit < m_vpModels.size(); ++iUnit)
		for (size_t iHoldup = 0; iHoldup < m_vpModels[iUnit]->GetHoldupsCount(); ++iHoldup)
		{
			const CBaseStream* str = m_vpModels[iUnit]->GetHoldupInit(iHoldup);
			const std::vector<double> tp = str->GetAllTimePoints();
			for (size_t iTime = 0; iTime < tp.size(); ++iTime)
				file << TO_ARG_STR(EArguments::UNIT_HOLDUP_MTP) << " " << iUnit + 1 << " " << iHoldup + 1 << " " << iTime + 1 << " " <<
					str->GetMass(tp[iTime]) << " " << str->GetTemperature(tp[iTime]) << " " << str->GetPressure(tp[iTime]);
			file << "\t" << StrConst::COMMENT_SYMBOL << " " << m_vpModels[iUnit]->GetModelName() << " - " << str->GetName() << " - <Mass> - <Temperature> - <Pressure>" << std::endl;
		}
	file << std::endl;

	for (size_t iUnit = 0; iUnit < m_vpModels.size(); ++iUnit)
		for (size_t iHoldup = 0; iHoldup < m_vpModels[iUnit]->GetHoldupsCount(); ++iHoldup)
		{
			const CBaseStream* str = m_vpModels[iUnit]->GetHoldupInit(iHoldup);
			const std::vector<double> tp = str->GetAllTimePoints();
			for (size_t iTime = 0; iTime < tp.size(); ++iTime)
			{
				file << TO_ARG_STR(EArguments::UNIT_HOLDUP_PHASES) << " " << iUnit + 1 << " " << iHoldup + 1 << " " << iTime + 1;
				for (unsigned int phase : m_vPhasesSOA)
					file << " " << str->GetPhaseFraction(tp[iTime], PhaseSOA2EPhase(phase));
				file << "\t" << StrConst::COMMENT_SYMBOL << " " << m_vpModels[iUnit]->GetModelName() << " - " << str->GetName() << " - " << tp[iTime] << "[s]";
				for (const auto& n : m_vPhasesNames)
					file << " - " << n;
				file << std::endl;
			}
		}
	file << std::endl;

	for (size_t iUnit = 0; iUnit < m_vpModels.size(); ++iUnit)
		for (size_t iHoldup = 0; iHoldup < m_vpModels[iUnit]->GetHoldupsCount(); ++iHoldup)
		{
			const CBaseStream* str = m_vpModels[iUnit]->GetHoldupInit(iHoldup);
			const std::vector<double> tp = str->GetAllTimePoints();
			for (size_t iPhase = 0; iPhase < m_vPhasesSOA.size(); ++iPhase)
				for (size_t iTime = 0; iTime < tp.size(); ++iTime)
				{
					file << TO_ARG_STR(EArguments::UNIT_HOLDUP_COMP) << " " << iUnit + 1 << " " << iHoldup + 1 << " " << iPhase + 1 << " " << iTime + 1;
					for (const auto& comp : m_vCompoundsKeys)
						file << " " << str->GetCompoundFraction(tp[iTime], comp, PhaseSOA2EPhase(m_vPhasesSOA[iPhase]));
					file << "\t" << StrConst::COMMENT_SYMBOL << " " << m_vpModels[iUnit]->GetModelName() << " - " << str->GetName() << " - " <<
						m_vPhasesNames[iPhase] << " - " << tp[iTime] << "[s]";
					for (const auto& n : GetCompoundsNames())
						file << " - " << n;
					file << std::endl;
				}
		}
	file << std::endl;

	for (size_t iUnit = 0; iUnit < m_vpModels.size(); ++iUnit)
		for (size_t iHoldup = 0; iHoldup < m_vpModels[iUnit]->GetHoldupsCount(); ++iHoldup)
		{
			const CBaseStream* str = m_vpModels[iUnit]->GetHoldupInit(iHoldup);
			const std::vector<double> tp = str->GetAllTimePoints();
			const std::vector<EDistrTypes> distrs = m_pDistributionsGrid->GetDistrTypes();
			for (size_t iDistr = 1; iDistr < distrs.size(); ++iDistr)
				for (size_t iComp = 0; iComp < m_vCompoundsKeys.size(); ++iComp)
					for (size_t iTime = 0; iTime < tp.size(); ++iTime)
					{
						file << TO_ARG_STR(EArguments::UNIT_HOLDUP_SOLID) << " " << iUnit + 1 << " " << iHoldup + 1 << " " << iDistr + 1 << " " << iComp + 1 << " " << iTime + 1 << " " <<
							PSD_MassFrac << " " << E2I(EDistrFunction::Manual) << " " << E2I(EPSDGridType::DIAMETER);
						for (auto v : str->GetDistribution(tp[iTime], distrs[iDistr], m_vCompoundsKeys[iComp]))
							file << " " << v;
						file << "\t" << StrConst::COMMENT_SYMBOL << " " << m_vpModels[iUnit]->GetModelName() << " - " << str->GetName() << " - " <<
							GetCompoundsNames()[iComp] << " - " << tp[iTime] << "[s] - PSD_MassFrac - Manual - DIAMETER - <Values>" << std::endl;
					}
		}
}

CDistributionsGrid* CFlowsheet::GetDistributionsGrid() const
{
	return m_pDistributionsGrid;
}

bool CFlowsheet::Empty() const
{
	if (!m_calculationSequence.IsEmpty())
		return false;

	if (!m_vpModels.empty())
		return false;

	if (!m_vpStreams.empty())
		return false;

	return true;
}

std::string CFlowsheet::GenerateUniqueModelKey(const std::string& _key /*= ""*/) const
{
	std::vector<std::string> keys;
	for (auto model : m_vpModels)
		keys.push_back(model->GetModelKey());
	return StringFunctions::GenerateUniqueKey(_key, keys);
}

std::string CFlowsheet::GenerateUniqueStreamKey(const std::string& _key /*= ""*/) const
{
	std::vector<std::string> keys;
	for (auto model : m_vpStreams)
		keys.push_back(model->GetKey());
	return StringFunctions::GenerateUniqueKey(_key, keys);
}

void CFlowsheet::EnsureUniqueModelsKeys()
{
	for (size_t i = 0; i < m_vpModels.size(); ++i)
		for (size_t j = i + 1; j < m_vpModels.size(); ++j)
			if (m_vpModels[i]->GetModelKey() == m_vpModels[j]->GetModelKey())
				m_vpModels[j]->SetModelKey(GenerateUniqueModelKey(m_vpModels[j]->GetModelKey()));
}

void CFlowsheet::EnsureUniqueStreamsKeys()
{
	for (size_t i = 0; i < m_vpStreams.size(); ++i)
		for (size_t j = i + 1; j < m_vpStreams.size(); ++j)
			if (m_vpStreams[i]->GetKey() == m_vpStreams[j]->GetKey())
				m_vpStreams[j]->SetKey(GenerateUniqueStreamKey(m_vpStreams[j]->GetKey()));
}

EPhase CFlowsheet::PhaseSOA2EPhase(unsigned _soa)
{
	switch (_soa)
	{
	case SOA_SOLID:		return EPhase::SOLID;
	case SOA_LIQUID:	return EPhase::LIQUID;
	case SOA_VAPOR:		return EPhase::VAPOR;
	default:			return EPhase::UNDEFINED;
	}
}

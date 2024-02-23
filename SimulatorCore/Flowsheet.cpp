/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "Flowsheet.h"
#include "BaseUnit.h"
#include "Topology.h"
#include "MaterialsDatabase.h"
#include "ContainerFunctions.h"
#include "DyssolStringConstants.h"
#include "DyssolUtilities.h"

CFlowsheet::CFlowsheet(CModelsManager* _modelsManager, const CMaterialsDatabase* _materialsDB)
	: m_materialsDB{ _materialsDB }
	, m_modelsManager{ _modelsManager }
{
	Create();
}

CFlowsheet::CFlowsheet(const CFlowsheet& _other)
	: m_fileName{ _other.m_fileName }
	, m_materialsDB{ _other.m_materialsDB }
	, m_modelsManager{ _other.m_modelsManager }
	, m_parameters{ _other.m_parameters }
	, m_mainGrid{ _other.m_mainGrid }
	, m_overall{ _other.m_overall }
	, m_phases{ _other.m_phases }
	, m_cacheStreams{ _other.m_cacheStreams }
	, m_cacheHoldups{ _other.m_cacheHoldups }
	, m_tolerance{ _other.m_tolerance }
	, m_thermodynamics{ _other.m_thermodynamics }
	, m_calculationSequence{ _other.m_calculationSequence }
	, m_topologyModified{ _other.m_topologyModified }
{
	for (const auto& unit : _other.m_units)
		m_units.emplace_back(new CUnitContainer{ *unit });
	for (const auto& stream : _other.m_streams)
		m_streams.emplace_back(new CStream{ *stream });
	for (const auto& stream : _other.m_streamsI)
		m_streamsI.emplace_back(new CStream{ *stream });
	m_calculationSequence.SetPointers(&m_units, &m_streams);
	m_calculationSequence.SetSequence(_other.m_calculationSequence.GetModelsKeys(), _other.m_calculationSequence.GetStreamsKeys());
}

CFlowsheet& CFlowsheet::operator=(const CFlowsheet& _other)
{
	if (this != &_other)
	{
		m_fileName = _other.m_fileName;
		m_materialsDB = _other.m_materialsDB;
		m_modelsManager = _other.m_modelsManager;
		m_parameters = _other.m_parameters;
		m_mainGrid = _other.m_mainGrid;
		m_overall = _other.m_overall;
		m_phases = _other.m_phases;
		m_cacheStreams = _other.m_cacheStreams;
		m_cacheHoldups = _other.m_cacheHoldups;
		m_tolerance = _other.m_tolerance;
		m_thermodynamics = _other.m_thermodynamics;
		m_calculationSequence = _other.m_calculationSequence;
		m_units.clear();
		for (const auto& unit : _other.m_units)
			m_units.emplace_back(new CUnitContainer{ *unit });
		m_streams.clear();
		for (const auto& stream : _other.m_streams)
			m_streams.emplace_back(new CStream{ *stream });
		m_streamsI.clear();
		for (const auto& stream : _other.m_streamsI)
			m_streamsI.emplace_back(new CStream{ *stream });
		m_calculationSequence.SetPointers(&m_units, &m_streams);
		m_calculationSequence.SetSequence(_other.m_calculationSequence.GetModelsKeys(), _other.m_calculationSequence.GetStreamsKeys());
		m_topologyModified = _other.m_topologyModified;
	}
	return *this;
}

void CFlowsheet::SetFileName(const std::filesystem::path& _path)
{
	m_fileName = _path;
}

std::filesystem::path CFlowsheet::GetFileName() const
{
	return m_fileName;
}

void CFlowsheet::Create()
{
	m_mainGrid.AddSymbolicDimension(DISTR_COMPOUNDS);
	m_overall.emplace_back(SOverallDescriptor{ EOverall::OVERALL_MASS, "Mass flow" , "kg/s" });
	// TODO: remove when added by user
	m_overall.emplace_back(SOverallDescriptor{ EOverall::OVERALL_TEMPERATURE, "Temperature" , "K" });
	m_overall.emplace_back(SOverallDescriptor{ EOverall::OVERALL_PRESSURE, "Pressure" , "Pa" });
	m_calculationSequence.SetPointers(&m_units, &m_streams);
}

void CFlowsheet::Clear()
{
	// clear all data
	m_units.clear();
	m_streams.clear();
	m_streamsI.clear();
	m_calculationSequence.Clear();
	m_overall.clear();
	m_phases.clear();
	m_mainGrid.Clear();

	// set parameters to default
	m_parameters.SetDefaultValues();

	// set the topology is modified
	SetTopologyModified(true);

	// clear file name
	m_fileName.clear();

	// initialize flowsheet with default structure
	Create();
}

bool CFlowsheet::IsEmpty() const
{
	return m_units.empty()
		&& m_streams.empty()
		&& m_calculationSequence.IsEmpty()
		&& m_overall.size() <= 1
		&& m_phases.empty()
		&& m_mainGrid.GetDimensionsNumber() <= 1;
}

size_t CFlowsheet::GetUnitsNumber() const
{
	return m_units.size();
}

CUnitContainer* CFlowsheet::AddUnit(const std::string& _key)
{
	const std::string uniqueID = StringFunctions::GenerateUniqueKey(_key, GetAllUnitsKeys());
	auto* unit = new CUnitContainer(uniqueID, m_modelsManager, m_materialsDB, &m_mainGrid, &m_overall, &m_phases, &m_cacheHoldups, &m_tolerance, &m_thermodynamics);
	m_units.emplace_back(unit);
	SetTopologyModified(true);
	return unit;
}

void CFlowsheet::DeleteUnit(const std::string& _key)
{
	// check if it exists
	if (!GetUnit(_key)) return;
	// remove unit from the calculation sequence
	m_calculationSequence.DeleteModel(_key);
	SetTopologyModified(true);
	// delete unit
	VectorDelete(m_units, [&](const auto& u) { return u->GetKey() == _key; });
}

void CFlowsheet::ShiftUnit(const std::string& _key, EDirection _direction)
{
	const size_t index = VectorFind(m_units, [&](const auto& u) { return u->GetKey() == _key; });
	if (index == static_cast<size_t>(-1)) return;

	switch (_direction)
	{
	case EDirection::UP:
		if (index < m_units.size() && index != 0)
			std::iter_swap(m_units.begin() + index, m_units.begin() + index - 1);
		break;
	case EDirection::DOWN:
		if (index < m_units.size() && index != m_units.size() - 1)
			std::iter_swap(m_units.begin() + index, m_units.begin() + index + 1);
		break;
	}
	SetTopologyModified(true);
}

const CUnitContainer* CFlowsheet::GetUnit(size_t _index) const
{
	if (_index >= m_units.size()) return nullptr;
	return m_units[_index].get();
}

CUnitContainer* CFlowsheet::GetUnit(size_t _index)
{
	return const_cast<CUnitContainer*>(static_cast<const CFlowsheet&>(*this).GetUnit(_index));
}

const CUnitContainer* CFlowsheet::GetUnit(const std::string& _key) const
{
	for (const auto& unit : m_units)
		if (unit->GetKey() == _key)
			return unit.get();
	return nullptr;
}

CUnitContainer* CFlowsheet::GetUnit(const std::string& _key)
{
	return const_cast<CUnitContainer*>(static_cast<const CFlowsheet&>(*this).GetUnit(_key));
}

const CUnitContainer* CFlowsheet::GetUnitByName(const std::string& _name) const
{
	for (const auto& unit : m_units)
		if (unit->GetName() == _name)
			return unit.get();
	return nullptr;
}

CUnitContainer* CFlowsheet::GetUnitByName(const std::string& _name)
{
	return const_cast<CUnitContainer*>(static_cast<const CFlowsheet&>(*this).GetUnitByName(_name));
}

std::vector<const CUnitContainer*> CFlowsheet::GetAllUnits() const
{
	auto res = ReservedVector<const CUnitContainer*>(m_units.size());
	for (const auto& unit : m_units)
		res.push_back(unit.get());
	return res;
}

std::vector<CUnitContainer*> CFlowsheet::GetAllUnits()
{
	auto res = ReservedVector<CUnitContainer*>(m_units.size());
	for (const auto& unit : m_units)
		res.push_back(unit.get());
	return res;
}

void CFlowsheet::PrepareInputStreams(const CUnitContainer* _unit, double _timeBeg, double _timeEnd) const
{
	for (const auto& port : _unit->GetModel()->GetPortsManager().GetAllInputPorts())
	{
		auto* streamI = DoGetStream(port->GetStreamKey(), m_streamsI);
		auto* streamO = DoGetStream(port->GetStreamKey(), m_streams);
		if (streamI != streamO)
			streamI->CopyFromStream(_timeBeg, _timeEnd, streamO);
	}
}

size_t CFlowsheet::GetStreamsNumber() const
{
	return m_streams.size();
}

CStream* CFlowsheet::AddStream(const std::string& _key)
{
	const std::string uniqueID = StringFunctions::GenerateUniqueKey(_key, GetAllStreamsKeys());
	auto* stream = new CStream(uniqueID, m_materialsDB, m_mainGrid, &m_overall, &m_phases, &m_cacheStreams, &m_tolerance, &m_thermodynamics);
	m_streams.emplace_back(stream);
	SetTopologyModified(true);
	return stream;
}

void CFlowsheet::DeleteStream(const std::string& _key)
{
	// check if it exists
	if (!GetStream(_key)) return;
	// remove stream from the calculation sequence
	m_calculationSequence.DeleteStream(_key);
	SetTopologyModified(true);
	// delete stream
	VectorDelete(m_streams, [&](const auto& s) { return s->GetKey() == _key; });
}

void CFlowsheet::ShiftStream(const std::string& _key, EDirection _direction)
{
	const size_t index = VectorFind(m_streams, [&](const auto& s) { return s->GetKey() == _key; });
	if (index == static_cast<size_t>(-1)) return;
	VectorShift(m_streams, index, _direction);
	SetTopologyModified(true);
}

const CStream* CFlowsheet::GetStream(size_t _index) const
{
	if (_index >= m_streams.size()) return nullptr;
	return m_streams[_index].get();
}

CStream* CFlowsheet::GetStream(size_t _index)
{
	return const_cast<CStream*>(static_cast<const CFlowsheet&>(*this).GetStream(_index));
}

const CStream* CFlowsheet::GetStream(const std::string& _key) const
{
	return DoGetStream(_key, m_streams);
}

CStream* CFlowsheet::GetStream(const std::string& _key)
{
	return const_cast<CStream*>(static_cast<const CFlowsheet&>(*this).GetStream(_key));
}

const CStream* CFlowsheet::GetStreamByName(const std::string& _name) const
{
	for (const auto& stream : m_streams)
		if (stream->GetName() == _name)
			return stream.get();
	return nullptr;
}

CStream* CFlowsheet::GetStreamByName(const std::string& _name)
{
	return const_cast<CStream*>(static_cast<const CFlowsheet&>(*this).GetStreamByName(_name));
}

std::vector<const CStream*> CFlowsheet::GetAllStreams() const
{
	auto res = ReservedVector<const CStream*>(m_streams.size());
	for (const auto& stream : m_streams)
		res.push_back(stream.get());
	return res;
}

std::vector<CStream*> CFlowsheet::GetAllStreams()
{
	auto res = ReservedVector<CStream*>(m_streams.size());
	for (const auto& stream : m_streams)
		res.push_back(stream.get());
	return res;
}

bool CFlowsheet::DetermineCalculationSequence()
{
	// TODO: move the whole function into CalculationSequence
	// returns unique key of a stream connecting two units
	const auto ConnectionStreamKey = [&](size_t _iSrcUnit, size_t _iDstUnit) -> std::string
	{
		for (const auto& srcPort : m_units[_iSrcUnit]->GetModel()->GetPortsManager().GetAllPorts())
			for (const auto& dstPort : m_units[_iDstUnit]->GetModel()->GetPortsManager().GetAllPorts())
				if (srcPort->GetStreamKey() == dstPort->GetStreamKey())
					return srcPort->GetStream()->GetKey();
		return {};
	};

	// build a topology graph
	CTopology top(m_units.size());
	for (size_t iSrc = 0; iSrc < m_units.size(); ++iSrc)
		for (const auto& srcPort : m_units[iSrc]->GetModel()->GetPortsManager().GetAllOutputPorts())
			for (size_t iDst = 0; iDst < m_units.size(); ++iDst)
				for (const auto& dstPort : m_units[iDst]->GetModel()->GetPortsManager().GetAllInputPorts())
					if (dstPort->GetStreamKey() == srcPort->GetStreamKey())
						top.AddEdge(iSrc, iDst);

	// analyze topology
	std::vector<std::vector<size_t>> iUnits;								// indices of units for each partition
	std::vector<std::vector<std::pair<size_t, size_t>>> iUnitsConnections;	// indices of units connected by tear streams for each partition
	const bool res = top.Analyse(iUnits, iUnitsConnections);

	// gather keys of units in each partition
	std::vector<std::vector<std::string>> unitsKeys;	// keys of units for each partition
	for (auto& partition : iUnits)
	{
		unitsKeys.emplace_back();
		for (size_t i : partition)
			unitsKeys.back().push_back(m_units[i]->GetKey());
	}

	// gather keys of tear streams in each partition
	std::vector<std::vector<std::string>> streamsKeys;	// keys of tear streams for each partition
	for (auto& partition : iUnitsConnections)
	{
		streamsKeys.emplace_back();
		for (auto& [iSrc, iDst] : partition)
			streamsKeys.back().push_back(ConnectionStreamKey(iSrc, iDst));
	}

	// set final calculation sequence
	m_calculationSequence.SetSequence(unitsKeys, streamsKeys);

	return res;
}

void CFlowsheet::SetTopologyModified(bool _flag)
{
	m_topologyModified = _flag;
}

const CCalculationSequence* CFlowsheet::GetCalculationSequence() const
{
	return &m_calculationSequence;
}

CCalculationSequence* CFlowsheet::GetCalculationSequence()
{
	return &m_calculationSequence;
}

std::vector<SFlowsheetConnection> CFlowsheet::GenerateConnectionsDescription() const
{
	std::vector<SFlowsheetConnection> res;

	// iterate all units
	for (const auto& u : GetAllUnits())
	{
		if (!u->GetModel()) continue;
		const auto& ports = u->GetModel()->GetPortsManager();
		// iterate all input ports of the unit
		for (const auto& p : ports.GetAllInputPorts())
		{
			// find connected stream in the results
			const auto stream = p->GetStreamKey();
			if (stream.empty()) continue;
			const size_t i = VectorFind(res, [&](const SFlowsheetConnection& c) { return c.unitI.empty() && c.stream == stream; });
			// a new stream - add it and fill its inlet port and unit
			if (i == static_cast<size_t>(-1))
				res.emplace_back(stream, "", "", u->GetKey(), p->GetName());
			// the stream is already added - fill its inlet port and unit
			else
			{
				res[i].unitI = u->GetKey();
				res[i].portI = p->GetName();
			}
		}
		// iterate all output ports of the unit
		for (const auto& p : ports.GetAllOutputPorts())
		{
			// find connected stream in the results
			const auto stream = p->GetStreamKey();
			const size_t i = VectorFind(res, [&](const SFlowsheetConnection& c) { return c.unitO.empty() && c.stream == stream; });
			// a new stream - add it and fill its outlet port and unit
			if (i == static_cast<size_t>(-1))
				res.emplace_back(stream, u->GetKey(), p->GetName(), "", "");
			// the stream is already added - fill its outlet port and unit
			else
			{
				res[i].unitO = u->GetKey();
				res[i].portO = p->GetName();
			}
		}
	}

	return res;
}

size_t CFlowsheet::GetCompoundsNumber() const
{
	if (!m_mainGrid.HasDimension(DISTR_COMPOUNDS)) return 0;
	return m_mainGrid.GetGridDimension(DISTR_COMPOUNDS)->ClassesNumber();
}

void CFlowsheet::AddCompound(const std::string& _key)
{
	// check if already exists
	if (VectorContains(GetCompounds(), _key)) return;

	// add to streams
	for (auto& stream : m_streams)
		stream->AddCompound(_key);

	// add to units
	for (auto& unit : m_units)
		if (auto* model = unit->GetModel())
			model->AddCompound(_key);

	// add to the grid
	m_mainGrid.GetGridDimensionSymbolic(DISTR_COMPOUNDS)->AddClass(_key);

	// update initial tear streams according to new settings
	m_calculationSequence.UpdateInitialStreams();
}

void CFlowsheet::RemoveCompound(const std::string& _key)
{
	// check if exists
	if (!VectorContains(GetCompounds(), _key)) return;

	// remove from streams
	for (auto& stream : m_streams)
		stream->RemoveCompound(_key);

	// remove from units
	for (auto& unit : m_units)
		if (auto* model = unit->GetModel())
			model->RemoveCompound(_key);

	// remove from the grid
	m_mainGrid.GetGridDimensionSymbolic(DISTR_COMPOUNDS)->RemoveClass(_key);

	// update initial tear streams according to new settings
	m_calculationSequence.UpdateInitialStreams();
}

void CFlowsheet::SetCompounds(const std::vector<std::string>& _keys)
{
	if (GetCompounds() == _keys) return;
	// TODO: do not clean, just add/remove required and sort properly
	// remove all current compounds
	for (const auto& c : GetCompounds())
		RemoveCompound(c);
	// add new compounds
	for (const auto& c : _keys)
		AddCompound(c);
}

std::vector<std::string> CFlowsheet::GetCompounds() const
{
	return m_mainGrid.GetSymbolicGrid(DISTR_COMPOUNDS);
}

size_t CFlowsheet::GetOverallPropertiesNumber() const
{
	return m_overall.size();
}

void CFlowsheet::AddOverallProperty(EOverall _property, const std::string& _name, const std::string& _units)
{
	// check if already exists
	if (HasOverallProperty(_property)) return;

	// add to the list of overall properties
	m_overall.push_back({ _property, _name, _units });

	// add to streams
	for (auto& stream : m_streams)
		stream->AddOverallProperty(_property, _name, _units);

	// adds to units
	for (auto& unit : m_units)
		if (auto* model = unit->GetModel())
			model->AddOverallProperty(_property, _name, _units);

	// update initial tear streams according to new settings
	m_calculationSequence.UpdateInitialStreams();
}

void CFlowsheet::RemoveOverallProperty(EOverall _property)
{
	// check if exists
	if (!HasOverallProperty(_property)) return;

	// remove from streams
	for (auto& stream : m_streams)
		stream->RemoveOverallProperty(_property);

	// remove from units
	for (auto& unit : m_units)
		if (auto* model = unit->GetModel())
			model->RemoveOverallProperty(_property);

	// update initial tear streams according to new settings
	m_calculationSequence.UpdateInitialStreams();

	// remove from the list of overall properties
	VectorDelete(m_overall, [&](const auto& o) { return o.type == _property; });
}

std::vector<SOverallDescriptor> CFlowsheet::GetOveralProperties() const
{
	return m_overall;
}

bool CFlowsheet::HasOverallProperty(EOverall _property) const
{
	return VectorContains(m_overall, [&](const auto& o) { return o.type == _property; });
}

size_t CFlowsheet::GetPhasesNumber() const
{
	return m_phases.size();
}

void CFlowsheet::AddPhase(EPhase _phase, const std::string& _name)
{
	// check if already exists
	if (VectorContains(m_phases, [&](const auto& p) { return p.state == _phase; })) return;

	// add to the list of overall properties
	m_phases.push_back({ _phase, _name });

	// add to streams
	for (auto& stream : m_streams)
		stream->AddPhase(_phase, _name);

	// adds to units
	for (auto& unit : m_units)
		if (auto* model = unit->GetModel())
			model->AddPhase(_phase, _name);

	// update initial tear streams according to new settings
	m_calculationSequence.UpdateInitialStreams();
}

void CFlowsheet::RemovePhase(EPhase _phase)
{
	// check if exists
	if (!VectorContains(m_phases, [&](const auto& p) { return p.state == _phase; })) return;

	// remove from streams
	for (auto& stream : m_streams)
		stream->RemovePhase(_phase);

	// remove from units
	for (auto& unit : m_units)
		if (auto* model = unit->GetModel())
			model->RemovePhase(_phase);

	// update initial tear streams according to new settings
	m_calculationSequence.UpdateInitialStreams();

	// remove from the list of overall properties
	VectorDelete(m_phases, [&](const auto& p) { return p.state == _phase; });
}

void CFlowsheet::SetPhases(const std::vector<SPhaseDescriptor>& _phases)
{
	if (GetPhases() == _phases) return;
	// TODO: do not clean, just add/remove required and sort properly
	// remove all current phases
	for (const auto& p : GetPhases())
		RemovePhase(p.state);
	// add new phases
	for (const auto& p : _phases)
		AddPhase(p.state, p.name);
}

std::vector<SPhaseDescriptor> CFlowsheet::GetPhases() const
{
	return m_phases;
}

bool CFlowsheet::HasPhase(EPhase _phase)
{
	return VectorContains(m_phases, [&](const auto& p) { return p.state == _phase; });
}

std::string CFlowsheet::Initialize()
{
	// clear previous results
	ClearSimulationResults();

	// check that all units have assigned models
	for (const auto& unit : m_units)
		if (!unit->GetModel())
			return StrConst::Flow_ErrEmptyUnit(unit->GetName());

	// set pointers to the streams into the corresponding unit's ports
	SetStreamsToPorts();

	// check that all ports and streams are properly connected
	std::string err = CheckPortsConnections();
	if (!err.empty()) return err;

	// determine and check calculation sequence
	err = m_calculationSequence.Check();
	if (!err.empty() || m_topologyModified)
	{
		DetermineCalculationSequence();
		err = m_calculationSequence.Check();
		if (!err.empty()) return err;
	}
	SetTopologyModified(false);

	// load and check external solvers in units
	for (auto& unit : m_units)
	{
		err = unit->InitializeExternalSolvers();
		if (!err.empty()) return err;
	}

	// check compounds
	if (m_materialsDB->CompoundsNumber() == 0)
		return StrConst::Flow_ErrEmptyMDB;
	if (GetCompounds().empty())
		return StrConst::Flow_ErrNoCompounds;
	for (const auto& key : GetCompounds())
		if (!m_materialsDB->GetCompound(key))
			return StrConst::Flow_ErrWrongCompound(key);
	for (const auto& unit : m_units)
		for (const auto& param : unit->GetModel()->GetUnitParametersManager().GetAllCompoundParameters())
			if (!m_materialsDB->GetCompound(param->GetCompound()))
				return StrConst::Flow_ErrWrongCompoundParam(unit->GetName(), param->GetName(), param->GetCompound());

	// check phases
	if (m_phases.empty())
		return StrConst::Flow_ErrNoPhases;

	// check for empty feeds and holdups
	for (const auto& unit : m_units)
		for (const auto& h : unit->GetModel()->GetStreamsManager().GetAllInit())
			if (h->GetAllTimePoints().empty())
				return StrConst::Flow_ErrEmptyHoldup(unit->GetName(), h->GetName());

	// check that mass fractions in feeds and holdups add to 1
	for (const auto& unit : m_units)
		for (const auto& h : unit->GetModel()->GetStreamsManager().GetAllInit())
			for (double t : h->GetAllTimePoints())
			{
				if (h->GetMass(t) == 0.0) continue;
				// check phase fractions
				const double phaseSum = std::accumulate(m_phases.begin(), m_phases.end(), 0.0, [&](double a, const auto& p) { return a + h->GetPhaseFraction(t, p.state); });
				if (std::fabs(phaseSum - 1.0) > 1e-10)
					return StrConst::Flow_ErrPhaseFractions(unit->GetName(), h->GetName(), t);
				// check compound fractions
				const auto& compounds = GetCompounds();
				for (const auto& p : m_phases)
				{
					const double compSum = std::accumulate(compounds.begin(), compounds.end(), 0.0, [&](double a, const auto& c) { return a + h->GetCompoundFraction(t, c, p.state); });
					if (std::fabs(compSum - 1.0) > 1e-10)
						return StrConst::Flow_ErrCompoundFractions(unit->GetName(), h->GetName(), p.name, t);
				}
			}

	return {};
}

void CFlowsheet::SetStreamsToPorts()
{
	// setup output streams with proper grids
	for (const auto& unit : m_units)
		for (const auto* port : unit->GetModel()->GetPortsManager().GetAllOutputPorts())
			if (auto* str = DoGetStream(port->GetStreamKey(), m_streams))
				str->SetGrid(unit->GetModel()->GetGrid());

	// create input streams with proper grids
	m_streamsI = m_streams; // copy pointers to main (output) streams
	for (const auto& unit : m_units)
		for (const auto* port : unit->GetModel()->GetPortsManager().GetAllInputPorts())
		{
			// find the corresponding stream in the list
			const size_t index = VectorFind(m_streamsI, [&](auto& s) { return s->GetKey() == port->GetStreamKey(); });
			if (index < m_streamsI.size() && unit->GetModel()->GetGrid() != m_streamsI[index]->GetGrid()) // separate input stream is needed
				// create new with proper grid
				m_streamsI[index] = std::make_shared<CStream>(m_streamsI[index]->GetKey(), m_materialsDB, unit->GetModel()->GetGrid(), &m_overall, &m_phases, &m_cacheStreams, &m_tolerance, &m_thermodynamics);
		}

	// set stream pointers to ports
	for (const auto& unit : m_units)
	{
		for (auto* port : unit->GetModel()->GetPortsManager().GetAllInputPorts())
			if (auto* str = DoGetStream(port->GetStreamKey(), m_streamsI))
				port->SetStream(str);
		for (auto* port : unit->GetModel()->GetPortsManager().GetAllOutputPorts())
			if (auto* str = DoGetStream(port->GetStreamKey(), m_streams))
				port->SetStream(str);
	}
}

std::string CFlowsheet::CheckPortsConnections()
{
	// check that all ports have connected stream
	for (const auto& unit : m_units)
		for (const auto& port : unit->GetModel()->GetPortsManager().GetAllPorts())
			if (!port->GetStream())
				return StrConst::Flow_ErrUnconnectedPorts(unit->GetName(), port->GetName());

	// check that each stream is connected either to only one input and one output port or not connected at all
	for (const auto& stream : m_streams)
	{
		size_t nPortsI = 0, nPortsO = 0;
		for (const auto& unit : m_units)
		{
			for (const auto& port : unit->GetModel()->GetPortsManager().GetAllInputPorts())
				if (port->GetStreamKey() == stream->GetKey())
					++nPortsI;
			for (const auto& port : unit->GetModel()->GetPortsManager().GetAllOutputPorts())
				if (port->GetStreamKey() == stream->GetKey())
					++nPortsO;
		}
		if (nPortsI != nPortsO || nPortsI != 1 && nPortsI != 0)
			return StrConst::Flow_ErrWrongStreams(stream->GetName());
	}

	return {};
}

void CFlowsheet::ClearSimulationResults()
{
	for (auto& stream : m_streams)
		stream->RemoveAllTimePoints();
	m_streamsI.clear();
	for (auto& unit : m_units)
		if (auto* model = unit->GetModel())
			model->ClearSimulationResults();
}

void CFlowsheet::SetMainGrid(const CMultidimensionalGrid& _grid)
{
	for (auto& unit : m_units)
		if (unit->GetModel() && unit->GetModel()->GetGrid() == m_mainGrid)
			unit->GetModel()->SetGrid(_grid);
	for (auto& stream : m_streams)
		if (stream->GetGrid() == m_mainGrid)
			stream->SetGrid(_grid);
	m_mainGrid = _grid;
	m_calculationSequence.UpdateInitialStreams();
}

void CFlowsheet::UpdateGrids()
{
	std::vector<std::string> updated;
	// update in connected streams
	for (auto& unit : m_units)
		if (unit->GetModel())
			for (const auto& port : unit->GetModel()->GetPortsManager().GetAllOutputPorts())
				if (auto* stream = DoGetStream(port->GetStreamKey(), m_streams))
				{
					stream->SetGrid(unit->GetModel()->GetGrid());
					updated.push_back(stream->GetKey());
				}
	// update in the rest streams
	for (auto& key : VectorDifference(GetAllStreamsKeys(), updated))
		GetStream(key)->SetGrid(m_mainGrid);
	// update initial tear streams
	m_calculationSequence.UpdateInitialStreams();
}

void CFlowsheet::UpdateCacheSettings()
{
	m_cacheStreams = { m_parameters.cacheFlagStreams, m_parameters.cacheWindow, m_parameters.cachePath };
	m_cacheHoldups = { m_parameters.cacheFlagHoldups, m_parameters.cacheWindow, m_parameters.cachePath };

	for (auto& stream : m_streams)
		stream->SetCacheSettings(m_cacheStreams);
	for (auto& unit : m_units)
		if (auto* model = unit->GetModel())
			model->UpdateCacheSettings();
	m_calculationSequence.UpdateCacheSettings(m_cacheStreams);
}

void CFlowsheet::UpdateToleranceSettings()
{
	m_tolerance = { m_parameters.absTol, m_parameters.relTol, m_parameters.minFraction };

	for (auto& stream : m_streams)
		stream->SetToleranceSettings(m_tolerance);
	for (auto& unit : m_units)
		if (auto* model = unit->GetModel())
			model->UpdateToleranceSettings();
	m_calculationSequence.UpdateToleranceSettings(m_tolerance);
}

void CFlowsheet::UpdateThermodynamicsSettings()
{
	m_thermodynamics = { { m_parameters.enthalpyMinT, m_parameters.enthalpyMaxT }, m_parameters.enthalpyInt };

	for (auto& stream : m_streams)
		stream->SetThermodynamicsSettings(m_thermodynamics);
	for (auto& unit : m_units)
		if (auto* model = unit->GetModel())
			model->UpdateThermodynamicsSettings();
	m_calculationSequence.UpdateThermodynamicsSettings(m_thermodynamics);
}

void CFlowsheet::SetMaterialDatabase(const CMaterialsDatabase* _materialsDB)
{
	m_materialsDB = _materialsDB;

	for (size_t i = 0; i < m_units.size(); i++)
		m_units[i]->SetMaterialsDatabase(m_materialsDB);
	for (size_t i = 0; i < m_streams.size(); i++)
		m_streams[i]->SetMaterialsDatabase(m_materialsDB);
	for (size_t i = 0; i < m_streamsI.size(); i++)
		m_streamsI[i]->SetMaterialsDatabase(m_materialsDB);
}

const CMultidimensionalGrid& CFlowsheet::GetGrid() const
{
	return m_mainGrid;
}

const CMaterialsDatabase& CFlowsheet::GetMaterialDatabase() const
{
	return *m_materialsDB;
}

void CFlowsheet::SetMaterialsDatabase(const CMaterialsDatabase* _materialsDB)
{
	m_materialsDB = _materialsDB;
	for (const auto& unit : m_units)
		unit->SetMaterialsDatabase(m_materialsDB);
	for (const auto& stream : m_streams)
		stream->SetMaterialsDatabase(m_materialsDB);
	for (const auto& stream : m_streamsI)
		stream->SetMaterialsDatabase(m_materialsDB);
}

const CParametersHolder* CFlowsheet::GetParameters() const
{
	return &m_parameters;
}

CParametersHolder* CFlowsheet::GetParameters()
{
	return &m_parameters;
}

bool CFlowsheet::SaveToFile(CH5Handler& _h5File, const std::string& _path)
{
	// current version of save procedure
	_h5File.WriteAttribute(_path, StrConst::H5AttrSaveVersion, m_saveVersion);

	// units
	_h5File.WriteAttribute(_path, StrConst::Flow_H5AttrUnitsNum, static_cast<int>(m_units.size()));
	const std::string unitsGroup = _h5File.CreateGroup(_path, StrConst::Flow_H5GroupUnits);
	for (size_t i = 0; i < m_units.size(); ++i)
		m_units[i]->SaveToFile(_h5File, _h5File.CreateGroup(unitsGroup, StrConst::Flow_H5GroupUnitName + std::to_string(i)));

	// streams
	_h5File.WriteAttribute(_path, StrConst::Flow_H5AttrStreamsNum, static_cast<int>(m_streams.size()));
	const std::string streamsGroup = _h5File.CreateGroup(_path, StrConst::Flow_H5GroupStreams);
	for (size_t i = 0; i < m_streams.size(); ++i)
		m_streams[i]->SaveToFile(_h5File, _h5File.CreateGroup(streamsGroup, StrConst::Flow_H5GroupStreamName + std::to_string(i)));

	// calculation sequence
	m_calculationSequence.SaveToFile(_h5File, _h5File.CreateGroup(_path, StrConst::Flow_H5GroupCalcSeq));

	// distributions grid
	m_mainGrid.SaveToFile(_h5File, _h5File.CreateGroup(_path, StrConst::H5GroupDistrGrid));

	// overall properties
	const std::string overallGroup = _h5File.CreateGroup(_path, StrConst::Flow_H5GroupOveralls);
	_h5File.WriteAttribute(overallGroup, StrConst::Flow_H5AttrOverallsNum, static_cast<int>(m_overall.size()));
	for (size_t i = 0; i < m_overall.size(); ++i)
	{
		const std::string group = _h5File.CreateGroup(overallGroup, StrConst::Flow_H5GroupOverallName + std::to_string(i));
		_h5File.WriteData(group, StrConst::Flow_H5OverallType,  E2I(m_overall[i].type));
		_h5File.WriteData(group, StrConst::Flow_H5OverallName,  m_overall[i].name);
		_h5File.WriteData(group, StrConst::Flow_H5OverallUnits, m_overall[i].units);
	}

	// phases
	const std::string phasesGroup = _h5File.CreateGroup(_path, StrConst::Flow_H5GroupPhases);
	_h5File.WriteAttribute(phasesGroup, StrConst::Flow_H5AttrPhasesNum, static_cast<int>(m_phases.size()));
	for (size_t i = 0; i < m_phases.size(); ++i)
	{
		const std::string group = _h5File.CreateGroup(phasesGroup, StrConst::Flow_H5GroupPhaseName + std::to_string(i));
		_h5File.WriteData(group, StrConst::Flow_H5PhaseType, E2I(m_phases[i].state));
		_h5File.WriteData(group, StrConst::Flow_H5PhaseName, m_phases[i].name);
	}

	// parameters
	m_parameters.SaveToFile(_h5File, _h5File.CreateGroup(_path, StrConst::Flow_H5GroupOptions));

	return true;
}

bool CFlowsheet::LoadFromFile(CH5Handler& _h5File, const std::string& _path)
{
	Clear();

	// version of save procedure
	const int version = _h5File.ReadAttribute(_path, StrConst::H5AttrSaveVersion);
	if (version < 4)
	{
		return LoadFromFile_v3(_h5File, _path);
	}

	// parameters
	m_parameters.LoadFromFile(_h5File, _path + "/" + StrConst::Flow_H5GroupOptions);
	UpdateToleranceSettings();		// needed to fill global tolerance structure with possibly updated data
	UpdateCacheSettings();			// needed to fill global cache structure with possibly updated data
	UpdateThermodynamicsSettings();	// needed to fill global thermodynamics structure with possibly updated data

	// distributions grid
	m_mainGrid.LoadFromFile(_h5File, _path + "/" + StrConst::H5GroupDistrGrid);
	if (version < 5)
	{
		// replace compound names with keys
		std::vector<std::string> keys;
		_h5File.ReadData(_path + "/", StrConst::Flow_H5Compounds, keys);
		m_mainGrid.GetGridDimensionSymbolic(DISTR_COMPOUNDS)->SetGrid(keys);
	}

	// overall properties
	const std::string overallsGroup = _path + "/" + StrConst::Flow_H5GroupOveralls;
	const int nOverallProperties = _h5File.ReadAttribute(overallsGroup, StrConst::Flow_H5AttrOverallsNum);
	m_overall.resize(nOverallProperties);
	for (size_t i = 0; i < m_overall.size(); ++i)
	{
		const std::string group = overallsGroup + "/" + StrConst::Flow_H5GroupOverallName + std::to_string(i);
		uint32_t type;
		_h5File.ReadData(group, StrConst::Flow_H5OverallType, type);
		m_overall[i].type = static_cast<EOverall>(type);
		_h5File.ReadData(group, StrConst::Flow_H5OverallName, m_overall[i].name);
		_h5File.ReadData(group, StrConst::Flow_H5OverallUnits, m_overall[i].units);
	}

	// phases
	const std::string phasesGroup = _path + "/" + StrConst::Flow_H5GroupPhases;
	const int nPhases = _h5File.ReadAttribute(phasesGroup, StrConst::Flow_H5AttrPhasesNum);
	m_phases.resize(nPhases);
	for (size_t i = 0; i < m_phases.size(); ++i)
	{
		const std::string group = phasesGroup + "/" + StrConst::Flow_H5GroupPhaseName + std::to_string(i);
		uint32_t type;
		_h5File.ReadData(group, StrConst::Flow_H5PhaseType, type);
		m_phases[i].state = static_cast<EPhase>(type);
		_h5File.ReadData(group, StrConst::Flow_H5PhaseName, m_phases[i].name);
	}

	// streams
	const size_t nStreams = static_cast<size_t>(_h5File.ReadAttribute(_path + "/", StrConst::Flow_H5AttrStreamsNum));
	for (size_t i = 0; i < nStreams; ++i)
		AddStream()->LoadFromFile(_h5File, _path + "/" + StrConst::Flow_H5GroupStreams + "/" + StrConst::Flow_H5GroupStreamName + std::to_string(i));

	// models
	const size_t nUnits = static_cast<size_t>(_h5File.ReadAttribute(_path + "/", StrConst::Flow_H5AttrUnitsNum));
	for (size_t i = 0; i < nUnits; ++i)
		AddUnit()->LoadFromFile(_h5File, _path + "/" + StrConst::Flow_H5GroupUnits + "/" + StrConst::Flow_H5GroupUnitName + std::to_string(i));

	// calculation sequence
	m_calculationSequence.LoadFromFile(_h5File, _path + "/" + StrConst::Flow_H5GroupCalcSeq);

	SetTopologyModified(false);

	return true;
}

bool CFlowsheet::LoadFromFile_v3(CH5Handler& _h5File, const std::string& _path)
{
	const std::string root = "/";

	// parameters
	m_parameters.LoadFromFile(_h5File, root + StrConst::Flow_H5GroupOptions);
	UpdateToleranceSettings();	// needed to fill global tolerance structure with possibly updated data
	UpdateCacheSettings();		// needed to fill global cache structure with possibly updated data

	// distributions grid
	m_mainGrid.LoadFromFile(_h5File, root + StrConst::H5GroupDistrGrid);
	// replace compound names with keys
	std::vector<std::string> keys;
	_h5File.ReadData(root, StrConst::Flow_H5Compounds, keys);
	m_mainGrid.GetGridDimensionSymbolic(DISTR_COMPOUNDS)->SetGrid(keys);

	// overall properties
	AddOverallProperty(EOverall::OVERALL_MASS,        "Mass flow",   "kg/s");
	AddOverallProperty(EOverall::OVERALL_TEMPERATURE, "Temperature", "K");
	AddOverallProperty(EOverall::OVERALL_PRESSURE,    "Pressure",    "Pa");

	// phases
	const auto& PhaseSOA2EPhase = [](uint32_t _soa)
	{
		switch (_soa)
		{
		case SOA_SOLID:		return EPhase::SOLID;
		case SOA_LIQUID:	return EPhase::LIQUID;
		case SOA_VAPOR:		return EPhase::VAPOR;
		default:			return EPhase::UNDEFINED;
		}
	};
	std::vector<std::string> names;
	std::vector<uint32_t> states;
	_h5File.ReadData(root + StrConst::Flow_H5GroupPhases, StrConst::Flow_H5PhasesNames, names);
	_h5File.ReadData(root + StrConst::Flow_H5GroupPhases, StrConst::Flow_H5PhasesSOA, states);
	for (size_t i = 0; i < names.size(); ++i)
		AddPhase(PhaseSOA2EPhase(states[i]), names[i]);

	// streams
	const size_t nStreams = static_cast<size_t>(_h5File.ReadAttribute(root, StrConst::Flow_H5AttrStreamsNum));
	for (size_t i = 0; i < nStreams; ++i)
		AddStream()->LoadFromFile(_h5File, root + StrConst::Flow_H5GroupStreams + "/" + StrConst::Flow_H5GroupStreamName + std::to_string(i));
	// ensure stream keys are unique
	for (size_t i = 0; i < m_streams.size(); ++i)
		for (size_t j = i + 1; j < m_streams.size(); ++j)
			if (m_streams[i]->GetKey() == m_streams[j]->GetKey())
				m_streams[j]->SetKey(StringFunctions::GenerateUniqueKey(GetAllStreamsKeys()));

	// load models
	const size_t nUnits = static_cast<size_t>(_h5File.ReadAttribute(root, StrConst::Flow_H5AttrUnitsNum));
	for (size_t i = 0; i < nUnits; ++i)
		AddUnit()->LoadFromFile(_h5File, root + StrConst::Flow_H5GroupUnits + "/" + StrConst::Flow_H5GroupUnitName + std::to_string(i));
	// ensure unit keys are unique
	for (size_t i = 0; i < m_units.size(); ++i)
		for (size_t j = i + 1; j < m_units.size(); ++j)
			if (m_units[i]->GetKey() == m_units[j]->GetKey())
				m_units[j]->SetKey(StringFunctions::GenerateUniqueKey(GetAllUnitsKeys()));

	// calculation sequence
	m_calculationSequence.LoadFromFile(_h5File, root + StrConst::Flow_H5GroupCalcSeq);

	SetTopologyModified(false);
	return true;
}

CStream* CFlowsheet::DoGetStream(const std::string& _key, const std::vector<std::shared_ptr<CStream>>& _streams)
{
	for (auto& stream : _streams)
		if (stream->GetKey() == _key)
			return stream.get();
	return nullptr;
}

std::vector<std::string> CFlowsheet::GetAllUnitsKeys() const
{
	std::vector<std::string> keys;
	for (const auto& unit : m_units)
		keys.push_back(unit->GetKey());
	return keys;
}

std::vector<std::string> CFlowsheet::GetAllStreamsKeys() const
{
	std::vector<std::string> keys;
	for (const auto& unit : m_streams)
		keys.push_back(unit->GetKey());
	return keys;
}

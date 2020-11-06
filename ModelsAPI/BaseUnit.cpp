/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "BaseUnit.h"
#include "TransformMatrix.h"
#include "TimeDependentValue.h"
#include "DistributionsGrid.h"
#include "ContainerFunctions.h"
#include "DyssolUtilities.h"
#include "DyssolStringConstants.h"
#include <stdexcept>
#include <numeric>

void CBaseUnit::SetPointers(const CMaterialsDatabase* _materialsDB, const CDistributionsGrid* _grid, const std::vector<std::string>* _compounds, const std::vector<SOverallDescriptor>* _overall,
	const std::vector<SPhaseDescriptor>* _phases, const SCacheSettings* _cache, const SToleranceSettings* _tolerance)
{
	m_materialsDB  = _materialsDB;
	m_grid         = _grid;
	m_compounds    = _compounds;
	m_overall      = _overall;
	m_phases       = _phases;
	m_cache        = _cache;
	m_tolerance    = _tolerance;
	m_streams.SetPointers(m_materialsDB, m_grid, m_compounds, m_overall, m_phases, m_cache, m_tolerance);
	m_lookupTables.SetPointers(m_materialsDB, m_compounds);
}

std::string CBaseUnit::GetUnitName() const
{
	return m_unitName;
}

std::string CBaseUnit::GetAuthorName() const
{
	return m_authorName;
}

size_t CBaseUnit::GetVersion() const
{
	return m_version;
}

std::string CBaseUnit::GetUniqueID() const
{
	return m_uniqueID;
}

void CBaseUnit::SetUnitName(const std::string& _name)
{
	m_unitName = _name;
}

void CBaseUnit::SetAuthorName(const std::string& _author)
{
	m_authorName = _author;
}

void CBaseUnit::SetVersion(size_t _version)
{
	m_version = _version;
}

void CBaseUnit::SetUniqueID(const std::string& _id)
{
	m_uniqueID = _id;
}

const CPortsManager& CBaseUnit::GetPortsManager() const
{
	return m_ports;
}

CPortsManager& CBaseUnit::GetPortsManager()
{
	return m_ports;
}

CUnitPort* CBaseUnit::AddPort(const std::string& _portName, EUnitPort _type)
{
	if (m_ports.GetPort(_portName)) // already exists
		throw std::logic_error(StrConst::BUnit_ErrAddPort(m_unitName, _portName, __func__));
	return m_ports.AddPort(_portName, _type);
}

const CUnitPort* CBaseUnit::GetPort(const std::string& _portName) const
{
	if (const auto* port = m_ports.GetPort(_portName))
		return port;
	throw std::logic_error(StrConst::BUnit_ErrGetPort(m_unitName, _portName, __func__));
}

CUnitPort* CBaseUnit::GetPort(const std::string& _portName)
{
	return const_cast<CUnitPort*>(const_cast<const CBaseUnit&>(*this).GetPort(_portName));
}

CStream* CBaseUnit::GetPortStream(const std::string& _portName) const
{
	const auto* port = m_ports.GetPort(_portName);
	if (!port)
		throw std::logic_error(StrConst::BUnit_ErrGetPort(m_unitName, _portName, __func__));
	if (!port->GetStream())
		throw std::logic_error(StrConst::BUnit_ErrGetPortEmpty(m_unitName, _portName, __func__));
	return port->GetStream();
}

const CStreamManager& CBaseUnit::GetStreamsManager() const
{
	return m_streams;
}

CStreamManager& CBaseUnit::GetStreamsManager()
{
	return m_streams;
}

CStream* CBaseUnit::AddFeed(const std::string& _name)
{
	if (m_streams.GetFeed(_name)) // already exists
		throw std::logic_error(StrConst::BUnit_ErrAddFeed(m_unitName, _name, __func__));
	return m_streams.AddFeed(_name);
}

CStream* CBaseUnit::GetFeed(const std::string& _name)
{
	if (CStream* feed = m_streams.GetFeed(_name))
		return feed;
	throw std::logic_error(StrConst::BUnit_ErrGetFeed(m_unitName, _name, __func__));
}

void CBaseUnit::RemoveFeed(const std::string& _name)
{
	if (!m_streams.GetFeed(_name))
		throw std::logic_error(StrConst::BUnit_ErrGetFeed(m_unitName, _name, __func__));
	m_streams.RemoveFeed(_name);
}

CHoldup* CBaseUnit::AddHoldup(const std::string& _name)
{
	if (m_streams.GetHoldup(_name)) // already exists
		throw std::logic_error(StrConst::BUnit_ErrAddHoldup(m_unitName, _name, __func__));
	return m_streams.AddHoldup(_name);
}

CHoldup* CBaseUnit::GetHoldup(const std::string& _name)
{
	if (CHoldup* holdup = m_streams.GetHoldup(_name))
		return holdup;
	throw std::logic_error(StrConst::BUnit_ErrGetHoldup(m_unitName, _name, __func__));
}

void CBaseUnit::RemoveHoldup(const std::string& _name)
{
	if (!m_streams.GetHoldup(_name))
		throw std::logic_error(StrConst::BUnit_ErrGetHoldup(m_unitName, _name, __func__));
	m_streams.RemoveHoldup(_name);
}

CStream* CBaseUnit::AddStream(const std::string& _name)
{
	if (m_streams.GetStream(_name)) // already exists
		throw std::logic_error(StrConst::BUnit_ErrAddStream(m_unitName, _name, __func__));
	return m_streams.AddStream(_name);
}

CStream* CBaseUnit::GetStream(const std::string& _name)
{
	if (CStream* stream = m_streams.GetStream(_name))
		return stream;
	throw std::logic_error(StrConst::BUnit_ErrGetStream(m_unitName, _name, __func__));
}

void CBaseUnit::RemoveStream(const std::string& _name)
{
	if (!m_streams.GetStream(_name))
		throw std::logic_error(StrConst::BUnit_ErrGetStream(m_unitName, _name, __func__));
	m_streams.RemoveStream(_name);
}

void CBaseUnit::SetupStream(CBaseStream* _stream) const
{
	if (!_stream) return;
	_stream->Clear();
	_stream->SetGrid(m_grid);
	_stream->SetMaterialsDatabase(m_materialsDB);
	m_streams.SetupStreamStructure(*_stream);
}

const CUnitParametersManager& CBaseUnit::GetUnitParametersManager() const
{
	return m_unitParameters;
}

CUnitParametersManager& CBaseUnit::GetUnitParametersManager()
{
	return m_unitParameters;
}

CConstRealUnitParameter* CBaseUnit::AddConstRealParameter(const std::string& _name, double _initValue, const std::string& _units, const std::string& _description, double _minValue, double _maxValue)
{
	if (m_unitParameters.IsNameExist(_name))
		throw std::logic_error(StrConst::BUnit_ErrAddParam(m_unitName, _name, __func__));
	m_unitParameters.AddConstRealParameter(_name, _units, _description, _minValue, _maxValue, _initValue);
	return m_unitParameters.GetConstRealParameter(_name);
}

CConstIntUnitParameter* CBaseUnit::AddConstIntParameter(const std::string& _name, int64_t _initValue, const std::string& _units, const std::string& _description, int64_t _minValue, int64_t _maxValue)
{
	if (m_unitParameters.IsNameExist(_name))
		throw std::logic_error(StrConst::BUnit_ErrAddParam(m_unitName, _name, __func__));
	m_unitParameters.AddConstIntParameter(_name, _units, _description, _minValue, _maxValue, _initValue);
	return m_unitParameters.GetConstIntParameter(_name);
}

CConstUIntUnitParameter* CBaseUnit::AddConstUIntParameter(const std::string& _name, uint64_t _initValue, const std::string& _units, const std::string& _description, uint64_t _minValue, uint64_t _maxValue)
{
	if (m_unitParameters.IsNameExist(_name))
		throw std::logic_error(StrConst::BUnit_ErrAddParam(m_unitName, _name, __func__));
	m_unitParameters.AddConstUIntParameter(_name, _units, _description, _minValue, _maxValue, _initValue);
	return m_unitParameters.GetConstUIntParameter(_name);
}

CTDUnitParameter* CBaseUnit::AddTDParameter(const std::string& _name, double _initValue, const std::string& _units, const std::string& _description, double _minValue, double _maxValue)
{
	if (m_unitParameters.IsNameExist(_name))
		throw std::logic_error(StrConst::BUnit_ErrAddParam(m_unitName, _name, __func__));
	m_unitParameters.AddTDParameter(_name, _units, _description, _minValue, _maxValue, _initValue);
	return m_unitParameters.GetTDParameter(_name);
}

CStringUnitParameter* CBaseUnit::AddStringParameter(const std::string& _name, const std::string& _initValue, const std::string& _description)
{
	if (m_unitParameters.IsNameExist(_name))
		throw std::logic_error(StrConst::BUnit_ErrAddParam(m_unitName, _name, __func__));
	m_unitParameters.AddStringParameter(_name, _description, _initValue);
	return m_unitParameters.GetStringParameter(_name);
}

CCheckBoxUnitParameter* CBaseUnit::AddCheckBoxParameter(const std::string& _name, bool _initValue, const std::string& _description)
{
	if (m_unitParameters.IsNameExist(_name))
		throw std::logic_error(StrConst::BUnit_ErrAddParam(m_unitName, _name, __func__));
	m_unitParameters.AddCheckBoxParameter(_name, _description, _initValue);
	return m_unitParameters.GetCheckboxParameter(_name);
}

CComboUnitParameter* CBaseUnit::AddComboParameter(const std::string& _name, size_t _initValue, const std::vector<size_t>& _items, const std::vector<std::string>& _itemsNames, const std::string& _description)
{
	if (m_unitParameters.IsNameExist(_name))
		throw std::logic_error(StrConst::BUnit_ErrAddParam(m_unitName, _name, __func__));
	std::vector<size_t> values = _items;
	if (values.empty())
	{
		values.resize(_itemsNames.size());
		std::iota(values.begin(), values.end(), 0);
	}

	if (values.size() != _itemsNames.size())
		throw std::logic_error(StrConst::BUnit_ErrAddComboParam(m_unitName, _name, __func__));
	m_unitParameters.AddComboParameter(_name, _description, _initValue, _items, _itemsNames);
	return m_unitParameters.GetComboParameter(_name);
}

CCompoundUnitParameter* CBaseUnit::AddCompoundParameter(const std::string& _name, const std::string& _description)
{
	if (m_unitParameters.IsNameExist(_name))
		throw std::logic_error(StrConst::BUnit_ErrAddParam(m_unitName, _name, __func__));
	m_unitParameters.AddCompoundParameter(_name, _description);
	return m_unitParameters.GetCompoundParameter(_name);
}

CAgglomerationSolver* CBaseUnit::AddSolverAgglomeration(const std::string& _name, const std::string& _description)
{
	if (m_unitParameters.IsNameExist(_name))
		throw std::logic_error(StrConst::BUnit_ErrAddParam(m_unitName, _name, __func__));
	m_unitParameters.AddSolverParameter(_name, _description, ESolverTypes::SOLVER_AGGLOMERATION_1);
	return dynamic_cast<CAgglomerationSolver*>(m_unitParameters.GetSolverParameter(_name));
}

CPBMSolver* CBaseUnit::AddSolverPBM(const std::string& _name, const std::string& _description)
{
	if (m_unitParameters.IsNameExist(_name))
		throw std::logic_error(StrConst::BUnit_ErrAddParam(m_unitName, _name, __func__));
	m_unitParameters.AddSolverParameter(_name, _description, ESolverTypes::SOLVER_PBM_1);
	return dynamic_cast<CPBMSolver*>(m_unitParameters.GetSolverParameter(_name));
}

void CBaseUnit::AddParametersToGroup(const std::string& _unitParamName, const std::string& _unitParamValueName, const std::vector<std::string>& _groupedParamNames)
{
	const auto* groupParameter = m_unitParameters.GetComboParameter(_unitParamName);
	if (!groupParameter)								// check that group parameter exists
		throw std::logic_error(StrConst::BUnit_ErrGroupParamBlock(m_unitName, _unitParamName, _unitParamValueName, __func__));
	if (!groupParameter->HasName(_unitParamValueName))	// check that group exists
		throw std::logic_error(StrConst::BUnit_ErrGroupParamGroup(m_unitName, _unitParamName, _unitParamValueName, __func__));
	for (const auto& name : _groupedParamNames)			// check that all parameters exist
		if (!m_unitParameters.GetParameter(name))
			throw std::logic_error(StrConst::BUnit_ErrGroupParamParam(m_unitName, _unitParamName, _unitParamValueName, name, __func__));
	m_unitParameters.AddParametersToGroup(_unitParamName, _unitParamValueName, _groupedParamNames);
}

double CBaseUnit::GetConstRealParameterValue(const std::string& _name) const
{
	if (const CConstRealUnitParameter* param = m_unitParameters.GetConstRealParameter(_name))
		return param->GetValue();
	throw std::logic_error(StrConst::BUnit_ErrGetParam(m_unitName, _name, __func__));
}

int64_t CBaseUnit::GetConstIntParameterValue(const std::string& _name) const
{
	if (const CConstIntUnitParameter* param = m_unitParameters.GetConstIntParameter(_name))
		return param->GetValue();
	throw std::logic_error(StrConst::BUnit_ErrGetParam(m_unitName, _name, __func__));
}

uint64_t CBaseUnit::GetConstUIntParameterValue(const std::string& _name) const
{
	if (const CConstUIntUnitParameter* param = m_unitParameters.GetConstUIntParameter(_name))
		return param->GetValue();
	throw std::logic_error(StrConst::BUnit_ErrGetParam(m_unitName, _name, __func__));
}

double CBaseUnit::GetTDParameterValue(const std::string& _name, double _time) const
{
	if (const CTDUnitParameter* param = m_unitParameters.GetTDParameter(_name))
		return param->GetValue(_time);
	throw std::logic_error(StrConst::BUnit_ErrGetParam(m_unitName, _name, __func__));
}

std::string CBaseUnit::GetStringParameterValue(const std::string& _name) const
{
	if (const CStringUnitParameter* param = m_unitParameters.GetStringParameter(_name))
		return param->GetValue();
	throw std::logic_error(StrConst::BUnit_ErrGetParam(m_unitName, _name, __func__));
}

bool CBaseUnit::GetCheckboxParameterValue(const std::string& _name) const
{
	if (const CCheckBoxUnitParameter* param = m_unitParameters.GetCheckboxParameter(_name))
		return param->IsChecked();
	throw std::logic_error(StrConst::BUnit_ErrGetParam(m_unitName, _name, __func__));
}

size_t CBaseUnit::GetComboParameterValue(const std::string& _name) const
{
	if (const CComboUnitParameter* param = m_unitParameters.GetComboParameter(_name))
		return param->GetValue();
	throw std::logic_error(StrConst::BUnit_ErrGetParam(m_unitName, _name, __func__));
}

std::string CBaseUnit::GetCompoundParameterValue(const std::string& _name) const
{
	if (const CCompoundUnitParameter* param = m_unitParameters.GetCompoundParameter(_name))
		return param->GetCompound();
	throw std::logic_error(StrConst::BUnit_ErrGetParam(m_unitName, _name, __func__));
}

CAgglomerationSolver* CBaseUnit::GetSolverAgglomeration(const std::string& _name) const
{
	if (const CSolverUnitParameter* param = m_unitParameters.GetSolverParameter(_name))
		return dynamic_cast<CAgglomerationSolver*>(param->GetSolver());
	throw std::logic_error(StrConst::BUnit_ErrGetParam(m_unitName, _name, __func__));
}

CPBMSolver* CBaseUnit::GetSolverPBM(const std::string& _name) const
{
	if (const CSolverUnitParameter* param = m_unitParameters.GetSolverParameter(_name))
		return dynamic_cast<CPBMSolver*>(param->GetSolver());
	throw std::logic_error(StrConst::BUnit_ErrGetParam(m_unitName, _name, __func__));
}

const CStateVariablesManager& CBaseUnit::GetStateVariablesManager() const
{
	return m_stateVariables;
}

CStateVariablesManager& CBaseUnit::GetStateVariablesManager()
{
	return m_stateVariables;
}

CStateVariable* CBaseUnit::AddStateVariable(const std::string& _name, double _initValue)
{
	if (m_stateVariables.GetStateVariable(_name)) // already exists
		throw std::logic_error(StrConst::BUnit_ErrAddSV(m_unitName, _name, __func__));
	return m_stateVariables.AddStateVariable(_name, _initValue);
}

double CBaseUnit::GetStateVariable(const std::string& _name) const
{
	if (const CStateVariable* variable = m_stateVariables.GetStateVariable(_name))
		return variable->GetValue();
	throw std::logic_error(StrConst::BUnit_ErrGetSV(m_unitName, _name, __func__));
}

void CBaseUnit::SetStateVariable(const std::string& _name, double _value)
{
	if (CStateVariable* variable = m_stateVariables.GetStateVariable(_name))
		variable->SetValue(_value);
	else
		throw std::logic_error(StrConst::BUnit_ErrGetSV(m_unitName, _name, __func__));
}

void CBaseUnit::SetStateVariable(const std::string& _name, double _value, double _time)
{
	if (CStateVariable* variable = m_stateVariables.GetStateVariable(_name))
		variable->SetValue(_time, _value);
	else
		throw std::logic_error(StrConst::BUnit_ErrGetSV(m_unitName, _name, __func__));
}

const CPlotManager& CBaseUnit::GetPlotsManager() const
{
	return m_plots;
}

CPlotManager& CBaseUnit::GetPlotsManager()
{
	return m_plots;
}

CPlot* CBaseUnit::AddPlot(const std::string& _plotName, const std::string& _labelX, const std::string& _labelY)
{
	if (m_plots.GetPlot(_plotName))
		throw std::logic_error(StrConst::BUnit_ErrAddPlot(m_unitName, _plotName, __func__));
	return m_plots.AddPlot(_plotName, _labelX, _labelY);
}

CPlot* CBaseUnit::AddPlot(const std::string& _plotName, const std::string& _labelX, const std::string& _labelY, const std::string& _labelZ)
{
	if (m_plots.GetPlot(_plotName))
		throw std::logic_error(StrConst::BUnit_ErrAddPlot(m_unitName, _plotName, __func__));
	return m_plots.AddPlot(_plotName, _labelX, _labelY, _labelZ);
}

CCurve* CBaseUnit::AddCurveOnPlot(const std::string& _plotName, const std::string& _curveName)
{
	auto* plot = m_plots.GetPlot(_plotName);
	if (!plot)
		throw std::logic_error(StrConst::BUnit_ErrGetPlot(m_unitName, _plotName, __func__));
	if (plot->GetCurve(_curveName))
		throw std::logic_error(StrConst::BUnit_ErrAddCurve2D(m_unitName, _plotName, _curveName, __func__));
	return plot->AddCurve(_curveName);
}

CCurve* CBaseUnit::AddCurveOnPlot(const std::string& _plotName, double _valueZ)
{
	auto* plot = m_plots.GetPlot(_plotName);
	if (!plot)
		throw std::logic_error(StrConst::BUnit_ErrGetPlot(m_unitName, _plotName, __func__));
	if (plot->GetCurve(_valueZ))
		throw std::logic_error(StrConst::BUnit_ErrAddCurve3D(m_unitName, _plotName, _valueZ, __func__));
	return plot->AddCurve(_valueZ);
}

void CBaseUnit::AddPointOnCurve(const std::string& _plotName, const std::string& _curveName, double _x, double _y)
{
	auto* plot = m_plots.GetPlot(_plotName);
	if (!plot)
		throw std::logic_error(StrConst::BUnit_ErrGetPlot(m_unitName, _plotName, __func__));
	auto* curve = plot->GetCurve(_curveName);
	if (!curve)
		throw std::logic_error(StrConst::BUnit_ErrGetCurve2D(m_unitName, _plotName, _curveName, __func__));
	curve->AddPoint(_x, _y);
}

void CBaseUnit::AddPointOnCurve(const std::string& _plotName, double _valueZ, double _x, double _y)
{
	auto* plot = m_plots.GetPlot(_plotName);
	if (!plot)
		throw std::logic_error(StrConst::BUnit_ErrGetPlot(m_unitName, _plotName, __func__));
	auto* curve = plot->GetCurve(_valueZ);
	if (!curve)
		throw std::logic_error(StrConst::BUnit_ErrGetCurve3D(m_unitName, _plotName, _valueZ, __func__));
	curve->AddPoint(_x, _y);
}

void CBaseUnit::AddPointsOnCurve(const std::string& _plotName, const std::string& _curveName, const std::vector<double>& _x, const std::vector<double>& _y)
{
	auto* plot = m_plots.GetPlot(_plotName);
	if (!plot)
		throw std::logic_error(StrConst::BUnit_ErrGetPlot(m_unitName, _plotName, __func__));
	auto* curve = plot->GetCurve(_curveName);
	if (!curve)
		throw std::logic_error(StrConst::BUnit_ErrGetCurve2D(m_unitName, _plotName, _curveName, __func__));
	curve->AddPoints(_x, _y);
}

void CBaseUnit::AddPointsOnCurve(const std::string& _plotName, double _valueZ, const std::vector<double>& _x, const std::vector<double>& _y)
{
	auto* plot = m_plots.GetPlot(_plotName);
	if (!plot)
		throw std::logic_error(StrConst::BUnit_ErrGetPlot(m_unitName, _plotName, __func__));
	auto* curve = plot->GetCurve(_valueZ);
	if (!curve)
		throw std::logic_error(StrConst::BUnit_ErrGetCurve3D(m_unitName, _plotName, _valueZ, __func__));
	curve->AddPoints(_x, _y);
}

void CBaseUnit::CopyStreamToStream(double _time, const CStream* _srcStream, CStream* _dstStream)
{
	_dstStream->CopyFromStream(_time, _srcStream);
}

void CBaseUnit::CopyStreamToStream(double _timeBeg, double _timeEnd, const CStream* _srcStream, CStream* _dstStream)
{
	_dstStream->CopyFromStream(_timeBeg, _timeEnd, _srcStream);
}

void CBaseUnit::CopyStreamToPort(double _time, const CStream* _stream, CUnitPort* _port) const
{
	if (!_port->GetStream())
		throw std::logic_error(StrConst::BUnit_ErrGetPortEmpty(m_unitName, "", __func__));
	if (_port->GetType() != EUnitPort::OUTPUT)
		throw std::logic_error(StrConst::BUnit_ErrCopyToPort(m_unitName, _stream->GetName(), _port->GetName(), __func__));
	_port->GetStream()->CopyFromStream(_time, _stream);
}

void CBaseUnit::CopyStreamToPort(double _time, const CStream* _stream, const std::string& _portName)
{
	auto* port = GetPort(_portName);
	if (!port)
		throw std::logic_error(StrConst::BUnit_ErrGetPort(m_unitName, _portName, __func__));
	CopyStreamToPort(_time, _stream, port);
}

void CBaseUnit::CopyStreamToPort(double _timeBeg, double _timeEnd, const CStream* _stream, CUnitPort* _port) const
{
	if (!_port->GetStream())
		throw std::logic_error(StrConst::BUnit_ErrGetPortEmpty(m_unitName, "", __func__));
	if (_port->GetType() != EUnitPort::OUTPUT)
		throw std::logic_error(StrConst::BUnit_ErrCopyToPort(m_unitName, _stream->GetName(), _port->GetName(), __func__));
	_port->GetStream()->CopyFromStream(_timeBeg, _timeEnd, _stream);
}

void CBaseUnit::CopyStreamToPort(double _timeBeg, double _timeEnd, const CStream* _stream, const std::string& _portName)
{
	auto* port = GetPort(_portName);
	if (!port)
		throw std::logic_error(StrConst::BUnit_ErrGetPort(m_unitName, _portName, __func__));
	CopyStreamToPort(_timeBeg, _timeEnd, _stream, port);
}

void CBaseUnit::CopyPortToStream(double _time, const CUnitPort* _port, CStream* _stream) const
{
	if (!_port->GetStream())
		throw std::logic_error(StrConst::BUnit_ErrGetPortEmpty(m_unitName, "", __func__));
	if (_port->GetType() != EUnitPort::INPUT)
		throw std::logic_error(StrConst::BUnit_ErrCopyFromPort(m_unitName, _stream->GetName(), _port->GetName(), __func__));
	_port->GetStream()->CopyFromStream(_time, _stream);
}

void CBaseUnit::CopyPortToStream(double _time, const std::string& _portName, CStream* _stream)
{
	const auto* port = GetPort(_portName);
	if (!port)
		throw std::logic_error(StrConst::BUnit_ErrGetPort(m_unitName, _portName, __func__));
	CopyPortToStream(_time, port, _stream);
}

void CBaseUnit::CopyPortToStream(double _timeBeg, double _timeEnd, const CUnitPort* _port, CStream* _stream) const
{
	if (!_port->GetStream())
		throw std::logic_error(StrConst::BUnit_ErrGetPortEmpty(m_unitName, "", __func__));
	if (_port->GetType() != EUnitPort::INPUT)
		throw std::logic_error(StrConst::BUnit_ErrCopyFromPort(m_unitName, _stream->GetName(), _port->GetName(), __func__));
	_port->GetStream()->CopyFromStream(_timeBeg, _timeEnd, _stream);
}

void CBaseUnit::CopyPortToStream(double _timeBeg, double _timeEnd, const std::string& _portName, CStream* _stream)
{
	const auto* port = GetPort(_portName);
	if (!port)
		throw std::logic_error(StrConst::BUnit_ErrGetPort(m_unitName, _portName, __func__));
	CopyPortToStream(_timeBeg, _timeEnd, port, _stream);
}

std::vector<double> CBaseUnit::GetAllTimePoints(double _timeBeg, double _timeEnd) const
{
	return VectorsUnionSorted(GetInputTimePoints(_timeBeg, _timeEnd), m_unitParameters.GetAllTimePoints(_timeBeg, _timeEnd));
}

std::vector<double> CBaseUnit::GetAllTimePointsClosed(double _timeBeg, double _timeEnd) const
{
	return CloseInterval(GetAllTimePoints(_timeBeg, _timeEnd), _timeBeg, _timeEnd);
}

std::vector<double> CBaseUnit::GetInputTimePoints(double _timeBeg, double _timeEnd) const
{
	std::vector<double> res;
	for (const auto& port : m_ports.GetAllInputPorts())
		res = VectorsUnionSorted(res, port->GetStream()->GetTimePoints(_timeBeg, _timeEnd));
	return res;
}

std::vector<double> CBaseUnit::GetInputTimePointsClosed(double _timeBeg, double _timeEnd) const
{
	return CloseInterval(GetInputTimePoints(_timeBeg, _timeEnd), _timeBeg, _timeEnd);
}

std::vector<double> CBaseUnit::GetStreamsTimePoints(double _timeBeg, double _timeEnd, const std::vector<CStream*>& _srteams) const
{
	std::vector<double> res;
	for (const auto& srteam : _srteams)
		res = VectorsUnionSorted(res, srteam->GetTimePoints(_timeBeg, _timeEnd));
	return res;
}

std::vector<double> CBaseUnit::GetStreamsTimePointsClosed(double _timeBeg, double _timeEnd, const std::vector<CStream*>& _srteams) const
{
	return CloseInterval(GetStreamsTimePoints(_timeBeg, _timeEnd, _srteams), _timeBeg, _timeEnd);
}

void CBaseUnit::ReduceTimePoints(double _timeBeg, double _timeEnd, double _step)
{
	m_streams.ReduceTimePoints(_timeBeg, _timeEnd, _step);
}

void CBaseUnit::AddCompound(const std::string& _compoundKey)
{
	m_streams.AddCompound(_compoundKey);
	m_lookupTables.Clear();
}

void CBaseUnit::RemoveCompound(const std::string& _compoundKey)
{
	m_streams.RemoveCompound(_compoundKey);
	m_lookupTables.Clear();
}

std::string CBaseUnit::GetCompoundName(const std::string& _compoundKey) const
{
	if (const auto* compound = m_materialsDB->GetCompound(_compoundKey))
		return compound->GetName();
	return {};
}

std::string CBaseUnit::GetCompoundKey(const std::string& _compoundName) const
{
	if (const auto* compound = m_materialsDB->GetCompoundByName(_compoundName))
		return compound->GetKey();
	return {};
}

std::string CBaseUnit::GetCompoundName(size_t _index) const
{
	if (_index >= m_compounds->size()) return {};
	return GetCompoundName(m_compounds->at(_index));
}

std::string CBaseUnit::GetCompoundKey(size_t _index) const
{
	if (_index >= m_compounds->size()) return {};
	return GetCompoundKey(m_compounds->at(_index));
}

std::vector<std::string> CBaseUnit::GetAllCompounds() const
{
	return *m_compounds;
}

std::vector<std::string> CBaseUnit::GetAllCompoundsNames() const
{
	std::vector<std::string> res;
	res.reserve(m_compounds->size());
	for (const auto& compound : *m_compounds)
		res.push_back(GetCompoundName(compound));
	return res;
}

size_t CBaseUnit::GetCompoundsNumber() const
{
	return m_compounds->size();
}

bool CBaseUnit::IsCompoundDefined(const std::string& _compoundKey) const
{
	return VectorContains(*m_compounds, _compoundKey);
}

bool CBaseUnit::IsCompoundNameDefined(const std::string& _compoundName) const
{
	return VectorContains(GetAllCompoundsNames(), _compoundName);
}

void CBaseUnit::AddOverallProperty(EOverall _property, const std::string& _name, const std::string& _units)
{
	m_streams.AddOverallProperty(_property, _name, _units);
}

void CBaseUnit::RemoveOverallProperty(EOverall _property)
{
	m_streams.RemoveOverallProperty(_property);
}

void CBaseUnit::AddPhase(EPhase _phase, const std::string& _name)
{
	m_streams.AddPhase(_phase, _name);
}

void CBaseUnit::RemovePhase(EPhase _phase)
{
	m_streams.RemovePhase(_phase);
}

std::string CBaseUnit::GetPhaseName(EPhase _phase) const
{
	for (const auto& phase : *m_phases)
		if (phase.state == _phase)
			return phase.name;
	return {};
}

EPhase CBaseUnit::GetPhaseType(size_t _index) const
{
	if (_index >= m_phases->size()) return EPhase::UNDEFINED;
	return m_phases->at(_index).state;
}

std::vector<EPhase> CBaseUnit::GetAllPhases() const
{
	std::vector<EPhase> res;
	for (const auto& phase : *m_phases)
		res.push_back(phase.state);
	return res;
}

size_t CBaseUnit::GetPhasesNumber() const
{
	return m_phases->size();
}

bool CBaseUnit::IsPhaseDefined(EPhase _phase) const
{
	for (const auto& phase : *m_phases)
		if (phase.state == _phase)
			return true;
	return false;
}

void CBaseUnit::UpdateDistributionsGrid()
{
	m_streams.UpdateDistributionsGrid();
}

size_t CBaseUnit::GetDistributionsNumber() const
{
	return m_grid->GetDistributionsNumber();
}

std::vector<EDistrTypes> CBaseUnit::GetDistributionsTypes() const
{
	return m_grid->GetDistrTypes();
}

std::vector<size_t> CBaseUnit::GetDistributionsClasses() const
{
	return vector_cast<size_t>(m_grid->GetClasses());
}

EGridEntry CBaseUnit::GetDistributionGridType(EDistrTypes _distribution) const
{
	return m_grid->GetGridEntryByDistr(_distribution);
}

size_t CBaseUnit::GetClassesNumber(EDistrTypes _distribution) const
{
	return m_grid->GetClassesByDistr(_distribution);
}

std::vector<double> CBaseUnit::GetNumericGrid(EDistrTypes _distribution) const
{
	return m_grid->GetNumericGridByDistr(_distribution);
}

std::vector<std::string> CBaseUnit::GetSymbolicGrid(EDistrTypes _distribution) const
{
	return m_grid->GetSymbolicGridByDistr(_distribution);
}

std::vector<double> CBaseUnit::GetClassesSizes(EDistrTypes _distribution) const
{
	return m_grid->GetClassSizesByDistr(_distribution);
}

std::vector<double> CBaseUnit::GetClassesMeans(EDistrTypes _distribution) const
{
	return m_grid->GetClassMeansByDistr(_distribution);
}

std::vector<double> CBaseUnit::GetPSDGridDiameters() const
{
	return m_grid->GetNumericGridByDistr(DISTR_SIZE);
}

std::vector<double> CBaseUnit::GetPSDGridSurfaces() const
{
	return DiameterToSurface(m_grid->GetNumericGridByDistr(DISTR_SIZE));
}

std::vector<double> CBaseUnit::GetPSDGridVolumes() const
{
	return DiameterToVolume(m_grid->GetNumericGridByDistr(DISTR_SIZE));
}

std::vector<double> CBaseUnit::GetPSDMeanDiameters() const
{
	return m_grid->GetClassMeansByDistr(DISTR_SIZE);
}

std::vector<double> CBaseUnit::GetPSDMeanSurfaces() const
{
	return DiameterToSurface(m_grid->GetClassMeansByDistr(DISTR_SIZE));
}

std::vector<double> CBaseUnit::GetPSDMeanVolumes() const
{
	return DiameterToVolume(m_grid->GetClassMeansByDistr(DISTR_SIZE));
}

bool CBaseUnit::IsDistributionDefined(EDistrTypes _distribution) const
{
	return m_grid->IsDistrTypePresent(_distribution);
}

void CBaseUnit::CalculateTM(EDistrTypes _distribution, const std::vector<double>& _inValue, const std::vector<double>& _outValue, CTransformMatrix& _matrix)
{
	if (_inValue.empty() || _inValue.size() != _outValue.size()) return;

	// normalize
	std::vector<double> vectorI = Normalize(_inValue);
	std::vector<double> vectorO = Normalize(_outValue);

	_matrix.SetDimensions(_distribution, static_cast<unsigned>(vectorI.size()));
	unsigned indexI = 0;
	unsigned indexO = 0;
	double valueI = vectorI.front();
	double valueO = vectorO.front();
	double initI = vectorI.front();

	while (indexI < vectorI.size() && indexO < vectorO.size())
	{
		double valueRes;
		if (valueI >= valueO)
		{
			if (valueI == initI)
				valueRes = valueI != 0.0 ? valueO / valueI : 0.0;
			else
				valueRes = initI  != 0.0 ? valueO / initI  : 0.0;
			valueI -= valueO;
			valueO = 0.0;
		}
		else
		{
			valueRes = initI != 0.0 ? valueI / initI : 0.0;
			valueO -= valueI;
			valueI = 0.0;
		}

		_matrix.SetValue(indexI, indexO, valueRes);

		if (valueO == 0.0)
		{
			++indexO;
			if (indexO < vectorO.size())
				valueO = vectorO[indexO];
		}

		if (valueI == 0.0)
		{
			++indexI;
			if (indexI < vectorI.size())
				valueI = initI = vectorI[indexI];
		}
	}
}

const CMaterialsDatabase* CBaseUnit::GetMaterialsDatabase() const
{
	return m_materialsDB;
}

const CDistributionsGrid* CBaseUnit::GetGrid() const
{
	return m_grid;
}

double CBaseUnit::GetAbsTolerance() const
{
	return m_tolerance->toleranceAbs;
}

double CBaseUnit::GetRelTolerance() const
{
	return m_tolerance->toleranceRel;
}

void CBaseUnit::UpdateToleranceSettings()
{
	m_streams.UpdateToleranceSettings();
}

void CBaseUnit::UpdateCacheSettings()
{
	m_streams.UpdateCacheSettings();
}

double CBaseUnit::GetCompoundProperty(const std::string& _compoundKey, ECompoundConstProperties _property) const
{
	return m_materialsDB->GetConstPropertyValue(_compoundKey, _property);
}

double CBaseUnit::GetCompoundProperty(const std::string& _compoundKey, ECompoundTPProperties _property, double _temperature, double _pressure) const
{
	return m_materialsDB->GetTPPropertyValue(_compoundKey, _property, _temperature, _pressure);
}

double CBaseUnit::GetCompoundProperty(const std::string& _compoundKey1, const std::string& _compoundKey2, EInteractionProperties _property, double _temperature, double _pressure) const
{
	return m_materialsDB->GetInteractionPropertyValue(_compoundKey1, _compoundKey2, _property, _temperature, _pressure);
}

bool CBaseUnit::IsPropertyDefined(ECompoundConstProperties _property) const
{
	return m_materialsDB->IsPropertyDefined(_property);
}

bool CBaseUnit::IsPropertyDefined(ECompoundTPProperties _property) const
{
	return m_materialsDB->IsPropertyDefined(_property);
}

bool CBaseUnit::IsPropertyDefined(EInteractionProperties _property) const
{
	return m_materialsDB->IsPropertyDefined(_property);
}

CUnitLookupTables* CBaseUnit::GetLookupTables()
{
	return &m_lookupTables;
}

double CBaseUnit::CalculateTemperatureFromProperty(ECompoundTPProperties _property, double _value, const std::vector<double>& _fractions) const
{
	return m_lookupTables.CalcTemperature(_property, _value, _fractions);
}

double CBaseUnit::CalculatePressureFromProperty(ECompoundTPProperties _property, double _value, const std::vector<double>& _fractions) const
{
	return m_lookupTables.CalcPressure(_property, _value, _fractions);
}

double CBaseUnit::CalculatePropertyFromTemperature(ECompoundTPProperties _property, double _T, const std::vector<double>& _fractions) const
{
	return m_lookupTables.CalcFromTemperature(_property, _T, _fractions);
}

double CBaseUnit::CalculatePropertyFromPressure(ECompoundTPProperties _property, double _P, const std::vector<double>& _fractions) const
{
	return m_lookupTables.CalcFromPressure(_property, _P, _fractions);
}

void CBaseUnit::HeatExchange(double _time, CBaseStream* _stream1, CBaseStream* _stream2, double _efficiency) const
{
	// no heat transfer if efficiency is bigger then 1 or smaller/equal to 0
	if (_efficiency <= 0.0 || _efficiency > 1.0)
		return;

	// mass flows and temperatures of both streams
	const double mass1 = _stream1->GetMass(_time);
	const double mass2 = _stream2->GetMass(_time);
	const double massMix = mass1 + mass2;
	if (massMix == 0.0)
		return;

	// calculate enthalpy
	const double enthalpy1 = _stream1->GetLookupTables()->CalcFromTemperature(_time, ENTHALPY);
	const double enthalpy2 = _stream2->GetLookupTables()->CalcFromTemperature(_time, ENTHALPY);
	const double enthalpyMix = (mass1 * enthalpy1 + mass2 * enthalpy2) / massMix;

	// add up both enthalpy tables weighted with their respective mass fraction of total mass flow
	const CLookupTable* lookup1 = _stream1->GetLookupTables()->GetLookupTable(ENTHALPY, EDependencyTypes::DEPENDENCE_TEMP);
	const CLookupTable* lookup2 = _stream2->GetLookupTables()->GetLookupTable(ENTHALPY, EDependencyTypes::DEPENDENCE_TEMP);
	CLookupTable lookupMix(m_materialsDB, *m_compounds, ENTHALPY, EDependencyTypes::DEPENDENCE_TEMP);
	lookupMix.Add(*lookup1, mass1 / massMix);
	lookupMix.Add(*lookup2, mass2 / massMix);

	// get ideal heat exchange temperature, i.e. temperature for maximum heat exchange between both streams (here: mixing temperature)
	const double temperatureMix = lookupMix.GetParam(enthalpyMix);

	if (_efficiency == 1.0)	// use ideal heat exchange temperature for both streams
	{
		// set stream temperatures
		_stream1->SetTemperature(_time, temperatureMix);
		_stream2->SetTemperature(_time, temperatureMix);
	}
	else	// calculate heat transfer with respective efficiency, i.e. maximum heat exchange multiplied with efficiency
	{
		// actual heat exchange between both streams
		double enthalpyRes1 = _efficiency * mass1 * (lookup1->GetValue(temperatureMix) - enthalpy1);
		double enthalpyRes2 = -enthalpyRes1;

		// new enthalpies of both streams
		enthalpyRes1 += mass1 * enthalpy1;
		enthalpyRes2 += mass2 * enthalpy2;

		// new specific enthalpies of both streams
		enthalpyRes1 /= mass1;
		enthalpyRes2 /= mass2;

		// read out temperatures for new specific enthalpies from enthalpy lookup tables
		const double temperatureRes1 = lookup1->GetParam(enthalpyRes1);
		const double temperatureRes2 = lookup2->GetParam(enthalpyRes2);

		// set stream temperatures
		_stream1->SetTemperature(_time, temperatureRes1);
		_stream2->SetTemperature(_time, temperatureRes2);
	}
}

void CBaseUnit::RaiseError(const std::string& _message)
{
	m_hasError = true;
	const std::string suffix = m_errorMessage.empty() ? "" : "\n";
	const std::string text = !_message.empty() ? _message : StrConst::BUnit_UnknownError;
	m_errorMessage = suffix + text;
}

void CBaseUnit::RaiseWarning(const std::string& _message)
{
	m_hasWarning = true;
	const std::string suffix = m_warningMessage.empty() ? "" : "\n";
	const std::string text = !_message.empty() ? _message : StrConst::BUnit_UnknownWarning;
	m_warningMessage = suffix + text;
}

void CBaseUnit::ShowInfo(const std::string& _message)
{
	if (_message.empty()) return;
	m_hasInfo = true;
	const std::string suffix = m_infoMessage.empty() ? "" : "\n";
	m_infoMessage = suffix + _message;
}

bool CBaseUnit::HasError() const
{
	return m_hasError;
}

bool CBaseUnit::HasWarning() const
{
	return m_hasWarning;
}

bool CBaseUnit::HasInfo() const
{
	return m_hasInfo;
}

std::string CBaseUnit::GetErrorMessage() const
{
	return m_errorMessage;
}

std::string CBaseUnit::GetWarningMessage() const
{
	return m_warningMessage;
}

std::string CBaseUnit::GetInfoMessage() const
{
	return m_infoMessage;
}

void CBaseUnit::ClearError()
{
	m_hasError = false;
	m_errorMessage.clear();
}

void CBaseUnit::ClearWarning()
{
	m_hasWarning = false;
	m_warningMessage.clear();
}

void CBaseUnit::ClearInfo()
{
	m_hasInfo = false;
	m_infoMessage.clear();
}

void CBaseUnit::InitializeUnit()
{
	ClearError();
	ClearWarning();
	ClearInfo();
	m_stateVariables.Clear();
	m_plots.Clear();
	m_streams.Initialize();
	for (auto& param : m_unitParameters.GetAllSolverParameters())
		param->GetSolver()->Initialize();
	Initialize(0.0);
	SaveStateUnit(0.0, std::numeric_limits<double>::max());
}

void CBaseUnit::FinalizeUnit()
{
	Finalize();
	for (auto& param : m_unitParameters.GetAllSolverParameters())
		param->GetSolver()->Finalize();
	m_streams.RemoveTemporary();
}

void CBaseUnit::SaveStateUnit(double _timeBeg, double _timeEnd)
{
	SaveState();
	for (auto& param : m_unitParameters.GetAllSolverParameters())
		param->GetSolver()->SaveState();
	m_stateVariables.SaveState();
	m_streams.SaveState(_timeBeg, _timeEnd);
	m_plots.SaveState();
}

void CBaseUnit::LoadStateUnit()
{
	LoadState();
	for (auto& param : m_unitParameters.GetAllSolverParameters())
		param->GetSolver()->LoadState();
	m_stateVariables.LoadState();
	m_streams.LoadState();
	m_plots.LoadState();
}

void CBaseUnit::ClearSimulationResults()
{
	m_streams.ClearResults();
	m_stateVariables.Clear();
	m_plots.Clear();
}

void CBaseUnit::SaveToFile(CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;

	// current version of save procedure
	_h5File.WriteAttribute(_path, StrConst::H5AttrSaveVersion, m_saveVersion);

	_h5File.WriteData(_path, StrConst::BUnit_H5UnitKey, m_uniqueID);

	m_ports.SaveToFile(_h5File, _h5File.CreateGroup(_path, StrConst::BUnit_H5GroupPorts));
	m_streams.SaveToFile(_h5File, _h5File.CreateGroup(_path, StrConst::BUnit_H5GroupInternalMaterials));
	m_unitParameters.SaveToFile(_h5File, _h5File.CreateGroup(_path, StrConst::BUnit_H5GroupParams));
	m_stateVariables.SaveToFile(_h5File, _h5File.CreateGroup(_path, StrConst::BUnit_H5GroupStateVars));
	m_plots.SaveToFile(_h5File, _h5File.CreateGroup(_path, StrConst::BUnit_H5GroupPlots));
}

void CBaseUnit::LoadFromFile(CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;

	// version of save procedure
	const int version = _h5File.ReadAttribute(_path, StrConst::H5AttrSaveVersion);

	// compatibility with older versions
	if (version <= 1)
	{
		LoadFromFile_v1(_h5File, _path);
		return;
	}
	if (version == 2)
	{
		LoadFromFile_v2(_h5File, _path);
		return;
	}

	_h5File.ReadData(_path, StrConst::BUnit_H5UnitKey, m_uniqueID);

	m_ports.LoadFromFile(_h5File, _path + "/" + StrConst::BUnit_H5GroupPorts);
	m_streams.LoadFromFile(_h5File, _path + "/" + StrConst::BUnit_H5GroupInternalMaterials);
	m_unitParameters.LoadFromFile(_h5File, _path + "/" + StrConst::BUnit_H5GroupParams);
	m_stateVariables.LoadFromFile(_h5File, _path + "/" + StrConst::BUnit_H5GroupStateVars);
	m_plots.LoadFromFile(_h5File, _path + "/" + StrConst::BUnit_H5GroupPlots);
}

void CBaseUnit::LoadFromFile_v2(const CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;

	_h5File.ReadData(_path, StrConst::BUnit_H5UnitKey, m_uniqueID);

	m_ports.LoadFromFile_v0(_h5File, _path + "/" + StrConst::BUnit_H5GroupPorts);
	m_streams.LoadFromFile_v0(_h5File, _path);
	m_unitParameters.LoadFromFile(_h5File, _path + "/" + StrConst::BUnit_H5GroupParams);
	m_stateVariables.LoadFromFile_v0(_h5File, _path);
	m_plots.LoadFromFile_v0(_h5File, _path);
}

void CBaseUnit::LoadFromFile_v1(const CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;

	_h5File.ReadData(_path, StrConst::BUnit_H5UnitKey, m_uniqueID);

	m_ports.LoadFromFile_v00(_h5File, _path);
	m_streams.LoadFromFile_v00(_h5File, _path);
	m_unitParameters.LoadFromFile(_h5File, _path + "/" + StrConst::BUnit_H5GroupParams);
	m_stateVariables.LoadFromFile_v0(_h5File, _path);
	m_plots.LoadFromFile_v0(_h5File, _path);
}

////////////////////////////////////////////////////////////////////////////////
/// Deprecated functions

std::string& CBaseUnit::GetNameRef()
{
	return m_unitName;
}

std::string& CBaseUnit::GetAuthorRef()
{
	return m_authorName;
}

size_t& CBaseUnit::GetVersionRef()
{
	return m_version;
}

std::string& CBaseUnit::GetKeyRef()
{
	return m_uniqueID;
}

const CMaterialsDatabase* CBaseUnit::GetMaterialsDBRef() const
{
	return m_materialsDB;
}

unsigned CBaseUnit::AddPort(const std::string& _name, unsigned _type)
{
	switch (_type)
	{
	case 0:		AddPort(_name, EUnitPort::INPUT);		break;
	case 1:		AddPort(_name, EUnitPort::OUTPUT);		break;
	default:	AddPort(_name, EUnitPort::UNDEFINED);	break;
	}
	return static_cast<unsigned>(m_ports.GetPortsNumber()) - 1;
}

CStream* CBaseUnit::GetPortStream(unsigned _index) const
{
	if (_index >= m_ports.GetPortsNumber()) return nullptr;
	return m_ports.GetAllPorts()[_index]->GetStream();
}

CStream* CBaseUnit::AddFeed(const std::string& _name, [[maybe_unused]] const std::string& _key)
{
	return AddFeed(_name);
}

CStream* CBaseUnit::AddMaterialStream(const std::string& _name, [[maybe_unused]] const std::string& key)
{
	return AddStream(_name);
}

CStream* CBaseUnit::GetMaterialStream(const std::string& _name)
{
	return GetStream(_name);
}

void CBaseUnit::AddConstParameter(const std::string& _name, double _minValue, double _maxValue, double _initValue, const std::string& _units, const std::string& _description)
{
	AddConstRealParameter(_name, _initValue, _units, _description, _minValue, _maxValue);
}

void CBaseUnit::AddTDParameter(const std::string& _name, double _minValue, double _maxValue, double _initValue, const std::string& _units, const std::string& _description)
{
	AddTDParameter(_name, _initValue, _units, _description, _minValue, _maxValue);
}

void CBaseUnit::AddGroupParameter(const std::string& _name, size_t _initValue, const std::vector<size_t>& _values, const std::vector<std::string>& _valuesNames, const std::string& _description)
{
	AddComboParameter(_name, _initValue, _values, _valuesNames, _description);
}

double CBaseUnit::GetConstParameterValue(const std::string& _name) const
{
	return GetConstRealParameterValue(_name);
}

size_t CBaseUnit::GetGroupParameterValue(const std::string& _name) const
{
	return GetComboParameterValue(_name);
}

unsigned CBaseUnit::AddStateVariable(const std::string& _name, double _initValue, [[maybe_unused]] bool _saveHistory)
{
	AddStateVariable(_name, _initValue);
	return static_cast<unsigned>(m_stateVariables.GetStateVariablesNumber()) - 1;
}

void CBaseUnit::SetStateVariable(unsigned _index, double _value)
{
	if (_index >= m_stateVariables.GetStateVariablesNumber()) return;
	m_stateVariables.GetAllStateVariables()[_index]->SetValue(_value);
}

void CBaseUnit::SaveStateVariables(double _time)
{
	for (auto& sv : m_stateVariables.GetAllStateVariables())
		sv->SetValue(_time, sv->GetValue());
}

void CBaseUnit::AddPointOnCurve(const std::string& _plotName, const std::string& _curveName, const std::vector<double>& _x, const std::vector<double>& _y)
{
	AddPointsOnCurve(_plotName, _curveName, _x, _y);
}

void CBaseUnit::AddPointOnCurve(const std::string& _plotName, double _valueZ, const std::vector<double>& _x, const std::vector<double>& _y)
{
	AddPointsOnCurve(_plotName, _valueZ, _x, _y);
}

unsigned CBaseUnit::GetPlotsNumber() const
{
	return static_cast<unsigned>(m_plots.GetPlotsNumber());
}

unsigned CBaseUnit::GetCurvesNumber(size_t _iPlot) const
{
	if (_iPlot >= m_plots.GetPlotsNumber()) return {};
	return static_cast<unsigned>(m_plots.GetAllPlots()[_iPlot]->GetCurvesNumber());
}

std::string CBaseUnit::GetPlotName(size_t _iPlot) const
{
	if (_iPlot >= m_plots.GetPlotsNumber()) return {};
	return m_plots.GetAllPlots()[_iPlot]->GetName();
}

std::string CBaseUnit::GetPlotXAxisName(unsigned _iPlot) const
{
	if (_iPlot >= m_plots.GetPlotsNumber()) return {};
	return m_plots.GetAllPlots()[_iPlot]->GetLabelX();
}

std::string CBaseUnit::GetPlotYAxisName(unsigned _iPlot) const
{
	if (_iPlot >= m_plots.GetPlotsNumber()) return {};
	return m_plots.GetAllPlots()[_iPlot]->GetLabelY();
}

std::string CBaseUnit::GetPlotZAxisName(unsigned _iPlot) const
{
	if (_iPlot >= m_plots.GetPlotsNumber()) return {};
	return m_plots.GetAllPlots()[_iPlot]->GetLabelZ();
}

std::string CBaseUnit::GetCurveName(unsigned _iPlot, unsigned _iCurve) const
{
	if (_iPlot >= m_plots.GetPlotsNumber() || _iCurve >= m_plots.GetAllPlots()[_iPlot]->GetCurvesNumber()) return {};
	return m_plots.GetAllPlots()[_iPlot]->GetAllCurves()[_iCurve]->GetName();
}

std::vector<double> CBaseUnit::GetCurveX(unsigned _iPlot, unsigned _iCurve) const
{
	if (_iPlot >= m_plots.GetPlotsNumber() || _iCurve >= m_plots.GetAllPlots()[_iPlot]->GetCurvesNumber()) return {};
	return m_plots.GetAllPlots()[_iPlot]->GetAllCurves()[_iCurve]->GetXValues();
}

std::vector<double> CBaseUnit::GetCurveY(unsigned _iPlot, unsigned _iCurve) const
{
	if (_iPlot >= m_plots.GetPlotsNumber() || _iCurve >= m_plots.GetAllPlots()[_iPlot]->GetCurvesNumber()) return {};
	return m_plots.GetAllPlots()[_iPlot]->GetAllCurves()[_iCurve]->GetYValues();
}

double CBaseUnit::GetCurveZ(size_t _iPlot, size_t _iCurve) const
{
	if (_iPlot >= m_plots.GetPlotsNumber() || _iCurve >= m_plots.GetAllPlots()[_iPlot]->GetCurvesNumber()) return {};
	return m_plots.GetAllPlots()[_iPlot]->GetAllCurves()[_iCurve]->GetZValue();
}

bool CBaseUnit::IsPlot2D(unsigned _iPlot)
{
	if (_iPlot >= m_plots.GetPlotsNumber()) return {};
	return m_plots.GetAllPlots()[_iPlot]->Is2D();
}

std::vector<double> CBaseUnit::GetAllDefinedTimePoints(double _timeBeg, double _timeEnd, bool _forceStartBoundary, bool _forceEndBoundary) const
{
	if (!_forceStartBoundary && !_forceEndBoundary)
		return GetAllTimePoints(_timeBeg, _timeEnd);
	else if (_forceStartBoundary && _forceEndBoundary)
		return GetAllTimePointsClosed(_timeBeg, _timeEnd);
	else
	{
		auto res = GetAllTimePoints(_timeBeg, _timeEnd);
		if (_forceStartBoundary && std::fabs(res.front() - _timeBeg) > 16 * std::numeric_limits<double>::epsilon())
			res.insert(res.begin(), _timeBeg);
		if (_forceEndBoundary && std::fabs(res.back() - _timeEnd) > 16 * std::numeric_limits<double>::epsilon())
			res.push_back(_timeEnd);
		return res;
	}
}

std::vector<double> CBaseUnit::GetAllInputTimePoints(double _timeBeg, double _timeEnd, bool _forceStartBoundary, bool _forceEndBoundary) const
{
	if (!_forceStartBoundary && !_forceEndBoundary)
		return GetInputTimePoints(_timeBeg, _timeEnd);
	else if (_forceStartBoundary && _forceEndBoundary)
		return GetInputTimePointsClosed(_timeBeg, _timeEnd);
	else
	{
		auto res = GetInputTimePoints(_timeBeg, _timeEnd);
		if (_forceStartBoundary && std::fabs(res.front() - _timeBeg) > 16 * std::numeric_limits<double>::epsilon())
			res.insert(res.begin(), _timeBeg);
		if (_forceEndBoundary && std::fabs(res.back() - _timeEnd) > 16 * std::numeric_limits<double>::epsilon())
			res.push_back(_timeEnd);
		return res;
	}
}

std::vector<double> CBaseUnit::GetAllStreamsTimePoints(const std::vector<CStream*>& _srteams, double _timeBeg, double _timeEnd) const
{
	return GetStreamsTimePoints(_timeBeg, _timeEnd, _srteams);
}

std::vector<std::string> CBaseUnit::GetCompoundsList() const
{
	return GetAllCompounds();
}

std::vector<std::string> CBaseUnit::GetCompoundsNames() const
{
	return GetAllCompoundsNames();
}

bool CBaseUnit::IsCompoundKeyDefined(const std::string& _compoundKey) const
{
	return IsCompoundDefined(_compoundKey);
}

unsigned CBaseUnit::GetPhaseIndex(unsigned _soa) const
{
	const auto state = SOA2EPhase(_soa);
	for (unsigned i = 0; i < m_phases->size(); ++i)
		if (m_phases->at(i).state == state)
			return i;
	return -1;
}

unsigned CBaseUnit::GetPhaseSOA(unsigned _index) const
{
	return EPhase2SOA(GetPhaseType(_index));
}

bool CBaseUnit::IsPhaseDefined(unsigned _soa) const
{
	return IsPhaseDefined(SOA2EPhase(_soa));
}

double CBaseUnit::GetCompoundConstant(const std::string& _compoundKey, unsigned _property) const
{
	return GetCompoundProperty(_compoundKey, static_cast<ECompoundConstProperties>(_property));
}

double CBaseUnit::GetCompoundTPDProp(const std::string& _compoundKey, unsigned _property, double _temperature, double _pressure) const
{
	return GetCompoundProperty(_compoundKey, static_cast<ECompoundTPProperties>(_property), _temperature, _pressure);
}

double CBaseUnit::GetCompoundsInteractionProp(const std::string& _compoundKey1, const std::string& _compoundKey2, unsigned _property, double _temperature, double _pressure) const
{
	return GetCompoundProperty(_compoundKey1, _compoundKey2, static_cast<EInteractionProperties>(_property), _temperature, _pressure);
}

double CBaseUnit::CalcTemperatureFromProperty(ECompoundTPProperties _property, const std::vector<double>& _fractions, double _value) const
{
	return CalculateTemperatureFromProperty(_property, _value, _fractions);
}

double CBaseUnit::CalcPressureFromProperty(ECompoundTPProperties _property, const std::vector<double>& _fractions, double _value) const
{
	return CalculatePressureFromProperty(_property, _value, _fractions);
}

void CBaseUnit::HeatExchange(CStream* _stream1, CStream* _stream2, double _time, double _efficiency) const
{
	HeatExchange(_time, _stream1, _stream2, _efficiency);
}

bool CBaseUnit::CheckError() const
{
	return HasError();
}

EPhase CBaseUnit::SOA2EPhase(unsigned _soa)
{
	switch (_soa)
	{
	case 0:		return EPhase::SOLID;
	case 1:		return EPhase::LIQUID;
	case 2:		return EPhase::VAPOR;
	default:	return EPhase::UNDEFINED;
	}
}

unsigned CBaseUnit::EPhase2SOA(EPhase _phase)
{
	switch (_phase)
	{
	case EPhase::SOLID:		return 0;
	case EPhase::LIQUID:	return 1;
	case EPhase::VAPOR:		return 2;
	case EPhase::UNDEFINED:	return 4;
	}
	return 4;
}


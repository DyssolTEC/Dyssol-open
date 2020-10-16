/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "PlotManager.h"
#include "H5Handler.h"
#include "ContainerFunctions.h"
#include "DyssolStringConstants.h"

////////////////////////////////////////////////////////////////////////////////
// CCurve
//

CCurve::CCurve(std::string _name) :
	m_name{ std::move(_name) }
{
}

CCurve::CCurve(double _z) :
	m_name{ StringFunctions::Double2String(_z) },
	m_valueZ{ _z }
{
}

std::string CCurve::GetName() const
{
	return m_name;
}

void CCurve::SetName(const std::string& _name)
{
	m_name = _name;
}

double CCurve::GetZValue() const
{
	return m_valueZ;
}

void CCurve::SetZValue(double _z)
{
	m_valueZ = _z;
	SetName(StringFunctions::Double2String(_z));
}

void CCurve::AddPoint(double _x, double _y)
{
	m_values.emplace_back(_x, _y);
}

void CCurve::AddPoint(const CPoint& _point)
{
	m_values.push_back(_point);
}

void CCurve::AddPoints(const std::vector<double>& _x, const std::vector<double>& _y)
{
	if (_x.size() != _y.size()) return;
	m_values.reserve(m_values.size() + _x.size());
	for (size_t i = 0; i < _x.size(); ++i)
		m_values.emplace_back(_x[i], _y[i]);
}

void CCurve::AddPoints(const std::vector<CPoint>& _points)
{
	m_values.insert(m_values.end(), _points.begin(), _points.end());
}

std::vector<double> CCurve::GetXValues() const
{
	std::vector<double> res;
	res.reserve(m_values.size());
	for (const auto& point : m_values)
		res.push_back(point.x);
	return res;
}

std::vector<double> CCurve::GetYValues() const
{
	std::vector<double> res;
	res.reserve(m_values.size());
	for (const auto& point : m_values)
		res.push_back(point.y);
	return res;
}

std::vector<CPoint> CCurve::GetPoints() const
{
	return m_values;
}

void CCurve::ClearData()
{
	m_values.clear();
}

void CCurve::Clear()
{
	ClearData();
	m_name.clear();
}

void CCurve::SaveToFile(CH5Handler& _h5File, const std::string& _path) const
{
	if (!_h5File.IsValid()) return;

	// current version of save procedure
	_h5File.WriteAttribute(_path, StrConst::H5AttrSaveVersion, m_saveVersion);

	_h5File.WriteData(_path, StrConst::PlotMngr_H5CurveName,   m_name);
	_h5File.WriteData(_path, StrConst::PlotMngr_H5CurveZValue, m_valueZ);
	_h5File.WriteData(_path, StrConst::PlotMngr_H5CurveData,   m_values);
}

void CCurve::LoadFromFile(CH5Handler& _h5File, const std::string& _path)
{
	Clear();

	if (!_h5File.IsValid()) return;

	// current version of save procedure
	//const int version = _h5File.ReadAttribute(_path, StrConst::H5AttrSaveVersion);

	_h5File.ReadData(_path, StrConst::PlotMngr_H5CurveName,   m_name);
	_h5File.ReadData(_path, StrConst::PlotMngr_H5CurveZValue, m_valueZ);
	_h5File.ReadData(_path, StrConst::PlotMngr_H5CurveData,   m_values);
}

////////////////////////////////////////////////////////////////////////////////
// CPlot
//

CPlot::CPlot(std::string _name) :
	m_name{ std::move(_name) }
{
}

CPlot::CPlot(std::string _name, std::string _labelX, std::string _labelY) :
	m_name{ std::move(_name) },
	m_labelX{ std::move(_labelX) },
	m_labelY{ std::move(_labelY) }
{
}

CPlot::CPlot(std::string _name, std::string _labelX, std::string _labelY, std::string _labelZ) :
	m_name{ std::move(_name) },
	m_labelX{ std::move(_labelX) },
	m_labelY{ std::move(_labelY) },
	m_labelZ{ std::move(_labelZ) }
{
}

CPlot::CPlot(const CPlot& _other)
{
	m_curves.reserve(_other.m_curves.size());
	for (const auto& curve : _other.m_curves)
		m_curves.push_back(std::make_unique<CCurve>(*curve));
}

CPlot& CPlot::operator=(const CPlot& _other)
{
	if (this != &_other)
	{
		m_name = _other.m_name;
		m_labelX = _other.m_labelX;
		m_labelY = _other.m_labelY;
		m_labelZ = _other.m_labelZ;
		m_curves.resize(_other.m_curves.size());
		for (size_t i = 0; i < m_curves.size(); ++i)
			*m_curves[i] = *_other.m_curves[i];
	}
	return *this;
}

std::string CPlot::GetName() const
{
	return m_name;
}

void CPlot::SetName(const std::string& _name)
{
	m_name = _name;
}

std::string CPlot::GetLabelX() const
{
	return m_labelX;
}

std::string CPlot::GetLabelY() const
{
	return m_labelY;
}

std::string CPlot::GetLabelZ() const
{
	return m_labelZ;
}

void CPlot::SetLabelX(const std::string& _label)
{
	m_labelX = _label;
}

void CPlot::SetLabelY(const std::string& _label)
{
	m_labelY = _label;
}

void CPlot::SetLabelZ(const std::string& _label)
{
	m_labelZ = _label;
}

void CPlot::SetLabels(const std::string& _labelX, const std::string& _labelY)
{
	SetLabelX(_labelX);
	SetLabelY(_labelY);
}

void CPlot::SetLabels(const std::string& _labelX, const std::string& _labelY, const std::string& _labelZ)
{
	SetLabelX(_labelX);
	SetLabelY(_labelY);
	SetLabelZ(_labelZ);
}

CCurve* CPlot::AddCurve(const std::string& _name)
{
	if (GetCurve(_name)) return nullptr;
	m_curves.emplace_back(new CCurve{ _name });
	return m_curves.back().get();
}

CCurve* CPlot::AddCurve(const std::string& _name, const std::vector<double>& _x, const std::vector<double>& _y)
{
	auto* curve = AddCurve(_name);
	if (curve) curve->AddPoints(_x, _y);
	return curve;
}

CCurve* CPlot::AddCurve(const std::string& _name, const std::vector<CPoint>& _points)
{
	auto* curve = AddCurve(_name);
	if (curve) curve->AddPoints(_points);
	return curve;
}

CCurve* CPlot::AddCurve(double _z)
{
	auto* curve = AddCurve(StringFunctions::Double2String(_z));
	if (curve) curve->SetZValue(_z);
	return curve;
}

CCurve* CPlot::AddCurve(double _z, const std::vector<double>& _x, const std::vector<double>& _y)
{
	auto* curve = AddCurve(StringFunctions::Double2String(_z), _x, _y);
	if (curve) curve->SetZValue(_z);
	return curve;
}

CCurve* CPlot::AddCurve(double _z, const std::vector<CPoint>& _points)
{
	auto* curve = AddCurve(StringFunctions::Double2String(_z), _points);
	if (curve) curve->SetZValue(_z);
	return curve;
}

const CCurve* CPlot::GetCurve(const std::string& _name) const
{
	for (const auto& curve : m_curves)
		if (curve->GetName() == _name)
			return curve.get();
	return nullptr;
}

CCurve* CPlot::GetCurve(const std::string& _name)
{
	return const_cast<CCurve*>(static_cast<const CPlot&>(*this).GetCurve(_name));
}

const CCurve* CPlot::GetCurve(double _z) const
{
	for (const auto& curve : m_curves)
		if (curve->GetZValue() == _z)
			return curve.get();
	return nullptr;
}

CCurve* CPlot::GetCurve(double _z)
{
	return const_cast<CCurve*>(static_cast<const CPlot&>(*this).GetCurve(_z));
}

void CPlot::RemoveCurve(const std::string& _name)
{
	VectorDelete(m_curves, [&](const std::unique_ptr<CCurve>& s) { return s->GetName() == _name; });
}

void CPlot::RemoveCurve(double _z)
{
	VectorDelete(m_curves, [&](const std::unique_ptr<CCurve>& s) { return s->GetZValue() == _z; });
}

std::vector<const CCurve*> CPlot::GetAllCurves() const
{
	std::vector<const CCurve*> res;
	res.reserve(m_curves.size());
	for (const auto& curve : m_curves)
		res.push_back(curve.get());
	return res;
}

std::vector<CCurve*> CPlot::GetAllCurves()
{
	std::vector<CCurve*> res;
	res.reserve(m_curves.size());
	for (const auto& curve : m_curves)
		res.push_back(curve.get());
	return res;
}

std::vector<double> CPlot::GetZValues() const
{
	std::vector<double> res;
	res.reserve(m_curves.size());
	for (const auto& curve : m_curves)
		res.push_back(curve->GetZValue());
	return res;
}

size_t CPlot::GetCurvesNumber() const
{
	return m_curves.size();
}

void CPlot::ClearData()
{
	m_curves.clear();
}

void CPlot::Clear()
{
	ClearData();
	m_name = {};
	m_labelX = {};
	m_labelY = {};
	m_labelZ = {};
}

void CPlot::SaveToFile(CH5Handler& _h5File, const std::string& _path) const
{
	if (!_h5File.IsValid()) return;

	// current version of save procedure
	_h5File.WriteAttribute(_path, StrConst::H5AttrSaveVersion, m_saveVersion);

	_h5File.WriteData(_path, StrConst::PlotMngr_H5PlotName,  m_name);
	_h5File.WriteData(_path, StrConst::PlotMngr_H5PlotXAxis, m_labelX);
	_h5File.WriteData(_path, StrConst::PlotMngr_H5PlotYAxis, m_labelY);
	_h5File.WriteData(_path, StrConst::PlotMngr_H5PlotZAxis, m_labelZ);

	_h5File.WriteAttribute(_path, StrConst::PlotMngr_H5AttrCurvesNum, static_cast<int>(m_curves.size()));
	const std::string curvesGroup = _h5File.CreateGroup(_path, StrConst::PlotMngr_H5GroupCurves);
	for (size_t i = 0; i < m_curves.size(); ++i)
	{
		const std::string curvePath = _h5File.CreateGroup(curvesGroup, StrConst::PlotMngr_H5GroupCurveName + std::to_string(i));
		m_curves[i]->SaveToFile(_h5File, curvePath);
	}
}

void CPlot::LoadFromFile(CH5Handler& _h5File, const std::string& _path)
{
	Clear();

	if (!_h5File.IsValid()) return;

	// current version of save procedure
	//const int version = _h5File.ReadAttribute(_path, StrConst::H5AttrSaveVersion);

	_h5File.ReadData(_path, StrConst::PlotMngr_H5PlotName,  m_name);
	_h5File.ReadData(_path, StrConst::PlotMngr_H5PlotXAxis, m_labelX);
	_h5File.ReadData(_path, StrConst::PlotMngr_H5PlotYAxis, m_labelY);
	_h5File.ReadData(_path, StrConst::PlotMngr_H5PlotZAxis, m_labelZ);

	const size_t nCurves = _h5File.ReadAttribute(_path, StrConst::PlotMngr_H5AttrCurvesNum);
	const std::string curvesGroup = _path + "/" + StrConst::PlotMngr_H5GroupCurves;
	for (size_t i = 0; i < nCurves; ++i)
	{
		const std::string curvePath = curvesGroup + "/" + StrConst::PlotMngr_H5GroupCurveName + std::to_string(i);
		AddCurve("")->LoadFromFile(_h5File, curvePath);
	}
}

////////////////////////////////////////////////////////////////////////////////
// CPlotManager
//

CPlot* CPlotManager::AddPlot(const std::string& _name)
{
	if (GetPlot(_name)) return nullptr;
	m_plots.emplace_back(new CPlot{ _name });
	return m_plots.back().get();
}

CPlot* CPlotManager::AddPlot(const std::string& _name, const std::string& _labelX, const std::string& _labelY)
{
	if (GetPlot(_name)) return nullptr;
	m_plots.emplace_back(new CPlot{ _name, _labelX, _labelY });
	return m_plots.back().get();
}

CPlot* CPlotManager::AddPlot(const std::string& _name, const std::string& _labelX, const std::string& _labelY, const std::string& _labelZ)
{
	if (GetPlot(_name)) return nullptr;
	m_plots.emplace_back(new CPlot{ _name, _labelX, _labelY, _labelZ });
	return m_plots.back().get();
}

const CPlot* CPlotManager::GetPlot(const std::string& _name) const
{
	for (const auto& plot : m_plots)
		if (plot->GetName() == _name)
			return plot.get();
	return nullptr;
}

CPlot* CPlotManager::GetPlot(const std::string& _name)
{
	return const_cast<CPlot*>(static_cast<const CPlotManager&>(*this).GetPlot(_name));
}

void CPlotManager::RemovePlot(const std::string& _name)
{
	VectorDelete(m_plots, [&](const std::unique_ptr<CPlot>& s) { return s->GetName() == _name; });
}

std::vector<const CPlot*> CPlotManager::GetAllPlots() const
{
	std::vector<const CPlot*> res;
	res.reserve(m_plots.size());
	for (const auto& plot : m_plots)
		res.push_back(plot.get());
	return res;
}

std::vector<CPlot*> CPlotManager::GetAllPlots()
{
	std::vector<CPlot*> res;
	res.reserve(m_plots.size());
	for (const auto& plot : m_plots)
		res.push_back(plot.get());
	return res;
}

size_t CPlotManager::GetPlotsNumber() const
{
	return m_plots.size();
}

void CPlotManager::ClearData()
{
	m_plots.clear();
}

void CPlotManager::Clear()
{
	ClearData();
	m_plotsStored.clear();
}

void CPlotManager::SaveState()
{
	m_plotsStored.resize(m_plots.size());
	for (size_t i = 0; i < m_plots.size(); ++i)
		*m_plotsStored[i] = *m_plots[i];
}

void CPlotManager::LoadState()
{
	m_plots.resize(m_plotsStored.size());
	for (size_t i = 0; i < m_plotsStored.size(); ++i)
		*m_plots[i] = *m_plotsStored[i];
}

void CPlotManager::SaveToFile(CH5Handler& _h5File, const std::string& _path) const
{
	if (!_h5File.IsValid()) return;

	// current version of save procedure
	_h5File.WriteAttribute(_path, StrConst::H5AttrSaveVersion, m_saveVersion);

	_h5File.WriteAttribute(_path, StrConst::PlotMngr_H5AttrPlotsNum, static_cast<int>(m_plots.size()));
	for (size_t i = 0; i < m_plots.size(); ++i)
	{
		const std::string plotPath = _h5File.CreateGroup(_path, StrConst::PlotMngr_H5GroupPlotName + std::to_string(i));
		m_plots[i]->SaveToFile(_h5File, plotPath);
	}
}

void CPlotManager::LoadFromFile(CH5Handler& _h5File, const std::string& _path)
{
	Clear();

	if (!_h5File.IsValid()) return;

	// current version of save procedure
	//const int version = _h5File.ReadAttribute(_path, StrConst::H5AttrSaveVersion);

	const size_t nPlots = _h5File.ReadAttribute(_path, StrConst::PlotMngr_H5AttrPlotsNum);
	for (size_t i = 0; i < nPlots; ++i)
	{
		const std::string plotPath = _path + "/" + StrConst::PlotMngr_H5GroupPlotName + std::to_string(i);
		AddPlot("")->LoadFromFile(_h5File, plotPath);
	}
}

void CPlotManager::LoadFromFile_v0(const CH5Handler& _h5File, const std::string& _path)
{
	Clear();

	if (!_h5File.IsValid()) return;

	const size_t nPlots = _h5File.ReadAttribute(_path, StrConst::PlotMngr_H5AttrPlotsNum);
	if (nPlots == static_cast<size_t>(-1)) return;
	const std::string plotsPath = _path + "/" + StrConst::BUnit_H5GroupPlots + "/" + StrConst::PlotMngr_H5GroupPlotName;
	for (size_t i = 0; i < nPlots; ++i)
	{
		const std::string plotPath = plotsPath + std::to_string(i);
		auto* plot = AddPlot("");
		std::string plotName;
		_h5File.ReadData(plotPath, StrConst::BUnit_H5PlotName, plotName);
		plot->SetName(plotName);
		std::string axisX, axisY, axisZ;
		_h5File.ReadData(plotPath, StrConst::BUnit_H5PlotXAxis, axisX);
		_h5File.ReadData(plotPath, StrConst::BUnit_H5PlotYAxis, axisY);
		_h5File.ReadData(plotPath, StrConst::BUnit_H5PlotZAxis, axisZ);
		plot->SetLabels(axisX, axisY, axisZ);

		const size_t nCurves = _h5File.ReadAttribute(plotPath, StrConst::BUnit_H5AttrCurvesNum);
		if (nCurves == static_cast<size_t>(-1)) continue;
		std::string curvesPath = plotPath + "/" + StrConst::BUnit_H5GroupCurves + "/" + StrConst::BUnit_H5GroupCurveName;
		for (size_t j = 0; j < static_cast<size_t>(nCurves); ++j)
		{
			std::string curvePath = curvesPath + std::to_string(j);
			auto* curve = plot->AddCurve("");
			std::string curveName;
			_h5File.ReadData(curvePath, StrConst::BUnit_H5CurveName, curveName);
			curve->SetName(curveName);
			std::vector<double> x, y;
			_h5File.ReadData(curvePath, StrConst::BUnit_H5CurveX, x);
			_h5File.ReadData(curvePath, StrConst::BUnit_H5CurveY, y);
			curve->AddPoints(x, y);
			double valueZ;
			_h5File.ReadData(curvePath, StrConst::BUnit_H5CurveZ, valueZ);
			curve->SetZValue(valueZ);
		}
	}
}


/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "BaseUnit2.h"
#include "ContainerFunctions.h"

////////////////////////////////////////////////////////////////////////////////
// CUnitLookupTables
//

CUnitLookupTables::CUnitLookupTables(const CMaterialsDatabase* _materialsDB, const std::vector<std::string>* _compounds) :
	m_materialsDB{ _materialsDB },
	m_compounds{ _compounds }
{
}

void CUnitLookupTables::Clear() const
{
	m_tablesT.clear();
	m_tablesP.clear();
}

CLookupTable* CUnitLookupTables::GetLookupTable(ECompoundTPProperties _property, EDependencyTypes _dependency) const
{
	if (!IsDefined(_property, _dependency))
		AddPropertyTable(_property, _dependency);

	if (_dependency == EDependencyTypes::DEPENDENCE_TEMP)	return m_tablesT.at(_property).get();
	if (_dependency == EDependencyTypes::DEPENDENCE_PRES)	return m_tablesP.at(_property).get();

	return {};
}

double CUnitLookupTables::CalcTemperature(ECompoundTPProperties _property, double _value, const std::vector<double>& _fractions) const
{
	auto* table = GetLookupTable(_property, EDependencyTypes::DEPENDENCE_TEMP);
	table->SetCompoundFractions(_fractions);
	return table->GetParam(_value);
}

double CUnitLookupTables::CalcPressure(ECompoundTPProperties _property, double _value, const std::vector<double>& _fractions) const
{
	auto* table = GetLookupTable(_property, EDependencyTypes::DEPENDENCE_PRES);
	table->SetCompoundFractions(_fractions);
	return table->GetParam(_value);
}

double CUnitLookupTables::CalcFromTemperature(ECompoundTPProperties _property, double _T, const std::vector<double>& _fractions) const
{
	auto* table = GetLookupTable(_property, EDependencyTypes::DEPENDENCE_TEMP);
	table->SetCompoundFractions(_fractions);
	return table->GetValue(_T);
}

double CUnitLookupTables::CalcFromPressure(ECompoundTPProperties _property, double _P, const std::vector<double>& _fractions) const
{
	auto* table = GetLookupTable(_property, EDependencyTypes::DEPENDENCE_PRES);
	table->SetCompoundFractions(_fractions);
	return table->GetValue(_P);
}

bool CUnitLookupTables::IsDefined(ECompoundTPProperties _property, EDependencyTypes _dependency) const
{
	if (_dependency == EDependencyTypes::DEPENDENCE_TEMP)	return MapContainsKey(m_tablesT, _property);
	if (_dependency == EDependencyTypes::DEPENDENCE_PRES)	return MapContainsKey(m_tablesP, _property);
	return false;
}

void CUnitLookupTables::AddPropertyTable(ECompoundTPProperties _property, EDependencyTypes _dependency) const
{
	if (_dependency == EDependencyTypes::DEPENDENCE_TEMP)		m_tablesT.insert_or_assign(_property, std::make_unique<CLookupTable>(m_materialsDB, *m_compounds, _property, _dependency));
	else if (_dependency == EDependencyTypes::DEPENDENCE_PRES)	m_tablesP.insert_or_assign(_property, std::make_unique<CLookupTable>(m_materialsDB, *m_compounds, _property, _dependency));
}

////////////////////////////////////////////////////////////////////////////////
// CStreamLookupTables
//

CStreamLookupTables::CStreamLookupTables(const CBaseStream& _stream, const CMaterialsDatabase* _materialsDB, const std::vector<std::string>* _compounds) :
	CUnitLookupTables{ _materialsDB, _compounds },
	m_stream{ _stream }
{
}

void CStreamLookupTables::SetMaterialsDatabase(const CMaterialsDatabase* _materialsDB)
{
	m_materialsDB = _materialsDB;
}

void CStreamLookupTables::SetCompounds(const std::vector<std::string>* _compounds)
{
	m_compounds = _compounds;
}

double CStreamLookupTables::CalcTemperature(double _time, ECompoundTPProperties _property) const
{
	auto* table = GetLookupTable(_property, EDependencyTypes::DEPENDENCE_TEMP);
	table->SetCompoundFractions(m_stream.GetCompoundsFractions(_time));
	return table->GetParam(m_stream.GetMixtureProperty(_time, _property));
}

double CStreamLookupTables::CalcPressure(double _time, ECompoundTPProperties _property) const
{
	auto* table = GetLookupTable(_property, EDependencyTypes::DEPENDENCE_PRES);
	table->SetCompoundFractions(m_stream.GetCompoundsFractions(_time));
	return table->GetParam(m_stream.GetMixtureProperty(_time, _property));
}

double CStreamLookupTables::CalcFromTemperature(double _time, ECompoundTPProperties _property) const
{
	auto* table = GetLookupTable(_property, EDependencyTypes::DEPENDENCE_TEMP);
	table->SetCompoundFractions(m_stream.GetCompoundsFractions(_time));
	return table->GetValue(m_stream.GetTemperature(_time));
}

double CStreamLookupTables::CalcFromPressure(double _time, ECompoundTPProperties _property) const
{
	auto* table = GetLookupTable(_property, EDependencyTypes::DEPENDENCE_PRES);
	table->SetCompoundFractions(m_stream.GetCompoundsFractions(_time));
	return table->GetValue(m_stream.GetPressure(_time));
}

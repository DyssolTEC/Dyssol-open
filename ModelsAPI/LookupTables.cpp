/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "LookupTables.h"
#include "ContainerFunctions.h"
#include "Stream.h"

CLookupTables::CLookupTables(const CMaterialsDatabase* _database, const std::vector<std::string>* _compounds) :
	m_materialsDB{ _database },
	m_compounds{ _compounds }
{
}

void CLookupTables::Clear()
{
	m_tablesT.clear();
	m_tablesP.clear();
}

CLookupTable* CLookupTables::GetLookupTable(double _time, ECompoundTPProperties _property, EDependencyTypes _dependency)
{
	if (!IsDefined(_property, _dependency))
		AddPropertyTable(_property, _dependency);

	if (_dependency == EDependencyTypes::DEPENDENCE_TEMP)	return m_tablesT.at(_property).get();
	if (_dependency == EDependencyTypes::DEPENDENCE_PRES)	return m_tablesP.at(_property).get();

	return {};
}

double CLookupTables::CalcTemperatureFromProperty(double _time, ECompoundTPProperties _property, double _value, const std::vector<double>& _fractions)
{
	auto* table = GetLookupTable(_time, _property, EDependencyTypes::DEPENDENCE_TEMP);
	table->SetCompoundFractions(_fractions);
	return table->GetParam(_value);
}

double CLookupTables::CalcPressureFromProperty(double _time, ECompoundTPProperties _property, double _value, const std::vector<double>& _fractions)
{
	auto* table = GetLookupTable(_time, _property, EDependencyTypes::DEPENDENCE_PRES);
	table->SetCompoundFractions(_fractions);
	return table->GetParam(_value);
}

double CLookupTables::CalcPropertyFromTemperature(double _time, ECompoundTPProperties _property, double _T, const std::vector<double>& _fractions)
{
	auto* table = GetLookupTable(_time, _property, EDependencyTypes::DEPENDENCE_TEMP);
	table->SetCompoundFractions(_fractions);
	return table->GetValue(_T);
}

double CLookupTables::CalcPropertyFromPressure(double _time, ECompoundTPProperties _property, double _P, const std::vector<double>& _fractions)
{
	auto* table = GetLookupTable(_time, _property, EDependencyTypes::DEPENDENCE_PRES);
	table->SetCompoundFractions(_fractions);
	return table->GetValue(_P);
}

bool CLookupTables::IsDefined(ECompoundTPProperties _property, EDependencyTypes _dependency) const
{
	if (_dependency == EDependencyTypes::DEPENDENCE_TEMP)	return MapContainsKey(m_tablesT, _property);
	if (_dependency == EDependencyTypes::DEPENDENCE_PRES)	return MapContainsKey(m_tablesP, _property);
	return false;
}

void CLookupTables::AddPropertyTable(ECompoundTPProperties _property, EDependencyTypes _dependency)
{
	if (_dependency == EDependencyTypes::DEPENDENCE_TEMP)		m_tablesT.insert_or_assign(_property, CLookupTable(m_materialsDB, *m_compounds, _property, _dependency));
	else if (_dependency == EDependencyTypes::DEPENDENCE_PRES)	m_tablesP.insert_or_assign(_property, CLookupTable(m_materialsDB, *m_compounds, _property, _dependency));
}
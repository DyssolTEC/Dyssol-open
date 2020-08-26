/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "LookupTables.h"
#include "ContainerFunctions.h"
#include "Stream.h"

CLookupTables::CLookupTables(const CBaseStream* _stream) :
	m_stream{ _stream }
{
}

void CLookupTables::Clear() const
{
	m_tablesT.clear();
	m_tablesP.clear();
}

CLookupTable* CLookupTables::GetLookupTable(ECompoundTPProperties _property, EDependencyTypes _dependency) const
{
	if (!IsDefined(_property, _dependency))
		AddPropertyTable(_property, _dependency);

	if (_dependency == EDependencyTypes::DEPENDENCE_TEMP)	return m_tablesT.at(_property).get();
	if (_dependency == EDependencyTypes::DEPENDENCE_PRES)	return m_tablesP.at(_property).get();

	return {};
}

double CLookupTables::CalcTemperature(ECompoundTPProperties _property, double _value, const std::vector<double>& _fractions) const
{
	auto* table = GetLookupTable(_property, EDependencyTypes::DEPENDENCE_TEMP);
	table->SetCompoundFractions(_fractions);
	return table->GetParam(_value);
}

double CLookupTables::CalcPressure(ECompoundTPProperties _property, double _value, const std::vector<double>& _fractions) const
{
	auto* table = GetLookupTable(_property, EDependencyTypes::DEPENDENCE_PRES);
	table->SetCompoundFractions(_fractions);
	return table->GetParam(_value);
}

double CLookupTables::CalcFromTemperature(ECompoundTPProperties _property, double _T, const std::vector<double>& _fractions) const
{
	auto* table = GetLookupTable(_property, EDependencyTypes::DEPENDENCE_TEMP);
	table->SetCompoundFractions(_fractions);
	return table->GetValue(_T);
}

double CLookupTables::CalcFromPressure(ECompoundTPProperties _property, double _P, const std::vector<double>& _fractions) const
{
	auto* table = GetLookupTable(_property, EDependencyTypes::DEPENDENCE_PRES);
	table->SetCompoundFractions(_fractions);
	return table->GetValue(_P);
}

double CLookupTables::CalcTemperature(double _time, ECompoundTPProperties _property) const
{
	auto* table = GetLookupTable(_property, EDependencyTypes::DEPENDENCE_TEMP);
	table->SetCompoundFractions(m_stream->GetCompoundsFractions(_time));
	return table->GetParam(m_stream->GetMixtureProperty(_time, _property));
}

double CLookupTables::CalcPressure(double _time, ECompoundTPProperties _property) const
{
	auto* table = GetLookupTable(_property, EDependencyTypes::DEPENDENCE_PRES);
	table->SetCompoundFractions(m_stream->GetCompoundsFractions(_time));
	return table->GetParam(m_stream->GetMixtureProperty(_time, _property));
}

double CLookupTables::CalcFromTemperature(double _time, ECompoundTPProperties _property) const
{
	auto* table = GetLookupTable(_property, EDependencyTypes::DEPENDENCE_TEMP);
	table->SetCompoundFractions(m_stream->GetCompoundsFractions(_time));
	return table->GetValue(m_stream->GetTemperature(_time));
}

double CLookupTables::CalcFromPressure(double _time, ECompoundTPProperties _property) const
{
	auto* table = GetLookupTable(_property, EDependencyTypes::DEPENDENCE_PRES);
	table->SetCompoundFractions(m_stream->GetCompoundsFractions(_time));
	return table->GetValue(m_stream->GetPressure(_time));
}

bool CLookupTables::IsDefined(ECompoundTPProperties _property, EDependencyTypes _dependency) const
{
	if (_dependency == EDependencyTypes::DEPENDENCE_TEMP)	return MapContainsKey(m_tablesT, _property);
	if (_dependency == EDependencyTypes::DEPENDENCE_PRES)	return MapContainsKey(m_tablesP, _property);
	return false;
}

void CLookupTables::AddPropertyTable(ECompoundTPProperties _property, EDependencyTypes _dependency) const
{
	if (_dependency == EDependencyTypes::DEPENDENCE_TEMP)		m_tablesT.insert_or_assign(_property, std::make_unique<CLookupTable>(m_stream->GetMaterialsDatabase(), m_stream->GetAllCompounds(), _property, _dependency));
	else if (_dependency == EDependencyTypes::DEPENDENCE_PRES)	m_tablesP.insert_or_assign(_property, std::make_unique<CLookupTable>(m_stream->GetMaterialsDatabase(), m_stream->GetAllCompounds(), _property, _dependency));
}

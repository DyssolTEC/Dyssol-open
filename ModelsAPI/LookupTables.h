/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DefinesMDB.h"
#include "LookupTable.h"
#include <mutex>

class CBaseStream;
class CMaterialsDatabase;

class CLookupTables
{
	mutable std::map<ECompoundTPProperties, std::unique_ptr<CLookupTable>> m_tablesT;	// Map with all lookup tables for temperature.
	mutable std::map<ECompoundTPProperties, std::unique_ptr<CLookupTable>> m_tablesP;	// Map with all lookup tables for pressure.

	const CBaseStream* m_stream{ nullptr };					// Pointer to a stream.

public:
	CLookupTables(const CBaseStream* _stream);

	// Clears all calculated lookup tables.
	void Clear() const;

	//Returns a corresponding lookup table.
	CLookupTable* GetLookupTable(ECompoundTPProperties _property, EDependencyTypes _dependency) const;

	// Calculates temperature of the corresponding property for the specified property value and compound fractions.
	double CalcTemperature(ECompoundTPProperties _property, double _value, const std::vector<double>& _fractions) const;
	// Calculates pressure of the corresponding property for the specified property value and compound fractions.
	double CalcPressure(ECompoundTPProperties _property, double _value, const std::vector<double>& _fractions) const;
	// Calculates the value of the corresponding property for the given temperature and compound fractions.
	double CalcFromTemperature(ECompoundTPProperties _property, double _T, const std::vector<double>& _fractions) const;
	// Calculates the value of the corresponding property for the given pressure and compound fractions.
	double CalcFromPressure(ECompoundTPProperties _property, double _P, const std::vector<double>& _fractions) const;

	// Calculates temperature of the corresponding property for the property value and compound fractions defined at the given time point.
	double CalcTemperature(double _time, ECompoundTPProperties _property) const;
	// Calculates pressure of the corresponding property for the property value and compound fractions defined at the given time point.
	double CalcPressure(double _time, ECompoundTPProperties _property) const;
	// Calculates the value of the corresponding property for the temperature and compound fractions defined at the given time point.
	double CalcFromTemperature(double _time, ECompoundTPProperties _property) const;
	// Calculates the value of the corresponding property for the pressure and compound fractions defined at the given time point.
	double CalcFromPressure(double _time, ECompoundTPProperties _property) const;

private:
	// Checks whether the lookup table for a certain property was already defined.
	bool IsDefined(ECompoundTPProperties _property, EDependencyTypes _dependency) const;

	// Adds a new lookup table object for a certain property.
	void AddPropertyTable(ECompoundTPProperties _property, EDependencyTypes _dependency) const;
};


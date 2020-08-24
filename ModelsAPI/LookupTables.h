/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DefinesMDB.h"
#include "LookupTable.h"

class CMaterialsDatabase;

class CLookupTables
{
	std::map<ECompoundTPProperties, std::unique_ptr<CLookupTable>> m_tablesT;	// Map with all lookup tables for temperature.
	std::map<ECompoundTPProperties, std::unique_ptr<CLookupTable>> m_tablesP;	// Map with all lookup tables for pressure.

	const CMaterialsDatabase* m_materialsDB{ nullptr };	// Pointer to a database of materials.
	const std::vector<std::string>* m_compounds;		// Keys of defined chemical compounds.

public:
	CLookupTables(const CMaterialsDatabase* _database, const std::vector<std::string>* _compounds);

	// Clears all calculated lookup tables.
	void Clear();

	//Returns a corresponding lookup table.
	CLookupTable* GetLookupTable(double _time, ECompoundTPProperties _property, EDependencyTypes _dependency);

	// Calculates temperature of the corresponding property for the specified property value and compound fractions.
	double CalcTemperatureFromProperty(double _time, ECompoundTPProperties _property, double _value, const std::vector<double>& _fractions);

	// Calculates pressure of the corresponding property for the specified property value and compound fractions.
	double CalcPressureFromProperty(double _time, ECompoundTPProperties _property, double _value, const std::vector<double>& _fractions);

	// Calculates the value of the corresponding property for the given temperature and compound fractions.
	double CalcPropertyFromTemperature(double _time, ECompoundTPProperties _property, double _T, const std::vector<double>& _fractions);

	// Calculates the value of the corresponding property for the given pressure and compound fractions.
	double CalcPropertyFromPressure(double _time, ECompoundTPProperties _property, double _P, const std::vector<double>& _fractions);

private:
	// Checks whether the lookup table for a certain property was already defined.
	bool IsDefined(ECompoundTPProperties _property, EDependencyTypes _dependency) const;

	// Adds a new lookup table object for a certain property.
	void AddPropertyTable(ECompoundTPProperties _property, EDependencyTypes _dependency);
};


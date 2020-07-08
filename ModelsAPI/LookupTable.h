/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "MaterialsDatabase.h"

#define DEFAULT_LOWER_LIMIT		273
#define DEFAULT_UPPER_LIMIT		2773
#define DEFAULT_STEP			25
#define MAX_TEMPERATURE			2500
#define MAX_PRESSURE			1000000
#define MAX_TEMPERATURE_STEPS	25
#define MAX_PRESSURE_STEPS		1000

class CLookupTable
{
private:
	CDependentValues m_table;
	mutable CDependentValues m_flippedTable;
	mutable bool m_bFlippedConsistent;

	const CMaterialsDatabase* m_pMaterialsDB; /// Pointer to a database of materials

	double m_dMin;			/// Lower limit of the lookup table
	double m_dMax;			/// Upper limit of the lookup table
	double m_dStep;			/// Interval size of the lookup table

	EDependencyTypes m_nDependenceType;
	ECompoundTPProperties m_nProperty;
	std::vector<std::string> m_vCompoundKeys;			// Vector with keys of the chemical compounds which contains this lookuptable
	std::vector<CDependentValues> m_vCompoundTables;	// Vector with compound lookup tables

public:
	CLookupTable();
	CLookupTable(const CMaterialsDatabase* _pMaterialsDB, const std::vector<std::string>& _vCompoundKeys, ECompoundTPProperties _nProperty, EDependencyTypes _nDependenceType);

	/// Initializes the lookup table with new initials.
	void Initialize(const CMaterialsDatabase* _pMaterialsDB, const std::vector<std::string>& _vCompoundKeys, ECompoundTPProperties _nProperty, EDependencyTypes _nDependenceType);

	/// Removes all infpormation from the table.
	void Clear();

	/// Returns true if it set up and ready to use.
	bool IsValid() const;

	/** Sets the compound information of the lookup table and calculates the total lookup table from these information.
	*	\param _vFractions Vector with compound fractions. */
	void SetCompoundFractions(const std::vector<double>& _vFractions);

	/** Returns the value of the property (e.g. enthalpy) for the given parameter value.
	*	\param _dParam Parameter value (temperature, pressure, etc.). */
	double GetValue(double _dParam) const;
	/**	Returns the value of the parameter (e.g. temperature) for the given property value.
	*	\param _dValue Property value (enthalpy, etc.). */
	double GetParam(double _dValue) const;

	/*	Sets table, i.e. CDependentValue, of the lookup table.
	*	\param _table New table of the lookup table.
	*	\param _dWeight Coefficient with which the table is to be multiplied. */
	void SetTable(const CDependentValues& _table, double _dWeight = 1.);

	/**	Adds a value with a certain weight to each entry of the table, if table is defined, otherwise does nothing.
	*	\param _dValue Value to be added to the table.
	*	\param _dWeight Coefficient with which the value is to be multiplied. */
	void Add(double _dValue, double _dWeight = 1.);
	/**	Add a table with a certain weight to the member table. If member table is not defined set _table as member table with respective weight.
	*	\param _table Table to be added to the member table.
	*	\param _dWeight Coefficient with which the additional table is to be multiplied. */
	void Add(const CDependentValues& _table, double _dWeight = 1.);
	/**	Add a table with a certain weight to the member table. If member table is not defined set _lookupTable as member table with respective weight.
	*	\param _lookupTable Lookup table to be added to the member table.
	*	\param _dWeight Coefficient with which the additional table is to be multiplied. */
	void Add(const CLookupTable& _lookupTable, double _dWeight = 1.);

	/**	Multiply each entry of the table with a weight, if table is defined, otherwise do nothing.
	*	\param _dWeight Coefficient with which the table entries are to be multiplied. */
	void MultiplyTable(double _dWeight);

private:
	/** Initializes m_vCompoundTables according to the defined m_vCompoundKeys list. */
	void InitializeCompoundTables();

	/** Get upper and lower limits as well as interval size of lookup table from compound lookup tables in material database. */
	void UpdateTableLimits();

	/** Calculates a lookup table of a compound from values of the material database between the limits m_dMin and m_dMax with interval m_dStep.
	*	\param _sCompoundKey Compound key.
	*	\return Lookup table*/
	CDependentValues GetParametersList(const std::string& _sCompoundKey) const;

	// Updates a flipped table in the m_flippedTable by swapping [param <-> value].
	void UpdateFlippedTable() const;
};

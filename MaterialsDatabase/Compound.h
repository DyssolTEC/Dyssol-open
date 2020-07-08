/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ConstProperty.h"
#include "TPDProperty.h"

class CCompound : public CDescriptable
{
	std::string m_sName;							// Compound name.
	std::string m_sUniqueKey;						// Unique compound key.
	std::string m_sDescription;						// User provided description.
	std::vector<CConstProperty> m_vConstProperties; // List of constant properties.
	std::vector<CTPDProperty> m_vTPProperties;		// List of temperature/pressure-dependent properties.

public:
	CCompound(const MDBDescriptors::constDescr& _constDescrs, const MDBDescriptors::tpdepDescr& _tpdDescrs, const std::string& _sUniqueCompoundKey = "");

	//////////////////////////////////////////////////////////////////////////
	/// Basic parameters

	// Returns compound's name
	std::string GetName() const;
	// Sets new compound's name
	void SetName(const std::string& _sName);

	// Returns compound's unique key
	std::string GetKey() const;
	// Sets new compound's key
	void SetKey(const std::string& _sKey);

	// Returns number of const properties
	size_t ConstPropertiesNumber() const;
	// Returns number of temperature/pressure-dependent properties.
	size_t TPPropertiesNumber() const;

	// Returns true if the compound contains specified constant property.
	bool HasConstProperty(ECompoundConstProperties _nType) const;
	// Returns true if the compound contains specified temperature/pressure-dependent property.
	bool HasTPProperty(ECompoundTPProperties _nType) const;
	// Returns true if the compound contains specified constant or temperature/pressure-dependent property.
	bool HasProperty(unsigned _nType) const;

	// Adds new constant property if it does not exist yet.
	void AddConstProperty(ECompoundConstProperties _key, const std::string& _name, const std::wstring& _units, double _defaultValue);
	// Adds new temperature/pressure-dependent property if it does not exist yet.
	void AddTPDepProperty(ECompoundTPProperties _key, const std::string& _name, const std::wstring& _units, double _defaultValue);
	// Removes constant property if it exists.
	void RemoveConstProperty(ECompoundConstProperties _key);
	// Removes temperature/pressure-dependent property if it exists.
	void RemoveTPDepProperty(ECompoundTPProperties _key);

	//////////////////////////////////////////////////////////////////////////
	/// Pointers getters

	// Returns pointer to a specified const property. Returns nullptr if property is not found.
	CConstProperty* GetConstProperty(ECompoundConstProperties _nType);
	// Returns constant pointer to a specified const property. Returns nullptr if property is not found.
	const CConstProperty* GetConstProperty(ECompoundConstProperties _nType) const;
	// Returns vector of defined const properties.
	const std::vector<CConstProperty>& GetConstProperties() const;
	// Returns pointer to a specified temperature/pressure-dependent property. Returns nullptr if property is not found.
	CTPDProperty* GetTPProperty(ECompoundTPProperties _nType);
	// Returns constant pointer to a specified temperature/pressure-dependent property. Returns nullptr if property is not found.
	const CTPDProperty* GetTPProperty(ECompoundTPProperties _nType) const;
	// Returns pointer to a specified temperature/pressure-dependent property by its index. Returns nullptr if property is not found.
	CTPDProperty* GetTPPropertyByIndex(size_t _index);
	// Returns constant pointer to a specified temperature/pressure-dependent property by its index. Returns nullptr if property is not found.
	const CTPDProperty* GetTPPropertyByIndex(size_t _index) const;
	// Returns vector of defined temperature/pressure-dependent properties.
	const std::vector<CTPDProperty>& GetTPProperties() const;


	//////////////////////////////////////////////////////////////////////////
	/// Values getters

	// Returns value of a constant property. Returns 0 if such property doesn't exist.
	double GetConstPropertyValue(ECompoundConstProperties _nType) const;
	// Returns value of a temperature/pressure-dependent property by specified temperature [K] and pressure [Pa]. Returns 0 if such property doesn't exist.
	double GetTPPropertyValue(ECompoundTPProperties _nType, double _dT, double _dP) const;
};

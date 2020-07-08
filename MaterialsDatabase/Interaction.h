/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "TPDProperty.h"

// Interaction properties between two pure compounds.
class CInteraction
{
	std::string m_sCompoundKey1;				// Unique key of the first interacting compound.
	std::string m_sCompoundKey2;				// Unique key of the second interacting compound.
	std::vector<CTPDProperty> m_vProperties;	// List of interaction properties.

public:
	CInteraction(const MDBDescriptors::interDescr& _interDescrs, std::string _sKey1, std::string _sKey2);

	// Returns unique key of the first interacting compound.
	std::string GetKey1() const;
	// Returns unique key of the second interacting compound.
	std::string GetKey2() const;
	// Sets unique keys of interacting compounds.
	void SetKeys(const std::string& _sKey1, const std::string& _sKey2);
	// Returns number of properties.
	size_t PropertiesNumber() const;

	// Returns true if the interaction contains specified temperature/pressure-dependent property.
	bool HasProperty(EInteractionProperties _key) const;

	// Adds new temperature/pressure-dependent property if it does not exist yet.
	void AddProperty(EInteractionProperties _key, const std::string& _name, const std::wstring& _units, double _defaultValue);
	// Removes temperature/pressure-dependent property if it exists.
	void RemoveProperty(EInteractionProperties _key);

	// Returns pointer to a specified property. Returns nullptr if property is not found.
	CTPDProperty* GetProperty(EInteractionProperties _nType);
	// Returns constant pointer to a specified property. Returns nullptr if property is not found.
	const CTPDProperty* GetProperty(EInteractionProperties _nType) const;
	// Returns pointer to a specified property by its index. Returns nullptr if property is not found.
	CTPDProperty* GetPropertyByIndex(size_t _index);
	// Returns constant pointer to a specified property by its index. Returns nullptr if property is not found.
	const CTPDProperty* GetPropertyByIndex(size_t _index) const;
	// Returns vector of defined properties.
	const std::vector<CTPDProperty>& GetProperties() const;

	// Returns value of a TP-property by specified temperature [K] and pressure [Pa]. Returns 0 if such property doesn't exist.
	double GetPropertyValue(EInteractionProperties _nType, double _dT, double _dP) const;

	///Returns true if this interaction describes relation between specified compounds.
	bool IsBetween(const std::string& _sKey1, const std::string& _sKey2) const;
};


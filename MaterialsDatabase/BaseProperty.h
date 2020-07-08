/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DefinesMDB.h"
#include "Descriptable.h"

// Base description of a property of a pure compound.
class CBaseProperty : public CDescriptable
{
protected:
	unsigned m_nType;			// Type of the property (MOLAR_MASS, STANDARD_FORMATION_ENTHALPY, DENSITY, VISCOSITY, etc.).
	std::string m_sName;		// Name of the property.
	std::wstring m_sUnits;		// Property units (Pa, K, J/mol, etc.).

public:
	CBaseProperty(unsigned _nType, std::string _sName, std::wstring _sUnits);
	virtual ~CBaseProperty() = 0;
	CBaseProperty(const CBaseProperty& _other) = default;
	CBaseProperty(CBaseProperty&& _other) noexcept = default;
	CBaseProperty& operator=(const CBaseProperty& _other) = default;
	CBaseProperty& operator=(CBaseProperty&& _other) noexcept = default;

	// Returns type of the property.
	unsigned GetType() const;
	// Sets type of the property.
	void SetType(unsigned _nType);

	// Returns name of the property.
	std::string GetName() const;
	// Set name of the property.
	void SetName(const std::string& _sName);

	// Returns measurements units of the property.
	std::wstring GetUnits() const;
	// Sets measurements units of the property.
	void SetUnits(const std::wstring& _sUnits);

	// Checks if the default value is set.
	virtual bool IsDefaultValue() const = 0;
};

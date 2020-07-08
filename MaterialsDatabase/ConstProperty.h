/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BaseProperty.h"

// Description of a constant property of a pure compound.
class CConstProperty : public CBaseProperty
{
	double m_defaultValue;	// Default value.
	double m_dValue;		// Current value of the constant property.

public:
	CConstProperty(unsigned _nProperty, const std::string& _sName, const std::wstring& _sUnits, double _defaultValue);

	// Returns value of the constant property.
	double GetValue() const;
	// Sets value of the constant property.
	void SetValue(double _dValue);

	// Checks if the default value is set.
	bool IsDefaultValue() const override;
};

/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once
#include <string>

class CDescriptable
{
	std::string m_sDescription;	// User provided description.

public:
	// Returns property's description
	std::string GetDescription() const { return m_sDescription; }
	// Sets new property's description
	void SetDescription(const std::string& _sText) { m_sDescription = _sText; }
};


/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BaseProperty.h"
#include "Correlation.h"

// Description of a temperature/pressure-dependent property of a pure compound.
class CTPDProperty : public CBaseProperty
{
	CCorrelation m_defaultValue;				// Default value.
	std::vector<CCorrelation> m_vCorrelations;	// List of correlations for different combinations of T and P.

public:
	CTPDProperty(unsigned _nProperty, const std::string& _sName, const std::wstring& _sUnits, const CCorrelation& _defaultValue);

	// Returns property value for specified T and P. If no correlation defined for specified T and P, a nearest-neighbor extrapolation from some correlation will be done.
	double GetValue(double _dT, double _dP) const;

	// Returns boundaries of the temperature interval, on which this property is defined. If no correlations defined, returns interval (-1;-1).
	SInterval GetTInterval() const;
	// Returns boundaries of the pressure interval, on which this property is defined. If no correlations defined, returns interval (-1;-1).
	SInterval GetPInterval() const;

	// Returns number of defined correlations.
	size_t CorrelationsNumber() const;

	// Returns pointer to a correlation with the specified index. Returns nullptr if such correlation has not been defined.
	CCorrelation* GetCorrelation(size_t _index);
	// Returns pointer to a correlation with the specified index. Returns nullptr if such correlation has not been defined.
	const CCorrelation* GetCorrelation(size_t _index) const;

	// Adds specified correlation to the property, if it does not exist yet.
	void AddCorrelation(const CCorrelation& _correlation);
	// Adds new correlation with specified parameters to the property, if it does not exist yet.
	void AddCorrelation(ECorrelationTypes _nType, const std::vector<double>& _vParams, const SInterval& _TInterval = {MDBDescriptors::TEMP_MIN , MDBDescriptors::TEMP_MAX}, const SInterval& _PInterval = {MDBDescriptors::PRES_MIN , MDBDescriptors::PRES_MAX});

	// Sets settings to the existing correlation, if such does not exist yet.
	void SetCorrelation(size_t _index, ECorrelationTypes _nType, const std::vector<double>& _vParams, const SInterval& _TInterval = {MDBDescriptors::TEMP_MIN , MDBDescriptors::TEMP_MAX}, const SInterval& _PInterval = {MDBDescriptors::PRES_MIN , MDBDescriptors::PRES_MAX});

	// Removes specified correlation from the list.
	void RemoveCorrelation(size_t _index);
	// Removes all defined correlations.

	// Shift selected correlation upwards in correlations list.
	bool ShiftCorrelationUp(size_t _index);
	// Shift selected correlation downwards in correlations list.
	bool ShiftCorrelationDown(size_t _index);

	// Removes all existing correlations.
	void RemoveAllCorrelations();

	// Checks if the default value is set.
	bool IsDefaultValue() const override;
};


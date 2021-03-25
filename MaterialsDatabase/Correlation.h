/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "Descriptable.h"
#include "DyssolTypes.h"
#include "DefinesMDB.h"
#include "DependentValues.h"

// Correlation between the value of the property and the temperature(T)/pressure(P) in a certain T/P-interval. Is used to describe TP-dependent parameters of pure compounds
class CCorrelation : public CDescriptable
{
	ECorrelationTypes m_nType{};		// Type of this correlation.
	SInterval m_TInterval{};			// Left and right boundaries of temperature interval in [K], for which this correlation is defined.
	SInterval m_PInterval{};			// Left and right boundaries of pressure interval in [Pa], for which this correlation is defined.
	std::vector<double> m_vParameters;	// Correlation parameters for all types except LIST_OF_T_VALUES and LIST_OF_P_VALUES.
	CDependentValues m_valuesList;		// List of pairs [parameter:value] in the case of LIST_OF_T_VALUES and LIST_OF_P_VALUES.

public:
	CCorrelation();
	CCorrelation(ECorrelationTypes _nType, const std::vector<double>& _vParams, const SInterval& _TInterval = { MDBDescriptors::TEMP_MIN , MDBDescriptors::TEMP_MAX }, const SInterval& _PInterval = { MDBDescriptors::PRES_MIN , MDBDescriptors::PRES_MAX });

	// Returns boundaries of the Temperature interval.
	SInterval GetTInterval() const;
	// Sets boundaries of the Temperature interval. Returns true if the interval has been successfully set.
	bool SetTInterval(const SInterval& _TInterval);
	// Returns boundaries of the Pressure interval.
	SInterval GetPInterval() const;
	// Sets boundaries of the Pressure interval. Returns true if the interval has been successfully set.
	bool SetPInterval(const SInterval& _PInterval);

	// Returns type of the correlation.
	ECorrelationTypes GetType() const;
	// Sets type of the correlation and resizes vector of parameters accordingly.
	void SetType(ECorrelationTypes _nType);

	// Returns vector of correlation parameters.
	std::vector<double> GetParameters() const;
	// Sets vector of correlation parameters. In the case of LIST_OF_T_VALUES and LIST_OF_P_VALUES, it must be a list of pairs [parameter:value].
	bool SetParameters(const std::vector<double>& _vParams);

	// Returns value of the correlation at the specified Temperature and Pressure. For the LIST_OF_T_VALUES and LIST_OF_P_VALUES returns a linearly interpolated value.
	// If the specified T or P are out of defined intervals, returns value at the nearest boundary.
	double GetValue(double _dT, double _dP) const;

	// Returns true if T lays within the defined interval for this correlation.
	bool IsTInInterval(double _dT) const;
	// Returns true if P lays within the defined interval for this correlation.
	bool IsPInInterval(double _dP) const;
	// Returns true if both parameters are within the defined interval for this correlation.
	bool IsInInterval(double _dT, double _dP) const;

	// Stream extraction.
	friend std::ostream& operator<<(std::ostream& os, const CCorrelation& val);
	// Stream insertion.
	friend std::istream& operator>>(std::istream& is, CCorrelation& val);

	// Comparison.
	bool operator==(const CCorrelation& _c) const;

private:
	// Initializes Correlation with specified parameters. If some error occurs during initialization, sets default parameters.
	void Initialize(ECorrelationTypes _nType, const std::vector<double>& _vParams, const SInterval& _TInterval, const SInterval& _PInterval);
	// Set new correlation with parameters. It some error occurs, returns false and leaves the Correlation in inconsistent state.
	bool TrySetCorrelation(ECorrelationTypes _nType, const std::vector<double>& _vParams, const SInterval& _TInterval, const SInterval& _PInterval);
};

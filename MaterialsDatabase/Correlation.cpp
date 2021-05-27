/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "Correlation.h"
#include "StringFunctions.h"
#include "DyssolStringConstants.h"
#include <sstream>
#include <cmath>

CCorrelation::CCorrelation()
{
	Initialize(ECorrelationTypes::LIST_OF_T_VALUES, std::vector<double>(), { MDBDescriptors::TEMP_MIN , MDBDescriptors::TEMP_MAX }, { MDBDescriptors::PRES_MIN , MDBDescriptors::PRES_MAX });
}

CCorrelation::CCorrelation(ECorrelationTypes _nType, const std::vector<double>& _vParams, const SInterval& _TInterval /*= { TEMP_MIN , TEMP_MAX }*/, const SInterval& _PInterval /*= { PRES_MIN , PRES_MAX }*/)
{
	Initialize(_nType, _vParams, _TInterval, _PInterval);
}

SInterval CCorrelation::GetTInterval() const
{
	return m_TInterval;
}

bool CCorrelation::SetTInterval(const SInterval& _TInterval)
{
	if (_TInterval.min < 0 || _TInterval.max < 0) return false;
	m_TInterval = _TInterval;
	return true;
}

SInterval CCorrelation::GetPInterval() const
{
	return m_PInterval;
}

bool CCorrelation::SetPInterval(const SInterval& _PInterval)
{
	if (_PInterval.min < 0 || _PInterval.max < 0) return false;
	m_PInterval = _PInterval;
	return true;
}

ECorrelationTypes CCorrelation::GetType() const
{
	return m_nType;
}

void CCorrelation::SetType(ECorrelationTypes _nType)
{
	if (_nType == m_nType) return;	// the same type

	m_nType = _nType;

	// invalidate old values
	m_vParameters.clear();
	m_valuesList.Clear();

	// prepare the parameters list
	m_vParameters.resize(MDBDescriptors::correlations[m_nType].parametersNumber, 1.0);
}

std::vector<double> CCorrelation::GetParameters() const
{
	if (m_nType != ECorrelationTypes::LIST_OF_T_VALUES && m_nType != ECorrelationTypes::LIST_OF_P_VALUES)
		return m_vParameters;
	else
	{
		std::vector<double> vRes;
		for (size_t i = 0; i < m_valuesList.Size(); ++i)
		{
			vRes.push_back(m_valuesList.GetParamAt(i));
			vRes.push_back(m_valuesList.GetValueAt(i));
		}
		return vRes;
	}
}

bool CCorrelation::SetParameters(const std::vector<double>& _vParams)
{
	if (m_nType == ECorrelationTypes::LIST_OF_T_VALUES || m_nType == ECorrelationTypes::LIST_OF_P_VALUES)
	{
		// check that the length of parameters is an even value, as it must contain pairs [parameter:value]
		if (_vParams.size() % 2 != 0) return false;
		// remove old data
		m_valuesList.Clear();
		// set the list of pairs
		for (size_t i = 0; i < _vParams.size(); i += 2)
			m_valuesList.SetValue(_vParams[i], _vParams[i + 1]);
	}
	else
	{
		// check parameters number
		if (_vParams.size() != MDBDescriptors::correlations[m_nType].parametersNumber) return false;

		// set parameters
		m_vParameters = _vParams;
	}

	return true;
}

double CCorrelation::GetValue(double _dT, double _dP) const
{
	if (!IsTInInterval(_dT)) _dT = _dT < m_TInterval.min ? m_TInterval.min : m_TInterval.max;
	if (!IsPInInterval(_dP)) _dP = _dP < m_PInterval.min ? m_PInterval.min : m_PInterval.max;

	switch (m_nType)
	{
	case ECorrelationTypes::LIST_OF_T_VALUES:
		return m_valuesList.GetValue(_dT);
	case ECorrelationTypes::LIST_OF_P_VALUES:
		return m_valuesList.GetValue(_dP);
	case ECorrelationTypes::CONSTANT:
		return m_vParameters[0];
	case ECorrelationTypes::LINEAR:
		return _dT*m_vParameters[0] + _dP*m_vParameters[1] + m_vParameters[2];
	case ECorrelationTypes::EXPONENT_1:
		if (m_vParameters[6] * _dT + m_vParameters[7] != 0)
			return m_vParameters[0] * std::pow(m_vParameters[1], m_vParameters[2] + m_vParameters[3] * _dT + (m_vParameters[4] * _dT + m_vParameters[5]) / (m_vParameters[6] * _dT + m_vParameters[7])) + m_vParameters[8];
	case ECorrelationTypes::POW_1:
		return m_vParameters[0] * std::pow(_dT, m_vParameters[1]);
	case ECorrelationTypes::POLYNOMIAL_1:
		return m_vParameters[0] + m_vParameters[1] * _dT + m_vParameters[2] * std::pow(_dT, 2) + m_vParameters[3] * std::pow(_dT, 3) + m_vParameters[4] * std::pow(_dT, 4) + m_vParameters[5] * std::pow(_dT, 5) + m_vParameters[6] * std::pow(_dT, 6) + m_vParameters[7] * std::pow(_dT, 7);
	case ECorrelationTypes::POLYNOMIAL_CP:
		if(_dT != 0)
			return m_vParameters[0] + m_vParameters[1] * _dT + m_vParameters[2] * std::pow(_dT, 2.) + m_vParameters[3] * std::pow(_dT, 3.) + m_vParameters[4] / std::pow(_dT, 2.);
	case ECorrelationTypes::POLYNOMIAL_H:
		if (_dT != 0)
			return m_vParameters[0] * _dT + m_vParameters[1] * std::pow(_dT, 2.) / 2. + m_vParameters[2] * std::pow(_dT, 3.) / 3. + m_vParameters[3] * std::pow(_dT, 4.) / 4. - m_vParameters[4] / _dT + m_vParameters[5] - m_vParameters[6];
	case ECorrelationTypes::POLYNOMIAL_S:
		if (_dT != 0)
			return m_vParameters[0] * std::log(_dT) + m_vParameters[1] * _dT + m_vParameters[2] * std::pow(_dT, 2.) / 2. + m_vParameters[3] * std::pow(_dT, 3.) / 3. - m_vParameters[4] / (2 * std::pow(_dT, 2.)) + m_vParameters[5];
	}

	return 0;
}

bool CCorrelation::IsTInInterval(double _dT) const
{
	return _dT >= m_TInterval.min && _dT <= m_TInterval.max;
}

bool CCorrelation::IsPInInterval(double _dP) const
{
	return (_dP >= m_PInterval.min) && (_dP <= m_PInterval.max);
}

bool CCorrelation::IsInInterval(double _dT, double _dP) const
{
	return IsTInInterval(_dT) && IsPInInterval(_dP);
}

std::ostream& operator<<(std::ostream& os, const CCorrelation& val)
{
	os << static_cast<unsigned>(val.GetType()) << " " << val.GetTInterval() << " " << val.GetPInterval() << " ";
	for (auto v : val.GetParameters())
		os << v << " ";
	return os;
}

std::istream& operator>>(std::istream& is, CCorrelation& val)
{
	unsigned type;
	SInterval TInterval{ MDBDescriptors::TEMP_MIN , MDBDescriptors::TEMP_MAX };
	SInterval PInterval{ MDBDescriptors::PRES_MIN , MDBDescriptors::PRES_MAX };
	is >> type >> TInterval >> PInterval;
	std::stringstream valuesStream(StringFunctions::TrimFromSymbols(StringFunctions::GetRestOfLine(&is), StrConst::COMMENT_SYMBOL));
	std::vector<double> params;
	while (!valuesStream.eof())
	{
		double dTemp;
		valuesStream >> dTemp;
		params.push_back(dTemp);
	}
	val = CCorrelation{ static_cast<ECorrelationTypes>(type), params, TInterval, PInterval };
	return is;
}

bool CCorrelation::operator==(const CCorrelation& _c) const
{
	return
		m_nType       == _c.m_nType       &&
		m_TInterval   == _c.m_TInterval   &&
		m_PInterval   == _c.m_PInterval   &&
		m_vParameters == _c.m_vParameters &&
		m_valuesList  == _c.m_valuesList;
}

void CCorrelation::Initialize(ECorrelationTypes _nType, const std::vector<double>& _vParams, const SInterval& _TInterval, const SInterval& _PInterval)
{
	if (!TrySetCorrelation(_nType, _vParams, _TInterval, _PInterval)) // try to set all parameters
	{
		// set default values
		m_nType = ECorrelationTypes::LIST_OF_T_VALUES;
		m_vParameters.clear();
		m_valuesList.Clear();
		m_TInterval = { MDBDescriptors::TEMP_MIN , MDBDescriptors::TEMP_MIN };
		m_PInterval = { MDBDescriptors::PRES_MIN , MDBDescriptors::PRES_MAX };
	}
}

bool CCorrelation::TrySetCorrelation(ECorrelationTypes _nType, const std::vector<double>& _vParams, const SInterval& _TInterval, const SInterval& _PInterval)
{
	SetType(_nType); // set type
	return SetTInterval(_TInterval) && SetPInterval(_PInterval) && SetParameters(_vParams); // try to set other parameters
}

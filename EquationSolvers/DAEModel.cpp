/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DAEModel.h"
#include "ContainerFunctions.h"
#include <cfloat>

CDAEModel::CDAEModel( void )
{
	m_dRTol = DEFAULT_RTOL;
	m_dATol = DEFAULT_ATOL;

	m_pUserData = nullptr;
}

CDAEModel::~CDAEModel( void )
{
}

void CDAEModel::Clear()
{
	m_vVariables.clear();
	m_pUserData = nullptr;
	m_dRTol = DEFAULT_RTOL;
	m_dATol = DEFAULT_ATOL;
	m_vATol.clear();
}

size_t CDAEModel::AddDAEVariable(bool _isDifferentiable, double _variableInit, double _derivativeInit, double _constraint /*= 0.0 */)
{
	m_vVariables.emplace_back(_variableInit, _derivativeInit, _isDifferentiable, _constraint);
	m_vATol.push_back(m_dATol);
	return m_vVariables.size() - 1;
}

std::vector<size_t> CDAEModel::AddDAEVariables(bool _isDifferentiable, const std::vector<double>& _variablesInit, double _derivativesInit, double _constraint)
{
	std::vector<size_t> res;
	if (_variablesInit.empty()) return res;
	for (const auto& v : _variablesInit)
		res.push_back(AddDAEVariable(_isDifferentiable, v, _derivativesInit, _constraint));
	return res;
}

std::vector<size_t> CDAEModel::AddDAEVariables(bool _isDifferentiable, const std::vector<double>& _variablesInit, const std::vector<double>& _derivativesInit, double _constraint)
{
	std::vector<size_t> res;
	if (_variablesInit.empty() || _variablesInit.size() != _derivativesInit.size()) return res;
	for (size_t i = 0; i < _variablesInit.size(); ++i)
		res.push_back(AddDAEVariable(_isDifferentiable, _variablesInit[i], _derivativesInit[i], _constraint));
	return res;
}

size_t CDAEModel::GetVariablesNumber() const
{
	return m_vVariables.size();
}

double CDAEModel::GetVarInitValue(size_t _dIndex)
{
	if( _dIndex < m_vVariables.size() )
		return m_vVariables[_dIndex].dVariableInit;
	return 0;
}

std::vector<double> CDAEModel::GetVarInitValues() const
{
	std::vector<double> res = ReservedVector<double>(GetVariablesNumber());
	for (const auto& v : m_vVariables)
		res.push_back(v.dVariableInit);
	return res;
}

double CDAEModel::GetDerInitValue(size_t _dIndex)
{
	if( _dIndex < m_vVariables.size() )
		return m_vVariables[_dIndex].dDerivativeInit;
	return 0;
}

std::vector<double> CDAEModel::GetDerInitValues() const
{
	std::vector<double> res = ReservedVector<double>(GetVariablesNumber());
	for (const auto& v : m_vVariables)
		res.push_back(v.dDerivativeInit);
	return res;
}

double CDAEModel::GetConstraintValue(size_t _dIndex)
{
	if( _dIndex < m_vVariables.size() )
		return m_vVariables[_dIndex].dConstraint;
	return 0;
}

std::vector<double> CDAEModel::GetConstraintValues() const
{
	std::vector<double> res = ReservedVector<double>(GetVariablesNumber());
	for (const auto& v : m_vVariables)
		res.push_back(v.dConstraint);
	return res;
}

bool CDAEModel::IsConstraintsDefined() const
{
	return std::any_of(m_vVariables.begin(), m_vVariables.end(), [](const auto& v) { return v.dConstraint != 0.0; });
}

void CDAEModel::ClearVariables()
{
	m_vVariables.clear();
	m_vATol.clear();
}

double CDAEModel::GetVarType(size_t _dIndex)
{
	if( _dIndex < m_vVariables.size() )
		return m_vVariables[_dIndex].bIsDifferential ? 1 : 0;
	return 0;
}

std::vector<double> CDAEModel::GetVarTypes() const
{
	std::vector<double> res = ReservedVector<double>(GetVariablesNumber());
	for (const auto& v : m_vVariables)
		res.push_back(v.bIsDifferential ? 1. : 0.);
	return res;
}

void CDAEModel::SetTolerance( double _dRTol, double _dATol )
{
	m_dRTol = _dRTol;
	m_dATol = _dATol;
	for(size_t i=0; i<m_vATol.size(); ++i )
		m_vATol[i] = _dATol;
}

void CDAEModel::SetTolerance( double _dRTol, std::vector<double>& _vATol )
{
	m_dRTol = _dRTol;
	if( _vATol.size() < m_vATol.size() )
		return;
	for(size_t i=0; i<m_vATol.size(); ++i )
		m_vATol[i] = _vATol[i];
}

double CDAEModel::GetRTol()
{
	return m_dRTol;
}

double CDAEModel::GetATol(size_t _dIndex)
{
	if( _dIndex < m_vVariables.size() )
		return m_vATol[ _dIndex ];
	return DEFAULT_ATOL;
}

std::vector<double> CDAEModel::GetATols() const
{
	if (!m_vATol.empty())
		return m_vATol;
	return std::vector<double>(GetVariablesNumber(), DEFAULT_ATOL);
}

void CDAEModel::SetUserData( void* _pUserData )
{
	m_pUserData = _pUserData;
}

void CDAEModel::CalculateResiduals( double _dTime, double* _pVars, double* _pDerivs, double* _pRes, void* _pUserData )
{

}

void CDAEModel::ResultsHandler( double _dTime, double* _pVars, double* _pDerivs, void* _pUserData )
{

}

bool CDAEModel::GetResiduals( double _dTime, double* _pVars, double* _pDerivs, double* _pRes )
{
	CalculateResiduals( _dTime, _pVars, _pDerivs, _pRes, m_pUserData );
	bool bRet = false;
	if( !m_vVariables.empty() )
	{
		if( !m_vVariables.front().bIsDifferential )
		{
			if( ( _pVars[0] <= DBL_MAX ) && ( _pVars[0] >= -DBL_MAX ) )
				bRet = true;
		}
		else
		{
			if( ( _pDerivs[0] <= DBL_MAX ) && ( _pDerivs[0] >= -DBL_MAX ) )
				bRet = true;
		}
	}
	return bRet;
}

void CDAEModel::HandleResults( double _dTime, double* _pVars, double* _pDerivs )
{
	ResultsHandler( _dTime, _pVars, _pDerivs, m_pUserData );
}

/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DAEModel.h"
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

size_t CDAEModel::AddDAEVariables(bool _isDifferentiable, const std::vector<double>& _variablesInit, double _derivativesInit, double _constraint)
{
	if (_variablesInit.empty()) return -1;
	for (const auto& v : _variablesInit)
		AddDAEVariable(_isDifferentiable, v, _derivativesInit, _constraint);
	return m_vVariables.size() - _variablesInit.size();
}

size_t CDAEModel::AddDAEVariables(bool _isDifferentiable, const std::vector<double>& _variablesInit, const std::vector<double>& _derivativesInit, double _constraint)
{
	if (_variablesInit.empty() || _variablesInit.size() != _derivativesInit.size()) return -1;
	for (size_t i = 0; i < _variablesInit.size(); ++i)
		AddDAEVariable(_isDifferentiable, _variablesInit[i], _derivativesInit[i], _constraint);
	return m_vVariables.size() - _variablesInit.size();
}

size_t CDAEModel::GetVariablesNumber()
{
	return m_vVariables.size();
}

double CDAEModel::GetVarInitValue(size_t _dIndex)
{
	if( _dIndex < m_vVariables.size() )
		return m_vVariables[_dIndex].dVariableInit;
	return 0;
}

double CDAEModel::GetDerInitValue(size_t _dIndex)
{
	if( _dIndex < m_vVariables.size() )
		return m_vVariables[_dIndex].dDerivativeInit;
	return 0;
}

double CDAEModel::GetConstraintValue(size_t _dIndex)
{
	if( _dIndex < m_vVariables.size() )
		return m_vVariables[_dIndex].dConstraint;
	return 0;
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

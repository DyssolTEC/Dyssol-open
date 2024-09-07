/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "NLModel.h"
#include <cfloat>

CNLModel::CNLModel()
{
	m_pUserData = nullptr;
}

CNLModel::~CNLModel()
{
}

void CNLModel::Clear()
{
	m_vVariables.clear();
	m_pUserData = NULL;
}

void CNLModel::SetStrategy(ENLSolverStrategy _eStrategy)
{
	m_eStrategy = _eStrategy;
}

size_t CNLModel::AddNLVariable(double _dVariableInit, double _dConstraint /*= 0.0 */, double _dUScale /*= 1.0 */, double _dFScale /*= 1.0 */)
{
	m_vVariables.push_back(SNLVariable( _dVariableInit, _dConstraint, _dUScale, _dFScale) );
	return m_vVariables.size()-1;
}

std::vector<size_t> CNLModel::AddNLVariables(const std::vector<double>& _variablesInit, double _constraint, double _uScale, double _fScale)
{
	std::vector<size_t> res;
	if (_variablesInit.empty()) return res;
	for (const auto& v : _variablesInit)
		res.push_back(AddNLVariable(v, _constraint, _uScale, _fScale));
	return res;
}

size_t CNLModel::GetVariablesNumber() const
{
	return m_vVariables.size();
}

double CNLModel::GetVarInitValue(size_t _dIndex)
{
	if( _dIndex < m_vVariables.size() )
		return m_vVariables[_dIndex].dVariableInit;
	return 0;
}

double CNLModel::GetConstraintValue(size_t _dIndex)
{
	if( _dIndex < m_vVariables.size() )
		return m_vVariables[_dIndex].dConstraint;
	return 0;
}

double CNLModel::GetUScaleValue(size_t _dIndex)
{
	if (_dIndex < m_vVariables.size())
		return m_vVariables[_dIndex].dUScale;
	return 0;
}

double CNLModel::GetFScaleValue(size_t _dIndex)
{
	if (_dIndex < m_vVariables.size())
		return m_vVariables[_dIndex].dFScale;
	return 0;
}

void CNLModel::ClearVariables()
{
	m_vVariables.clear();
}

void CNLModel::SetUserData( void* _pUserData )
{
	m_pUserData = _pUserData;
}

void CNLModel::CalculateFunctions(double* _pVars, double* _pFunc, void* _pUserData)
{

}

void CNLModel::ResultsHandler(double _dTime, double* _pVars, void* _pUserData)
{

}

bool CNLModel::GetFunctions(double* _pVars, double* _pFunc)
{
	CalculateFunctions(_pVars, _pFunc, m_pUserData );
	bool bRet = false;
	if( !m_vVariables.empty() )
	{
		if( ( _pVars[0] <= DBL_MAX ) && ( _pVars[0] >= -DBL_MAX ) )
				bRet = true;

		//if (m_eStrategy == ENLSolverStrategy::Newton || m_eStrategy == ENLSolverStrategy::Linesearch)
		//{
		//	for (size_t i = 0; i < m_vVariables.size(); ++i)
		//		_pFunc[i] = _pFunc[i] - _pVars[i];
		//}
	}
	return bRet;
}

void CNLModel::HandleResults( double _dTime, double* _pVars )
{
	ResultsHandler( _dTime, _pVars, m_pUserData );
}

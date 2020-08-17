/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "NLSolver.h"
#include <kinsol/kinsol.h>
#include <kinsol/kinsol_ls_impl.h>
#include <sunmatrix/sunmatrix_dense.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <cstring>

#ifdef _MSC_VER
#else
// To get rid of ErrorHandler() function, which must take char*  as a parameter.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

CNLSolver::CNLSolver():
	m_pModel(nullptr),
	m_pKINmem(nullptr),
	m_vectorVars(nullptr),
	m_vectorUScales(nullptr),
	m_vectorFScales(nullptr),
	m_sErrorDescription(""),
	m_StoreVectorVars(nullptr),
	m_nMaxIter(200)
{
}

CNLSolver::~CNLSolver()
{
	ClearMemory();
}

void CNLSolver::SetSolverMaxIter(size_t _nMaxIter)
{
	m_nMaxIter = _nMaxIter;
}

size_t CNLSolver::GetSolverMaxIter()
{
	return m_nMaxIter;
}

unsigned CNLSolver::GetSolverIter()
{
	long int nIter = 0;
	if (KINGetNumNonlinSolvIters(m_pKINmem, &nIter) != KIN_SUCCESS)
		return 0;
	else
		return unsigned(nIter);
}

bool CNLSolver::SetModel( CNLModel* _pModel , unsigned _nMAA)
{
	ClearMemory();

	m_pModel = _pModel;

	// Create IDA memory
	m_pKINmem = KINCreate();
	if (m_pKINmem == nullptr)
	{
		ErrorHandler(-1, "KIN", "KINCreate", "Cannot allocate memory for solver.", &m_sErrorDescription);
		return false;
	}
	if (KINSetErrHandlerFn(m_pKINmem, &CNLSolver::ErrorHandler, &m_sErrorDescription) != KIN_SUCCESS)
	{
		ErrorHandler(-1, "KIN", "KINSetErrHandlerFn", "Cannot setup error handler function.", &m_sErrorDescription);
		return false;
	}

	const sunindextype nVarsCnt = static_cast<sunindextype>(m_pModel->GetVariablesNumber());

	// Allocate N-vectors
	m_vectorVars    = N_VNew_Serial(nVarsCnt);
	m_vectorUScales = N_VNew_Serial(nVarsCnt);
	m_vectorFScales = N_VNew_Serial(nVarsCnt);


	if (!m_vectorVars || !m_vectorUScales || !m_vectorFScales)
	{
		ErrorHandler(-1, "KIN", "N_VNew_Serial", "Cannot allocate memory for solver.", &m_sErrorDescription);
		return false;
	}

	// Create and initialize variables, uscales and fscales
	for (size_t i = 0; i < static_cast<size_t>(nVarsCnt); ++i)
	{
		NV_DATA_S(m_vectorVars)[i]    = m_pModel->GetVarInitValue(i);	// values of variables
		NV_DATA_S(m_vectorUScales)[i] = m_pModel->GetUScaleValue(i);	// values of uscales
		NV_DATA_S(m_vectorFScales)[i] = m_pModel->GetFScaleValue(i);	// values of fscales
	}

	// Set constraints
	N_Vector vConstrVars = N_VNew_Serial(nVarsCnt);
	bool bAllZero = true;
	for (size_t i = 0; i < static_cast<size_t>(nVarsCnt); ++i)
	{
		NV_DATA_S(vConstrVars)[i] = m_pModel->GetConstraintValue(i);
		if (NV_DATA_S(vConstrVars)[i] != 0)
			bAllZero = false;
	}
	if (!bAllZero)
		if (KINSetConstraints(m_pKINmem, vConstrVars) != KIN_SUCCESS)
		{
			N_VDestroy_Serial(vConstrVars);
			return false;
		}
	N_VDestroy_Serial(vConstrVars);

	// Set solver constants
	KINSetNumMaxIters(m_pKINmem, static_cast<long>(m_nMaxIter));

	// Initialize IDA memory
	KINSetMAA(m_pKINmem, _nMAA);
	if (KINInit(m_pKINmem, &CNLSolver::ResidualFunction, m_vectorVars) != KIN_SUCCESS)
		return false;

	// Set model as user data
	if( KINSetUserData( m_pKINmem, m_pModel) != KIN_SUCCESS)
		return false;

	/* Create dense SUNMatrix for use in linear solves */
	SUNMatrix A = SUNDenseMatrix(nVarsCnt, nVarsCnt);
	/* Create dense SUNMatrix for use in linear solves */
	SUNLinearSolver LS = SUNDenseLinearSolver(m_vectorVars, A);
	/* Attach the matrix and linear solver */
	if (KINSetLinearSolver(m_pKINmem, LS, A) != KINLS_SUCCESS)
		return false;



	SaveState();

	return true;
}

bool CNLSolver::Calculate(realtype _dTime, unsigned _nModel)
{
	/* Solver strategies:
	*	KIN_NONE: Basic Newton iteration
	*	KIN_LINESEARCH: Newton with globalization
	*	KIN_PF: Fixed-point iteration with Anderson Acceleration (no linear solver needed)
	*	KIN_PICARD: Picard iteration with Anderson Acceleration (uses linear solver) */
	if (_nModel > 3)
		_nModel = KIN_FP;
	const int ret = KINSol(m_pKINmem, m_vectorVars, _nModel, m_vectorUScales, m_vectorFScales);


	if (ret == KIN_SUCCESS || ret == KIN_INITIAL_GUESS_OK || ret ==  KIN_STEP_LT_STPTOL)
		m_pModel->HandleResults(_dTime, NV_DATA_S(m_vectorVars));
	else
		return false;

	return true;
}

void CNLSolver::SaveState()
{
	CopyNVector(m_StoreVectorVars, m_vectorVars);
}

void CNLSolver::LoadState()
{
	CopyNVector(m_vectorVars, m_StoreVectorVars);
}

std::string CNLSolver::GetError() const
{
	return m_sErrorDescription;
}

int CNLSolver::ResidualFunction(N_Vector _value, N_Vector _func, void *_pModel)
{
	realtype *pValue = NV_DATA_S( _value );
	realtype *pFunc = NV_DATA_S(_func);

	const bool bRes = static_cast<CNLModel*>(_pModel)->GetFunctions(pValue, pFunc);

	return bRes ? 0 : -1;
}

void CNLSolver::ClearMemory()
{
	m_sErrorDescription.clear();
	// free memory associates with the KINDls system solver interface.
	auto* originMem = static_cast<KINMemRec*>(m_pKINmem);
	if (originMem)
		SUNLinSolFree(((KINLsMemRec*)originMem->kin_lmem)->LS);

	// free KIN memory
	if (m_pKINmem)			{ KINFree(&m_pKINmem);					m_pKINmem = nullptr; }

	// free vectors
	if (m_vectorVars) { N_VDestroy_Serial(m_vectorVars);		m_vectorVars = nullptr; }
	if (m_vectorUScales) { N_VDestroy_Serial(m_vectorUScales);	m_vectorUScales = nullptr; }
	if (m_vectorFScales) { N_VDestroy_Serial(m_vectorFScales);	m_vectorFScales = nullptr; }

	if (m_StoreVectorVars) { N_VDestroy_Serial(m_StoreVectorVars);			m_StoreVectorVars = nullptr; }
}

void CNLSolver::CopyNVector(N_Vector _dst, N_Vector _src)
{
	if (_dst == nullptr || _src == nullptr)	return;
	std::memcpy(NV_DATA_S(_dst), NV_DATA_S(_src), sizeof(realtype)*static_cast<size_t>(NV_LENGTH_S(_src)));
}

void CNLSolver::ErrorHandler(int _nErrorCode, const char *_pModule, const char *_pFunction, char *_pMsg, void *_sOutString)
{
	if( _sOutString == nullptr ) return;

	if( _nErrorCode < 0 )
	{
		std::string sDescr = "[";
		sDescr += _pModule;
		sDescr += " ERROR] in ";
		sDescr += _pFunction;
		sDescr += ": ";
		sDescr += _pMsg;
		*static_cast<std::string*>(_sOutString) = sDescr;
	}
}

#ifdef _MSC_VER
#else
#pragma GCC diagnostic pop
#endif

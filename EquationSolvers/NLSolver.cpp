/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "NLSolver.h"
#include <kinsol/kinsol.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <nvector/nvector_serial.h>
#include "DyssolUtilities.h"
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
	m_eStrategy(ENLSolverStrategy::Newton),
	m_nMaxIter(200),
	m_nMaxSet(10),
	m_nMaxSubSet(5),
	m_nMAA(0),
	m_dDampingAA(1.0)
{
}

CNLSolver::CNLSolver(ENLSolverStrategy _eStrategy):
	m_pModel(nullptr),
	m_pKINmem(nullptr),
	m_vectorVars(nullptr),
	m_vectorUScales(nullptr),
	m_vectorFScales(nullptr),
	m_sErrorDescription(""),
	m_StoreVectorVars(nullptr),
	m_eStrategy(_eStrategy),
	m_nMaxIter(200),
	m_nMaxSet(10),
	m_nMaxSubSet(5),
	m_nMAA(0),
	m_dDampingAA(1.0)
{
}

CNLSolver::~CNLSolver()
{
	ClearMemory();
}

void CNLSolver::SetStrategy(ENLSolverStrategy _eStrategy)
{
	m_eStrategy = _eStrategy;
}

ENLSolverStrategy CNLSolver::GetStrategy()
{
	return m_eStrategy;
}

void CNLSolver::SetMaxIter(size_t _nMaxIter)
{
	m_nMaxIter = static_cast<long>(_nMaxIter);
}

bool CNLSolver::SetNewtonSolverParameters(unsigned _nMaxSet, unsigned _nMaxSubSet)
{
	if (m_eStrategy == ENLSolverStrategy::Newton || m_eStrategy == ENLSolverStrategy::Linesearch)
	{
		m_nMaxSet = static_cast<long>(_nMaxSet);
		m_nMaxSubSet = static_cast<long>(_nMaxSubSet);
		return true;
	}
	else
		return false;
}

bool CNLSolver::SetFixedPointSolverParameters(size_t _nMAA, double _dDampingAA, double _dDamping)
{
	if (m_eStrategy == ENLSolverStrategy::Fixedpoint || m_eStrategy == ENLSolverStrategy::Picard)
	{
		m_nMAA = static_cast<long>(_nMAA);
		m_dDampingAA = _dDampingAA;
		m_dDamping = _dDamping;
		return true;
	}
	else
		return false;
}

size_t CNLSolver::GetSolverMaxIter() const
{
	return static_cast<size_t>(m_nMaxIter);
}

void CNLSolver::SetSolverMaxIter(size_t _nMaxIter)
{
	m_nMaxIter = static_cast<long>(_nMaxIter);
	if (m_pKINmem)
		KINSetNumMaxIters(m_pKINmem, m_nMaxIter);
}

unsigned CNLSolver::GetSolverIter()
{
	long int nIter = 0;
	if (KINGetNumNonlinSolvIters(m_pKINmem, &nIter) != KIN_SUCCESS)
		return 0;
	else
		return unsigned(nIter);
}

bool CNLSolver::SetModel(CNLModel* _pModel)
{
	ClearMemory();

	// create context
#if SUNDIALS_VERSION_MAJOR == 6
	SUNErrCode res = SUNContext_Create(nullptr, &m_sunctx);
#else
	SUNErrCode res = SUNContext_Create(SUN_COMM_NULL, &m_sunctx);
#endif
#if SUNDIALS_VERSION_MAJOR >= 6
	if (!res)
	{
		ErrorHandler(-1, "KIN", "SUNContext_Create", "Cannot create SUNDIALS context.", &m_sErrorDescription);
		return false;
	}
#endif

	m_pModel = _pModel;

	// Create IDA memory
#if SUNDIALS_VERSION_MAJOR < 6
	m_pKINmem = KINCreate();
#else
	m_pKINmem = KINCreate(m_sunctx);
#endif
	if (m_pKINmem == nullptr)
	{
		ErrorHandler(-1, "KIN", "KINCreate", "Cannot allocate memory for solver.", &m_sErrorDescription);
		return false;
	}
	// set error handler function
#if SUNDIALS_VERSION_MAJOR < 7
	res = KINSetErrHandlerFn(m_pKINmem, &CNLSolver::ErrorHandler, &m_sErrorDescription));
	if (res != KIN_SUCCESS)
	{
		ErrorHandler(-1, "KIN", "KINSetErrHandlerFn", "Cannot setup error handler function.", &m_sErrorDescription);
		return false;
	}
#else
	res = SUNContext_PushErrHandler(m_sunctx, &CNLSolver::ErrorHandler, &m_sErrorDescription);
	if (res != KIN_SUCCESS)
	{
		ErrorHandler(-1, "KIN", "SUNContext_PushErrHandler", "Cannot setup error handler function.", &m_sErrorDescription);
		return false;
	}
#endif

	const sunindextype nVarsCnt = static_cast<sunindextype>(m_pModel->GetVariablesNumber());
	m_pModel->SetStrategy(m_eStrategy);

	// Allocate N-vectors
#if SUNDIALS_VERSION_MAJOR < 6
	m_vectorVars    = N_VNew_Serial(nVarsCnt);
	m_vectorUScales = N_VNew_Serial(nVarsCnt);
	m_vectorFScales = N_VNew_Serial(nVarsCnt);
#else
	m_vectorVars    = N_VNew_Serial(nVarsCnt, m_sunctx);
	m_vectorUScales = N_VNew_Serial(nVarsCnt, m_sunctx);
	m_vectorFScales = N_VNew_Serial(nVarsCnt, m_sunctx);
#endif


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
#if SUNDIALS_VERSION_MAJOR < 6
	N_Vector vConstrVars = N_VNew_Serial(nVarsCnt);
#else
	N_Vector vConstrVars = N_VNew_Serial(nVarsCnt, m_sunctx);
#endif
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

	// Set solver parameters
	// Max. number of nonlinear iterations
	KINSetNumMaxIters(m_pKINmem, m_nMaxIter);
	// Newton method settings
	if (m_eStrategy == ENLSolverStrategy::Newton || m_eStrategy == ENLSolverStrategy::Linesearch)
	{
		KINSetMaxSetupCalls(m_pKINmem, m_nMaxSet);
		KINSetMaxSubSetupCalls(m_pKINmem, m_nMaxSubSet);
	}
	// Fixed point method settings
	else
	{
		KINSetMAA(m_pKINmem, m_nMAA);
		KINSetDampingAA(m_pKINmem, static_cast<sun_real>(m_dDampingAA));
		KINSetDamping(m_pKINmem, static_cast<sun_real>(m_dDamping));
	}

	// Initialize IDA memory
	if (KINInit(m_pKINmem, &CNLSolver::ResidualFunction, m_vectorVars) != KIN_SUCCESS)
		return false;

	// Set model as user data
	if( KINSetUserData( m_pKINmem, m_pModel) != KIN_SUCCESS)
		return false;

	/* Create dense SUNMatrix for use in linear solves */
#if SUNDIALS_VERSION_MAJOR < 6
	m_A = SUNDenseMatrix(nVarsCnt, nVarsCnt);
#else
	m_A = SUNDenseMatrix(nVarsCnt, nVarsCnt, m_sunctx);
#endif
	/* Create dense SUNMatrix for use in linear solves */
#if SUNDIALS_VERSION_MAJOR < 6
	m_LS = SUNLinSol_Dense(m_vectorVars, m_A);
#else
	m_LS = SUNLinSol_Dense(m_vectorVars, m_A, m_sunctx);
#endif
	/* Attach the matrix and linear solver */
	if (KINSetLinearSolver(m_pKINmem, m_LS, m_A) != KINLS_SUCCESS)
		return false;

	SaveState();

	return true;
}

bool CNLSolver::Calculate(sun_real _dTime)
{
	const int ret = KINSol(m_pKINmem, m_vectorVars, (int)E2I(m_eStrategy), m_vectorUScales, m_vectorFScales);

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
	sun_real *pValue = NV_DATA_S(_value);
	sun_real *pFunc = NV_DATA_S(_func);

	const bool bRes = static_cast<CNLModel*>(_pModel)->GetFunctions(pValue, pFunc);

	return bRes ? 0 : -1;
}

void CNLSolver::ClearMemory()
{
	m_sErrorDescription.clear();
	// free memory associates with the KINDls system solver interface.
	SUNMatDestroy(m_A);
	SUNLinSolFree(m_LS);

	// free KIN memory
	if (m_pKINmem)			{ KINFree(&m_pKINmem);					m_pKINmem = nullptr; }

	// free vectors
	if (m_vectorVars) { N_VDestroy_Serial(m_vectorVars);		m_vectorVars = nullptr; }
	if (m_vectorUScales) { N_VDestroy_Serial(m_vectorUScales);	m_vectorUScales = nullptr; }
	if (m_vectorFScales) { N_VDestroy_Serial(m_vectorFScales);	m_vectorFScales = nullptr; }

	if (m_StoreVectorVars) { N_VDestroy_Serial(m_StoreVectorVars);			m_StoreVectorVars = nullptr; }

#if SUNDIALS_VERSION_MAJOR < 6
#else
	// free context
	SUNContext_Free(&m_sunctx);
#endif
}

void CNLSolver::CopyNVector(N_Vector _dst, N_Vector _src)
{
	if (_dst == nullptr || _src == nullptr)	return;
	std::memcpy(NV_DATA_S(_dst), NV_DATA_S(_src), sizeof(sun_real)*static_cast<size_t>(NV_LENGTH_S(_src)));
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

void CNLSolver::ErrorHandler(int _line, const char* _function, const char* _file, const char* _message, SUNErrCode _errCode, void* _outString, SUNContext _sunctx)
{
	if (!_outString) return;
	if (!_errCode) return;
	std::string description = "[";
	description += _file;
	description += " ERROR] in ";
	description += _function;
	description += ": ";
	description += _message;
	*static_cast<std::string*>(_outString) = description;
}

#ifdef _MSC_VER
#else
#pragma GCC diagnostic pop
#endif

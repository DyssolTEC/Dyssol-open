/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2024, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "NLSolver.h"
#include "DyssolUtilities.h"
PRAGMA_WARNING_PUSH
PRAGMA_WARNING_DISABLE
#include <kinsol/kinsol.h>
#if SUNDIALS_VERSION_MAJOR <= 3
#include <kinsol/kinsol_direct.h>
#endif
#if SUNDIALS_VERSION_MAJOR > 2
#include <sunlinsol/sunlinsol_dense.h>
#endif
PRAGMA_WARNING_POP

// Macros for convenient adding context to functions depending on the sundials version
#if SUNDIALS_VERSION_MAJOR >= 6
#define MAYBE_CONTEXT(ctx) m_sunctx
#define MAYBE_COMMA_CONTEXT(ctx) ,m_sunctx
#else
#define MAYBE_CONTEXT(ctx)
#define MAYBE_COMMA_CONTEXT(ctx)
#endif

CNLSolver::CNLSolver():
	m_pModel(nullptr),
	m_pKINmem(nullptr),
	m_vectorVars(nullptr),
	m_vectorUScales(nullptr),
	m_vectorFScales(nullptr),
	m_errorMessage(""),
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
	m_errorMessage(""),
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

	int res; // return value

	// create context
#if SUNDIALS_VERSION_MAJOR == 6
	res = SUNContext_Create(nullptr, &m_sunctx);
#elif SUNDIALS_VERSION_MAJOR == 7
	SUNErrCode res = SUNContext_Create(SUN_COMM_NULL, &m_sunctx);
#endif
#if SUNDIALS_VERSION_MAJOR >= 6
	if (!res)
		return WriteError("KIN", "SUNContext_Create", "Cannot create SUNDIALS context.");
#endif

	m_pModel = _pModel;

	// Create IDA memory
	m_pKINmem = KINCreate(MAYBE_CONTEXT(m_sunctx));
	if (m_pKINmem == nullptr)
		return WriteError("KIN", "KINCreate", "Cannot allocate memory for solver.");
	// set error handler function
#if SUNDIALS_VERSION_MAJOR < 7
	res = KINSetErrHandlerFn(m_pKINmem, &CNLSolver::ErrorHandler, &m_errorMessage);
	if (res != KIN_SUCCESS)
		return WriteError("KIN", "KINSetErrHandlerFn", "Cannot setup error handler function.");
#else
	res = SUNContext_PushErrHandler(m_sunctx, &CNLSolver::ErrorHandler, &m_errorMessage);
	if (res != KIN_SUCCESS)
		return WriteError("KIN", "SUNContext_PushErrHandler", "Cannot setup error handler function.");
#endif

	const auto nVarsCnt = m_pModel->GetVariablesNumber();
	m_pModel->SetStrategy(m_eStrategy);

	// Allocate N-vectors
	m_vectorVars    = N_VNew_Serial(nVarsCnt MAYBE_COMMA_CONTEXT(m_sunctx));
	m_vectorUScales = N_VNew_Serial(nVarsCnt MAYBE_COMMA_CONTEXT(m_sunctx));
	m_vectorFScales = N_VNew_Serial(nVarsCnt MAYBE_COMMA_CONTEXT(m_sunctx));


	if (!m_vectorVars || !m_vectorUScales || !m_vectorFScales)
		return WriteError("KIN", "N_VNew_Serial", "Cannot allocate memory for solver.");

	// Create and initialize variables, uscales and fscales
	for (size_t i = 0; i < static_cast<size_t>(nVarsCnt); ++i)
	{
		NV_DATA_S(m_vectorVars)[i]    = m_pModel->GetVarInitValue(i);	// values of variables
		NV_DATA_S(m_vectorUScales)[i] = m_pModel->GetUScaleValue(i);	// values of uscales
		NV_DATA_S(m_vectorFScales)[i] = m_pModel->GetFScaleValue(i);	// values of fscales
	}

	// Set constraints
	N_Vector vConstrVars = N_VNew_Serial(nVarsCnt MAYBE_COMMA_CONTEXT(m_sunctx));
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
#if SUNDIALS_VERSION_MAJOR >= 5
		KINSetDampingAA(m_pKINmem, m_dDampingAA);
		KINSetDamping(m_pKINmem, m_dDamping);
#endif
	}

	// Initialize IDA memory
	if (KINInit(m_pKINmem, &CNLSolver::ResidualFunction, m_vectorVars) != KIN_SUCCESS)
		return false;

	// Set model as user data
	if( KINSetUserData( m_pKINmem, m_pModel) != KIN_SUCCESS)
		return false;

#if SUNDIALS_VERSION_MAJOR > 2
	/* Create dense SUNMatrix for use in linear solves */
	m_A = SUNDenseMatrix(nVarsCnt, nVarsCnt MAYBE_COMMA_CONTEXT(m_sunctx));
#endif
#if SUNDIALS_VERSION_MAJOR == 3
	/* Create dense SUNMatrix for use in linear solves */
	m_LS = SUNDenseLinearSolver(m_vectorVars, m_A);
	/* Attach the matrix and linear solver */
	if (KINDlsSetLinearSolver(m_pKINmem, m_LS, m_A) != KINDLS_SUCCESS)
		return false;
#elif SUNDIALS_VERSION_MAJOR > 3
	/* Create dense SUNMatrix for use in linear solves */
	m_LS = SUNLinSol_Dense(m_vectorVars, m_A MAYBE_COMMA_CONTEXT(m_sunctx));
	/* Attach the matrix and linear solver */
	if (KINSetLinearSolver(m_pKINmem, m_LS, m_A) != KINLS_SUCCESS)
		return false;
#endif

	SaveState();

	return true;
}

bool CNLSolver::Calculate(double _dTime)
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
	return m_errorMessage;
}

int CNLSolver::ResidualFunction(N_Vector _value, N_Vector _func, void *_pModel)
{
	double* pValue = NV_DATA_S(_value);
	double* pFunc = NV_DATA_S(_func);

	const bool bRes = static_cast<CNLModel*>(_pModel)->GetFunctions(pValue, pFunc);

	return bRes ? 0 : -1;
}

void CNLSolver::ClearMemory()
{
	m_errorMessage.clear();

#if SUNDIALS_VERSION_MAJOR > 2
	// free memory associates with the KINDls system solver interface.
	SUNMatDestroy(m_A);
	SUNLinSolFree(m_LS);
#endif

	// free KIN memory
	if (m_pKINmem)			{ KINFree(&m_pKINmem);					m_pKINmem = nullptr; }

	// free vectors
	if (m_vectorVars) { N_VDestroy_Serial(m_vectorVars);		m_vectorVars = nullptr; }
	if (m_vectorUScales) { N_VDestroy_Serial(m_vectorUScales);	m_vectorUScales = nullptr; }
	if (m_vectorFScales) { N_VDestroy_Serial(m_vectorFScales);	m_vectorFScales = nullptr; }

	if (m_StoreVectorVars) { N_VDestroy_Serial(m_StoreVectorVars);			m_StoreVectorVars = nullptr; }

#if SUNDIALS_VERSION_MAJOR >= 6
	// free context
	SUNContext_Free(&m_sunctx);
#endif
}

void CNLSolver::CopyNVector(N_Vector _dst, N_Vector _src)
{
	if (_dst == nullptr || _src == nullptr)	return;
	std::memcpy(NV_DATA_S(_dst), NV_DATA_S(_src), sizeof(double)*static_cast<size_t>(NV_LENGTH_S(_src)));
}

#if SUNDIALS_VERSION_MAJOR < 7
void CNLSolver::ErrorHandler(int _errorCode, const char *_module, const char *_function, char *_message, void *_outString)
{
	if (!_outString) return;
	if (_errorCode >= 0) return;
	std::string& out = *static_cast<std::string*>(_outString);
	AppendMessage(_module, _function, _message, out);
}
#else
void CNLSolver::ErrorHandler(int _line, const char* _function, const char* _file, const char* _message, SUNErrCode _errCode, void* _outString, SUNContext _sunctx)
{
	if (!_outString) return;
	if (!_errCode) return;
	std::string& out = *static_cast<std::string*>(_outString);
	AppendMessage(_file, _function, _message, out);
}
#endif

std::string CNLSolver::BuildErrorMessage(const std::string& _module, const std::string& _function, const std::string& _message)
{
	return "[" + _module + " ERROR] in " + _function + ": " + _message;
}

void CNLSolver::AppendMessage(const std::string& _module, const std::string& _function, const std::string& _message, std::string& _out)
{
	if (!_out.empty())
		_out += "\n";
	_out += BuildErrorMessage(_module, _function, _message);
}

bool CNLSolver::WriteError(const std::string& _module, const std::string& _function, const std::string& _message)
{
	AppendMessage(_module, _function, _message, m_errorMessage);
	return false;
}

/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DAESolver.h"
#if defined(_MSC_VER)
__pragma(warning(push))
__pragma(warning(disable : 4305 4309))
#endif
#include <ida/ida.h>
#include <ida/ida_impl.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <nvector/nvector_serial.h>
#if defined(_MSC_VER)
__pragma(warning(pop))
#endif
#include <cstring>

#ifdef _MSC_VER
#else
// To get rid of ErrorHandler() function, which must take char*  as a parameter.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

// Macros for convenient adding context to functions depending on the sundials version
#if SUNDIALS_VERSION_MAJOR >= 6
#define MAYBE_CONTEXT(mem) mem.sunctx
#define MAYBE_COMMA_CONTEXT(mem) ,mem.sunctx
#else
#define MAYBE_CONTEXT(mem)
#define MAYBE_COMMA_CONTEXT(mem)
#endif

CDAESolver::~CDAESolver()
{
	Clear();
}

bool CDAESolver::SetModel(CDAEModel* _model)
{
	Clear();
	m_model = _model;

	if (!InitSolverMemory(m_solverMem))
	{
		ClearSolverMemory(m_solverMem);
		return false;
	}
	InitStoreMemory(m_solverMem_store);

	SaveState();
	return true;
}

bool CDAESolver::Calculate(sun_real _time)
{
	if (_time == 0.0)
	{
		const bool success = CalculateInitialConditions();
		if (!success)
			return false;
	}
	else
	{
		const int res = IDASolve(m_solverMem.idamem, _time, &m_timeLast, m_solverMem.vars, m_solverMem.ders, IDA_NORMAL);
		if (res < 0)
			return WriteError("IDA", "IDASolve", "Cannot integrate.");
		m_model->HandleResults(m_timeLast, N_VGetArrayPointer(m_solverMem.vars), N_VGetArrayPointer(m_solverMem.ders));
	}

	return true;
}

bool CDAESolver::Calculate(sun_real _timeBeg, sun_real _timeEnd)
{
	int res;

	if (_timeBeg == _timeEnd)
		return WriteError("IDA", "Calculate", "Start and end time points are equal. Cannot perform calculations for dynamic model.");

	if (_timeBeg == 0.0)
	{
		const bool success = CalculateInitialConditions();
		if (!success)
			return false;
	}

	const auto interval = _timeEnd - _timeBeg; // current calculation time interval
	auto allowedStep = interval / 2.0;         // allowed integration step - half of the time interval
	if (m_maxStep != 0.0 && allowedStep > m_maxStep)
		allowedStep = m_maxStep;

	res = IDASetMaxStep(m_solverMem.idamem, allowedStep);
	if (res != IDA_SUCCESS)
		return WriteError("IDA", "IDASetMaxStep", "Cannot set maximum absolute step size");

	/* Get current integration step.*/
	sun_real currStep;
	res = IDAGetCurrentStep(m_solverMem.idamem, &currStep);
	if (res != IDA_SUCCESS)
		return WriteError("IDA", "IDAGetCurrentStep", "Cannot read current time step.");

	/* The value set by IDASetMaxStep will be applied only *after* next call to IDASolve.
	 * Therefore, if the previous time window was larger then the current one,
	 * the next call of IDASolve may use large step and jump over the whole interval.
	 * To ensure that at least one time point is generated inside the interval,
	 * we solve it with two calls to IDASolve, with half of a time interval each. */
	if (currStep < interval)
	{
		if (!IntegrateUntil(_timeEnd))
			return false;
	}
	else
	{
		if (!IntegrateUntil(_timeBeg + allowedStep))
			return false;
		if (!IntegrateUntil(_timeEnd))
			return false;
	}

	return true;
}

bool CDAESolver::CalculateInitialConditions()
{
	int res = IDACalcIC(m_solverMem.idamem, IDA_YA_YDP_INIT, 0.001);
	if (res != IDA_SUCCESS)
		return WriteError("IDA", "IDACalcIC", "Cannot calculate initial conditions.");

	const N_Vector consistVars = N_VNew_Serial(static_cast<sunindextype>(m_model->GetVariablesNumber()) MAYBE_COMMA_CONTEXT(m_solverMem));
	const N_Vector consistDers = N_VNew_Serial(static_cast<sunindextype>(m_model->GetVariablesNumber()) MAYBE_COMMA_CONTEXT(m_solverMem));

	res = IDAGetConsistentIC(m_solverMem.idamem, consistVars, consistDers);
	if (res != IDA_SUCCESS)
		return WriteError("IDA", "IDAGetConsistentIC", "Cannot obtain consistent initial conditions.");
	m_model->HandleResults(0.0, N_VGetArrayPointer(consistVars), N_VGetArrayPointer(consistDers));

	N_VDestroy_Serial(consistVars);
	N_VDestroy_Serial(consistDers);

	return true;
}

bool CDAESolver::IntegrateUntil(sun_real _time)
{
	/* set integration limit */
	int res = IDASetStopTime(m_solverMem.idamem, _time);
	if (res != IDA_SUCCESS)
		return WriteError("IDA", "IDASetStopTime", "Cannot set integration stop time");
	/* integrate */
	do
	{
		/* _time here is not a stop criterion, it only gives the direction of integration. */
		res = IDASolve(m_solverMem.idamem, _time, &m_timeLast, m_solverMem.vars, m_solverMem.ders, IDA_ONE_STEP);
		if (res < 0)
			return WriteError("IDA", "IDASolve", "Cannot integrate.");
		m_model->HandleResults(m_timeLast, N_VGetArrayPointer(m_solverMem.vars), N_VGetArrayPointer(m_solverMem.ders));
	} while (res != IDA_TSTOP_RETURN);
	return true;
}

void CDAESolver::SaveState()
{
	if (!m_model) return;
	if (!m_solverMem.idamem) return;

	const size_t len = m_model->GetVariablesNumber();
	const auto* src = static_cast<IDAMem>(m_solverMem.idamem);

	std::memcpy(m_solverMem_store.vars.data(), N_VGetArrayPointer(m_solverMem.vars), sizeof(sun_real) * len);
	std::memcpy(m_solverMem_store.ders.data(), N_VGetArrayPointer(m_solverMem.ders), sizeof(sun_real) * len);

	for (size_t i = 0; i < MXORDP1; ++i)
		std::memcpy(m_solverMem_store.ida_phi[i].data(), N_VGetArrayPointer(src->ida_phi[i]), sizeof(sun_real) * len);
	std::memcpy(m_solverMem_store.ida_psi.data(), src->ida_psi, sizeof(sun_real) * MXORDP1);
	m_solverMem_store.ida_kused = src->ida_kused;
	m_solverMem_store.ida_ns    = src->ida_ns;
	m_solverMem_store.ida_hh    = src->ida_hh;
	m_solverMem_store.ida_tn    = src->ida_tn;
	m_solverMem_store.ida_cj    = src->ida_cj;
	m_solverMem_store.ida_nst   = src->ida_nst;
}

void CDAESolver::LoadState() const
{
	if (!m_model) return;

	const size_t len = m_model->GetVariablesNumber();
	auto* dst = static_cast<IDAMem>(m_solverMem.idamem);

	std::memcpy(N_VGetArrayPointer(m_solverMem.vars), m_solverMem_store.vars.data(), sizeof(sun_real) * len);
	std::memcpy(N_VGetArrayPointer(m_solverMem.ders), m_solverMem_store.ders.data(), sizeof(sun_real) * len);

	for (size_t i = 0; i < MXORDP1; ++i)
		std::memcpy(N_VGetArrayPointer(dst->ida_phi[i]), m_solverMem_store.ida_phi[i].data(), sizeof(sun_real) * len);
	std::memcpy(dst->ida_psi, m_solverMem_store.ida_psi.data(), sizeof(sun_real) * MXORDP1);
	dst->ida_kused = m_solverMem_store.ida_kused;
	dst->ida_ns    = m_solverMem_store.ida_ns;
	dst->ida_hh    = m_solverMem_store.ida_hh;
	dst->ida_tn    = m_solverMem_store.ida_tn;
	dst->ida_cj    = m_solverMem_store.ida_cj;
	dst->ida_nst   = m_solverMem_store.ida_nst;
}

std::string CDAESolver::GetError() const
{
	return m_errorMessage;
}

size_t CDAESolver::GetSolverMaxNumSteps() const
{
	return m_maxNumSteps;
}

void CDAESolver::SetSolverMaxNumSteps(size_t _num)
{
	m_maxNumSteps = _num;
}

double CDAESolver::GetMaxStep() const
{
	return m_maxStep;
}

void CDAESolver::SetMaxStep(double _step)
{
	m_maxStep = _step;
}

bool CDAESolver::InitSolverMemory(SSolverMemory& _mem)
{
	int res; // return value

	// create context
#if SUNDIALS_VERSION_MAJOR == 6
	res = SUNContext_Create(nullptr, &_mem.sunctx);
#else
	res = SUNContext_Create(SUN_COMM_NULL, &_mem.sunctx);
#endif
#if SUNDIALS_VERSION_MAJOR >= 6
	if (res != IDA_SUCCESS)
		return WriteError("IDA", "SUNContext_Create", "Cannot create SUNDIALS context.");
#endif

	const auto len = static_cast<sunindextype>(m_model->GetVariablesNumber());

	// allocate vectors for y, y', tolerances, types, constraints
	_mem.vars   = N_VNew_Serial(len MAYBE_COMMA_CONTEXT(m_solverMem));
	_mem.ders   = N_VNew_Serial(len MAYBE_COMMA_CONTEXT(m_solverMem));
	_mem.atols  = N_VNew_Serial(len MAYBE_COMMA_CONTEXT(m_solverMem));
	_mem.types  = N_VNew_Serial(len MAYBE_COMMA_CONTEXT(m_solverMem));
	_mem.constr = N_VNew_Serial(len MAYBE_COMMA_CONTEXT(m_solverMem));
	if (!_mem.vars || !_mem.ders || !_mem.atols || !_mem.types || !_mem.constr)
		return WriteError("IDA", "N_VNew_Serial", "Cannot create vectors.");

	// initialize vectors
	std::memcpy(N_VGetArrayPointer(_mem.vars)  , m_model->GetVarInitValues()   .data(), sizeof(sun_real) * len);
	std::memcpy(N_VGetArrayPointer(_mem.ders)  , m_model->GetDerInitValues()   .data(), sizeof(sun_real) * len);
	std::memcpy(N_VGetArrayPointer(_mem.atols) , m_model->GetATols()           .data(), sizeof(sun_real) * len);
	std::memcpy(N_VGetArrayPointer(_mem.types) , m_model->GetVarTypes()        .data(), sizeof(sun_real) * len);
	std::memcpy(N_VGetArrayPointer(_mem.constr), m_model->GetConstraintValues().data(), sizeof(sun_real) * len);

	// create matrix object
	_mem.sunmatr = SUNDenseMatrix(len, len MAYBE_COMMA_CONTEXT(m_solverMem));
	if (!_mem.sunmatr)
		return WriteError("IDA", "SUNDenseMatrix", "Cannot create matrix.");

	// create linear solver object
	_mem.linsol = SUNLinSol_Dense(_mem.vars, _mem.sunmatr MAYBE_COMMA_CONTEXT(m_solverMem));
	if (!_mem.linsol)
		return WriteError("IDA", "SUNLinSol_Dense", "Cannot create linear solver.");

	// create IDA object
	_mem.idamem = IDACreate(MAYBE_CONTEXT(m_solverMem));
	if (!_mem.idamem)
		return WriteError("IDA", "IDACreate", "Cannot initialize IDA solver object.");

	// initialize IDA solver
	res = IDAInit(_mem.idamem, &CDAESolver::ResidualFunction, 0, _mem.vars, _mem.ders);
	if (res != IDA_SUCCESS)
		return WriteError("IDA", "IDAInit", "Cannot initialize IDA solver.");

	// set tolerances
	res = IDASVtolerances(_mem.idamem, m_model->GetRTol(), _mem.atols);
	if (res != IDA_SUCCESS)
		return WriteError("IDA", "IDASVtolerances", "Cannot set tolerances.");

	// attach linear solver
	res = IDASetLinearSolver(_mem.idamem, _mem.linsol, _mem.sunmatr);
	if (res != IDALS_SUCCESS)
		return WriteError("IDA", "IDASetLinearSolver", "Cannot set linear solver.");

	// set optional inputs
	// set error handler function
#if SUNDIALS_VERSION_MAJOR < 7
	res = IDASetErrHandlerFn(_mem.idamem, &CDAESolver::ErrorHandler, &m_errorMessage);
	if (res != IDA_SUCCESS)
		return WriteError("IDA", "IDASetErrHandlerFn", "Cannot setup error handler function.");
#else
	res = SUNContext_PushErrHandler(_mem.sunctx, &CDAESolver::ErrorHandler, &m_errorMessage);
	if (res != IDA_SUCCESS)
		return WriteError("IDA", "SUNContext_PushErrHandler", "Cannot setup error handler function.");
#endif
	// set model as user data
	res = IDASetUserData(_mem.idamem, m_model);
	if (res != IDA_SUCCESS)
		return WriteError("IDA", "IDASetUserData", "Cannot set user data.");
	// set max number of iteration steps
	res = IDASetMaxNumSteps(_mem.idamem, static_cast<long int>(m_maxNumSteps));
	if (res != IDA_SUCCESS)
		return WriteError("IDA", "IDASetMaxNumSteps", "Cannot set maximum number of steps.");
	// setup types of variables
	res = IDASetId(_mem.idamem, _mem.types);
	if (res != IDA_SUCCESS)
		return WriteError("IDA", "IDASetId", "Cannot set IDs.");
	// set constraints
	res = IDASetConstraints(_mem.idamem, m_model->IsConstraintsDefined() ? _mem.constr : nullptr);
	if (res != IDA_SUCCESS)
		return WriteError("IDA", "IDASetConstraints", "Cannot set constraints.");

	return true;
}

void CDAESolver::ClearSolverMemory(SSolverMemory& _mem)
{
	N_VDestroy_Serial(_mem.vars);   _mem.vars    = nullptr;
	N_VDestroy_Serial(_mem.ders);   _mem.ders    = nullptr;
	N_VDestroy_Serial(_mem.atols);  _mem.atols   = nullptr;
	N_VDestroy_Serial(_mem.types);  _mem.types   = nullptr;
	N_VDestroy_Serial(_mem.constr); _mem.constr  = nullptr;
	SUNMatDestroy(_mem.sunmatr);    _mem.sunmatr = nullptr;
	SUNLinSolFree(_mem.linsol);     _mem.linsol  = nullptr;
	IDAFree(&_mem.idamem);          _mem.idamem  = nullptr;
#if SUNDIALS_VERSION_MAJOR >= 6
	SUNContext_Free(&_mem.sunctx);  _mem.sunctx  = nullptr;
#endif
}

void CDAESolver::InitStoreMemory(SStoreMemory& _mem) const
{
	const auto len = static_cast<sunindextype>(m_model->GetVariablesNumber());
	_mem.vars.resize(len);
	_mem.ders.resize(len);
	_mem.ida_phi.resize(MXORDP1, std::vector<sun_real>(len));
	_mem.ida_psi.resize(MXORDP1);
}

void CDAESolver::Clear()
{
	m_model = nullptr;
	m_errorMessage.clear();

	ClearSolverMemory(m_solverMem);
}

int CDAESolver::ResidualFunction(sun_real _time, N_Vector _vals, N_Vector _ders, N_Vector _ress, void* _model)
{
	sun_real* vals = N_VGetArrayPointer(_vals);
	sun_real* ders = N_VGetArrayPointer(_ders);
	sun_real* ress = N_VGetArrayPointer(_ress);
	const bool res = static_cast<CDAEModel*>(_model)->GetResiduals(_time, vals, ders, ress);
	return res ? 0 : -1;
}

void CDAESolver::ErrorHandler(int _errorCode, const char* _module, const char* _function, char* _message, void* _outString)
{
	if (!_outString) return;
	if (_errorCode >= 0) return;
	std::string& out = *static_cast<std::string*>(_outString);
	AppendMessage(_module, _function, _message, out);
}

void CDAESolver::ErrorHandler(int _line, const char* _function, const char* _file, const char* _message, SUNErrCode _errCode, void* _outString, [[maybe_unused]] SUNContext _sunctx)
{
	if (!_outString) return;
	if (!_errCode) return;
	std::string& out = *static_cast<std::string*>(_outString);
	AppendMessage(_file, _function, _message, out);
}

std::string CDAESolver::BuildErrorMessage(const std::string& _module, const std::string& _function, const std::string& _message)
{
	return "[" + _module + " ERROR] in " + _function + ": " + _message;
}

void CDAESolver::AppendMessage(const std::string& _module, const std::string& _function, const std::string& _message, std::string& _out)
{
	if (!_out.empty())
		_out += "\n";
	_out += BuildErrorMessage(_module, _function, _message);
}

bool CDAESolver::WriteError(const std::string& _module, const std::string& _function, const std::string& _message)
{
	AppendMessage(_module, _function, _message, m_errorMessage);
	return false;
}

#ifdef _MSC_VER
#else
#pragma GCC diagnostic pop
#endif

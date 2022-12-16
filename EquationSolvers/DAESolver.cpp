/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DAESolver.h"
#if defined(_MSC_VER)
__pragma(warning(push))
__pragma(warning(disable : 4305 4309))
#endif
#include <ida/ida.h>
#include <ida/ida_ls_impl.h>
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
	if (!InitSolverMemory(m_solverMem_store))
	{
		ClearSolverMemory(m_solverMem_store);
		return false;
	}

	SaveState();
	return true;
}

bool CDAESolver::Calculate(realtype _time)
{
	int res;

	if (_time == 0.0)
	{
		res = IDACalcIC(m_solverMem.idamem, IDA_YA_YDP_INIT, 0.001);
		if (res != IDA_SUCCESS)
			return WriteError("IDA", "IDACalcIC", "Cannot calculate initial conditions.");
		m_model->HandleResults(0, N_VGetArrayPointer(m_solverMem.vars), N_VGetArrayPointer(m_solverMem.ders));
	}
	else
	{
		res = IDASetStopTime(m_solverMem.idamem, _time);
		if (res != IDA_SUCCESS)
			return WriteError("IDA", "IDASetStopTime", "Cannot set stop time.");
		do
		{
			res = IDASolve(m_solverMem.idamem, _time, &m_timeLast, m_solverMem.vars, m_solverMem.ders, IDA_ONE_STEP);
			if (res < 0)
				return WriteError("IDA", "IDASolve", "Cannot integrate.");
		} while (res != IDA_TSTOP_RETURN);
		m_model->HandleResults(m_timeLast, N_VGetArrayPointer(m_solverMem.vars), N_VGetArrayPointer(m_solverMem.ders));
	}

	return true;
}

bool CDAESolver::Calculate(realtype _timeBeg, realtype _timeEnd)
{
	int res;

	if (_timeBeg == _timeEnd)
		return WriteError("IDA", "Calculate", "Start and end time points are equal. Cannot perform calculations for dynamic model.");

	if (_timeBeg == 0.0)
	{
		res = IDACalcIC(m_solverMem.idamem, IDA_YA_YDP_INIT, 0.001);
		if (res != IDA_SUCCESS)
			return WriteError("IDA", "IDACalcIC", "Cannot calculate initial conditions.");

		N_Vector consistVars, consistDers;
#if SUNDIALS_VERSION_MAJOR < 6
		consistVars = N_VNew_Serial(static_cast<sunindextype>(m_model->GetVariablesNumber()));
		consistDers = N_VNew_Serial(static_cast<sunindextype>(m_model->GetVariablesNumber()));
#else
		consistVars = N_VNew_Serial(static_cast<sunindextype>(m_model->GetVariablesNumber()), m_solverMem.sunctx);
		consistDers = N_VNew_Serial(static_cast<sunindextype>(m_model->GetVariablesNumber()), m_solverMem.sunctx);
#endif
		res = IDAGetConsistentIC(m_solverMem.idamem, consistVars, consistDers);
		if (res != IDA_SUCCESS)
			return WriteError("IDA", "IDAGetConsistentIC", "Cannot obtain consistent initial conditions.");
		m_model->HandleResults(0, N_VGetArrayPointer(consistVars), N_VGetArrayPointer(consistDers));
		N_VDestroy_Serial(consistVars);
		N_VDestroy_Serial(consistDers);
	}

	realtype step = (_timeEnd - _timeBeg) / 2.0;
	if (m_maxStep != 0.0 && step > m_maxStep)
		step = m_maxStep;
	res = IDASetMaxStep(m_solverMem.idamem, step);
	if (res != IDA_SUCCESS)
		return WriteError("IDA", "IDASetMaxStep", "Cannot set maximum absolute step size");

	res = IDASetStopTime(m_solverMem.idamem, _timeEnd);
	if (res != IDA_SUCCESS)
		return WriteError("IDA", "IDASetStopTime", "Cannot set integration stop time");

	do
	{
		res = IDASolve(m_solverMem.idamem, _timeEnd, &m_timeLast, m_solverMem.vars, m_solverMem.ders, IDA_ONE_STEP);
		if (res < 0)
			return WriteError("IDA", "IDASolve", "Cannot integrate.");
		m_model->HandleResults(m_timeLast, N_VGetArrayPointer(m_solverMem.vars), N_VGetArrayPointer(m_solverMem.ders));
	} while (res != IDA_TSTOP_RETURN);

	return true;
}

void CDAESolver::SaveState() const
{
	CopyIDAmem (m_solverMem_store.idamem, m_solverMem.idamem);
	CopyNVector(m_solverMem_store.vars  , m_solverMem.vars  );
	CopyNVector(m_solverMem_store.ders  , m_solverMem.ders  );
}

void CDAESolver::LoadState() const
{
	CopyIDAmem (m_solverMem.idamem, m_solverMem_store.idamem);
	CopyNVector(m_solverMem.vars  , m_solverMem_store.vars  );
	CopyNVector(m_solverMem.ders  , m_solverMem_store.ders  );
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
#if SUNDIALS_VERSION_MAJOR < 6
#else
	res = SUNContext_Create(nullptr, &_mem.sunctx);
	if (res != IDA_SUCCESS)
		return WriteError("IDA", "SUNContext_Create", "Cannot create SUNDIALS context.");
#endif

	const auto len = static_cast<sunindextype>(m_model->GetVariablesNumber());

	// allocate vectors for y, y', tolerances, types, constraints
#if SUNDIALS_VERSION_MAJOR < 6
	_mem.vars   = N_VNew_Serial(len);
	_mem.ders   = N_VNew_Serial(len);
	_mem.atols  = N_VNew_Serial(len);
	_mem.types  = N_VNew_Serial(len);
	_mem.constr = N_VNew_Serial(len);
#else
	_mem.vars   = N_VNew_Serial(len, m_solverMem.sunctx);
	_mem.ders   = N_VNew_Serial(len, m_solverMem.sunctx);
	_mem.atols  = N_VNew_Serial(len, m_solverMem.sunctx);
	_mem.types  = N_VNew_Serial(len, m_solverMem.sunctx);
	_mem.constr = N_VNew_Serial(len, m_solverMem.sunctx);
#endif
	if (!_mem.vars || !_mem.ders || !_mem.atols || !_mem.types || !_mem.constr)
		return WriteError("IDA", "N_VNew_Serial", "Cannot create vectors.");

	// initialize vectors
	std::memcpy(N_VGetArrayPointer(_mem.vars)  , m_model->GetVarInitValues()   .data(), sizeof(realtype) * len);
	std::memcpy(N_VGetArrayPointer(_mem.ders)  , m_model->GetDerInitValues()   .data(), sizeof(realtype) * len);
	std::memcpy(N_VGetArrayPointer(_mem.atols) , m_model->GetATols()           .data(), sizeof(realtype) * len);
	std::memcpy(N_VGetArrayPointer(_mem.types) , m_model->GetVarTypes()        .data(), sizeof(realtype) * len);
	std::memcpy(N_VGetArrayPointer(_mem.constr), m_model->GetConstraintValues().data(), sizeof(realtype) * len);

	// create matrix object
#if SUNDIALS_VERSION_MAJOR < 6
	_mem.sunmatr = SUNDenseMatrix(len, len);
#else
	_mem.sunmatr = SUNDenseMatrix(len, len, _mem.sunctx);
#endif
	if (!_mem.sunmatr)
		return WriteError("IDA", "SUNDenseMatrix", "Cannot create matrix.");

	// create linear solver object
#if SUNDIALS_VERSION_MAJOR < 6
	_mem.linsol = SUNLinSol_Dense(_mem.vars, _mem.sunmatr);
#else
	_mem.linsol = SUNLinSol_Dense(_mem.vars, _mem.sunmatr, _mem.sunctx);
#endif
	if (!_mem.linsol)
		return WriteError("IDA", "SUNLinSol_Dense", "Cannot create linear solver.");

	// create IDA object
#if SUNDIALS_VERSION_MAJOR < 6
	_mem.idamem = IDACreate();
#else
	_mem.idamem = IDACreate(_mem.sunctx);
#endif
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
	res = IDASetErrHandlerFn(_mem.idamem, &CDAESolver::ErrorHandler, &m_errorMessage);
	if (res != IDA_SUCCESS)
		return WriteError("IDA", "IDASetErrHandlerFn", "Cannot setup error handler function.");
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
#if SUNDIALS_VERSION_MAJOR < 6
#else
	SUNContext_Free(&_mem.sunctx);  _mem.sunctx  = nullptr;
#endif
}

void CDAESolver::Clear()
{
	m_model = nullptr;
	m_errorMessage.clear();

	ClearSolverMemory(m_solverMem);
	ClearSolverMemory(m_solverMem_store);
}

void CDAESolver::CopyNVector(N_Vector _dst, N_Vector _src)
{
	if (_dst == nullptr || _src == nullptr) return;
	std::memcpy(N_VGetArrayPointer(_dst), N_VGetArrayPointer(_src), sizeof(realtype) * static_cast<size_t>(NV_LENGTH_S(_src)));
}

void CDAESolver::CopyIDAmem(void* _dst, void* _src)
{
	if (_src == nullptr || _dst == nullptr) return;

	const auto* src = static_cast<IDAMem>(_src);
	auto* dst = static_cast<IDAMem>(_dst);

	for (size_t i = 0; i < MXORDP1; ++i)
		CopyNVector(dst->ida_phi[i], src->ida_phi[i]);
	std::memcpy(dst->ida_psi  , src->ida_psi  , sizeof(realtype) * MXORDP1);
	dst->ida_kused = src->ida_kused;
	dst->ida_ns    = src->ida_ns;
	dst->ida_hh    = src->ida_hh;
	dst->ida_tn    = src->ida_tn;
	dst->ida_cj    = src->ida_cj;
	dst->ida_nst   = src->ida_nst;
}

int CDAESolver::ResidualFunction(realtype _time, N_Vector _vals, N_Vector _ders, N_Vector _ress, void* _model)
{
	realtype* vals = N_VGetArrayPointer(_vals);
	realtype* ders = N_VGetArrayPointer(_ders);
	realtype* ress = N_VGetArrayPointer(_ress);
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

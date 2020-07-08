/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolHelperDefines.h"
#include <string>

#ifdef _MSC_VER
	#if defined(DLL_EXPORT)
		#define DECLDIR __declspec(dllexport)
	#else
		#define DECLDIR __declspec(dllimport)
	#endif
#else
	#define DECLDIR
#endif

enum class ESolverTypes : unsigned
{
	SOLVER_NONE = 0,
	SOLVER_AGGLOMERATION_1 = 1,
	SOLVER_PBM_1 = 2
};

#define SOLVERS_TYPES_NUMBER	2
#define SOLVERS_TYPE_NAMES { "Undefined", "Agglomeration #1", "PBM #1" }
#define CREATE_SOLVER_FUN_AGG1	CREATE_SOLVER_FUN(1)
#define CREATE_SOLVER_FUN_PBM1	CREATE_SOLVER_FUN(2)
#define CREATE_SOLVER_FUN_NAMES { CREATE_SOLVER_FUN_NAME(0), CREATE_SOLVER_FUN_NAME(1), CREATE_SOLVER_FUN_NAME(2) }

#if _DEBUG
#define CREATE_SOLVER_FUNCTION_BASE CreateDYSSOLSolverV2_DEBUG
#else
#define CREATE_SOLVER_FUNCTION_BASE CreateDYSSOLSolverV2
#endif
#define CREATE_SOLVER_FUN(X)		MACRO_CONCAT(CREATE_SOLVER_FUNCTION_BASE, X)
#define CREATE_SOLVER_FUN_NAME(X)	MACRO_TOSTRING(CREATE_SOLVER_FUN(X))

class CExternalSolver
{
public:
	unsigned m_nCompilerVer;

protected:
	ESolverTypes m_solverType;
	std::string m_solverName;
	std::string m_authorName;
	std::string m_solverUniqueKey;
	unsigned m_solverVersion;

public:
	CExternalSolver();
	virtual ~CExternalSolver() = default;

	/// Will be called once during initialization of unit.
	virtual void Initialize();
	/// Will be called once after the whole simulation is finished.
	virtual void Finalize();
	/// Will be called when storing of current internal state is needed.
	virtual void SaveState();
	/// Will be called when loading of last stored internal state is needed.
	virtual void LoadState();

	/// Returns type to which belongs this solver.
	ESolverTypes GetType() const;
	/// Returns name of solver.
	std::string GetName() const;
	/// Returns string key, unique for all solvers.
	std::string GetUniqueID() const;
	/// Returns name of the solver's author.
	std::string GetAuthorName() const;
	/// Returns version of solver.
	unsigned GetVersion() const;

protected:
	void RaiseError(const std::string& _sDescription) const;
};

typedef DECLDIR CExternalSolver* (*CreateExternalSolver)();

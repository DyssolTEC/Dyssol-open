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

enum class ESolverTypes : uint32_t
{
	SOLVER_NONE            = 0,
	SOLVER_AGGLOMERATION_1 = 1,
	SOLVER_PBM_1           = 2
};

#define CREATE_SOLVER_FUNCTION_BASE CreateDYSSOLSolverV4
#define SOLVERS_TYPES_NUMBER 2
#define SOLVERS_TYPE_NAMES { "Undefined", "Agglomeration", "PBM" }
#define CREATE_SOLVER_FUN_AGG1	CREATE_SOLVER_FUN(1)
#define CREATE_SOLVER_FUN_PBM1	CREATE_SOLVER_FUN(2)
#define CREATE_SOLVER_FUN_NAMES { CREATE_SOLVER_FUN_NAME(0), CREATE_SOLVER_FUN_NAME(1), CREATE_SOLVER_FUN_NAME(2) }
#if _DEBUG
#define CREATE_SOLVER_FUNCTION_CONF MACRO_CONCAT(CREATE_SOLVER_FUNCTION_BASE, _DEBUG)
#else
#define CREATE_SOLVER_FUNCTION_CONF CREATE_SOLVER_FUNCTION_BASE
#endif
#define CREATE_SOLVER_FUN(X)		MACRO_CONCAT(CREATE_SOLVER_FUNCTION_CONF, X)
#define CREATE_SOLVER_FUN_NAME(X)	MACRO_TOSTRING(CREATE_SOLVER_FUN(X))

class CBaseSolver
{
protected:
	ESolverTypes m_type{ ESolverTypes::SOLVER_NONE };	// Type of the solver (SOLVER_AGGLOMERATION_1/SOLVER_PBM_1/...).
	std::string m_name{};								// User-friendly name of the solver.
	std::string m_authorName{};							// Name of solver's author.
	std::string m_uniqueID{};							// Unique identifier of the solver.
	size_t m_version{};									// Version of the solver.
	size_t m_compilerVersion{ COMPILER_VERSION };		// Version of compiler used to build the solver.

public:
	CBaseSolver()                                     = default;
	virtual ~CBaseSolver()                            = default;
	CBaseSolver(const CBaseSolver& _other)            = default;
	CBaseSolver(CBaseSolver&& _other)                 = default;
	CBaseSolver& operator=(const CBaseSolver& _other) = default;
	CBaseSolver& operator=(CBaseSolver&& _other)      = default;

	// Returns type to which belongs this solver.
	ESolverTypes GetType() const;
	// Returns name of solver.
	std::string GetName() const;
	// Returns name of the solver's author.
	std::string GetAuthorName() const;
	// Returns version of solver.
	size_t GetVersion() const;
	// Returns string key, unique for all solvers.
	std::string GetUniqueID() const;

	// Sets the name of the solver.
	void SetName(const std::string& _name);
	// Sets the name of solver's author.
	void SetAuthorName(const std::string& _author);
	// Sets the version of the solver.
	void SetVersion(size_t _version);
	// Sets the unique identifier of the solver.
	void SetUniqueID(const std::string& _id);

	// Will be called once during creation of the solver (name, author, key, version).
	virtual void CreateBasicInfo() = 0;
	// Will be called to initialize the solver.
	virtual void Initialize();
	// Will be called once after the whole simulation is finished.
	virtual void Finalize();
	// Will be called when storing of current internal state is needed.
	virtual void SaveState();
	// Will be called when loading of last stored internal state is needed.
	virtual void LoadState();

protected:
	[[noreturn]] void RaiseError(const std::string& _message) const;
};

typedef DECLDIR CBaseSolver* (*CreateExternalSolver)();

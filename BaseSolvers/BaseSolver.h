/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolHelperDefines.h"
#include <string>
#include <cstdint>

#ifdef _MSC_VER
	#if defined(DLL_EXPORT)
		#define DECLDIR __declspec(dllexport)
	#else
		#define DECLDIR __declspec(dllimport)
	#endif
#else
	#define DECLDIR
#endif

/**
 * \brief Types of solver parameters.
 */
enum class ESolverTypes : uint32_t
{
	SOLVER_NONE            = 0,	///< Undefined.
	SOLVER_AGGLOMERATION_1 = 1,	///< Agglomeration solver.
	SOLVER_PBM_1           = 2, ///< Population balance solver.
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

/**
 * \brief Base solver.
 */
class CBaseSolver
{
protected:
	ESolverTypes m_type{ ESolverTypes::SOLVER_NONE };	///< Type of the solver (SOLVER_AGGLOMERATION_1/SOLVER_PBM_1/...).
	std::string m_name{};								///< User-friendly name of the solver.
	std::string m_authorName{};							///< Name of solver's author.
	std::string m_uniqueID{};							///< Unique identifier of the solver.
	std::string m_helpLink{};							///< Link to help file for solver.
	size_t m_version{};									///< Version of the solver.
	size_t m_compilerVersion{ COMPILER_VERSION };		///< Version of compiler used to build the solver.

public:
	/**
	 * \private
	 */
	CBaseSolver()                                     = default;
	/**
	 * \private
	 */
	virtual ~CBaseSolver()                            = default;
	/**
	 * \private
	 */
	CBaseSolver(const CBaseSolver& _other)            = default;
	/**
	 * \private
	 */
	CBaseSolver(CBaseSolver&& _other)                 = default;
	/**
	 * \private
	 */
	CBaseSolver& operator=(const CBaseSolver& _other) = default;
	/**
	 * \private
	 */
	CBaseSolver& operator=(CBaseSolver&& _other)      = default;

	/**
	 * \brief Returns type to which belongs this solver.
	 * \return Solver's type.
	 */
	ESolverTypes GetType() const;
	/**
	 * \brief Returns name of solver.
	 * \return Solver's name.
	 */
	std::string GetName() const;
	/**
	 * \brief Returns name of the solver's author.
	 * \return Author's type.
	 */
	std::string GetAuthorName() const;
	/**
	 * \brief Returns version of solver.
	 * \return Solver's version.
	 */
	size_t GetVersion() const;
	/**
	 * \brief Returns string key, unique for all solvers.
	 * \return Solver's unique ID.
	 */
	std::string GetUniqueID() const;
	/**
	 * \brief Returns the help link of the solver.
	 * \return Help link of the solver.
	 */
	std::string GetHelpLink() const;

	/**
	 * \brief Sets the name of the solver.
	 * \param _name Solver's name.
	 */
	void SetName(const std::string& _name);
	/**
	 * \brief Sets the name of solver's author.
	 * \param _author Solver's author.
	 */
	void SetAuthorName(const std::string& _author);
	/**
	 * \brief Sets the version of the solver.
	 * \param _version Solver's version.
	 */
	void SetVersion(size_t _version);
	/**
	 * \brief Sets the unique identifier of the solver.
	 * \param _id Solver's unique ID.
	 */
	void SetUniqueID(const std::string& _id);
	/**
	 * \brief Sets the help link of the solver.
	 * \param _helpLink Help link of the solver.
	 */
	void SetHelpLink(const std::string& _helpLink);

	/**
	 * \brief Will be called once during creation of the solver (name, author, key, version).
	 */
	virtual void CreateBasicInfo() = 0;
	/**
	 * \brief Will be called to initialize the solver.
	 */
	virtual void Initialize();
	/**
	 * \brief Will be called once after the whole simulation is finished.
	 */
	virtual void Finalize();
	/**
	 * \brief Will be called when storing of current internal state is needed.
	 */
	virtual void SaveState();
	/**
	 * \brief Will be called when loading of last stored internal state is needed.
	 */
	virtual void LoadState();

protected:
	/**
	 * \brief Sets an error state of the solver, prints the message to the simulation log, and requests to stop simulation.
	 * \param _message Message to show in the simulation log.
	 */
	[[noreturn]] void RaiseError(const std::string& _message) const;
};

typedef DECLDIR CBaseSolver* (*CreateExternalSolver)();

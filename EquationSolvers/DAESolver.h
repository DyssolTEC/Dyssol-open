/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DAEModel.h"
#include <string>
#include <sundials/sundials_matrix.h>
#include <sundials/sundials_linearsolver.h>

/** Solver of differential algebraic equations. Uses IDA solver from SUNDIALS package*/
class CDAESolver
{
	/** Memory needed for solver. */
	struct SSolverMemory
	{
#if SUNDIALS_VERSION_MAJOR >= 6
		SUNContext sunctx{};      ///< SUNDIALS simulation context.
#endif
		void* idamem{};			  ///< Pointer to IDA memory.
		SUNMatrix sunmatr{};      ///< Matrix object.
		SUNLinearSolver linsol{}; ///< Linear solver object.
		N_Vector vars{};          ///< Vector of variables.
		N_Vector ders{};          ///< Vector of derivatives.
		N_Vector atols{};         ///< Vector of absolute tolerances.
		N_Vector types{};         ///< Vector of variables' types: algebraic/differential.
		N_Vector constr{};        ///< Vector of variables' constraints.
	};

	/** Data field from IDA memory needed to be temporary stored. */
	struct SStoreMemory
	{
		std::vector<realtype> vars;
		std::vector<realtype> ders;
		std::vector<std::vector<realtype>> ida_phi;
		std::vector<realtype> ida_psi;
		int ida_kused;
		int ida_ns;
		realtype ida_hh;
		realtype ida_tn;
		realtype ida_cj;
		long int ida_nst;
	};

	CDAEModel* m_model{};	          ///< Pointer to a DAE model.

	SSolverMemory m_solverMem{};      ///< Solver-specific memory.
	SStoreMemory m_solverMem_store{}; ///< Solver-specific memory for temporary storing.

	realtype m_timeLast{};            ///< Last calculated time point.
	realtype m_maxStep{};             ///< Maximum iteration time step.
	size_t m_maxNumSteps{ 500 };      ///< Maximum number of allowed solver iterations.

	std::string m_errorMessage;	      ///< Text description of the occurred errors.

public:
	/**	Basic constructor. */
	CDAESolver() = default;
	/**	Basic destructor. */
	~CDAESolver();

	/** Set model to a solver.
	*	\param _model Pointer to a model.
	*	\retval true No errors occurred. */
	bool SetModel(CDAEModel* _model);

	/** Solve problem on a given time point.
	*	\param _time Time point.
	*	\retval true No errors occurred. */
	bool Calculate(realtype _time);
	/** Solve problem on a given time interval.
	*	\param _timeBeg Start of the time interval.
	*	\param _timeEnd End of the time interval.
	*	\retval true No errors occurred. */
	bool Calculate(realtype _timeBeg, realtype _timeEnd);

	/** Calculates and applies corrected initial conditions.
	*	\retval true No errors occurred. */
	bool CalculateInitialConditions();
	/** Integrates the problem until the given time point.
	*	\param _time Final time of integration.
	*	\retval true No errors occurred. */
	bool IntegrateUntil(realtype _time);

	/** Save current state of solver.
	*	Should be called during saving of unit. */
	void SaveState();
	/** Load current state of solver.
	*	Should be called during loading of unit. */
	void LoadState() const;

	/** Returns error description.
	 *	\return Current error description. */
	[[nodiscard]] std::string GetError() const;

	/** Returns the maximum allowed steps number of the solver.
	 *	\return Number of steps. */
	[[nodiscard]] size_t GetSolverMaxNumSteps() const;
	/** Sets the maximum allowed steps number of the solver.
	 *	\param _num Number of steps. */
	void SetSolverMaxNumSteps(size_t _num);
	/** Returns the maximum time step for solver.
	 *	\return Time step. */
	[[nodiscard]] double GetMaxStep() const;
	/** Sets the maximum time step for solver.
	 *	\param _step Time step. */
	void SetMaxStep(double _step);

private:
	/** Allocates and initializes memory required for solver.
	 *	\param _mem Reference to the memory struct.
	 *	\retval true No errors occurred. */
	bool InitSolverMemory(SSolverMemory& _mem);
	/** Clear allocated solver-related memory.
	 *	\param _mem Reference to the memory struct. */
	static void ClearSolverMemory(SSolverMemory& _mem);
	/** Initializes memory required for storing solver data.
	 *	\param _mem Reference to the memory struct. */
	void InitStoreMemory(SStoreMemory& _mem) const;

	/** De-allocates and clears all internal data. */
	void Clear();

	/** A callback function called to calculate the problem residuals.
	*   The function computes residual for given values of the independent variables, state vectors, and derivatives.
	*	\param _time Current value of the independent variable.
	*	\param _vals Current value of the dependent variable vector, y(t).
	*	\param _ders Current value of derivative y'(t).
	*	\param _ress Output residual vector F(t, y, y').
	*	\param _model Pointer to a DAE model.
	*	\return Error code. */
	static int ResidualFunction(realtype _time, N_Vector _vals, N_Vector _ders, N_Vector _ress, void *_model);

	/** A callback function called by the solver to handle internal errors.
	*	\param _errorCode Error code
	*	\param _module Name of the  module reporting the error
	*	\param _function Name of the function in which the error occurred
	*	\param _message The error message
	*	\param _outString Pointer to a string to put error message*/
	static void ErrorHandler(int _errorCode, const char* _module, const char* _function, char* _message, void* _outString);
	/** Builds an error message from its parts.
	*	\param _module Name of the module reporting the error
	*	\param _function Name of the function in which the error occurred
	*	\param _message The error message
	*	\return Built error message*/
	static std::string BuildErrorMessage(const std::string& _module, const std::string& _function, const std::string& _message);
	/** Appends an error message to the given output string.
	*	\param _module Name of the module reporting the error
	*	\param _function Name of the function in which the error occurred
	*	\param _message The error message
	*	\param _out Output string */
	static void AppendMessage(const std::string& _module, const std::string& _function, const std::string& _message, std::string& _out);
	/** Appends an error message to the current error description.
	*	\param _module Name of the module reporting the error
	*	\param _function Name of the function in which the error occurred
	*	\param _message The error message
	*	\return false */
	bool WriteError(const std::string& _module, const std::string& _function, const std::string& _message);
};

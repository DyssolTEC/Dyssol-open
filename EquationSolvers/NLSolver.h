/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "NLModel.h"
#include <sundials/sundials_matrix.h>
#include <sundials/sundials_linearsolver.h>
#include <string>

#define ZERO RCONST(0.0)

/** Solver of differential algebraic equations. Uses IDA solver from SUNDIALS package*/
class CNLSolver
{
#if SUNDIALS_VERSION_MAJOR <= 6
	using sun_real = realtype;
#else
	using sun_real = sunrealtype;
#endif

private:
	CNLModel* m_pModel;					///< Pointer to a DAE model
	void* m_pKINmem;					///< KIN memory
	SUNMatrix m_A{};					///< Matrix.
	SUNLinearSolver m_LS{};				///< Linear solver.

	N_Vector m_vectorVars;				///< Vector of variables
	N_Vector m_vectorUScales;			///< Vector of uscales
	N_Vector m_vectorFScales;			///< Vector of fscales

	std::string m_sErrorDescription;	///< Text description of the last occurred error

	// Variables for storing
	N_Vector m_StoreVectorVars;			///< Memory for storing of vector of variables

	// Solver settings
	ENLSolverStrategy m_eStrategy;		///< Solver strategy

	// General setting
	long m_nMaxIter;				///< Max. number of nonlinear iterations

	// Newton method settings
	long m_nMaxSet;					///< Max. iterations without matrix setup
	long m_nMaxSubSet;				///< Max. iterations without residual check. nMaxSet should be multiple of nMaxSubSet.

	// Fixed point method settings
	long m_nMAA;						///< Anderson Acceleration subspace size. The value of nMAA should always be less than nMaxIter. From this value the Anderson Acceleration is applied.
	double m_dDampingAA;				///< Anderson Acceleration damping parameter between 0 and 1
	double m_dDamping{};				///< Damping parameter between 0 and 1

#if SUNDIALS_VERSION_MAJOR < 6
#else
	SUNContext m_sunctx{};              ///< SUNDIALS simulation context.
#endif

public:
	/**	Basic constructor.*/
	CNLSolver();
	CNLSolver(ENLSolverStrategy _eStrategy);
	/**	Basic destructor.*/
	~CNLSolver();

	/** Set model to a solver.
	 *	\param _eStrategy Solving strategy*/
	void SetStrategy(ENLSolverStrategy _eStrategy);

	/** Set model to a solver.
	 *	\retval Solving strategy*/
	ENLSolverStrategy GetStrategy();

	/** Set maximum number of nonlinear iterations.
	 *	\param _nMaxIter Max. number of nonlinear iterations */
	void SetMaxIter(size_t _nMaxIter);

	/** Set parameters for Newton based solvers.
	 *	\param _nMaxSet Max. iterations without matrix setup
	 *  \param _nMaxSubSet Max. iterations without residual check
	 *  \retval true No errors occurred */
	bool SetNewtonSolverParameters(unsigned _nMaxSet, unsigned _nMaxSubSet);

	/** Set parameters for fixed-point based solvers.
	 *	\param _nMAA Anderson Acceleration subspace size
	 *  \param _dDampingAA Anderson Acceleration damping parameter between 0 and 1
	 *  \retval true No errors occurred */
	bool SetFixedPointSolverParameters(size_t _nMAA, double _dDampingAA, double _dDamping = 1.0);

	/** Set model to a solver.
	 *	\param _pModel Pointer to a model
	 *	\retval true No errors occurred*/
	bool SetModel(CNLModel* _pModel);

	/** Solve problem on a given time point.
	 *	\param _dTime Time point
	 *	\retval true No errors occurred*/
	bool Calculate(sun_real _dTime);

	/** Save current state of solver. Should be called during saving of unit.*/
	void SaveState();
	/** Load current state of solver. Should be called during loading of unit.*/
	void LoadState();

	/** Return error description.*/
	std::string GetError() const;

private:
	/** Calculate residuals. Function computes residual for given values of the independent variable and the function value.
	 *	\param _value Current value of the dependent variable vector, y
	 *	\param _func  Current function value of value, f(y)
	 *	\param _pModel Pointer to a DAE model
	 *	\return Error code*/
	static int ResidualFunction(N_Vector _value, N_Vector _func, void *_pModel);

	/** Clear all allocated memory.*/
	void ClearMemory();

	/** Copy N_Vector.
	 *	\param _dst Pointer to the memory location to copy to
	 *	\param _src Pointer to the memory location to copy from*/
	void CopyNVector(N_Vector _dst, N_Vector _src);

	/** Function to handle errors.
	 *	\param _nErrorCode Error code
	 *	\param _pModule Name of the ida module reporting the error.
	 *	\param _pFunction Name of the function in which the error occurred
	 *	\param _pMsg The error message
	 *	\param _sOutString Pointer to a string to put error message*/
	static void ErrorHandler(int _nErrorCode, const char *_pModule, const char *_pFunction, char *_pMsg, void *_sOutString);
	/**
	 * \brief A callback function called by the solver to handle internal errors.
	 * \details A version for SUNDIALS 7+.
	 * \param _line The line number at which the error occured.
	 * \param _function The function in which the error occured.
	 * \param _file The file in which the error occured.
	 * \param _message The error message.
	 * \param _errCode The error code for the error that occured.
	 * \param _outString Pointer to a string to put error message.
	 * \param _sunctx Pointer to a valid SUNContext object.
	 */
	static void ErrorHandler(int _line, const char* _function, const char* _file, const char* _message, SUNErrCode _errCode, void* _outString, SUNContext _sunctx);

public:
	// ========== Functions to work with solver settings

	/** Returns the maximum iteration number of the solver (default: 200)*/
	size_t GetSolverMaxIter() const;
	/** Sets the maximum iteration number of the solver*/
	void SetSolverMaxIter(size_t _nMaxIter);
	/** Returns the number of iterations to solve the system*/
	unsigned GetSolverIter();
};

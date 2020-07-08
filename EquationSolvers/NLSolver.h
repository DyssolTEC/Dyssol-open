/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "NLModel.h"
#include <kinsol/kinsol.h>
#include <string>
#include <nvector/nvector_serial.h>

#define ZERO RCONST(0.0)

/** Solver of differential algebraic equations. Uses IDA solver from SUNDIALS package*/
class CNLSolver
{
private:
	CNLModel* m_pModel;					///< Pointer to a DAE model
	void* m_pKINmem;					///< KIN memory

	N_Vector m_vectorVars;				///< Vector of variables
	N_Vector m_vectorUScales;			///< Vector of uscales
	N_Vector m_vectorFScales;			///< Vector of fscales

	std::string m_sErrorDescription;	///< Text description of the last occurred error

	// Variables for storing
	N_Vector m_StoreVectorVars;			///< Memory for storing of vector of variables


	// Solver settings
	size_t m_nMaxIter;		///< Integer with maximum number of solver iterations

public:
	/**	Basic constructor.*/
	CNLSolver();
	/**	Basic destructor.*/
	~CNLSolver();

	/** Set model to a solver.
	 *	\param _pModel Pointer to a model
	 *	\retval true No errors occurred*/
	bool SetModel(CNLModel* _pModel, unsigned _nMAA = 0);

	/** Solve problem on a given time point.
	 *	\param _dTime Time point
	 *	\retval true No errors occurred*/
	bool Calculate(realtype _dTime, unsigned _nModel = KIN_NONE);

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

public:
	// ========== Functions to work with solver settings

	/** Returns the maximum iteration number of the solver (default: 200)*/
	size_t GetSolverMaxIter();
	/** Sets the maximum iteration number of the solver*/
	void SetSolverMaxIter(size_t _nMaxIter);
	/** Returns the number of iterations to solve the system*/
	unsigned GetSolverIter();
};

/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DAEModel.h"
#include <string>
#include <nvector/nvector_serial.h>

#define ZERO RCONST(0.0)

/** Solver of differential algebraic equations. Uses IDA solver from SUNDIALS package*/
class CDAESolver
{
private:
	CDAEModel * m_pModel;		///< Pointer to a DAE model
	void *m_pIDAmem;			///< IDA memory

	N_Vector m_vectorVars;		///< Vector of variables
	N_Vector m_vectorDers;		///< Vector of derivatives
	N_Vector m_vectorATols;		///< Vector of absolute tolerances
	N_Vector m_vectorId;		///< Vector of states (algebraic/differential)

	realtype m_dLastTime;		///< Last calculated time
	realtype m_dMaxStep;		// Maximum iteration time step.

	std::string m_sErrorDescription;	///< Text description of the last occurred error

	// Variables for storing
	void *m_pStoreIDAmem;		///< Memory for storing of IDA memory
	N_Vector m_StoreVectorVars;	///< Memory for storing of vector of variables
	N_Vector m_StoreVectorDers;	///< Memory for storing of vector of derivatives

	// Solver settings
	size_t m_nMaxIter;		///< Integer with maximum number of solver iterations

public:
	/**	Basic constructor.*/
	CDAESolver();
	/**	Basic destructor.*/
	~CDAESolver();

	/** Set model to a solver.
	*	\param _pModel Pointer to a model
	*	\retval true No errors occurred*/
	bool SetModel(CDAEModel* _pModel);

	/** Solve problem on a given time interval.
	*	\param _dStartTime Start of the time interval
	*	\param _dEndTime End of the time interval
	*	\retval true No errors occurred*/
	bool Calculate(realtype _dStartTime, realtype _dEndTime);
	/** Solve problem on a given time point.
	*	\param _dTime Time point
	*	\retval true No errors occurred*/
	bool Calculate(realtype _dTime);

	/** Save current state of solver. Should be called during saving of unit.*/
	void SaveState();
	/** Load current state of solver. Should be called during loading of unit.*/
	void LoadState();

	/** Return error description.*/
	std::string GetError();

	/** Sets maximum time step for solver.*/
	bool SetMaxStep(double _dStep);

private:
	/** Calculate residuals. Function computes residual for given values of the independent variable, state vector, and derivative.
	*	\param _dTime Current value of the independent variable
	*	\param _value Current value of the dependent variable vector, y(t)
	*	\param _deriv Current value of y'(t)
	*	\param _res Output residual vector F(t, y, y')
	*	\param _pModel Pointer to a DAE model
	*	\return Error code*/
	static int ResidualFunction(realtype _dTime, N_Vector _value, N_Vector _deriv, N_Vector _res, void *_pModel);

	/** Initialize memory for storing.*/
	bool InitStoringMemory();
	/** Clear all allocated memory.*/
	void ClearMemory();

	/** Copy N_Vector.
	*	\param _dst Pointer to the memory location to copy to
	*	\param _src Pointer to the memory location to copy from*/
	void CopyNVector(N_Vector _dst, N_Vector _src);
	/** Copy IDAMem.
	*	\param _pDst Pointer to the memory location to copy to
	*	\param _pSrc Pointer to the memory location to copy from*/
	void CopyIDAmem(void* _pDst, void* _pSrc);

	/** Function to handle errors.
	*	\param _nErrorCode Error code
	*	\param _pModule Name of the ida module reporting the error.
	*	\param _pFunction Name of the function in which the error occurred
	*	\param _pMsg The error message
	*	\param _sOutString Pointer to a string to put error message*/
	static void ErrorHandler(int _nErrorCode, const char *_pModule, const char *_pFunction, char *_pMsg, void *_sOutString);

	// ========== Functions to work with solver settings

	/** Returns the maximum iteration number of the solver (default: 200)*/
	size_t GetSolverMaxIter();
	/** Sets the maximum iteration number of the solver*/
	void SetSolverMaxIter(size_t _nMaxIter);
};

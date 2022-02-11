/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <vector>
#include "DyssolDefines.h"

#define DEFAULT_ATOL 1.0e-6
#define DEFAULT_RTOL 1.0e-4

/** Model of a differential algebraic equation.*/
class CNLModel
{
	/** Structure to describe state variables.*/
	struct SNLVariable
	{
		double dVariableInit;	///< Initial value for state variable
		double dConstraint;		/** Constraint flag
									0.0:  no constraint,
									1.0:  >= 0.0,
									-1.0: <= 0.0,
									2.0:  >  0.0,
									−2.0: <  0.0 */
		double dUScale;			///< UScale of variable
		double dFScale;			///< FScale of variable

		/** Default constructor*/
		SNLVariable( double _dVariableInit = 0, double _dConstraint = 0.0, double _dUScale = 1.0, double _dFScale = 1.0 )
		{
			dVariableInit = _dVariableInit;
			dConstraint = _dConstraint;
			dUScale = _dUScale;
			dFScale = _dFScale;
		}
	};

	std::vector<SNLVariable> m_vVariables;	///< Vector of state variables
	void *m_pUserData;						///< Pointer to a user data
	
											// Solver settings
	ENLSolverStrategy m_eStrategy;		///< Solver strategy

public:
	/**	Basic constructor.*/
	CNLModel();
	/**	Basic destructor*/
	virtual ~CNLModel();

	/** Remove all data from the model.*/
	void Clear();

	// ========== Functions to work with variables

	/**	Add new differentiable variable.
	 *	\param _dVariableInit Initial value of variable
	 *	\param _dConstraint Constraint for variable: '0.0' - no constraint; '1.0' - >=0.0; '−1.0' - <=0.0; '2.0' - >0.0; '−2.0' - <0.0
	 *	\param _dUScale
	 *	\param _dFScale
	 *	\return Index of variable*/
	size_t AddNLVariable(double _dVariableInit, double _dConstraint = 0.0, double _dUScale = 1.0, double _dFScale = 1.0);
	/**	Get current number of variables.*/
	size_t GetVariablesNumber() const;
	/**	Get initial value of variable.
	 *	\param _dIndex Index of variable.*/
	double GetVarInitValue(size_t _dIndex);
	/**	Get initial value of derivative.
	 *	\param _dIndex Index of variable.*/
	double GetConstraintValue(size_t _dIndex);
	/**
	 *  \param */
	double GetUScaleValue(size_t _dIndex);
	/**
	*  \param */
	double GetFScaleValue(size_t _dIndex);
	/** Remove all variables*/
	void ClearVariables();

	/**	Set pointer to user data. This pointer will be returned with functions \a CalculateResiduals and \a ResultsHandler.
	 *	\param _pUserData Pointer to user data*/
	void SetUserData(void* _pUserData);

	/** Set solver strategy to a model.
	 *	\param _eStrategy Solving strategy*/
	void SetStrategy(ENLSolverStrategy _eStrategy);

	// ========== Virtual functions which should be overridden in child classes

	/** Calculate nonlinear functions.
	 *	\param _pVars Current value of the dependent variable vector, u(t)
	 *	\param _pFunc Output function vector F(u)
	 *	\param _pUserData Pointer to user's data*/
	virtual void CalculateFunctions(double* _pVars, double* _pFunc, void* _pUserData);
	/** Handle results.
	 *	\param _dTime Current value of the independent variable
	 *	\param _pVars Current value of the dependent variable vector, u(t)
	 *	\param _pUserData Pointer to user's data*/
	virtual void ResultsHandler(double _dTime, double* _pVars, void* _pUserData);

	// ========== Functions for calling from solver

	/** Calculate functions. Calls CalculateFunctions.*/
	bool GetFunctions(double* _pVars, double* _pFunc);
	/** Handle results. Calls ResultsHandler.*/
	void HandleResults( double _dTime, double* _pVars );
};

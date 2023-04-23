/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <vector>
#include <cstddef>

#define DEFAULT_ATOL 1.0e-6
#define DEFAULT_RTOL 1.0e-4

/** Model of a differential algebraic equation.*/
class CDAEModel
{
	/** Structure to describe state variables.*/
	struct sStateVariable
	{
		double dVariableInit;	///< Initial value for state variable
		double dDerivativeInit;	///< Initial value for derivative
		bool bIsDifferential;	///< Contains \a true if the variable is differential, \a false if algebraic.
		double dConstraint;		/** Constraint flag
									0.0:  no constraint,
									1.0:  >= 0.0,
									-1.0: <= 0.0,
									2.0:  >  0.0,
									−2.0: <  0.0 */

		/** Default constructor*/
		sStateVariable( double _dVariableInit = 0, double _dDerivativeInit = 0, bool _bIsDifferential = true, double _dConstraint = 0.0 )
		{
			dVariableInit = _dVariableInit;
			dDerivativeInit = _dDerivativeInit;
			bIsDifferential = _bIsDifferential;
			dConstraint = _dConstraint;
		}
	};

private:
	std::vector<sStateVariable> m_vVariables;	///< Vector of state variables
	void *m_pUserData;							///< Pointer to a user data
	double m_dRTol;								///< Relative tolerance
	double m_dATol;								///< Absolute tolerance
	std::vector<double> m_vATol;				///< Absolute tolerance for each variable

public:
	/**	Basic constructor.*/
	CDAEModel();
	/**	Basic destructor*/
	virtual ~CDAEModel();

	/** Remove all data from the model.*/
	void Clear();

	// ========== Functions to work with variables

	/**	Add new differentiable variable.
	 *	\param _isDifferentiable Set true for differential, false for algebraic
	 *	\param _variableInit Initial value for variable
	 *	\param _derivativeInit Initial value for derivative
	 *	\param _constraint Constraint for variable: '0.0' - no constraint; '1.0' - >=0.0; '−1.0' - <=0.0; '2.0' - >0.0; '−2.0' - <0.0
	 *	\return Index of variable*/
	size_t AddDAEVariable(bool _isDifferentiable, double _variableInit, double _derivativeInit, double _constraint = 0.0);
	/**	Add multiple differentiable variables. The number of variables is determined by the length of the vector of variables.
	 *  Does nothing and returns empty vector if the list of variables is empty.
	 *	\param _isDifferentiable Set true for differential, false for algebraic
	 *	\param _variablesInit Initial values for all variables
	 *	\param _derivativesInit Initial value for all derivatives
	 *	\param _constraint Constraint for all variables: '0.0' - no constraint; '1.0' - >=0.0; '−1.0' - <=0.0; '2.0' - >0.0; '−2.0' - <0.0
	 *	\return Indices of the added variables.*/
	std::vector<size_t> AddDAEVariables(bool _isDifferentiable, const std::vector<double>& _variablesInit, double _derivativesInit, double _constraint = 0.0);
	/**	Add multiple differentiable variables. The number of variables is determined by the length of the vector of variables.
	 *  Does nothing and returns empty vector if the list of variables is empty or lengths of vectors are not equal.
	 *	\param _isDifferentiable Set true for differential, false for algebraic
	 *	\param _variablesInit Initial values for all variables
	 *	\param _derivativesInit Initial values for all derivatives, must have the same length as _variablesInit
	 *	\param _constraint Constraint for all variables: '0.0' - no constraint; '1.0' - >=0.0; '−1.0' - <=0.0; '2.0' - >0.0; '−2.0' - <0.0
	 *	\return Indices of the added variables.*/
	std::vector<size_t> AddDAEVariables(bool _isDifferentiable, const std::vector<double>& _variablesInit, const std::vector<double>& _derivativesInit, double _constraint = 0.0);
	/**	Get current number of variables.*/
	size_t GetVariablesNumber() const;
	/**	Get initial value of variable.
	 *	\param _dIndex Index of variable*/
	double GetVarInitValue(size_t _dIndex);
	/**	Get initial value of all variables.
	 *	\return Vector of initial values of all variables*/
	std::vector<double> GetVarInitValues() const;
	/**	Get initial value of derivative.
	 *	\param _dIndex Index of variable*/
	double GetDerInitValue(size_t _dIndex);
	/**	Get initial value of all derivatives.
	 *	\return Vector of initial values of all derivatives*/
	std::vector<double> GetDerInitValues() const;
	double GetConstraintValue(size_t _dIndex);
	/**	Get constraint values for all variables.
	 *	\return Vector of constraint values for all variables*/
	std::vector<double> GetConstraintValues() const;
	/** Checks if a constraint defined for any variable.
	 *	\return true There are defined constraints.
	 *	\return false No defined constraints. */
	bool IsConstraintsDefined() const;
	/** Remove all variables*/
	void ClearVariables();
	/**	Get type of the variable.
	 *	\param _dIndex Index of variable
	 *	\retval 0.0 Algebraic variable
	 *	\retval 1.0 Differential variable*/
	double GetVarType(size_t _dIndex);
	/**	Get types of all variables.
	 *	\return Vector of types of all variables: 0.0 - algebraic variable, 1.0 - differential variable */
	std::vector<double> GetVarTypes() const;

	// ========== Functions to work with tolerances

	/**	Set tolerances.
	 *	\param _dRTol Relative tolerance
	 *	\param _dATol Absolute tolerance*/
	void SetTolerance( double _dRTol, double _dATol );
	/**	Set tolerances.
	 *	\param _dRTol Relative tolerance
	 *	\param _vATol Vector of absolute tolerances for each variable*/
	void SetTolerance( double _dRTol, std::vector<double>& _vATol );
	/**	Get relative tolerance.*/
	double GetRTol();
	/**	Get absolute tolerance.
	 *	\param _dIndex Index of variable*/
	double GetATol(size_t _dIndex);
	/**	Get absolute tolerances for all variables.
	 *	\return Vector of absolute tolerances for all variables*/
	std::vector<double> GetATols() const;

	/**	Set pointer to user data. This pointer will be returned with functions \a CalculateResiduals and \a ResultsHandler.
	 *	\param _pUserData Pointer to user data*/
	void SetUserData( void* _pUserData );

	// ========== Virtual functions which should be overridden in child classes

	/** Calculate residuals.
	 *	\param _dTime Current value of the independent variable
	 *	\param _pVars Current value of the dependent variable vector, y(t)
	 *	\param _pDerivs Current value of y'(t)
	 *	\param _pRes Output residual vector F(t, y, y')
	 *	\param _pUserData Pointer to user's data*/
	virtual void CalculateResiduals( double _dTime, double* _pVars, double* _pDerivs, double* _pRes, void* _pUserData );
	/** Handle results.
	 *	\param _dTime Current value of the independent variable
	 *	\param _pVars Current value of the dependent variable vector, y(t)
	 *	\param _pDerivs Current value of y'(t)
	 *	\param _pUserData Pointer to user's data*/
	virtual void ResultsHandler( double _dTime, double* _pVars, double* _pDerivs, void* _pUserData );

	// ========== Functions for calling from solver

	/** Calculate residuals. Calls CalculateResiduals.*/
	bool GetResiduals( double _dTime, double* _pVars, double* _pDerivs, double* _pRes );
	/** Handle results. Calls ResultsHandler.*/
	void HandleResults( double _dTime, double* _pVars, double* _pDerivs );
};

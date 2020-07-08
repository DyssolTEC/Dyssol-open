/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CMyNLModel : public CNLModel
{
public:
	size_t m_nVariable0;
	size_t m_nVariable1;

	void CalculateFunctions(double* _pVars, double* _pFunc, void* _pUserData) override;
	void ResultsHandler(double _dTime, double* _pVars, void* _pUserData) override;
};

class CUnit : public CSteadyStateUnit
{
private:
	CMyNLModel m_NLModel;	// Model of nonlinear system of equations
	CNLSolver m_NLSolver;	// Solver of nonlinear system of equations

public:
	CUnit();
	~CUnit();

	void Initialize(double _dTime) override;
	void Simulate(double _dTime) override;
	void Finalize() override;
	void SaveState() override;
	void LoadState() override;
};

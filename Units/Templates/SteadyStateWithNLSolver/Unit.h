/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CMyNLModel : public CNLModel
{
public:
	size_t m_iVariable0{ 0 };
	size_t m_iVariable1{ 0 };

	void CalculateFunctions(double* _vars, double* _func, void* _unit) override;
	void ResultsHandler(double _time, double* _vars, void* _unit) override;
};

class CUnit : public CSteadyStateUnit
{
private:
	CMyNLModel m_NLModel{};	// Model of nonlinear system of equations
	CNLSolver m_NLSolver{};	// Solver of nonlinear system of equations

public:
	void CreateBasicInfo() override;
	void CreateStructure() override;
	void Initialize(double _time) override;
	void Simulate(double _time) override;
	void Finalize() override;
	void SaveState() override;
	void LoadState() override;
};

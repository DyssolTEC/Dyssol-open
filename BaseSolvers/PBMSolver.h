/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BaseSolver.h"
#include "Holdup.h"

class CPBMSolver : public CBaseSolver
{
public:
	CPBMSolver();
	~CPBMSolver() override                          = default;
	CPBMSolver(const CPBMSolver& _other)            = default;
	CPBMSolver(CPBMSolver&& _other)                 = default;
	CPBMSolver& operator=(const CPBMSolver& _other) = default;
	CPBMSolver& operator=(CPBMSolver&& _other)      = default;

	virtual void SetInitialHoldup(const CHoldup* _holdup);
	virtual void SetInputStream(const CStream* _stream);
	virtual void SetOutputStream(double _massFlow);
	virtual void Calculate(double _timeBeg, double _timeEnd);
	virtual CHoldup* GetResult();
};

typedef DECLDIR CPBMSolver* (*CreatePBMSolver)();

/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BaseSolver.h"
#include "Holdup.h"

class CPBMSolver : public CExternalSolver
{
public:
	CPBMSolver();
	virtual ~CPBMSolver() = default;

	virtual void SetInitialHoldup(const CHoldup* _pHoldup);
	virtual void SetInputStream(const CMaterialStream* _pStream);
	virtual void SetOutputStream(double _dMassFlow);
	virtual void Calculate(double _dTStart, double _dTEnd);
	virtual CHoldup* GetResult();
};

typedef DECLDIR CPBMSolver* (*CreatePBMSolver)();

/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DynamicUnit.h"
#include "DAESolver.h"
#include "MaterialStream.h"
#include "DistributionsFunctions.h"
#include <algorithm>

class CUnitDAEModel : public CDAEModel
{
public:
	/// Indexes of state variables for solver ///
	unsigned m_nq0;		// PSD

	std::vector<double> m_vBRate, m_vDRate; // Variables to store current birth- and death-rate

public:
	void CalculateResiduals(double _dTime, double* _pVars, double* _pDers, double* _pRes, void* _pUserData) override;
	void ResultsHandler(double _dTime, double* _pVars, double* _pDerivs, void *_pUserData) override;
};

class CAgglomerator : public CDynamicUnit
{
private:
	CUnitDAEModel m_Model;
	CDAESolver m_Solver;

public:
	CAgglomerationSolver *m_pAggSolver;	// External agglomeration calculator

	CHoldup* m_pHoldup;					// Internal holdup
	CMaterialStream* m_pInStream;		// Inlet
	CMaterialStream* m_pOutStream;		// Outlet

	unsigned m_nClassesNum;				// Number of classes for PSD
	std::vector<double> m_vSizeGrid;	// Size grid for PSD
	std::vector<double> m_vSizes;		// Class sizes for PSD

public:
	CAgglomerator();

	void Initialize(double _dTime) override;
	void SaveState() override;
	void LoadState() override;
	void Simulate(double _dStartTime, double _dEndTime) override;
};

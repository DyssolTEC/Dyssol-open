/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CMyDAEModel : public CDAEModel
{
public:
	size_t m_nMflow;			// Mass flow
	size_t m_nNormMflow;		// Norm for differences in mass flow
	size_t m_nNormT;			// Norm for differences in temperature
	size_t m_nNormP;			// Norm for differences in pressure
	size_t m_nNormPhases;		// Norm for differences in phase fractions
	std::vector<size_t> m_vnNormPhaseCompounds;	// Norm vector for differences in phase compound fractions
	std::vector<size_t> m_vnNormDistr;			// Norm vector for differences in distributions

public:
	void CalculateResiduals(double _dTime, double* _pVars, double* _pDers, double* _pRes, void* _pUserData) override;
	void ResultsHandler(double _dTime, double* _pVars, double* _pDerivs, void* _pUserData) override;
};

class CTimeDelay : public CDynamicUnit
{
public:
	double m_timeDelay;		// Requested time delay

private:
	CMyDAEModel m_Model;		// Model of DAE
	CDAESolver m_Solver;		// Solver of DAE

public:
	void CreateBasicInfo() override;
	void CreateStructure() override;
	void Initialize(double _dTime) override;
	void Simulate(double _dStartTime, double _dEndTime) override;
	void SaveState() override;
	void LoadState() override;
};

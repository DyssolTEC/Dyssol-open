/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CMyDAEModel : public CDAEModel
{
public:
	size_t m_nMassBunker;	// Index for temporary mass of bunker holdup
	size_t m_nMflow_Out;	// Index for outgoing mass flow

	size_t m_nNormMflow;		// Norm for differences in mass flow
	size_t m_nNormT;			// Norm for differences in temperature
	size_t m_nNormP;			// Norm for differences in pressure
	size_t m_nNormCompounds;	// Norm vector for differences in phase compound fractions
	std::vector<size_t> m_vnNormDistr;			// Norm vector for differences in distributions

public:
	void CalculateResiduals(double _time, double* _vars, double* _ders, double* _res, void* _unit) override;
	void ResultsHandler(double _time, double* _vars, double* _ders, void* _unit) override;
};

class CBunker : public CDynamicUnit
{
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

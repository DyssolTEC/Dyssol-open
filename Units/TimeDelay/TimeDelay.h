/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CMyDAEModel : public CDAEModel
{
public:
	size_t m_iMflow{};				// Mass flow
	size_t m_iNormMflow{};			// Norm for differences in mass flow
	size_t m_iNormT{};				// Norm for differences in temperature
	size_t m_iNormP{};				// Norm for differences in pressure
	size_t m_iNormPhases{};			// Norm for differences in phase fractions
	size_t m_iNormPhaseCompounds{};	// Norm vector for differences in phase compound fractions
	size_t m_iNormDistr{};			// Norm vector for differences in distributions

public:
	void CalculateResiduals(double _time, double* _vars, double* _ders, double* _res, void* _unit) override;
	void ResultsHandler(double _time, double* _vars, double* _ders, void* _unit) override;
};

class CTimeDelay : public CDynamicUnit
{
public:
	double m_timeDelay{};		// Requested time delay

private:
	CMyDAEModel m_model;		// Model of DAE
	CDAESolver m_solver;		// Solver of DAE

public:
	void CreateBasicInfo() override;
	void CreateStructure() override;
	void Initialize(double _time) override;
	void Simulate(double _timeBeg, double _timeEnd) override;
	void SaveState() override;
	void LoadState() override;
};

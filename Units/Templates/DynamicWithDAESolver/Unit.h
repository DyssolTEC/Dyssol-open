/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CMyDAEModel : public CDAEModel
{
public:
	size_t m_iVariable0{ 0 };

public:
	void CalculateResiduals(double _time, double* _vars, double* _derivs, double* _res, void* _unit) override;
	void ResultsHandler(double _time, double* _vars, double* _derivs, void* _unit) override;
};

class CUnit : public CDynamicUnit
{
private:
	CMyDAEModel m_Model{};		// Model of DAE
	CDAESolver m_Solver{};		// Solver of DAE

public:
	void CreateBasicInfo() override;
	void CreateStructure() override;
	void Initialize(double _time) override;
	void Simulate(double _timeBeg, double _timeEnd) override;
	void SaveState() override;
	void LoadState() override;
	void Finalize() override;
};

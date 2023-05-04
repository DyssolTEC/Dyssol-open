/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DynamicUnit.h"
#include "DAESolver.h"
#include "Stream.h"

class CUnitDAEModel : public CDAEModel
{
public:
	/// Indexes of state variables for solver ///
	std::vector<size_t> m_iq0{}; // PSD

public:
	void CalculateResiduals(double _time, double* _vars, double* _ders, double* _res, void* _unit) override;
	void ResultsHandler(double _time, double* _vars, double* _ders, void* _unit) override;
};

class CAgglomerator : public CDynamicUnit
{
private:
	CUnitDAEModel m_model{};
	CDAESolver m_solver{};

public:
	CAgglomerationSolver* m_aggSolver{};	// External agglomeration calculator

	CHoldup* m_holdup{};					// Internal holdup
	CMaterialStream* m_inStream{};			// Inlet
	CMaterialStream* m_outStream{};			// Outlet

	size_t m_classesNum{};					// Number of classes for PSD
	std::vector<double> m_sizeGrid;			// Size grid for PSD
	std::vector<double> m_sizes;			// Class sizes for PSD

public:
	void CreateBasicInfo() override;
	void CreateStructure() override;
	void Initialize(double _time) override;
	void SaveState() override;
	void LoadState() override;
	void Simulate(double _timeBeg, double _timeEnd) override;
};

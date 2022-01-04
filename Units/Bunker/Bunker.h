/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CMyDAEModel : public CDAEModel
{
public:
	size_t m_iMass{};			// Index for temporary mass of bunker holdup
	size_t m_iMflowOut{};		// Index for outgoing mass flow

	size_t m_iNormMflow{};		// Norm for differences in mass flow.
	size_t m_iNormT{};			// Norm for differences in temperature.
	size_t m_iNormP{};			// Norm for differences in pressure.
	size_t m_iNormCompounds{};	// Norm vector for differences in phase compound fractions.
	size_t m_iNormDistr{};		// Norm vector for differences in distributions.

public:
	void CalculateResiduals(double _time, double* _vars, double* _ders, double* _res, void* _unit) override;
	void ResultsHandler(double _time, double* _vars, double* _ders, void* _unit) override;
};

class CBunker : public CDynamicUnit
{
	CMyDAEModel m_model{};		// Model of DAE
	CDAESolver m_solver;		// Solver of DAE

public:
	enum EModel : size_t { Adaptive, Constant};
	double m_targetMass{};	// Target mass for bunker.

	CStream* m_inlet{};		// Pointer to inlet stream.
	CStream* m_outlet{};	// Pointer to outlet stream.
	CStream* m_inSolid{};	// Pointer to internal solid stream.
	CStream* m_inBypass{};	// Pointer to internal bypass stream.
	CHoldup* m_holdup{};	// Pointer to holdup.

	size_t m_compoundsNum{};					// Number of defined compounds.
	size_t m_distrsNum;							// Number of defined distributions.
	std::vector<EDistrTypes> m_distributions; 	// All defined distributed parameters.

	CComboUnitParameter *model_bunker{}; // Unit parameter: Bunker model.
	CTDUnitParameter *mass_flow{};       // Unit parameter: Mass flow. Only for constant model

public:
	void CreateBasicInfo() override;
	void CreateStructure() override;
	void Initialize(double _time) override;
	void Simulate(double _timeBeg, double _timeEnd) override;
	void SaveState() override;
	void LoadState() override;
};

/* Copyright (c) 2022, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CUnitDAEModel : public CDAEModel
{
public:
	/// Indices of state variables for solver ///
	size_t m_iAtot{};		// Total surface of all particles in the Granulator.
	size_t m_iMtot{};		// Total mass of all particles in the Granulator.
	size_t m_iMFlowExh{};	// Mass flow of output exhaust gas.
	size_t m_iG{};			// Growth rate.
	size_t m_iMtotq3{};		// Product of Mtot and q3

public:
	void CalculateResiduals(double _time, double* _vars, double* _ders, double* _res, void* _unit) override;
	void ResultsHandler(double _time, double* _vars, double* _ders, void* _unit) override;
};

class CGranulatorSimpleBatch : public CDynamicUnit
{
private:
	CUnitDAEModel m_model;
	CDAESolver m_solver;

public:
	CHoldup* m_holdup{};						// Holdup.
	CMaterialStream* m_inSuspStream{};			// Input of solution.
	CMaterialStream* m_inGasStream{};			// Input gas stream.
	CMaterialStream* m_outExhaustGasStream{};	// Output of solids.

	size_t m_classesNum{};				// Number of classes for PSD.
	std::vector<double> m_sizeGrid;		// Size grid for PSD.
	std::vector<double> m_avgDiam;		// Average values of size grid for PSD.
	std::vector<double> m_classSize;	// Class sizes of size grid for PSD.
	std::vector<double> m_diamRatio;	// Vector: stores ratio of two adjacent diameter values, for calculating G.

public:
	void CreateBasicInfo() override;
	void CreateStructure() override;
	void Initialize(double _time) override;
	void SaveState() override;
	void LoadState() override;
	void Simulate(double _timeBeg, double _timeEnd) override;
};

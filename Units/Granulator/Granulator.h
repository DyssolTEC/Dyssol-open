/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CUnitDAEModel : public CDAEModel
{
public:
	/// Indexes of state variables for solver ///
	size_t m_iAtot{};	// Total surface of all particles in the Granulator
	size_t m_iMtot{};	// Total mass of all particles in the Granulator
	size_t m_iMout{};	// Output mass flow of nuclei
	size_t m_iMdust{};	// Output dust
	size_t m_iG{};		// Growth rate
	std::vector<size_t> m_iq3{}; // PSD

public:
	void CalculateResiduals(double _time, double* _vars, double* _ders, double* _res, void* _unit) override;
	void ResultsHandler(double _time, double* _vars, double* _ders, void* _unit) override;
};

class CSimpleGranulator : public CDynamicUnit
{
private:
	CUnitDAEModel m_model;
	CDAESolver m_solver;

public:
	CHoldup* m_holdup{};				// Holdup
	CMaterialStream* m_inSolutStream{};	// Input of solution
	CMaterialStream* m_inNuclStream{};	// Input of nuclei
	CMaterialStream* m_inGasStream{};	// Input gas stream
	CMaterialStream* m_outNuclStream{};	// Output of solids
	CMaterialStream* m_outDustStream{};	// Output of solids

	size_t m_classesNum{};				// Number of classes for PSD
	std::vector<double> m_sizeGrid;		// Size grid for PSD
	std::vector<double> m_averDiam;		// Average values of size grid for PSD
	std::vector<double> m_classSize;	// Class sizes of size grid for PSD
	double m_initMass{};				// Initial mass in the Granulator
	std::vector<double> m_preCalc;		// Vector of precalculated values

public:
	void CreateBasicInfo() override;
	void CreateStructure() override;
	void Initialize(double _time) override;
	void SaveState() override;
	void LoadState() override;
	void Simulate(double _timeBeg, double _timeEnd) override;
};

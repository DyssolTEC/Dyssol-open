/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DynamicUnit.h"
#include "DAESolver.h"
#include "MaterialStream.h"

class CUnitDAEModel : public CDAEModel
{
public:
	/// Indexes of state variables for solver ///
	size_t m_nAtot;		// Total surface of all particles in the Granulator
	size_t m_nMtot;		// Total mass of all particles in the Granulator
	size_t m_nMout;		// Output mass flow of nuclei
	size_t m_nMdust;	// Output dust
	size_t m_nG;		// Growth rate
	size_t m_nq3;		// PSD

public:
	void CalculateResiduals(double _dTime, double* _pVars, double* _pDers, double* _pRes, void* _pUserData) override;
	void ResultsHandler(double _dTime, double* _pVars, double* _pDerivs, void *_pUserData) override;
};

class CSimpleGranulator : public CDynamicUnit
{
private:
	CUnitDAEModel m_Model;
	CDAESolver m_Solver;

public:
	CHoldup* m_pHoldup;					// Holdup
	CMaterialStream* m_pInSuspStream;	// Input of suspension
	CMaterialStream* m_pInNuclStream;	// Input of nuclei
	CMaterialStream* m_pInGasStream;	// Input gas stream
	CMaterialStream* m_pOutNuclStream;	// Output of solids
	CMaterialStream* m_pOutDustStream;	// Output of solids

	unsigned m_nClassesNum;				// Number of classes for PSD
	std::vector<double> m_vSizeGrid;	// Size grid for PSD
	std::vector<double> m_vAverDiam;	// Average values of size grid for PSD
	std::vector<double> m_vClassSize;	// Class sizes of size grid for PSD
	double m_dInitMass;					// Initial mass in the Granulator
	std::vector<double> m_dPreCalc;		// Vector of precalculated values

public:
	CSimpleGranulator();

	void Initialize(double _dTime) override;
	void SaveState() override;
	void LoadState() override;
	void Simulate(double _dStartTime, double _dEndTime) override;
};

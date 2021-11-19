/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "ScreenMulti.h"
#include "StringFunctions.h"
#include <cmath>

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CScreenPlitt();
}

void CScreenPlitt::CreateBasicInfo()
{
	SetUnitName("ScreenMulti");
	SetAuthorName("SPE TUHH, TU Bergakademit Freiberg IART");
	SetUniqueID("AAAFADC1877B46629B07A456C7FA22A1");
}

void CScreenPlitt::CreateStructure()
{
	AddPort("Input", EUnitPort::INPUT);
	AddPort("Coarse_D1", EUnitPort::OUTPUT);
	AddPort("Coarse_D2", EUnitPort::OUTPUT);
	AddPort("Coarse_D3", EUnitPort::OUTPUT);
	AddPort("Coarse_D4", EUnitPort::OUTPUT);
	AddPort("Coarse_D5", EUnitPort::OUTPUT);
	AddPort("Fines",     EUnitPort::OUTPUT);

	AddTDParameter("Xcut_D1" , 0.002, "m", "Cut size of the classification model, deck 1", 0     );
	AddTDParameter("Alpha_D1", 8,     "-", "Sharpness of separation, deck 1"             , 0, 100);
	AddTDParameter("Xcut_D2" , 0.0,   "m", "Cut size of the classification model, deck 2", 0     );
	AddTDParameter("Alpha_D2", 8,     "-", "Sharpness of separation, deck 2"             , 0, 100);
	AddTDParameter("Xcut_D3" , 0.0,   "m", "Cut size of the classification model, deck 3", 0     );
	AddTDParameter("Alpha_D3", 8,     "-", "Sharpness of separation, deck 3"             , 0, 100);
	AddTDParameter("Xcut_D4" , 0.0,   "m", "Cut size of the classification model, deck 4", 0     );
	AddTDParameter("Alpha_D4", 8,     "-", "Sharpness of separation, deck 4"             , 0, 100);
	AddTDParameter("Xcut_D5" , 0.0,   "m", "Cut size of the classification model, deck 5", 0     );
	AddTDParameter("Alpha_D5", 8,     "-", "Sharpness of separation, deck 5"             , 0, 100);
}

void CScreenPlitt::Initialize(double _time)
{
	/// Check flowsheet parameters ///
	if (!IsPhaseDefined(EPhase::SOLID))		RaiseError("Solid phase has not been defined.");
	if (!IsDistributionDefined(DISTR_SIZE))	RaiseError("Size distribution has not been defined.");

	/// Get pointers to streams ///
	m_inStream      = GetPortStream("Input");
	m_outStreamC_D1 = GetPortStream("Coarse_D1");
	m_outStreamC_D2 = GetPortStream("Coarse_D2");
	m_outStreamC_D3 = GetPortStream("Coarse_D3");
	m_outStreamC_D4 = GetPortStream("Coarse_D4");
	m_outStreamC_D5 = GetPortStream("Coarse_D5");
	m_outStreamF    = GetPortStream("Fines");

	/// Get PSD grid parameters ///
	m_classesNum = GetClassesNumber(DISTR_SIZE);
	m_sizeGrid   = GetNumericGrid(DISTR_SIZE);
	m_meanDiams  = GetClassesMeans(DISTR_SIZE); // Classes means of PSD

	/// Create and initialize transformation matrices ///
	m_transformCoarse_D1.Clear();
	m_transformCoarse_D2.Clear();
	m_transformCoarse_D3.Clear();
	m_transformCoarse_D4.Clear();
	m_transformCoarse_D5.Clear();
	m_transformFines.Clear();
	m_transformCoarse_D1.SetDimensions(DISTR_SIZE, (unsigned)m_classesNum);
	m_transformCoarse_D2.SetDimensions(DISTR_SIZE, (unsigned)m_classesNum);
	m_transformCoarse_D3.SetDimensions(DISTR_SIZE, (unsigned)m_classesNum);
	m_transformCoarse_D4.SetDimensions(DISTR_SIZE, (unsigned)m_classesNum);
	m_transformCoarse_D5.SetDimensions(DISTR_SIZE, (unsigned)m_classesNum);
	m_transformFines.SetDimensions(DISTR_SIZE, (unsigned)m_classesNum);

	m_outStreamC_D1_D2 = AddStream("D1-D2");
	m_outStreamC_D2_D3 = AddStream("D2-D3");
	m_outStreamC_D3_D4 = AddStream("D3-D4");
	m_outStreamC_D4_D5 = AddStream("D4-D5");
}

void CScreenPlitt::CalculateStream(double _time, CMaterialStream * in, CMaterialStream * outG, CMaterialStream * outF, const char * Xcut, const char * Alpha)
{
	outG->CopyFromStream(_time, in);
	outF->CopyFromStream(_time, in);

	// Get parameters
	const double d50   = GetTDParameterValue(Xcut, _time);
	const double alpha = GetTDParameterValue(Alpha, _time);

	if (d50 > 0)
	{

		CTransformMatrix m_transformCoarse (DISTR_SIZE, (unsigned)m_classesNum);
		CTransformMatrix m_transformFines (DISTR_SIZE, (unsigned)m_classesNum);
		// Set values to transformations matrices
		double factor = 0;
		const auto PSD = in->GetDistribution(_time, DISTR_SIZE);
		for (unsigned i = 0; i < m_classesNum; ++i)
		{
			const double val = 1 - std::exp(-0.693 * std::pow(m_meanDiams[i] / d50, alpha));
			factor += val * PSD[i];
			m_transformCoarse.SetValue(i, i, val);
			m_transformFines.SetValue(i, i, 1 - val);
		}

		// transform matrices
		outG->ApplyTM(_time, m_transformCoarse);
		outF->ApplyTM(_time, m_transformFines);

		// recalculate masses
		const double massFlowIn = in->GetMassFlow(_time);
		outG->SetMassFlow(_time, massFlowIn * factor);
		outF->SetMassFlow(_time, massFlowIn * (1 - factor));
	}
	else
	{
		outG->SetMassFlow(_time, 0.0);
	}
}

void CScreenPlitt::Simulate(double _time)
{
	CalculateStream(_time, m_inStream, m_outStreamC_D1, m_outStreamC_D1_D2, "Xcut_D1", "Alpha_D1");
	CalculateStream(_time, m_outStreamC_D1_D2, m_outStreamC_D2, m_outStreamC_D2_D3, "Xcut_D2", "Alpha_D2");
	CalculateStream(_time, m_outStreamC_D2_D3, m_outStreamC_D3, m_outStreamC_D3_D4, "Xcut_D3", "Alpha_D3");
	CalculateStream(_time, m_outStreamC_D3_D4, m_outStreamC_D4, m_outStreamC_D4_D5, "Xcut_D4", "Alpha_D4");
	CalculateStream(_time, m_outStreamC_D4_D5, m_outStreamC_D5, m_outStreamF, "Xcut_D5", "Alpha_D5");
}

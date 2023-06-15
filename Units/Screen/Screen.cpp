/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Screen.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CScreen();
}

//////////////////////////////////////////////////////////////////////////
/// Unit

void CScreen::CreateBasicInfo()
{
	/// Basic unit's info ///
	SetUnitName("Screen");
	SetAuthorName("SPE TUHH");
	SetUniqueID("F231006AE5AA46C6978B2DB563F31119");
	SetHelpLink("003_models/unit_screen.html");
}

void CScreen::CreateStructure()
{
	/// Add ports ///
	AddPort("Input",  EUnitPort::INPUT);
	AddPort("Coarse", EUnitPort::OUTPUT);
	AddPort("Fine",   EUnitPort::OUTPUT);

	/// Add unit parameters ///
	AddComboParameter("Model", EModels::Plitt, { EModels::Plitt, EModels::Molerus, EModels::Teipel, EModels::Probability }, { "Plitt", "Molerus & Hoffmann", "Teipel & Hennig", "Probability" }, "Classification model");
	AddTDParameter("Xcut"     ,	0.002 , "m", "Cut size of the classification model"                , 0     );
	AddTDParameter("Alpha"    ,	8     , "-", "Sharpness of separation"                             , 0, 100);
	AddTDParameter("Beta"     ,	0.5   , "-", "Sharpness of separation 2"                           , 0, 100);
	AddTDParameter("Offset"   ,	0.2   , "-", "Separation offset"                                   , 0, 1  );
	AddTDParameter("Mean"     ,	0.001 , "m", "Mean value of the normal output distribution"        , 0     );
	AddTDParameter("Deviation", 0.0001, "m", "Standard deviation of the normal output distribution", 0     );

	/// Group unit parameters ///
	AddParametersToGroup("Model", "Plitt",				{ "Xcut", "Alpha"                   });
	AddParametersToGroup("Model", "Molerus & Hoffmann",	{ "Xcut", "Alpha"                   });
	AddParametersToGroup("Model", "Teipel & Hennig",    { "Xcut", "Alpha", "Beta", "Offset" });
	AddParametersToGroup("Model", "Probability",        { "Mean", "Deviation"               });
}

void CScreen::Initialize(double _time)
{
	/// Check flowsheet parameters ///
	if (!IsPhaseDefined(EPhase::SOLID))		RaiseError("Solid phase has not been defined.");
	if (!IsDistributionDefined(DISTR_SIZE))	RaiseError("Size distribution has not been defined.");

	/// Get pointers to streams ///
	m_inlet   = GetPortStream("Input");
	m_outletC = GetPortStream("Coarse");
	m_outletF = GetPortStream("Fine");

	/// Get PSD grid parameters ///
	m_classesNumber = GetClassesNumber(DISTR_SIZE);
	m_grid          = GetNumericGrid(DISTR_SIZE);
	m_diameters     = GetClassesMeans(DISTR_SIZE);

	/// Create and initialize transformation matrices ///
	m_transformC.Clear();
	m_transformF.Clear();
	m_transformC.SetDimensions(DISTR_SIZE, (unsigned)m_classesNumber);
	m_transformF.SetDimensions(DISTR_SIZE, (unsigned)m_classesNumber);

	/// Get selected classification model ///
	m_model = V2E<EModels>(GetComboParameterValue("Model"));
}

void CScreen::Simulate(double _time)
{
	m_outletC->CopyFromStream(_time, m_inlet);
	m_outletF->CopyFromStream(_time, m_inlet);

	const double massFactor = CreateTransformMatrix(_time);
	if (massFactor == -1.0) return; // cannot calculate transformation matrix

	// apply transformation matrices
	m_outletC->ApplyTM(_time, m_transformC);
	m_outletF->ApplyTM(_time, m_transformF);

	// recalculate and apply mass flows
	const double massFlowIn = m_inlet->GetMassFlow(_time);
	m_outletC->SetMassFlow(_time, massFlowIn * massFactor);
	m_outletF->SetMassFlow(_time, massFlowIn * (1 - massFactor));
}

double CScreen::CreateTransformMatrix(double _time)
{
	switch (m_model)
	{
	case EModels::Plitt:		return CreateTransformMatrixPlitt(_time);
	case EModels::Molerus:		return CreateTransformMatrixMolerus(_time);
	case EModels::Teipel:		return CreateTransformMatrixTeipel(_time);
	case EModels::Probability:	return CreateTransformMatrixProbability(_time);
	}
	return -1;
}

double CScreen::CreateTransformMatrixPlitt(double _time)
{
	// get parameters
	const double xcut  = GetTDParameterValue("Xcut", _time);
	const double alpha = GetTDParameterValue("Alpha", _time);

	// check parameters
	if (xcut == 0.0)	RaiseError("Parameter 'Xcut' may not be equal to 0");

	// return if any error occurred
	if (HasError()) return -1;

	// calculate transformations matrices
	double factor = 0;
	const std::vector<double> psd = m_inlet->GetDistribution(_time, DISTR_SIZE);
	for (unsigned i = 0; i < psd.size(); ++i)
	{
		const double value = 1 - std::exp(-0.693 * std::pow(m_diameters[i] / xcut, alpha));
		factor += value * psd[i];
		m_transformC.SetValue(i, i, value);
		m_transformF.SetValue(i, i, 1 - value);
	}
	return factor;
}

double CScreen::CreateTransformMatrixMolerus(double _time)
{
	// get parameters
	const double xcut  = GetTDParameterValue("Xcut", _time);
	const double alpha = GetTDParameterValue("Alpha", _time);

	// Check parameters
	if (xcut == 0.0)	RaiseError("Parameter 'Xcut' may not be equal to 0");

	// return if any error occurred
	if (HasError()) return -1;

	// calculate transformations matrices
	double factor = 0;
	const std::vector<double> psd = m_inlet->GetDistribution(_time, DISTR_SIZE);
	for (unsigned i = 0; i < psd.size(); ++i)
	{
		const double value = 1 / (1 + std::pow(xcut / m_diameters[i], 2.) * std::exp(alpha * (1 - std::pow(m_diameters[i] / xcut, 2.0))));
		factor += value * psd[i];
		m_transformC.SetValue(i, i, value);
		m_transformF.SetValue(i, i, 1 - value);
	}
	return factor;
}

double CScreen::CreateTransformMatrixTeipel(double _time)
{
	// get parameters
	const double xcut   = GetTDParameterValue("Xcut", _time);
	const double alpha  = GetTDParameterValue("Alpha", _time);
	const double beta   = GetTDParameterValue("Beta", _time);
	const double offset = GetTDParameterValue("Offset", _time);

	// check parameters
	if (xcut == 0.0)	RaiseError("Parameter 'Xcut' may not be equal to 0");

	// return if any error occurred
	if (HasError()) return -1;

	// calculate transformations matrices
	double factor = 0;
	const std::vector<double> psd = m_inlet->GetDistribution(_time, DISTR_SIZE);
	for (unsigned i = 0; i < psd.size(); ++i)
	{
		const double value = (1 - std::pow(1 + 3 * std::pow(m_diameters[i] / xcut, (m_diameters[i] / xcut + alpha) * beta), -0.5)) * (1 - offset) + offset;
		factor += value * psd[i];
		m_transformC.SetValue(i, i, value);
		m_transformF.SetValue(i, i, 1 - value);
	}
	return factor;
}

double CScreen::CreateTransformMatrixProbability(double _time)
{
	// get parameters
	const double mu    = GetTDParameterValue("Mean", _time);
	const double sigma = GetTDParameterValue("Deviation", _time);

	// check parameters
	if (sigma == 0.0)	RaiseError("Parameter 'Deviation' may not be equal to 0");

	// return if any error occurred.
	if (HasError()) return -1;

	// calculate transformations matrices
	double factor = 0;
	const std::vector<double> psd = m_inlet->GetDistribution(_time, DISTR_SIZE);
	double totalSum = 0;
	for (unsigned i = 0; i < psd.size(); ++i)
		totalSum += std::exp(-std::pow(m_diameters[i] - mu, 2) / (2 * sigma * sigma));
	double currSum = 0;
	for (unsigned i = 0; i < psd.size(); ++i)
	{
		currSum += std::exp(-std::pow(m_diameters[i] - mu, 2) / (2 * sigma * sigma));
		const double value = currSum / totalSum;
		factor += value * psd[i];
		m_transformC.SetValue(i, i, value);
		m_transformF.SetValue(i, i, 1 - value);
	}

	return factor;
}

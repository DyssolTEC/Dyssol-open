/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "ScreenMultideck.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CScreenMultideck();
}

void CScreenMultideck::CreateBasicInfo()
{
	SetUnitName("Screen Multi-deck");
	SetAuthorName("SPE TUHH, TU Bergakademie Freiberg IART");
	SetUniqueID("AAAFADC1877B46629B07A456C7FA22A1");
}

void CScreenMultideck::CreateStructure()
{
	constexpr size_t decksNumber = 5;
	m_decks.resize(decksNumber);

	/// Ports
	m_portIn = AddPort("Input", EUnitPort::INPUT);
	m_portsCoarse.clear();
	for (size_t i = 0; i < decksNumber; ++i)
		m_portsCoarse.push_back(AddPort("Coarse " + std::to_string(i + 1), EUnitPort::OUTPUT));
	m_portFines = AddPort("Fines",     EUnitPort::OUTPUT);

	/// Unit parameters
	for (size_t i = 0; i < m_decks.size(); ++i)
	{
		// deck index as string
		const auto I = " " + std::to_string(i + 1);

		// add unit parameters
		m_decks[i].model     = AddComboParameter("Model" + I, None, { None, Plitt, Molerus, Teipel, Probability }, { "-", "Plitt", "Molerus & Hoffmann", "Teipel & Hennig", "Probability" }, "Classification model");
		m_decks[i].xCut      = AddTDParameter("Cut size"  + I, 0.002 , "m", "Cut size of the classification model, deck"                 + I, 0     );
		m_decks[i].alpha     = AddTDParameter("Alpha"     + I, 8     , "-", "Sharpness of separation, deck"                              + I, 0, 100);
		m_decks[i].beta      = AddTDParameter("Beta"      + I, 0.5   , "-", "Sharpness of separation 2, deck"                            + I, 0, 100);
		m_decks[i].offset    = AddTDParameter("Offset"    + I, 0.2   , "-", "Separation offset, deck"                                    + I, 0, 1  );
		m_decks[i].mean      = AddTDParameter("Mean"      + I, 0.001 , "m", "Mean value of the normal output distribution, deck"         + I, 0     );
		m_decks[i].deviation = AddTDParameter("Deviation" + I, 0.0001, "m", "Standard deviation of the normal output distribution, deck" + I, 0     );

		// group unit parameters
		AddParametersToGroup("Model" + I, "Plitt"             , { "Cut size" + I, "Alpha" + I });
		AddParametersToGroup("Model" + I, "Molerus & Hoffmann", { "Cut size" + I, "Alpha" + I });
		AddParametersToGroup("Model" + I, "Teipel & Hennig"   , { "Cut size" + I, "Alpha" + I, "Beta" + I, "Offset" + I });
		AddParametersToGroup("Model" + I, "Probability"       , { "Mean" + I, "Deviation" + I });
	}

	/// Internal streams
	for (size_t i = 0; i < decksNumber - 1; ++i)
	{
		// create internal fines stream for each deck except the last one.
		m_decks[i].streamOutF = AddStream("Deck " + std::to_string(i + 1) + " fine");
	}
}

void CScreenMultideck::Initialize(double _time)
{
	/// Check flowsheet parameters
	if (!IsPhaseDefined(EPhase::SOLID))		RaiseError("Solid phase has not been defined.");
	if (!IsDistributionDefined(DISTR_SIZE))	RaiseError("Size distribution has not been defined.");

	/// Get and set pointers to streams
	for (size_t i = 0; i < m_decks.size(); ++i)
	{
		m_decks[i].streamIn = i != 0 ? m_decks[i - 1].streamOutF : m_portIn->GetStream();
		m_decks[i].streamOutC = m_portsCoarse[i]->GetStream();
	}
	m_decks.back().streamOutF = m_portFines->GetStream();

	/// Get PSD grid parameters
	m_classesNum = GetClassesNumber(DISTR_SIZE);
	m_grid       = GetNumericGrid(DISTR_SIZE);
	m_diameters  = GetClassesMeans(DISTR_SIZE); // Classes means of PSD

	/// Create and initialize transformation matrices
	m_transformC.Clear();
	m_transformF.Clear();
	m_transformC.SetDimensions(DISTR_SIZE, static_cast<unsigned>(m_classesNum));
	m_transformF.SetDimensions(DISTR_SIZE, static_cast<unsigned>(m_classesNum));
}


void CScreenMultideck::Simulate(double _time)
{
	for (const auto& deck : m_decks)
	{
		deck.streamOutC->CopyFromStream(_time, deck.streamIn);
		deck.streamOutF->CopyFromStream(_time, deck.streamIn);

		// if deck is disabled - just propagate stream
		if (static_cast<EModel>(deck.model->GetValue()) == None)
		{
			deck.streamOutC->SetMassFlow(_time, 0.0);
			continue;
		}

		const double massFactor = CreateTransformMatrix(_time, deck);
		if (massFactor == -1.0) return; // cannot calculate transformation matrix

		// apply transformation matrices
		deck.streamOutC->ApplyTM(_time, m_transformC);
		deck.streamOutF->ApplyTM(_time, m_transformF);

		// recalculate and apply mass flows
		const double massFlowIn = deck.streamIn->GetMassFlow(_time);
		deck.streamOutC->SetMassFlow(_time, massFlowIn * massFactor);
		deck.streamOutF->SetMassFlow(_time, massFlowIn * (1 - massFactor));
	}
}

double CScreenMultideck::CreateTransformMatrix(double _time, const SDeck& _deck)
{
	for (const auto& deck : m_decks)
		switch(static_cast<EModel>(deck.model->GetValue()))
		{
		case Plitt:			return CreateTransformMatrixPlitt(_time, _deck);
		case Molerus:		return CreateTransformMatrixMolerus(_time, _deck);
		case Teipel:		return CreateTransformMatrixTeipel(_time, _deck);
		case Probability:	return CreateTransformMatrixProbability(_time, _deck);
		case None: break;
		}
	return -1;
}

double CScreenMultideck::CreateTransformMatrixPlitt(double _time, const SDeck& _deck)
{
	// get parameters
	const double xcut  = _deck.xCut->GetValue(_time);
	const double alpha = _deck.alpha->GetValue(_time);

	// check parameters
	if (xcut == 0.0)	RaiseError("Parameter '" + _deck.xCut->GetName() + "' may not be equal to 0");

	// return if any error occurred
	if (HasError()) return -1;

	// calculate transformations matrices
	double factor = 0;
	const std::vector<double> psd = _deck.streamIn->GetDistribution(_time, DISTR_SIZE);
	for (unsigned i = 0; i < psd.size(); ++i)
	{
		const double value = 1 - std::exp(-0.693 * std::pow(m_diameters[i] / xcut, alpha));
		factor += value * psd[i];
		m_transformC.SetValue(i, i, value);
		m_transformF.SetValue(i, i, 1 - value);
	}
	return factor;
}

double CScreenMultideck::CreateTransformMatrixMolerus(double _time, const SDeck& _deck)
{
	// get parameters
	const double xcut  = _deck.xCut->GetValue(_time);
	const double alpha = _deck.alpha->GetValue(_time);

	// Check parameters
	if (xcut == 0.0)	RaiseError("Parameter '" + _deck.xCut->GetName() + "' may not be equal to 0");

	// return if any error occurred
	if (HasError()) return -1;

	// calculate transformations matrices
	double factor = 0;
	const std::vector<double> psd = _deck.streamIn->GetDistribution(_time, DISTR_SIZE);
	for (unsigned i = 0; i < psd.size(); ++i)
	{
		const double value = 1 / (1 + std::pow(xcut / m_diameters[i], 2.) * std::exp(alpha * (1 - std::pow(m_diameters[i] / xcut, 2.0))));
		factor += value * psd[i];
		m_transformC.SetValue(i, i, value);
		m_transformF.SetValue(i, i, 1 - value);
	}
	return factor;
}

double CScreenMultideck::CreateTransformMatrixTeipel(double _time, const SDeck& _deck)
{
	// get parameters
	const double xcut   = _deck.xCut->GetValue(_time);
	const double alpha  = _deck.alpha->GetValue(_time);
	const double beta   = _deck.beta->GetValue(_time);
	const double offset = _deck.offset->GetValue(_time);

	// check parameters
	if (xcut == 0.0)	RaiseError("Parameter '" + _deck.xCut->GetName() + "' may not be equal to 0");

	// return if any error occurred
	if (HasError()) return -1;

	// calculate transformations matrices
	double factor = 0;
	const std::vector<double> psd = _deck.streamIn->GetDistribution(_time, DISTR_SIZE);
	for (unsigned i = 0; i < psd.size(); ++i)
	{
		const double value = (1 - std::pow(1 + 3 * std::pow(m_diameters[i] / xcut, (m_diameters[i] / xcut + alpha) * beta), -0.5)) * (1 - offset) + offset;
		factor += value * psd[i];
		m_transformC.SetValue(i, i, value);
		m_transformF.SetValue(i, i, 1 - value);
	}
	return factor;
}

double CScreenMultideck::CreateTransformMatrixProbability(double _time, const SDeck& _deck)
{
	// get parameters
	const double mu    = _deck.mean->GetValue(_time);
	const double sigma = _deck.deviation->GetValue(_time);

	// check parameters
	if (sigma == 0.0)	RaiseError("Parameter '" + _deck.deviation->GetName() + "' may not be equal to 0");

	// return if any error occurred.
	if (HasError()) return -1;

	// calculate transformations matrices
	double factor = 0;
	const std::vector<double> psd = _deck.streamIn->GetDistribution(_time, DISTR_SIZE);
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

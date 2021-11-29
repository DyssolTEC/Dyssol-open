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
	m_decks.clear();

	/// Ports
	m_portIn = AddPort("Input", EUnitPort::INPUT);
	m_portsCoarse.clear();
	for (size_t i = 0; i < decksNumber; ++i)
		m_portsCoarse.push_back(AddPort("Coarse " + std::to_string(i + 1), EUnitPort::OUTPUT));
	m_portFines = AddPort("Fines",     EUnitPort::OUTPUT);

	/// Unit parameters
	for (size_t i = 0; i < decksNumber; ++i)
	{
		auto& p = m_decks.emplace_back();
		p.xCut  = AddTDParameter("Cut size " + std::to_string(i + 1), !i ? 0.002 : 0.0, "m", "Cut size of the classification model, deck " + std::to_string(i + 1), 0     );
		p.alpha = AddTDParameter("Alpha "    + std::to_string(i + 1), 8               , "-", "Sharpness of separation, deck "              + std::to_string(i + 1), 0, 100);
	}

	/// Internal streams
	for (size_t i = 0; i < decksNumber - 1; ++i)
	{
		// Create internal fines stream for each deck except the last one.
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
	m_sizeGrid   = GetNumericGrid(DISTR_SIZE);
	m_meanDiams  = GetClassesMeans(DISTR_SIZE); // Classes means of PSD

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

		// get parameters
		const double d50   = deck.xCut->GetValue(_time);
		const double alpha = deck.alpha->GetValue(_time);

		if (d50 > 0)
		{
			// set values to transformations matrices
			double factor = 0;
			const auto PSD = deck.streamIn->GetDistribution(_time, DISTR_SIZE);
			for (unsigned i = 0; i < m_classesNum; ++i)
			{
				const double val = 1 - std::exp(-0.693 * std::pow(m_meanDiams[i] / d50, alpha));
				factor += val * PSD[i];
				m_transformC.SetValue(i, i, val);
				m_transformF.SetValue(i, i, 1 - val);
			}

			// apply transformation matrices
			deck.streamOutC->ApplyTM(_time, m_transformC);
			deck.streamOutF->ApplyTM(_time, m_transformF);

			// recalculate masses
			const double massFlowIn = deck.streamIn->GetMassFlow(_time);
			deck.streamOutC->SetMassFlow(_time, massFlowIn * factor);
			deck.streamOutF->SetMassFlow(_time, massFlowIn * (1 - factor));
		}
		else
		{
			deck.streamOutC->SetMassFlow(_time, 0.0);
		}
	}
}

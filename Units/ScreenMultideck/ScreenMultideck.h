/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CScreenMultideck : public CSteadyStateUnit
{
	// Unit parameters for each deck.
	struct SDeck
	{
		CTDUnitParameter* xCut{};		// Cut size.
		CTDUnitParameter* alpha{};		// Separation sharpness.

		CMaterialStream* streamIn{};	// Deck inlet stream.
		CMaterialStream* streamOutC{};	// Deck coarse outlet stream.
		CMaterialStream* streamOutF{};	// Deck fine outlet stream.
	};

	CUnitPort* m_portIn{};					// Inlet port.
	CUnitPort* m_portFines{};				// Fines outlet port.
	std::vector<CUnitPort*> m_portsCoarse;	// Coarse outlet ports for each deck.

	CTransformMatrix m_transformC;			// Transformation matrix for coarse.
	CTransformMatrix m_transformF;			// Transformation matrix for fine.

	std::vector<SDeck> m_decks;				// Parameters for each deck.

	size_t m_classesNum{};					// Number of classes for PSD
	std::vector<double> m_sizeGrid;			// Size grid for PSD
	std::vector<double> m_meanDiams;		// Means of size grid - particle diameters

public:
	void CreateBasicInfo() override;
	void CreateStructure() override;
	void Initialize(double _time) override;
	void Simulate(double _time) override;
};

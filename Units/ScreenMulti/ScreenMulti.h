/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CScreenPlitt : public CSteadyStateUnit
{
	CTransformMatrix m_transformCoarse_D1,
	                 m_transformCoarse_D2,
					 m_transformCoarse_D3,
					 m_transformCoarse_D4,
					 m_transformCoarse_D5,
					 m_transformFines;				                    // Transformation matrices
	CMaterialStream *m_inStream{},
	                *m_outStreamC_D1{},
	                *m_outStreamC_D2{},
	                *m_outStreamC_D3{},
	                *m_outStreamC_D4{},
	                *m_outStreamC_D5{},
					*m_outStreamF{};	                                // Streams
	CMaterialStream *m_outStreamC_D1_D2{},
					*m_outStreamC_D2_D3{},
					*m_outStreamC_D3_D4{},
					*m_outStreamC_D4_D5{};                              // Streams between decks

	size_t m_classesNum{};												// Number of classes for PSD
	std::vector<double> m_sizeGrid;										// Size grid for PSD
	std::vector<double> m_meanDiams;									// Means of size grid - particle diameters

public:
	void CreateBasicInfo() override;
	void CreateStructure() override;
	void Initialize(double _time) override;
	void Simulate(double _time) override;
};

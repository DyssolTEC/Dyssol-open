/* Copyright (c) 2021, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CScreenMultideck : public CSteadyStateUnit
{
	enum EModel : size_t
	{
		None, Plitt, Molerus, Teipel, Probability
	};

	// Unit parameters for each deck.
	struct SDeck
	{
		CComboUnitParameter* model{};	// Unit parameter: Classification model.
		CTDUnitParameter* xCut{};		// Unit parameter: Cut size.
		CTDUnitParameter* alpha{};		// Unit parameter: Separation sharpness.
		CTDUnitParameter* beta{};		// Unit parameter: Separation sharpness.
		CTDUnitParameter* offset{};		// Unit parameter: Separation offset.
		CTDUnitParameter* mean{};		// Unit parameter: Mean value.
		CTDUnitParameter* deviation{};	// Unit parameter: Standard deviation.

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
	std::vector<double> m_grid;			// Size grid for PSD
	std::vector<double> m_diameters;		// Means of size grid - particle diameters

public:
	void CreateBasicInfo() override;
	void CreateStructure() override;
	void Initialize(double _time) override;
	void Simulate(double _time) override;

private:
	double CreateTransformMatrix(double _time, const SDeck& _deck);

	double CreateTransformMatrixPlitt(double _time, const SDeck& _deck);
	double CreateTransformMatrixMolerus(double _time, const SDeck& _deck);
	double CreateTransformMatrixTeipel(double _time, const SDeck& _deck);
	double CreateTransformMatrixProbability(double _time, const SDeck& _deck);
};

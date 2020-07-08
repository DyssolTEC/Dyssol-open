/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CScreen : public CSteadyStateUnit
{
	enum EModels : size_t
	{
		Plitt, Molerus, Teipel, Probability
	};

	CMaterialStream *m_inlet, *m_outletC, *m_outletF;	// Streams.
	CTransformMatrix m_transformC, m_transformF;		// Transformation matrices for coarse and fine.
	unsigned m_classesNumber;							// Number of PSD classes.
	std::vector<double> m_grid;							// Diameter grid for PSD.
	std::vector<double> m_diameters;					// Mean diameters for each grid class.
	EModels m_model;									// Chosen classification model.

public:
	CScreen();

	void Initialize(double _time) override;
	void Simulate(double _time) override;

private:
	double CreateTransformMatrix(double _time);

	double CreateTransformMatrixPlitt(double _time);
	double CreateTransformMatrixMolerus(double _time);
	double CreateTransformMatrixTeipel(double _time);
	double CreateTransformMatrixProbability(double _time);
};

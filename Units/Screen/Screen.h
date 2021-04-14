/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CScreen : public CSteadyStateUnit
{
	enum EModels : size_t
	{
		Plitt, Molerus, Teipel, Probability
	};

	CMaterialStream* m_inlet{};			// Inlet stream.
	CMaterialStream* m_outletC{};		// Outlet coarse stream.
	CMaterialStream* m_outletF{};		// Outlet fine stream.
	CTransformMatrix m_transformC;		// Transformation matrices for coarse.
	CTransformMatrix m_transformF;		// Transformation matrices for fine.
	size_t m_classesNumber{};			// Number of PSD classes.
	std::vector<double> m_grid;			// Diameter grid for PSD.
	std::vector<double> m_diameters;	// Mean diameters for each grid class.
	EModels m_model{ Plitt };			// Chosen classification model.

public:
	void CreateBasicInfo() override;
	void CreateStructure() override;
	void Initialize(double _time) override;
	void Simulate(double _time) override;

private:
	double CreateTransformMatrix(double _time);

	double CreateTransformMatrixPlitt(double _time);
	double CreateTransformMatrixMolerus(double _time);
	double CreateTransformMatrixTeipel(double _time);
	double CreateTransformMatrixProbability(double _time);
};

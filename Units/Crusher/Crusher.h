/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CCrusher : public CSteadyStateUnit
{
	enum EModels : size_t
	{
		BondNormal, BondBimodal, Cone, Const
	};

	CMaterialStream* m_inlet{ nullptr };	// Pointer to inlet stream.
	CMaterialStream* m_outlet{ nullptr };	// Pointer to outlet stream.
	CMaterialStream* m_internal{ nullptr };	// Pointer to internal stream.
	CTransformMatrix m_transform;			// Transformation matrix.
	size_t m_classesNumber{};				// Number of PSD classes.
	std::vector<double> m_grid;				// Diameter grid for PSD.
	std::vector<double> m_diameters;		// Mean diameters for each grid class.
	std::vector<std::string> m_compounds;	// List of keys for defined compounds.
	EModels m_model{ Const };				// Chosen crusher model.

public:
	void CreateBasicInfo() override;
	void CreateStructure() override;
	void Initialize(double _time) override;
	void Simulate(double _time) override;

	void InitializeBondNormal(double _time);
	void SimulateBondNormal(double _time);

	void InitializeBondBimodal(double _time);
	void SimulateBondBimodal(double _time);

	void InitializeCone(double _time);
	void SimulateCone(double _time);

	void InitializeConst(double _time);
	void SimulateConst(double _time);
};

/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CCycloneMuschelknautz : public CSteadyStateUnit
{
	// Cyclone parameters

	// User input
	double r_a{};		// Outer radius of cyclone [m].
	double r_i{};		// Radius of the dip tube [m].
	double h_dip{};		// Height of the dip tube [m].
	double b{};			// Width of rectangular entry [m].
	double h_e{};		// Height of rectangular entry [m].
	double lambda_0{};	// Wall friction coefficient for pure gas [-].
	double h_tot{};		// Total height of cyclone [m].
	double h_cyl{};		// Height of the cylindrical part of the cyclone [m].
	double r_3{};		// Radius of the particle exit [m].
	double D_init{};	// Initial value for calculation of separation efficiency curve [-].
	double K_G{};		// Constant for solids loading threshold [-].
	double eta_sep{};	// Additional efficiency factor of separation [-].
	// Calculated
	double r_2{};		// Mean cone radius [m].
	double r_e{};		// Distance to the middle streamline of entry [m].
	double h_con{};		// Height of the cone part of the cyclone [m].
	double h_i{};		// Height of separation zone [m].
	double beta{};		// Geometry factor [-].
	double A_e1{};		// Wall area considered on average for the first round after entry [m^2].
	double A_cyl{};		// Lateral area of the cylindrical part of the wall [m^2].
	double A_con{};		// Lateral area of the conical part of the wall [m^2].
	double A_dip{};		// Lateral area of the dip tube [m^2].
	double A_top{};		// Area of the upper wall [m^2].
	double A_R{};		// Total wall friction area [m^2].
	double A_con_2{};	// Lateral area of the top half of the conical part of the wall [m^2].
	double A_w{};		// Sedimentation area [m^2].

	// Simulation parameters

	bool plot{ false };			// Flag to control plotting functions.

	// Flowsheet parameters

	size_t classesNum{};			// Number of PSD classes [#].
	std::vector<double> averDiam;	// Average values of size grid for PSD [m].
	std::vector<double> sizeGrid;	// Vector with bins of PSD

public:
	void CreateBasicInfo() override;
	void CreateStructure() override;
	void Initialize(double _time) override;
	void Simulate(double _time) override;

private:
	// Check input parameters.
	bool CheckCycloneParameters() const;
	// Main calculation function.
	void CalculateSeparationMuschelknauz(double _time, CStream* _inlet, CStream* _outletC, CStream* _outletF);
	// Calculation effective value of D parameter according to Muschelknautz.
	double CalculateDeff(double d_star, double D_init) const;
	// Calculate separation efficiency in internal vortex for particle of size d.
	double CalculateSeparationFun(double d_star, double D, double d) const;

};


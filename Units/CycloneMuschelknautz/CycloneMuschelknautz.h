/* Copyright (c) 2022, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

/*
 * Base on:
 * Muschelknautz, U. (2019).
 * L3.4 Zyklone zum Abscheiden fester Partikel aus Gasen.
 * In: Stephan, P., Kabelac, S., Kind, M., Mewes, D., Schaber, K., Wetzel, T. (eds)
 * VDI-Wärmeatlas. Springer Reference Technik. Springer Vieweg, Berlin, Heidelberg.
 * https://doi.org/10.1007/978-3-662-52989-8_91
 */

#pragma once

#include "UnitDevelopmentDefines.h"

class CCycloneMuschelknautz : public CSteadyStateUnit
{
	/*
	 * Shapes of gas entry.
	 */
	enum class EEntry : size_t
	{
		SLOT_RECT, SPIRAL_FULL, SPIRAL_HALF, AXIAL
	};
	/*
	 * Shapes of blades for axial entry.
	 */
	enum class EBlade : size_t
	{
		STRAIGHT, CURVED, CURVED_TWISTED
	};

	// Ports
	CUnitPort* port_inlet{};	// Port for inlet.
	CUnitPort* port_outlet_s{};	// Port for solids outlet.
	CUnitPort* port_outlet_g{};	// Port for gas outlet.

	// Unit parameters
	CConstRealUnitParameter* up_d_out{};
	CConstRealUnitParameter* up_h_tot{};
	CConstRealUnitParameter* up_h_cyl{};
	CConstRealUnitParameter* up_d_dip{};
	CConstRealUnitParameter* up_h_dip{};
	CConstRealUnitParameter* up_d_exit{};
	CConstRealUnitParameter* up_b{};
	CConstRealUnitParameter* up_h_e{};
	CConstRealUnitParameter* up_epsilon{};
	CConstRealUnitParameter* up_n_L{};
	CConstRealUnitParameter* up_d_s{};
	CConstRealUnitParameter* up_r_kern{};
	CComboUnitParameter    * up_bladeShape{};
	CConstRealUnitParameter* up_delta{};
	CConstRealUnitParameter* up_lambda_0{};
	CConstRealUnitParameter* up_D{};
	CConstRealUnitParameter* up_K_G{};
	CConstRealUnitParameter* up_eta_adj{};
	CComboUnitParameter    * up_entryShape{};
	CCheckBoxUnitParameter * up_plot{};

	// Streams
	CStream* inlet{};	// Inlet stream.
	CStream* outletS{};	// Outlet solids stream.
	CStream* outletG{};	// Outlet gas stream.

	// Transformation matrices.
	CTransformMatrix tm_i2s{}; // Transformation matrix, input -> solids.
	CTransformMatrix tm_i2g{}; // Transformation matrix, input -> gas.

	// User-defined cyclone parameters
	double r_a{};							// Outer radius of cyclone [m].
	double r_i{};							// Radius of the vortex finder [m].
	double h_dip{};							// Height of the vortex finder [m].
	double b{};								// Width of entry [m].
	double h_e{};							// Height of entry [m].
	double epsilon{};						// Spiral angle for spiral entry [deg].
	double n_L{};							// Number of blades for axial entry [#].
	double d_s{};							// Thickness of blades for axial gas entry [m].
	double r_kern{};						// Core radius of blades for axial entry [m].
	double delta{};							// Angle of attack of blades for axial gas entry [rad].
	double lambda_0{};						// Wall friction coefficient for pure gas [-].
	double h_tot{};							// Total height of cyclone [m].
	double h_cyl{};							// Height of the cylindrical part of the cyclone [m].
	double r_3{};							// Radius of the particle exit [m].
	double D{};						        // Coefficient for calculation of separation efficiency curve [-].
	double K_G{};							// Constant for solids loading threshold [-].
	double eta_adj{};						// Separation efficiency adjustment factor [-].
	EEntry entryShape{ EEntry::SLOT_RECT };	// Shape of gas entry.
	EBlade bladeShape{ EBlade::STRAIGHT };	// Shape of blades in the axial gas entry.

	// Calculated cyclone parameters
	double r_2{};		// Mean cone radius [m].
	double r_3_eff{};	// Effective radius of the particle exit [m].
	double r_e{};		// Radius of the middle streamline of entry [m].
	double h_con{};		// Height of the cone part of the cyclone [m].
	double h_con_eff{};	// Effective height of the cone part of the cyclone [m].
	double h_i{};		// Height of separation zone [m].
	double a{};			// Height of blade channel for axial entry [m].
	double beta{};		// Relative width of cyclone entry [-].
	double A_cyl{};		// Lateral area of the cylindrical part of the wall [m^2].
	double A_con{};		// Lateral area of the conical part of the wall [m^2].
	double A_top{};		// Area of the upper wall [m^2].
	double A_dip{};		// Lateral area of the vortex finder [m^2].
	double A_R{};		// Total wall friction area [m^2].
	double A_e1{};		// Wall area considered on average for the first round after entry [m^2].
	double A_R_sp{};	// Frictional area of the spiral in the case of spiral entry[m^2].
	double A_con_2{};	// Lateral area of the top half of the conical part of the wall [m^2].
	double A_w{};		// Sedimentation area [m^2].

	// Simulation parameters
	bool plot{ false };			// Flag to control plotting functions.

	// Flowsheet parameters

	size_t classesNum{};			// Number of PSD classes [#].
	std::vector<double> averDiam;	// Average particle diameters in each class [m].
	std::vector<double> sizeGrid;	// PSD grid [m].

public:
	void CreateBasicInfo() override;
	void CreateStructure() override;
	void Initialize(double _time) override;
	void Simulate(double _time) override;

private:
	// Check input parameters.
	bool CheckCycloneParameters() const;
	// Main calculation function.
	void CalculateSeparationMuschelknauz(double _time);
	// Calculates wall friction coefficient for solids-containing gas [-].
	double WallFrictionCoeff(double mu_e) const;
	// Calculates contraction coefficient [-].
	double ContractionCoefficient(double mu_e) const;
	// Calculates inlet velocity in the middle streamline [m/s].
	double InletVelocityStreamline(double Vflow_g) const;
	// Calculates tangential velocity at cyclone radius [m/s].
	double OuterTangVelocity(double Vflow_g, double v_e, double alpha, double lambda_s) const;
	// Calculates exponent for solids loading threshold [-].
	double SolidsLoadingExp(double mu_e) const;
	// Calculate separation efficiency in internal vortex for particle of size d.
	double CalculateSeparationEff(double Dval, double d_star, double d) const;

};


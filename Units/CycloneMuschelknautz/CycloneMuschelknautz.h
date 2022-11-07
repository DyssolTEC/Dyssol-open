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
	 * Shapes of blades for axial gas entry.
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
	CConstRealUnitParameter* up_d_o{};	       // Unit parameter: Outer diameter of cyclone.
	CConstRealUnitParameter* up_h_tot{};	   // Unit parameter: Total height of cyclone.
	CConstRealUnitParameter* up_h_cyl{};	   // Unit parameter: Height of the cylindrical part of cyclone.
	CConstRealUnitParameter* up_d_f{};	       // Unit parameter: Diameter of vortex finder.
	CConstRealUnitParameter* up_h_f{};	       // Unit parameter: Height (depth) of vortex finder.
	CConstRealUnitParameter* up_d_exit{};	   // Unit parameter: Diameter of particle exit.
	CComboUnitParameter    * up_entry_shape{}; // Unit parameter: Gas entry shape.
	CConstRealUnitParameter* up_b_e{};		   // Unit parameter: Width of gas entry.
	CConstRealUnitParameter* up_h_e{};		   // Unit parameter: Height of gas entry.
	CConstRealUnitParameter* up_epsilon{};	   // Unit parameter: Spiral angle in spiral gas entry.
	CConstRealUnitParameter* up_N_b{};		   // Unit parameter: Number of blades in axial gas entry.
	CConstRealUnitParameter* up_d_b{};		   // Unit parameter: Thickness of blades in axial gas entry.
	CConstRealUnitParameter* up_r_core{};	   // Unit parameter: Core radius of blades in axial gas entry.
	CComboUnitParameter    * up_blade_shape{}; // Unit parameter: Blades shapes in axial gas entry.
	CConstRealUnitParameter* up_delta{};	   // Unit parameter: Angle of attack of blades in axial gas entry.
	CConstRealUnitParameter* up_lambda_0{};	   // Unit parameter: Wall friction coefficient of pure gas.
	CConstRealUnitParameter* up_D{};		   // Unit parameter: Coefficient for grid efficiency curve calculation according to Muschelknautz.
	CConstRealUnitParameter* up_K_main{};	   // Unit parameter: Constant for solids loading threshold in main stream.
	CConstRealUnitParameter* up_eta_adj{};	   // Unit parameter: Separation efficiency adjustment factor.
	CCheckBoxUnitParameter * up_plot{};        // Unit parameter: Whether to generate plots.

	// Streams
	CStream* inlet{};		// Inlet stream.
	CStream* outlet_s{};	// Outlet solids stream.
	CStream* outlet_g{};	// Outlet gas stream.

	// Transformation matrices.
	CTransformMatrix tm_i2s{}; // Transformation matrix, input -> solids.
	CTransformMatrix tm_i2g{}; // Transformation matrix, input -> gas.

	// Plots
	CPlot* plot_sep_3d{};      // Plot: Separation efficiency.
	CPlot* plot_sep{};		   // Plot: Total separation efficiency.
	CPlot* plot_main_frac{};   // Plot: Main stream fraction.
	CCurve* curve_sep{};       // Curve: Total separation efficiency.
	CCurve* curve_main_frac{}; // Curve: Main stream fraction.

	// User-defined cyclone parameters
	double r_o{};							 // Outer radius of cyclone [m].
	double h_tot{};							 // Total height of cyclone [m].
	double h_cyl{};							 // Height of the cylindrical part of cyclone [m].
	double r_f{};							 // Radius of vortex finder [m].
	double h_f{};							 // Height (depth) of vortex finder [m].
	double r_exit{};						 // Radius of particles exit [m].
	EEntry entry_shape{ EEntry::SLOT_RECT }; // Shape of gas entry.
	double b_e{};							 // Width of gas entry/blade channel [m].
	double h_e{};							 // Height of gas entry [m].
	double epsilon{};						 // Spiral angle in spiral gas entry [deg].
	double N_b{};							 // Number of blades in axial gas entry [#].
	double d_b{};							 // Thickness of blades in axial gas entry [m].
	double r_core{};						 // Core radius of blades in axial gas entry [m].
	EBlade blade_shape{ EBlade::STRAIGHT };	 // Shape of blades in axial gas entry.
	double delta{};							 // Angle of attack of blades in axial gas entry [rad].
	double lambda_0{};						 // Wall friction coefficient of pure gas [-].
	double D{};						         // Coefficient for grid efficiency curve calculation according to Muschelknautz [-].
	double K_main{};						 // Constant for solids loading threshold in main stream [-].
	double eta_adj{};						 // Separation efficiency adjustment factor [-].
	bool   plot{ false };					 // Flag to control plotting functions.

	// Calculated cyclone parameters
	double r_con_mean{}; // Mean radius of the conical part [m].
	double r_exit_eff{}; // Effective radius of the particles exit [m].
	double r_e{};		 // Radius of the middle gas streamline at gas entry [m].
	double h_con{};		 // Height of the cone part of cyclone [m].
	double h_con_eff{};	 // Effective height of the cone part of cyclone [m].
	double h_sep{};		 // Height of separation zone [m].
	double a{};			 // Height of blades channel in axial gas entry [m].
	double beta{};		 // Relative width of cyclone gas entry [-].
	double A_cyl{};		 // Lateral area of the cylindrical part [m^2].
	double A_con{};		 // Lateral area of the conical part [m^2].
	double A_top{};		 // Area of upper wall [m^2].
	double A_f{};		 // Lateral area of vortex finder [m^2].
	double A_tot{};		 // Total wall friction area [m^2].
	double A_e1{};		 // Average wall area considered for the first revolution after entry [m^2].
	double A_sp{};       // Frictional area of the spiral in spiral gas entry [m^2].
	double A_con_2{};	 // Lateral area of the top half of conical part [m^2].
	double A_sed{};		 // Sedimentation area [m^2].

	// Flowsheet parameters
	size_t classes_num{};			// Number of PSD classes [#].
	std::vector<double> aver_diam;	// Average particle diameters in each class [m].
	std::vector<double> size_grid;	// PSD grid [m].

public:
	void CreateBasicInfo() override;
	void CreateStructure() override;
	void Initialize(double _time) override;
	void Simulate(double _time) override;

private:
	// Check input parameters.
	void CheckCycloneParameters();
	// Main calculation function.
	void CalculateSeparationMuschelknauz(double _time);
	// Calculates wall friction coefficient for solids-containing gas [-].
	double WallFrictionCoeff(double mu_in) const;
	// Calculates contraction coefficient [-].
	double ContractionCoefficient(double mu_in) const;
	// Calculates inlet velocity in the middle streamline [m/s].
	double InletVelocityStreamline(double Vflow_in_g) const;
	// Calculates tangential velocity at cyclone radius [m/s].
	double OuterTangVelocity(double Vflow_in_g, double v_e, double alpha, double lambda_s) const;
	// Calculates exponent for solids loading threshold [-].
	double SolidsLoadingExp(double mu_in) const;
	// Calculate separation efficiency in internal vortex for particle of size d.
	double CalculateSeparationEff(double Dval, double d_star, double d) const;
};

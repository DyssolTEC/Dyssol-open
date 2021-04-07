/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CCyclone2 : public CSteadyStateUnit
{
	/// Cyclone parameters
	// User input
	double m_d_r_out{ 0 };		// Outer radius of cyclone [m]
	double m_d_r_dip{ 0 };		// Radius of dip tube [m]
	double m_d_h_dip{ 0 };		// Height of dip tube [m]
	double m_d_b_e{ 0 };		// Width of rectangular entry [m]
	double m_d_h_e{ 0 };		// Height of rectangular entry [m]
	double m_d_lambda_0{ 0 };	// Friction coefficient [m]
	double m_d_h_tot{ 0 };		// Total height of cyclone [m]
	double m_d_h_cyl{ 0 };		// Height of cylindrical part of cyclone [m]
	double m_d_r_ex{ 0 };		// Radius of particle exit [m]
	double m_n_D{ 0 };
	double m_d_eta_sep{ 0 };	// Additional adjustment of separation [-]
	// Calculated
	double m_d_r_mean{ 0 };		// Mean radius [m]
	double m_d_r_cone{ 0 };		// Mean cone radius [m]
	double m_d_r_e{ 0 };		// Center stream line of entry [m]
	double m_d_h_cone{ 0 };		// Height of the cone [m]
	double m_d_h_inner{ 0 };	// Total height minus depth of dip tube [m]
	double m_d_beta{ 0 };		// Geometry factor [-]
	double m_d_A_first{ 0 };	// Area considered for first round after entry [m^2]
	double m_d_A_cyl{ 0 };		// Lateral area cylinder [m^2]
	double m_d_A_cone{ 0 };		// Complete cone area [m^2]
	double m_d_A_dip{ 0 };		// Lateral area dip tube [m^2]
	double m_d_A_top{ 0 };		// Area upper wall [m^2]
	double m_d_A_fric{ 0 };		// Total friction area [m^2]
	double m_d_Ahalf_cone{ 0 };	// Area at half of conical part [m^2]
	double m_d_A_sed{ 0 };		// Sedimentation area [m^2]
	/// Simulation parameters
	bool m_b_calc_D{ false };			// Variable to control calculation of D
	bool m_b_plot{ false };				// Variable to control plotting functions
	size_t m_n_num_size_classes{ 0 };	// Number of classes of particle size distribution [-]
	std::vector<double> m_v_size_grid;	// Vector with bins of PSD
	std::vector<double> m_v_dpart_grid; // Vector with mean diameters of PSD

public:
	void CreateBasicInfo() override;
	void CreateStructure() override;
	void Initialize(double _time) override;
	void Simulate(double _time) override;

private:
	/// Helper functions
	double CalculateDeff(double _d_dpart_star);
	void CalculateSeparationMuschelknauz(double _time, CMaterialStream* _inflow, CMaterialStream* _outflowCoarse, CMaterialStream* _outflowFines);

	/// Check functions
	bool CheckCycloneParameters() const;
	void CheckStream(double _time, CMaterialStream* _stream);
};


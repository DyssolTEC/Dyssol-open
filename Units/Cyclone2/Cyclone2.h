/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CCyclone2 : public CSteadyStateUnit
{
public:
	CCyclone2();

	void Initialize(double _dTime) override;
	void Simulate(double _dTime) override;

	/// Cyclone parameters
	// User input
	double m_d_r_out;	// Outer radius of cyclone [m]
	double m_d_r_dip;	// Radius of dip tupe [m]
	double m_d_h_dip;	// Height of dip tube [m]
	double m_d_b_e	;	// Width of rectangular entry [m]
	double m_d_h_e	;	// Height of rectangular entry [m]
	double m_d_lambda_0;	// Friction coefficient [m]
	double m_d_h_tot;	// Total height of cyclone [m]
	double m_d_h_cyl;	// Height of cyclindric part of cyclone [m]
	double m_d_r_ex;	// Radius of particle exit [m]
	double m_n_D;
	double m_d_eta_sep;	// Additional adjustment of separation [-]
	// Calculated
	double m_d_r_mean;	// Mean radius [m]
	double m_d_r_cone;	// Mean cone radius [m]
	double m_d_r_e;		// Center stream line of entry [m]
	double m_d_h_cone;	// Height of the cone [m]
	double m_d_h_inner;	// Total height minus depth of dip tube [m]
	double m_d_beta;	// Geometry factor [-]
	double m_d_A_first;	// Area considered for first round after entry [m^2]
	double m_d_A_cyl;	// Lateral area cylinder [m^2]
	double m_d_A_cone;	// Complete cone area [m^2]
	double m_d_A_dip;	// Lateral area dip tube [m^2]
	double m_d_A_top;	// Area upper wall [m^2]
	double m_d_A_fric;	// Total friction area [m^2]
	double m_d_Ahalf_cone;	// Area at half of conical part [m^2]
	double m_d_A_sed;	// Sedimentation area [m^2]
	/// Simulation parameters
	bool m_b_calc_D;				// Variable to control calculation of D
	bool m_b_plot;					// Variable to control plotting functions
	unsigned m_n_num_size_classes;	// Number of classes of particle size distribution [-]
	std::vector<double> m_v_size_grid; // Vector with bins of PSD
	std::vector<double> m_v_dpart_grid; // Vector with mean diameters of PSD

	/// Helper functions
	double CalculateDeff(double _d_dpart_star);
	void CalculateSeparationMuschelknauz(double _dTime, CMaterialStream* _pInflow, CMaterialStream* _pOutflowCoarse, CMaterialStream* _pOutflowFines);

	/// Check functions
	bool CheckCycloneParameters() const;
	void CheckStream(double _dTime, CMaterialStream* _pStream);
};


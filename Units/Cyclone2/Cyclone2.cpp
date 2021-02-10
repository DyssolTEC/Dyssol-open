/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Cyclone2.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CCyclone2();
}

//////////////////////////////////////////////////////////////////////////
/// Unit

CCyclone2::CCyclone2()
{
	/// Basic unit's info ///
	m_sUnitName = "Cyclone v2";
	m_sAuthorName = "Buchholz (SPE/TUHH)";
	m_sUniqueID = "170BEB3E368548BDBF8B3DB1C66DB179";

	/// Add ports ///
	AddPort("Inflow", INPUT_PORT);
	AddPort("Outflow_Coarse", OUTPUT_PORT);
	AddPort("Outflow_Fines", OUTPUT_PORT);

	/// Add unit parameters       ///
	AddConstParameter("d_out",    0.01, 100,  1,    "m", "Outer Diameter of cyclone");
	AddConstParameter("d_dip",    0.01, 100,  0.2,  "m", "Diameter of dip tube");
	AddConstParameter("h_dip",    0.01, 100,  0.2,  "m", "Depth of dip tube");
	AddConstParameter("b_e",      0.01, 100,  0.1,  "m", "Width of rectangular inlet");
	AddConstParameter("h_e",      0.01, 100,  0.2,  "m", "Width of rectangular inlet");
	AddConstParameter("lambda_0", 0,    1e+6, 0,    "-", "Coefficient of friction for unloading gas");
	AddConstParameter("h_tot",    0.01, 100,  1,    "m", "Total height of cyclone");
	AddConstParameter("h_cyl",    0.01, 100,  0.25, "m", "Height of cylindrical part");
	AddConstParameter("d_exit",   0.01, 100,  0.1,  "m", "Diameter of particle exit");
	AddConstParameter("D",        1,    4,    3,    "-", "Value for calculation of grade efficiency curve. Between 2 and 4. If D is set to a value smaller than 2, it is calculated according to Muschelknautz.");

	AddConstParameter("eta_sep",  0,    1,    1,    "-", "Additional efficiency factor for separation.");

	AddConstParameter("Plot",     0,    1,    1,    "",  "Value for controlling plotting functions. If it is unequal to 0, plots will be generates and state variables will be written. Turn off to increase performance.");

	/// Add material streams ///
	AddMaterialStream("Inflow_Mixed");
}

void CCyclone2::Initialize(double _dTime)
{
	/// Check Simulation Setup
	// Check for gas and solid phases
	if (!IsPhaseDefined(SOA_VAPOR))
		RaiseError("Gas phase not defined.");
	if (!IsPhaseDefined(SOA_SOLID))
		RaiseError("Solid phase not defined.");

	// Check for size distribution
	if (!IsDistributionDefined(DISTR_SIZE))
		RaiseError("Particle size distribution not defined.");

	/// Set unit parameters
	// Cyclone parameters
	m_d_r_out    = 0.5 * GetConstParameterValue("d_out");
	m_d_r_dip    = 0.5 * GetConstParameterValue("d_dip");
	m_d_r_ex     = 0.5* GetConstParameterValue("d_exit");
	m_d_b_e      = GetConstParameterValue("b_e");

	m_d_h_tot    = GetConstParameterValue("h_tot");
	m_d_h_dip    = GetConstParameterValue("h_dip");
	m_d_h_cyl    = GetConstParameterValue("h_cyl");
	m_d_h_e	     = GetConstParameterValue("h_e");

	m_d_lambda_0 = GetConstParameterValue("lambda_0");
	m_n_D		 = GetConstParameterValue("D");
	m_b_calc_D   = m_n_D < 2 ? true : false;

	m_d_eta_sep  = GetConstParameterValue("eta_sep");

	// Additional parameters
	const double dtemp = GetConstParameterValue("Plot");
	m_b_plot = dtemp > 0;

	// Calculation of geometric parameters
	m_d_r_mean = std::sqrt(m_d_r_out * m_d_r_dip);			// Mean radius [m]
	m_d_r_cone = (m_d_r_ex + m_d_r_out) / 2;				// Mean cone radius [m]
	m_d_r_e = m_d_r_out - m_d_b_e / 2;						// Center stream line for inlet [m]
	m_d_beta = m_d_b_e / m_d_r_out;							// Geometry factor [-]

	m_d_h_cone = m_d_h_tot - m_d_h_cyl;						// Height of the cone [m]
	m_d_h_inner = m_d_h_tot - m_d_h_dip;					// Total height minus depth of dip tube [m]

	m_d_A_first = 2 * MATH_PI * m_d_r_out * m_d_h_e / 2;	// Area considered for first round after entry [m^2]
	m_d_A_cyl = 2 * MATH_PI * m_d_r_out * m_d_h_cyl;		// Lateral area cylinder [m^2]
	m_d_A_cone = 2 * m_d_r_cone * MATH_PI * std::sqrt((m_d_r_out - m_d_r_ex) * (m_d_r_out - m_d_r_ex) + m_d_h_cone * m_d_h_cone);	// Complete cone area [m^2]
	m_d_A_dip = 2 * MATH_PI * m_d_h_dip * m_d_r_dip;						// Lateral area dip tube [m^2]
	m_d_A_top = MATH_PI * m_d_r_out * m_d_r_out - MATH_PI * m_d_r_dip * m_d_r_dip;		// Area upper wall [m^2]
	m_d_A_fric = m_d_A_cyl + m_d_A_cone + m_d_A_dip + m_d_A_top;							// Total friction area [m^2]
	m_d_Ahalf_cone = 2 * ((m_d_r_out + m_d_r_cone) / 2) * MATH_PI * std::sqrt((m_d_r_out - m_d_r_cone) * (m_d_r_out - m_d_r_cone) + m_d_h_cone / 2 * (m_d_h_cone / 2)); // Area at half of conical part [m^2]
	m_d_A_sed = m_d_A_cyl + m_d_Ahalf_cone;										// Sedimentation area [m^2]

	// Check for errors in definition of cyclone parameters
	if (!CheckCycloneParameters())
		RaiseError("Error in input parameters of cyclone.");

	// Simulation parameters
	m_n_num_size_classes = GetClassesNumber(DISTR_SIZE);
	m_v_size_grid = GetNumericGrid(DISTR_SIZE);
	m_v_dpart_grid = GetClassesMeans(DISTR_SIZE);

	// State variables
	AddStateVariable("D_eff", 3, true);
	AddStateVariable("Total Separation Efficiency [%]", 0, true);

	// Plots
	AddPlot("Separation", "Diameter [m]", "Separation Efficiency [%]", "Time [s]");
}

bool CCyclone2::CheckCycloneParameters() const
{
	// Check diameters
	if (m_d_r_out <= m_d_r_dip || m_d_r_out <= m_d_r_ex || m_d_r_out <= m_d_b_e)
		return false;

	// Check heights
	if (m_d_h_tot <= m_d_h_dip || m_d_h_tot <= m_d_h_cyl || m_d_h_tot <= m_d_h_e)
		return false;

	// Check calculated parameters
	// Heights an radii
	if (m_d_r_mean <= 0 || m_d_r_cone <= 0 || m_d_r_e <= 0 || m_d_h_cone <= 0 || m_d_h_inner <= 0)
		return false;
	// Areas
	if (m_d_A_first <= 0 || m_d_A_cyl <= 0 || m_d_A_cone <= 0 || m_d_A_dip <= 0 || m_d_A_top <= 0 || m_d_A_fric <= 0 || m_d_Ahalf_cone <= 0 || m_d_A_sed <= 0)
		return false;
	// Geometry factor
	if (m_d_beta <= 0)
		return false;

	return true;
}

void CCyclone2::Simulate(double _dTime)
{

	// To do: Implement heat transfer
	// Get pointers to inflow streams
	CMaterialStream* pInflow = GetPortStream("Inflow");
	CheckStream(_dTime, pInflow);

	// Addition of bot inflow streams
	CMaterialStream* pInflowMixed = GetMaterialStream("Inflow_Mixed");
	pInflowMixed->CopyFromStream(pInflow, _dTime);

	// Get pointers to outflow streams and initialize them
	CMaterialStream* pOutflowCoarse = GetPortStream("Outflow_Coarse");
	pOutflowCoarse->CopyFromStream(pInflowMixed, _dTime);
	pOutflowCoarse->SetMassFlow(_dTime, 0);
	CMaterialStream* pOutflowFines = GetPortStream("Outflow_Fines");
	pOutflowFines->CopyFromStream(pInflowMixed, _dTime);
	pOutflowFines->SetMassFlow(_dTime, 0);

	// Calculation of separation process in cyclone
	CalculateSeparationMuschelknauz(_dTime, pInflowMixed, pOutflowCoarse, pOutflowFines);
}

void CCyclone2::CheckStream(double _dTime, CMaterialStream* _pStream)
{
	// Check for valid PSD in stream
	// If no PSD has been defined, but the solid mass flow is zero, redefine a equal distributed PSD
	const double d_mflow_solid = _pStream->GetPhaseMassFlow(_dTime, SOA_SOLID);
	std::vector<double> v_psd = _pStream->GetPSD(_dTime, PSD_MassFrac);
	if (v_psd.empty())
	{
		if (d_mflow_solid == 0)
		{
			v_psd.resize(m_n_num_size_classes, 1. / m_n_num_size_classes);
			_pStream->SetPSD(_dTime, PSD_MassFrac, v_psd);
		}
		else
			RaiseError("No PSD defined for stream '" + _pStream->GetStreamName() + "'");
	}
}

double CCyclone2::CalculateDeff(double _d_dpart_star)
{
	// calculation of D according to Muschelknautz
	std::vector<double> v_eta_fines(m_n_num_size_classes);						// Vector of ethaF for each particle size class
	double d_dpart_eta_fines50 = 1;
	double d_dpart_eta_fines100 = 1;

	double n_D_eff = 3;		// Estimation for D
	for (unsigned i = 0; i<m_n_num_size_classes; i++)							// Save values of ethaF in vector
	{
		const double val = (m_v_size_grid[i] + m_v_size_grid[i + 1]) / 2;
		if (val >= (_d_dpart_star / n_D_eff) && val <= (n_D_eff*_d_dpart_star))
		{
			const double val2 = std::log(val / _d_dpart_star);
			const double val3 = std::log(n_D_eff);

			double dtemp = 0.5 * (1 + std::cos(MATH_PI * (1 - ((val2 + val3) / (2 * val3)))));
			v_eta_fines[i] = dtemp;
		}
		else if (val < (_d_dpart_star / n_D_eff))
		{
			v_eta_fines[i] = 0;
		}
		else if (val >(n_D_eff*_d_dpart_star))
		{
			v_eta_fines[i] = 1;
		}
	}

	for (unsigned i = 0; i<m_n_num_size_classes - 1; i++)  					// Finding values where ethaF equals 0.5 and 1
	{
		if (v_eta_fines[i]<0.5 && v_eta_fines[i + 1]>0.5)
			d_dpart_eta_fines50 = ((m_v_size_grid[i] + m_v_size_grid[i + 1]) / 2) + ((0.5 - v_eta_fines[i]) * ((((m_v_size_grid[i + 1] + m_v_size_grid[i + 2]) / 2) - ((m_v_size_grid[i] + m_v_size_grid[i + 1]) / 2)) / (v_eta_fines[i + 1] - v_eta_fines[i])));  //Ilateration f�r ethaF(val)=0.5
		if (v_eta_fines[i] == 1)
		{
			d_dpart_eta_fines100 = ((m_v_size_grid[i] + m_v_size_grid[i + 1]) / 2);
			break;
		}
	}

	n_D_eff = d_dpart_eta_fines100 / d_dpart_eta_fines50;					// Calculate Deff [-]

	if (n_D_eff > 4 || n_D_eff < 2)											// If D can't be obtained because of not enough particle fractions
		n_D_eff = 3;

	return n_D_eff;
}

void CCyclone2::CalculateSeparationMuschelknauz(double _dTime, CMaterialStream* _pInflow, CMaterialStream* _pOutflowCoarse, CMaterialStream* _pOutflowFines)
{
	/// Inflow parameters
	// Get inflow parameters
	double d_mflow_gas = _pInflow->GetPhaseMassFlow(_dTime, SOA_VAPOR); // Gas massflow at inlet (mixed streams) [kg/s]
	double d_mflow_solid = _pInflow->GetPhaseMassFlow(_dTime, SOA_SOLID); // Solid massflow at inlet (mixed streams) [kg/s]

	double d_rho_gas = _pInflow->GetPhaseTPDProp(_dTime, DENSITY, SOA_VAPOR); // Gas density at inlet (mixed streams) [kg/m^3]
	double d_rho_solid = _pInflow->GetPhaseTPDProp(_dTime, DENSITY, SOA_SOLID); // Solid density at inlet (mixed streams) [kg/m^3]
	double d_rho_diff = d_rho_solid - d_rho_gas; // Difference of densities at inlet (mixed streams) [kg/m^3]
	if (d_rho_diff <= 0)
		RaiseError("Gas density larger than solids density.");

	double d_vflow_gas = d_mflow_gas / d_rho_gas; // Gas volume flow at inlet (mixed streams) [m^3/s]


	// Get massflows at inlet
	// All gas leaves through top exit
	if (d_mflow_gas < 0)
	{
		if (std::fabs(d_mflow_gas) < 1e-3)
		{
			d_mflow_gas = 0;
		}
		else
		{
			RaiseError("Inlet gas massflow lower significantly lower than zero: " + std::to_string(d_mflow_gas));
		}
	}

	_pOutflowFines->SetPhaseMassFlow(_dTime, SOA_VAPOR, d_mflow_gas);
	// If there is no solid inflow, stop unit calculation
	if (d_mflow_solid == 0)
		return;
	// If there is no gas inflow, all solids leave through the bottom exit and stop unit calculation
	if (d_mflow_gas == 0)
	{
		_pOutflowCoarse->SetPhaseMassFlow(_dTime, SOA_SOLID, d_mflow_solid);
		return;
	}

	/// Calculate operational parameters of cyclone
	double d_mu_e = d_mflow_solid / d_mflow_gas;	// Solid loading at inlet (mixed streams) [kg/kg]
	double temp1 = (m_d_beta / 2) * (m_d_beta / 2) - (m_d_beta / 2);
	double temp2 = 1 - (1 - m_d_beta * m_d_beta) / (1 + d_mu_e) * (2 * m_d_beta - m_d_beta * m_d_beta);
	double d_alpha = (1 - std::sqrt(1 + 4 * temp1 * std::sqrt(temp2))) / m_d_beta;	// Constriction coefficient [-]

	double d_lambda_solid = m_d_lambda_0 * (1 + 2 * std::sqrt(d_mu_e));				// Wallfriction coefficient with solids loading [ - ]


	double d_r_et = m_d_r_out - (d_alpha * m_d_b_e) / 2;
	double d_r_zt = std::sqrt(d_r_et * m_d_r_cone);

	// Velocity cyclone calculations
	double d_w_sink50 = (0.5  * 0.9 * d_vflow_gas) / m_d_A_sed;				// Sinking speed of a particle going down the wall [m/s]


	double d_v_e = d_vflow_gas / (m_d_h_e*m_d_b_e);						// Velocity on middle stream line [m/s]
	double d_u_outer = d_v_e * m_d_r_e / m_d_r_out / d_alpha;		// Velocity on outer circle [m/s]

	double d_u_e = ((d_u_outer * m_d_r_out) / d_r_et) / (1 + (d_lambda_solid * m_d_A_first * d_u_outer * std::sqrt(m_d_r_out / d_r_et)) / (2 * 0.9 * d_vflow_gas));			// Velocity at outer diameter [m/s]


	double d_u_inner = (d_u_outer * (m_d_r_out / m_d_r_dip)) / (1 + ((d_lambda_solid / 2) * (m_d_A_fric / d_vflow_gas) * d_u_outer * std::sqrt(m_d_r_out / m_d_r_dip)));		// Inner tangential velocity [m/s]
	double d_u_2 = ((d_u_outer * m_d_r_out) / m_d_r_cone) / (1 + (d_lambda_solid / 2) * (m_d_A_sed / (0.9 * d_vflow_gas)) * d_u_outer * std::sqrt(m_d_r_out / m_d_r_cone));	// Velocity on middle cone [m/s]

	double d_z_et = (d_u_e * d_u_2) / d_r_zt;											// Mean centrifugal acceleration [m^2/s]

	///
	// Get inflow particle distributions
	std::vector<double> v_psd_inflow = _pInflow->GetPSD(_dTime, PSD_MassFrac);
	std::vector<double> v_Q3_inflow = _pInflow->GetPSD(_dTime, PSD_Q3);

	// Get inflow characteristic diameters
	double d_dpart_50_in = GetDistributionMedian(m_v_size_grid, v_Q3_inflow);

	// Calculate exponent
	double d_k;
	if (d_mu_e < 2.2 * std::pow(10, -5.))		// Calculation of exponent for calculating limiting loading myG
		d_k = 0.81;
	else if (d_mu_e > 0.1)
		d_k = 0.15;
	else
		d_k = 0.15 + 0.66 * std::exp(-std::pow((d_mu_e / 0.015), 0.6));

	double d_k_g = 0.025;


	// Calculate variables for separation at wall due to high solids loading
	double d_eta_gas = _pInflow->GetPhaseTPDProp(_dTime, VISCOSITY, SOA_VAPOR);
	double d_dpart_threshold = std::sqrt((d_w_sink50 * 18 * d_eta_gas) / (d_rho_diff * d_z_et));		// limiting diameter for overload [m]
	double d_mu_threshold = d_k_g * (d_dpart_threshold / d_dpart_50_in) * std::pow((10 * d_mu_e), d_k);		// threshold for solids loading [-]

	// Target separation efficiency at wall separation due to high solids loading
	double d_etaE = 1 - d_mu_threshold / d_mu_e;							// separation efficiency


	// different particle classes are separated accroding to separation function ethaF (Muschelknautz & Redemann)
	std::vector<double> v_etaF(m_n_num_size_classes);


	double d_dpart_star = std::sqrt((18 * d_rho_gas * 0.9 * d_vflow_gas) / (d_rho_diff * std::pow(d_u_inner, 2) * 2 * MATH_PI * m_d_h_inner)); // Cut size for separation in the cyclone vortex [m]

	if (m_b_calc_D)
		m_n_D = CalculateDeff(d_dpart_star);

	for (unsigned i = 0; i<m_n_num_size_classes; i++)							// Calculate seperation function [-]
	{
		double d_dpart_i = (m_v_size_grid[i] + m_v_size_grid[i + 1]) / 2;
		double d_etaF_i;

		if (d_dpart_i >= (d_dpart_star / m_n_D) && d_dpart_i <= (m_n_D * d_dpart_star))
		{
			const double val2 = std::log(d_dpart_i / d_dpart_star);
			const double val3 = std::log(m_n_D);
			d_etaF_i = 0.5 * (1 + std::cos(MATH_PI * (1 - ((val2 + val3) / (2 * val3)))));
		}
		else if (d_dpart_i < (d_dpart_star / m_n_D))
			d_etaF_i = 0;
		else if (d_dpart_i >(m_n_D * d_dpart_star))
			d_etaF_i = 1;

		v_etaF[i] = d_etaF_i;
	}

	CTransformMatrix TInputToCoarse(DISTR_SIZE, m_n_num_size_classes);			// Definition of transformation matrix
	CTransformMatrix TInputToFines(DISTR_SIZE, m_n_num_size_classes);
	double d_massfactor_fines = 0;

	std::vector<double> v_eta_ges(m_n_num_size_classes);
	for (unsigned i = 0; i<m_n_num_size_classes; i++)							// Calculate separation function [-]
	{
		double d_temp_frac_to_fines = (1. - d_etaE) * (1. - v_etaF[i]);

		// Additional decrease of separation efficiency
		//double d_temp_frac_to_coarse = pow((1 - d_temp_frac_to_fines), 2 - m_d_eta_sep);	// possible adjustment of the separation
		double d_temp_frac_to_coarse = (1 - d_temp_frac_to_fines) * m_d_eta_sep;
		d_temp_frac_to_fines = 1 - d_temp_frac_to_coarse;

		TInputToCoarse.SetValue(i, i, 1. - d_temp_frac_to_fines);
		TInputToFines.SetValue(i, i, d_temp_frac_to_fines);

		d_massfactor_fines = d_massfactor_fines + v_psd_inflow[i] * d_temp_frac_to_fines;		// Set mass flow

		v_eta_ges[i] = (1 - d_temp_frac_to_fines) * 100;
	}

	// Apply transformation matrices
	_pOutflowCoarse->ApplyTM(_dTime, TInputToCoarse);
	_pOutflowFines->ApplyTM(_dTime, TInputToFines);

	// Set output mass flows
	_pOutflowCoarse->SetPhaseMassFlow(_dTime, SOA_SOLID, (1 - d_massfactor_fines) * d_mflow_solid);
	_pOutflowFines->SetPhaseMassFlow(_dTime, SOA_SOLID, d_massfactor_fines * d_mflow_solid);

	// Set state variables
	SetStateVariable("D_eff", m_n_D);
	SetStateVariable("Total Separation Efficiency [%]", (1 - d_massfactor_fines) * 100);
	SaveStateVariables(_dTime);

	// Plotting
	AddCurveOnPlot("Separation", _dTime);
	AddPointOnCurve("Separation", _dTime, m_v_dpart_grid, v_eta_ges);
}

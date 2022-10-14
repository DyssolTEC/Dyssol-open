/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "CycloneMuschelknautz.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CCycloneMuschelknautz();
}

//////////////////////////////////////////////////////////////////////////
/// Unit

using namespace std;

void CCycloneMuschelknautz::CreateBasicInfo()
{
	/// Basic unit's info ///
	SetUnitName("Cyclone (Muschelknautz)");
	SetAuthorName("Moritz Buchholz & Vasyl Skorych (TUHH SPE)");
	SetUniqueID("170BEB3E368548BDBF8B3DB1C66DB179");
}

void CCycloneMuschelknautz::CreateStructure()
{
	/// Add ports ///
	AddPort("Inflow", EUnitPort::INPUT);
	AddPort("Outflow_Coarse", EUnitPort::OUTPUT);
	AddPort("Outflow_Fines", EUnitPort::OUTPUT);

	/// Add unit parameters       ///
	AddConstRealParameter("d_out"   , 1    , "m", "Outer Diameter of cyclone"                              , 0.01, 100 );
	AddConstRealParameter("d_dip"   , 0.2  , "m", "Diameter of dip tube"                                   , 0.01, 100 );
	AddConstRealParameter("h_dip"   , 0.2  , "m", "Depth of dip tube"                                      , 0.01, 100 );
	AddConstRealParameter("b_e"     , 0.1  , "m", "Width of rectangular inlet"                             , 0.01, 100 );
	AddConstRealParameter("h_e"     , 0.2  , "m", "Height of rectangular inlet"                            , 0.01, 100 );
	AddConstRealParameter("lambda_0", 0    , "-", "Wall friction coefficient for pure gas"                 , 0   , 1e+6);
	AddConstRealParameter("h_tot"   , 1    , "m", "Total height of cyclone"                                , 0.01, 100 );
	AddConstRealParameter("h_cyl"   , 0.25 , "m", "Height of cylindrical part"                             , 0.01, 100 );
	AddConstRealParameter("d_exit"  , 0.1  , "m", "Diameter of particle exit"                              , 0.01, 100 );
	AddConstRealParameter("D_init"  , 3    , "-", "Initial value for calculation of grade efficiency curve", 2   , 4   );
	AddConstRealParameter("K_G"     , 0.025, "-", "Constant for solids loading threshold"                  , 0.02, 0.03);
	AddConstRealParameter("eta_sep" , 1    , "-", "Additional efficiency factor for separation"            , 0   , 1   );

	AddCheckBoxParameter("Plot", true, "If set, plots will be generated and state variables will be written. Turn off to increase performance.");
}

void CCycloneMuschelknautz::Initialize(double _time)
{
	// Check for gas and solid phases
	if (!IsPhaseDefined(EPhase::VAPOR))
		RaiseError("Gas phase not defined.");
	if (!IsPhaseDefined(EPhase::SOLID))
		RaiseError("Solid phase not defined.");
	// Check for size distribution
	if (!IsDistributionDefined(DISTR_SIZE))
		RaiseError("Particle size distribution not defined.");

	// Get unit parameters
	r_a      = 0.5 * GetConstRealParameterValue("d_out");
	r_i      = 0.5 * GetConstRealParameterValue("d_dip");
	r_3      = 0.5 * GetConstRealParameterValue("d_exit");
	b        = GetConstRealParameterValue("b_e");
	h_tot    = GetConstRealParameterValue("h_tot");
	h_dip    = GetConstRealParameterValue("h_dip");
	h_cyl    = GetConstRealParameterValue("h_cyl");
	h_e	     = GetConstRealParameterValue("h_e");
	lambda_0 = GetConstRealParameterValue("lambda_0");
	D_init   = GetConstRealParameterValue("D_init");
	K_G      = GetConstRealParameterValue("K_G");
	eta_sep  = GetConstRealParameterValue("eta_sep");
	plot     = GetCheckboxParameterValue("Plot");

	// Calculation of geometric parameters
	r_2  = (r_3 + r_a) / 2;	// Mean cone radius [m]
	r_e  = r_a - b / 2;		// Distance to the middle streamline of inlet [m]
	beta = b / r_a;			// Geometry factor [-]

	h_con = h_tot - h_cyl;	// Height of the cone [m]
	h_i   = h_tot - h_dip;	// Height of separation zone [m]

	A_e1    = 2 * MATH_PI * r_a * h_e / 2;											// Wall area considered on average for the first round after entry [m^2]
	A_cyl   = 2 * MATH_PI * r_a * h_cyl;											// Lateral area of the cylindrical part of the wall [m^2]
	A_con   = MATH_PI * (r_a + r_3) * sqrt(pow(r_a - r_3, 2) + pow(h_con, 2));		// Lateral area of the conical part of the wall [m^2]
	A_dip   = 2 * MATH_PI * r_i * h_dip;											// Lateral area of the dip tube [m^2]
	A_top   = MATH_PI * pow(r_a, 2) - MATH_PI * pow(r_i, 2);						// Area of the upper wall [m^2]
	A_R     = A_cyl + A_con + A_dip + A_top;										// Total wall friction area [m^2]
	A_con_2 = MATH_PI * (r_a + r_2) * sqrt(pow(r_a - r_2, 2) + pow(h_con / 2, 2));	// Lateral area of the top half of the conical part [m^2]
	A_w     = A_cyl + A_con_2;														// Sedimentation area [m^2]

	// Check for errors in definition of cyclone parameters
	if (!CheckCycloneParameters())
		RaiseError("Error in input parameters of cyclone.");

	// Flowsheet parameters
	classesNum = GetClassesNumber(DISTR_SIZE);
	averDiam   = GetClassesMeans(DISTR_SIZE);
	sizeGrid   = GetNumericGrid(DISTR_SIZE);

	// State variables
	AddStateVariable("D_eff", 3);
	AddStateVariable("Total separation efficiency [%]", 0);

	// Plots
	AddPlot("Separation", "Diameter [m]", "Separation efficiency [%]", "Time [s]");
}

bool CCycloneMuschelknautz::CheckCycloneParameters() const
{
	// Check diameters
	if (r_a <= r_i || r_a <= r_3 || r_a <= b)
		return false;

	// Check heights
	if (h_tot <= h_dip || h_tot <= h_cyl || h_tot <= h_e)
		return false;

	// Check calculated parameters
	// Heights and radii
	if (r_a <= 0 || r_i <= 0 || r_2 <= 0 || r_e <= 0 || h_con <= 0 || h_i <= 0)
		return false;
	// Areas
	if (A_e1 <= 0 || A_cyl <= 0 || A_con <= 0 || A_dip <= 0 || A_top <= 0 || A_R <= 0 || A_con_2 <= 0 || A_w <= 0)
		return false;
	// Geometry factor
	if (beta <= 0)
		return false;

	return true;
}

void CCycloneMuschelknautz::Simulate(double _time)
{
	// Get pointers to inflow streams
	CStream* inlet = GetPortStream("Inflow");

	// Get pointers to outflow streams and initialize them
	CStream* outletC = GetPortStream("Outflow_Coarse");
	outletC->CopyFromStream(_time, inlet);
	outletC->SetMassFlow(_time, 0);
	CStream* outletF = GetPortStream("Outflow_Fines");
	outletF->CopyFromStream(_time, inlet);
	outletF->SetMassFlow(_time, 0);

	// Calculation of separation process in cyclone
	CalculateSeparationMuschelknauz(_time, inlet, outletC, outletF);
}

void CCycloneMuschelknautz::CalculateSeparationMuschelknauz(double _time, CStream* _inlet, CStream* _outletC, CStream* _outletF)
{
	// Get inflow parameters
	const double mflow_in_g = _inlet->GetPhaseMassFlow(_time, EPhase::VAPOR); // Gas mass flow at inlet [kg/s]
	const double mflow_in_s = _inlet->GetPhaseMassFlow(_time, EPhase::SOLID); // Solids mass flow at inlet [kg/s]

	const double rho_g = _inlet->GetPhaseProperty(_time, EPhase::VAPOR, DENSITY); // Gas density at inlet [kg/m^3]
	const double rho_s = _inlet->GetPhaseProperty(_time, EPhase::SOLID, DENSITY); // Solid density at inlet [kg/m^3]
	const double delta_rho = rho_s - rho_g; // Difference of densities at inlet [kg/m^3]
	if (delta_rho <= 0.0)
		RaiseError("Gas density larger than solids density.");

	const double Vflow_g = mflow_in_g / rho_g; // Gas volume flow at inlet [m^3/s]

	_outletF->SetPhaseMassFlow(_time, EPhase::VAPOR, mflow_in_g);
	// If there is no solid inflow, stop unit calculation
	if (mflow_in_s == 0.0)
		return;
	// If there is no gas inflow, all solids leave through the bottom exit and stop unit calculation
	if (mflow_in_g == 0.0)
	{
		_outletC->SetPhaseMassFlow(_time, EPhase::SOLID, mflow_in_s);
		return;
	}

	/// Calculate operational parameters of cyclone
	const double mu_e = mflow_in_s / mflow_in_g;	// Solid loading at inlet (mixed streams) [kg/kg]
	const double temp1 = beta / 2 * (beta / 2) - beta / 2;
	const double temp2 = 1 - (1 - beta * beta) / (1 + mu_e) * (2 * beta - beta * beta);
	const double alpha = (1 - sqrt(1 + 4 * temp1 * sqrt(temp2))) / beta;	// Constriction coefficient [-]

	const double lambda_s = lambda_0 * (1 + 2 * sqrt(mu_e));	// Wall friction coefficient for solids-containing gas [-]

	const double r_e_mean = (r_a - alpha * b) / 2;	// Mean inlet radius [m]
	const double r_z_mean = sqrt(r_e_mean * r_2);	// Reference mean radius [m]

	// Velocity cyclone calculations
	const double w_s_50 = 0.5 * 0.9 * Vflow_g / A_w;	// Sinking speed of a particle going down the wall [m/s]

	const double v_e = Vflow_g / (h_e * b);			// Velocity in the middle streamline [m/s]
	const double u_a = v_e * r_e / r_a / alpha;		// Circumferential velocity at the outer circle [m/s]

	const double u_e = u_a * r_a / r_e_mean / (1 + lambda_s / 2 * A_e1 / (0.9 * Vflow_g) * u_a * sqrt(r_a / r_e_mean));	// Circumferential velocity in streamline radius at feed level [m/s]

	const double u_i = u_a * (r_a / r_i) / (1 + lambda_s / 2 * (A_R / Vflow_g) * u_a * sqrt(r_a / r_i));			// Circumferential velocity at the inner circle [m/s]
	const double u_2 = u_a * (r_a / r_2) / (1 + lambda_s / 2 * (A_w / (0.9 * Vflow_g)) * u_a * sqrt(r_a / r_2));	// Circumferential velocity in mean cone radius at feed level [m/s]

	const double z_e_mean = u_e * u_2 / r_z_mean;	// Mean centrifugal acceleration along streamline [m^2/s]

	// Get inflow particle distributions
	vector<double> psd_inflow = _inlet->GetPSD(_time, PSD_MassFrac);
	vector<double> Q3_inflow  = _inlet->GetPSD(_time, PSD_Q3);

	const double d_50_A = GetDistributionMedian(sizeGrid, Q3_inflow); // Particle size median

	double k; // Exponent for solids loading threshold [-]
	if (mu_e < 2.2e-5)		k = 0.81;
	else if (mu_e > 0.1)	k = 0.15;
	else					k = 0.15 + 0.66 * exp(-pow(mu_e / 0.015, 0.6));

	// Calculate variables for separation at wall due to high solids loading
	const double eta_L = _inlet->GetPhaseProperty(_time, EPhase::VAPOR, VISCOSITY);	// Dynamic viscosity of gas [Pa*s]
	const double d_e_star = sqrt(w_s_50 * 18 * eta_L / (delta_rho * z_e_mean));		// Limiting particle diameter for sedimentation [m]
	const double mu_G = K_G * (d_e_star / d_50_A) * pow(10 * mu_e, k);				// Threshold for solids loading [kg/kg]

	const double eta_e = 1 - mu_G / mu_e;	// Separation efficiency due to exceeding of solids loading limit [-]

	const double d_star = sqrt(18 * eta_L * 0.9 * Vflow_g / (delta_rho * pow(u_i, 2) * 2 * MATH_PI * h_i)); // Cut size for separation in the cyclone vortex [m]

	//const double D_eff = CalculateDeff(d_star, m_D_init);	// Effective value of D parameter according to Muschelknautz [-]
	const double D_eff = D_init;	// Effective value of D parameter according to Muschelknautz [-]
	if (D_eff < 2 || D_eff > 4)
		RaiseWarning("D_eff = " + to_string(D_eff) + " is out or typical range [2, 4]");

	// Separation efficiency in inner vortex for each particle class (Muschelknautz & Redemann) [-]
	vector<double> eta_F(classesNum);
	for (size_t i = 0; i < averDiam.size(); ++i)
		eta_F[i] = CalculateSeparationFun(d_star, D_eff, averDiam[i]);

	CTransformMatrix I2C(DISTR_SIZE, classesNum); // Transformation matrix, input -> coarse
	CTransformMatrix I2F(DISTR_SIZE, classesNum); // Transformation matrix, input -> fine

	double massfactorFines = 0;
	vector<double> eta_ges(classesNum);
	for (size_t i = 0; i < classesNum; ++i)	// Calculate separation function [-]
	{
		double frac_to_fines = (1. - eta_e) * (1. - eta_F[i]);

		// Additional decrease of separation efficiency
		double frac_to_coarse = (1 - frac_to_fines) * eta_sep;
		frac_to_fines = 1 - frac_to_coarse;

		I2C.SetValue(i, i, 1. - frac_to_fines);
		I2F.SetValue(i, i, frac_to_fines);

		massfactorFines += psd_inflow[i] * frac_to_fines;	// for mass flow

		eta_ges[i] = (1 - frac_to_fines) * 100;
	}

	// Apply transformation matrices
	_outletC->ApplyTM(_time, I2C);
	_outletF->ApplyTM(_time, I2F);

	// Set output mass flows
	_outletC->SetPhaseMassFlow(_time, EPhase::SOLID, (1 - massfactorFines) * mflow_in_s);
	_outletF->SetPhaseMassFlow(_time, EPhase::SOLID, massfactorFines * mflow_in_s);

	if (plot)
	{
		// Set state variables
		SetStateVariable("D_eff", D_eff, _time);
		SetStateVariable("Total separation efficiency [%]", (1 - massfactorFines) * 100, _time);

		// Plotting
		AddCurveOnPlot("Separation", _time);
		AddPointsOnCurve("Separation", _time, averDiam, eta_ges);
	}
}

double CCycloneMuschelknautz::CalculateDeff(double d_star, double D_init) const
{
	CDependentValues dep; // Dependency of d_ref on eta_f
	for (const double d : averDiam)
	{
		const double d_ref = d / d_star;								// Reference diameter [m]
		const double eta_f = CalculateSeparationFun(d_star, D_init, d);	// Separation efficiency in internal vortex [-]
		dep.SetValue(eta_f, d_ref);
	}

	const double d_ref_05 = dep.GetValue(0.5);			// Value of d_ref where eta_f equals 0.5
	const double d_ref_10 = dep.GetValue(1.0);			// Value of d_ref where eta_f equals 1.0
	return d_ref_05 != 0.0 ? d_ref_10 / d_ref_05 : 3;	// D_eff [-]
}

double CCycloneMuschelknautz::CalculateSeparationFun(double d_star, double D, double d) const
{
	const double d_ref = d / d_star;	// Reference diameter [m]
	if (d_ref < 1 / D)
		return 0;
	if (1 / D <= d_ref && d_ref <= D)
		return 0.5 * (1 + cos(MATH_PI * (1 - (log(d_ref) + log(D)) / (2 * log(D)))));
	return 1;
}
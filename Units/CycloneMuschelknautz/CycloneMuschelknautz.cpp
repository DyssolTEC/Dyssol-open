/* Copyright (c) 2022, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

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
	// Basic unit's info
	SetUnitName("Cyclone (Muschelknautz)");
	SetAuthorName("Moritz Buchholz & Vasyl Skorych (TUHH SPE)");
	SetUniqueID("170BEB3E368548BDBF8B3DB1C66DB179");
}

void CCycloneMuschelknautz::CreateStructure()
{
	// Add ports
	port_inlet    = AddPort("Inlet"        , EUnitPort::INPUT);
	port_outlet_s = AddPort("Outlet solids", EUnitPort::OUTPUT);
	port_outlet_g = AddPort("Outlet gas"   , EUnitPort::OUTPUT);

	// Prepare values for combo parameters
	const std::vector<EEntry>      entry_types{ EEntry::SLOT_RECT , EEntry::SPIRAL_FULL, EEntry::SPIRAL_HALF, EEntry::AXIAL };
	const std::vector<std::string> entry_names{ "Rectangular slot", "Full spiral"      , "Half spiral"      , "Axial" };
	const std::vector<EBlade>      blade_types{ EBlade::STRAIGHT , EBlade::CURVED, EBlade::CURVED_TWISTED };
	const std::vector<std::string> blade_names{ "Simple straight", "Curved"      , "Curved and twisted"   };

	// Add unit parameters
	up_d_out      = AddConstRealParameter("d_out"   , 1    , "m"  , "Outer diameter of cyclone"                              , 0.01      );
	up_h_tot      = AddConstRealParameter("h_tot"   , 1    , "m"  , "Total height of cyclone"                                , 0.01      );
	up_h_cyl      = AddConstRealParameter("h_cyl"   , 0.25 , "m"  , "Height of cylindrical part"                             , 0.01      );
	up_d_dip      = AddConstRealParameter("d_dip"   , 0.2  , "m"  , "Diameter of vortex finder"                              , 0.01      );
	up_h_dip      = AddConstRealParameter("h_dip"   , 0.2  , "m"  , "Depth of vortex finder"                                 , 0.01      );
	up_d_exit     = AddConstRealParameter("d_exit"  , 0.1  , "m"  , "Diameter of particle exit"                              , 0.01      );
	up_entryShape = AddComboParameter("Entry shape", EEntry::SLOT_RECT, entry_types, entry_names, "Gas entry shape");
	up_b          = AddConstRealParameter("b"       , 0.1  , "m"  , "Width of inlet"							             , 0.01      );
	up_h_e        = AddConstRealParameter("h_e"     , 0.2  , "m"  , "Height of inlet"							             , 0.01      );
	up_epsilon    = AddConstRealParameter("epsilon" , 270  , "deg", "Spiral angle for spiral gas entry"     	             , 0   , 360 );
	up_n_L        = AddConstRealParameter("n_L"     , 8    , "#"  , "Number of blades for axial gas entry"	                 , 1         );
	up_d_s        = AddConstRealParameter("d_s"     , 0.005, "m"  , "Thickness of blades for axial gas entry"	             , 0         );
	up_r_kern     = AddConstRealParameter("r_kern"  , 0.05 , "m"  , "Core radius of blades for axial entry"	                 , 0         );
	up_bladeShape = AddComboParameter("Blade shape", EBlade::STRAIGHT, blade_types, blade_names, "Blades shapes for axial gas entry");
	up_delta      = AddConstRealParameter("delta"   , 20   , "deg", "Angle of attack of blades for axial gas entry"          , 15  , 30  );
	up_lambda_0   = AddConstRealParameter("lambda_0", 0.005, "-"  , "Wall friction coefficient for pure gas"                 , 0   , 1e+6);
	up_D          = AddConstRealParameter("D"       , 3    , "-"  , "Coefficient for calculation of grade efficiency curve.\n"
		"2 for long thin cyclones and 4 for wide squat cyclones"                                                             , 2   , 4   );
	up_K_G        = AddConstRealParameter("K_G"     , 0.025, "-"  , "Constant for solids loading threshold.\n"
		"Small values for fine particles with large angles of repose.\n"
		"Large values for coarse particles with small angles of repose."                                                     , 0.02, 0.03);
	up_eta_adj    = AddConstRealParameter("eta_adj" , 1    , "-"  , "Separation efficiency adjustment factor"                , 0   , 1   );
	up_plot       = AddCheckBoxParameter("Plot", true, "Whether to generate plots and store state variables. Turn off to increase performance.");

	AddParametersToGroup(up_entryShape, EEntry::SPIRAL_FULL, { up_epsilon });
	AddParametersToGroup(up_entryShape, EEntry::SPIRAL_HALF, { up_epsilon });
	AddParametersToGroup(up_entryShape, EEntry::AXIAL      , { up_n_L, up_d_s, up_r_kern, up_bladeShape, up_delta });
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

	// Pointers to streams
	inlet   = GetPortStream("Inlet");
	outletS = GetPortStream("Outlet solids");
	outletG = GetPortStream("Outlet gas");

	// Flowsheet parameters
	classesNum = GetClassesNumber(DISTR_SIZE);
	averDiam   = GetClassesMeans(DISTR_SIZE);
	sizeGrid   = GetNumericGrid(DISTR_SIZE);

	// Create and initialize transformation matrices
	tm_i2s.Clear();
	tm_i2g.Clear();
	tm_i2s.SetDimensions(DISTR_SIZE, (unsigned)classesNum);
	tm_i2g.SetDimensions(DISTR_SIZE, (unsigned)classesNum);


	// Read unit parameters
	r_a        = 0.5 * up_d_out->GetValue();
	r_i        = 0.5 * up_d_dip-> GetValue();
	r_3        = 0.5 * up_d_exit->GetValue();
	b          = up_b->GetValue();
	h_tot      = up_h_tot->GetValue();
	h_dip      = up_h_dip->GetValue();
	h_cyl      = up_h_cyl->GetValue();
	h_e	       = up_h_e->GetValue();
	epsilon	   = up_epsilon->GetValue();
	n_L	       = up_n_L->GetValue();
	d_s	       = up_d_s->GetValue();
	r_kern     = up_r_kern->GetValue();
	delta	   = up_delta->GetValue() * MATH_PI / 180.;
	lambda_0   = up_lambda_0->GetValue();
	D          = up_D->GetValue();
	K_G        = up_K_G->GetValue();
	eta_adj    = up_eta_adj->GetValue();
	entryShape = static_cast<EEntry>(up_entryShape->GetValue());
	bladeShape = static_cast<EBlade>(up_bladeShape->GetValue());
	plot       = up_plot->IsChecked();

	// Calculation of geometric parameters
	switch (entryShape)
	{
	case EEntry::SLOT_RECT:		r_e = r_a - b / 2;	break;
	case EEntry::SPIRAL_FULL:	r_e = r_a + b / 2;	break;
	case EEntry::SPIRAL_HALF:	r_e = r_a;			break;
	case EEntry::AXIAL:			r_e = r_a - b / 2;	break;
	}
	r_2  = 0.5 * (r_3 + r_a);
	r_3_eff = r_3 <= r_i ? r_i : r_3;
	beta = b / r_a;

	h_con = h_tot - h_cyl;
	h_con_eff = (r_a - r_3_eff) / (r_a - r_3) * h_con;
	h_i   = h_cyl + h_con_eff - h_dip;
	a     = n_L != 0.0 ? sin(delta) * (MATH_PI * (r_a + r_kern) / n_L) - d_s : h_e;

	// Calculation of areas
	A_cyl   = 2 * MATH_PI * r_a * h_cyl;
	A_con   = MATH_PI * (r_a + r_3_eff) * sqrt(pow(r_a - r_3_eff, 2) + pow(h_con_eff, 2));
	A_top   = MATH_PI * pow(r_a, 2) - MATH_PI * pow(r_i, 2);
	A_dip   = 2 * MATH_PI * r_i * h_dip;
	switch (entryShape)
	{
	case EEntry::SLOT_RECT:		A_R = A_cyl + A_con + A_dip + A_top;	break;
	case EEntry::SPIRAL_FULL:	A_R = epsilon * r_a * h_e;				break;
	case EEntry::SPIRAL_HALF:	A_R = epsilon * r_a * h_e;				break;
	case EEntry::AXIAL:			A_R = A_cyl + A_con + A_dip + A_top;	break;
	}
	A_con_2 = MATH_PI * (r_a + r_2) * sqrt(pow(r_a - r_2, 2) + pow(h_con / 2, 2));
	A_w     = A_cyl + A_con_2;
	A_e1    = 2 * MATH_PI * r_a * h_e / 2;
	switch (entryShape)
	{
	case EEntry::SLOT_RECT:		A_R_sp = 0;											break;
	case EEntry::SPIRAL_FULL:	A_R_sp = epsilon * ((b + 2 * r_a) / 2 * (b + h_e));	break;
	case EEntry::SPIRAL_HALF:	A_R_sp = epsilon * r_a * (b + h_e);					break;
	case EEntry::AXIAL:			A_R_sp = 0;											break;
	}

	// Check for errors in definition of cyclone parameters
	if (!CheckCycloneParameters())
		RaiseError("Error in input parameters of cyclone.");

	// State variables
	AddStateVariable("Total separation efficiency [%]", 0);

	// Plots
	AddPlot("Separation", "Diameter [m]", "Separation efficiency [%]", "Time [s]");
	AddPlot("Main stream fraction", "Time [s]", "Fraction [-]");
	AddCurveOnPlot("Main stream fraction", "Fraction");
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
	// Initialize outlets
	outletS->CopyFromStream(_time, inlet);
	outletG->CopyFromStream(_time, inlet);
	outletS->SetMassFlow(_time, 0);
	outletG->SetMassFlow(_time, 0);

	// Calculation of separation process in cyclone
	CalculateSeparationMuschelknauz(_time);
}

void CCycloneMuschelknautz::CalculateSeparationMuschelknauz(double _time)
{
	// Material parameters
	const double eta_visc = inlet->GetPhaseProperty(_time, EPhase::GAS  , VISCOSITY); // Dynamic viscosity of gas [Pa*s]
	const double rho_g    = inlet->GetPhaseProperty(_time, EPhase::GAS  , DENSITY);   // Gas density at inlet [kg/m^3]
	const double rho_s    = inlet->GetPhaseProperty(_time, EPhase::SOLID, DENSITY);   // Solid density at inlet [kg/m^3]
	const double delta_rho = rho_s - rho_g;	                                          // Difference of densities at inlet [kg/m^3]
	if (delta_rho <= 0.0)
		RaiseError("Gas density larger than solids density.");

	// Inflow parameters
	const double mflow_in_g = inlet->GetPhaseMassFlow(_time, EPhase::GAS  ); // Gas mass flow at inlet [kg/s]
	const double mflow_in_s = inlet->GetPhaseMassFlow(_time, EPhase::SOLID); // Solids mass flow at inlet [kg/s]

	// If there is no solids inflow, all input gas just leaves through the gas exit
	// If there is no gas inflow, all solids just leave through the solids exit
	if (mflow_in_s == 0.0 || mflow_in_g == 0.0)
	{
		outletG->SetPhaseMassFlow(_time, EPhase::GAS  , mflow_in_g);
		outletS->SetPhaseMassFlow(_time, EPhase::SOLID, mflow_in_s);
		return;
	}

	// Operational parameters of cyclone
	const double mu_e     = mflow_in_s / mflow_in_g;      // Solids loading at inlet [kg/kg]
	const double Vflow_g  = mflow_in_g / rho_g;           // Gas volume flow at inlet [m^3/s]
	const double lambda_s = WallFrictionCoeff(mu_e);      // Wall friction coefficient for solids-containing gas [-]
	const double alpha    = ContractionCoefficient(mu_e); // Contraction coefficient [-]

	// Geometrical parameters
	const double r_e_mean = r_a - 0.5 * alpha * b; // Mean inlet radius [m]
	const double r_z_mean = sqrt(r_e_mean * r_2);  // Reference mean radius [m]

	// Velocities
	const double v_e    = InletVelocityStreamline(Vflow_g);                                                                // Inlet velocity in the middle streamline [m/s]
	const double w_s_50 = 0.5 * 0.9 * Vflow_g / A_w;                                                                       // Sinking speed of a particle going down the wall [m/s]
	const double u_a    = OuterTangVelocity(Vflow_g, v_e, alpha, lambda_s);                                                // Tangential velocity at cyclone radius [m/s]
	const double u_i    = u_a * (r_a / r_i) / (1 + lambda_s / 2 * (A_R / Vflow_g) * u_a * sqrt(r_a / r_i));                // Tangential velocity at the vortex finder [m/s]
	const double u_e    = u_a * r_a / r_e_mean / (1 + lambda_s / 2 * A_e1 / (0.9 * Vflow_g) * u_a * sqrt(r_a / r_e_mean)); // Tangential velocity at streamline radius at feed level [m/s]
	const double u_2    = u_a * (r_a / r_2) / (1 + lambda_s / 2 * (A_w / (0.9 * Vflow_g)) * u_a * sqrt(r_a / r_2));	       // Tangential velocity at mean cone radius at feed level [m/s]

	// Mass separation between main and secondary streams
	const double n = log(u_i / u_a) / log(r_a / r_i);                              // Parameter for calculating secondary stream [-]
	const double Vflow_sek = Vflow_g * (0.0497 + 0.0684 * n + 0.0949 * pow(n, 2)); // Gas volume flow of the secondary stream [m^3/s]
	const double w_split = 1 - Vflow_sek / Vflow_g;                                // Fraction of material going to the main stream [-]

	// PSD data
	const vector<double> psd_inflow = inlet->GetPSD(_time, PSD_MassFrac);         // PSD as mass fractions
	const vector<double> Q3_inflow  = inlet->GetPSD(_time, PSD_Q3);               // PSD as Q3
	const double d_50_A             = GetDistributionMedian(sizeGrid, Q3_inflow); // Particle size median

	// Separation at wall due to exceeding the loading limit in the main stream
	const double z_e_mean = u_e * u_2 / r_z_mean;                               // Mean centrifugal acceleration along streamline [m^2/s]
	const double d_e_star = sqrt(w_s_50 * 18 * eta_visc / (delta_rho * z_e_mean)); // Cut size for sedimentation on the first round (separation due to exceeding the loading limit) [m]
	const double k        = SolidsLoadingExp(mu_e);                             // Exponent for solids loading threshold [-]
	const double mu_G     = K_G * (d_e_star / d_50_A) * pow(10 * mu_e, k);      // Threshold for solids loading in the main stream [kg/kg]
	const double eta_e    = 1 - mu_G / mu_e;                                    // Separation efficiency due to exceeding of solids loading limit in the main stream (from main stream to solids) [-]

	// Separation in the internal vortex of the main stream
	const double d_star = sqrt(18 * eta_visc * 0.9 * Vflow_g / (delta_rho * pow(u_i, 2) * 2 * MATH_PI * h_i)); // Cut size for separation in the internal vortex of the main stream [m]
	auto eta_F_i = ReservedVector<double>(classesNum);                                                         // Fractional separation efficiency in internal vortex of the main stream (from internal vortex to solids) [-]
	for (const double d : averDiam)
		eta_F_i.push_back(CalculateSeparationEff(D, d_star, d));

	// Separation at wall due to exceeding the loading limit in the secondary stream
	const double mu_D = mu_e >= 6 * mu_G ? 6 * mu_G : mu_e; // Threshold for solids loading in the secondary stream [kg/kg]
	const double eta_g = 1 - mu_D / mu_e;                   // Separation efficiency due to exceeding of solids loading limit in the secondary stream (from secondary stream to solids) [-]

	// Separation in the internal vortex of the secondary stream
	const double d_star_T = sqrt(18 * eta_visc * Vflow_sek / (delta_rho * pow(2. / 3. * u_i, 2) * 2 * MATH_PI * h_dip)); // Cut size for separation at the vortex finder in the secondary stream [m]
	auto eta_F_T = ReservedVector<double>(classesNum);                                                                   // Fractional separation efficiency at the vortex finder of the secondary stream (from vortex finder to solids) [-]
	for (const double d : averDiam)
		eta_F_T.push_back(CalculateSeparationEff(3, d_star_T, d));

	// Overall separation
	auto eta_haupt = ReservedVector<double>(classesNum); // Fractional overall separation efficiency in the main stream [-]
	for (const double eta_i : eta_F_i)
		eta_haupt.push_back(mu_e > mu_G ? eta_e + (1 - eta_e) * eta_i : eta_i);
	auto eta_sek = ReservedVector<double>(classesNum);   // Fractional overall separation efficiency in the secondary stream [-]
	for (const double eta_T : eta_F_T)
		eta_sek.push_back(mu_e > mu_D ? eta_g + (1 - eta_g) * eta_T : eta_T);
	auto eta_tot = ReservedVector<double>(classesNum);   // Fractional total separation efficiency [-]
	for (size_t i = 0; i < classesNum; ++i)
		eta_tot.push_back(w_split * eta_haupt[i] + (1 - w_split) * eta_sek[i]);

	double mass_fraction_s = 0; // Total fraction of inlet solids going to solids outlet [-]
	for (size_t i = 0; i < classesNum; ++i)
	{
		const double frac_to_s = eta_tot[i] * eta_adj; // Mass fraction of input solids going to solids outlet in this size class [-]
		const double frac_to_g = 1. - frac_to_s;       // Mass fraction of input solids going to gas outlet in this size class [-]

		tm_i2s.SetValue((unsigned)i, (unsigned)i, frac_to_s);
		tm_i2g.SetValue((unsigned)i, (unsigned)i, frac_to_g);

		mass_fraction_s += psd_inflow[i] * frac_to_s;
	}

	// Apply transformation matrices
	outletS->ApplyTM(_time, tm_i2s);
	outletG->ApplyTM(_time, tm_i2g);

	// Set output mass flows
	outletS->SetPhaseMassFlow(_time, EPhase::SOLID, mflow_in_s * mass_fraction_s);
	outletS->SetPhaseMassFlow(_time, EPhase::GAS, 0);
	outletG->SetPhaseMassFlow(_time, EPhase::SOLID, mflow_in_s * (1 - mass_fraction_s));
	outletG->SetPhaseMassFlow(_time, EPhase::GAS, mflow_in_g);

	if (plot)
	{
		vector<double> eta_ges(classesNum);
		std::transform(eta_tot.begin(), eta_tot.end(), eta_ges.begin(), [](const double d) { return d * 100; });

		// Set state variables
		SetStateVariable("Total separation efficiency [%]", mass_fraction_s * 100, _time);

		// Plotting
		AddCurveOnPlot("Separation", _time);
		AddPointsOnCurve("Separation", _time, averDiam, eta_ges);
		AddPointOnCurve("Main stream fraction", "Fraction", _time, w_split);
	}
}

double CCycloneMuschelknautz::WallFrictionCoeff(double mu_e) const
{
	if (mu_e <= 1.0)
		return lambda_0 * (1 + 2 * sqrt(mu_e));
	return lambda_0 * (1 + 3 * sqrt(mu_e));
}

double CCycloneMuschelknautz::ContractionCoefficient(double mu_e) const
{
	switch (entryShape)
	{
	case EEntry::SLOT_RECT:
	case EEntry::SPIRAL_FULL:
	case EEntry::SPIRAL_HALF:
		return (1 - sqrt(1 + 4 * (pow(beta / 2, 2) - beta / 2) * sqrt(1 - (1 - pow(beta, 2)) / (1 + mu_e) * (2 * beta - pow(beta, 2))))) / beta;
	case EEntry::AXIAL:
	{
		switch (bladeShape)
		{
		case EBlade::STRAIGHT:       return 0.85;
		case EBlade::CURVED:         return 0.95;
		case EBlade::CURVED_TWISTED: return 1.05;
		}
		break;
	}
	}
	return {};
}

double CCycloneMuschelknautz::InletVelocityStreamline(double Vflow_g) const
{
	switch (entryShape)
	{
	case EEntry::SLOT_RECT:
	case EEntry::SPIRAL_FULL:
	case EEntry::SPIRAL_HALF: return Vflow_g / (h_e * b);
	case EEntry::AXIAL:       return Vflow_g / (a * b * n_L);
	}
	return {};
}

double CCycloneMuschelknautz::OuterTangVelocity(double Vflow_g, double v_e, double alpha, double lambda_s) const
{
	switch (entryShape)
	{
	case EEntry::SLOT_RECT:   return v_e * r_e / r_a / alpha;
	case EEntry::SPIRAL_FULL:
	case EEntry::SPIRAL_HALF: return v_e * r_e / r_a / (1 + lambda_s / 2 * A_R_sp / Vflow_g * v_e * sqrt(r_e / r_a));
	case EEntry::AXIAL:       return v_e * cos(delta) * r_e / r_a / alpha;
	}
	return {};
}

double CCycloneMuschelknautz::SolidsLoadingExp(double mu_e) const
{
	if (mu_e < 2.2e-5) return 0.81;
	if (mu_e < 0.015)  return 0.15 + 0.66 * exp(-pow((mu_e - 2.2e-5) / (0.015 -2.2e-5), 0.6));
	if (mu_e <= 0.1)   return 0.15 + 0.66 * exp(-pow((0.1 - 0.015) / (0.1 - mu_e), 0.1) * pow(mu_e / 0.015, 0.6));
	return 0.15;
}

double CCycloneMuschelknautz::CalculateSeparationEff(double Dval, double d_star, double d) const
{
	const double d_r = d / d_star;
	if (d_r < 1 / Dval)                 return 0;
	if (1 / Dval <= d_r && d_r <= Dval) return 0.5 * (1 + cos(0.5 * MATH_PI * (1 - log(d_r) / log(Dval))));
	return 1;
}
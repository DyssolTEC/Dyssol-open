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
	SetUnitName("Cyclone Muschelknautz");
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
	up_d_o         = AddConstRealParameter("d_o"     , 1    , "m"  , "Outer diameter of cyclone"                    , 0.01      );
	up_h_tot       = AddConstRealParameter("h_tot"   , 1    , "m"  , "Total height of cyclone"                      , 0.01      );
	up_h_cyl       = AddConstRealParameter("h_cyl"   , 0.25 , "m"  , "Height of the cylindrical part of cyclone"    , 0.01      );
	up_d_f         = AddConstRealParameter("d_f"     , 0.2  , "m"  , "Diameter of vortex finder"                    , 0.01      );
	up_h_f         = AddConstRealParameter("h_f"     , 0.2  , "m"  , "Height (depth) of vortex finder"              , 0.01      );
	up_d_exit      = AddConstRealParameter("d_exit"  , 0.1  , "m"   , "Diameter of particle exit"                   , 0.01      );
	up_entry_shape = AddComboParameter("Entry shape", EEntry::SLOT_RECT, entry_types, entry_names, "Gas entry shape");
	up_b_e         = AddConstRealParameter("b_e"     , 0.1  , "m"  , "Width of gas entry"						    , 0.01      );
	up_h_e         = AddConstRealParameter("h_e"     , 0.2  , "m"  , "Height of gas entry"						    , 0.01      );
	up_epsilon     = AddConstRealParameter("epsilon" , 270  , "deg", "Spiral angle in spiral gas entry"     	    , 0   , 360 );
	up_N_b         = AddConstRealParameter("N_b"     , 8    , "#"  , "Number of blades in axial gas entry"	        , 1         );
	up_d_b         = AddConstRealParameter("d_b"     , 0.005, "m"  , "Thickness of blades in axial gas entry"	    , 0         );
	up_r_core      = AddConstRealParameter("r_core"  , 0.05 , "m"  , "Core radius of blades in axial entry"	        , 0         );
	up_blade_shape = AddComboParameter("Blade shape", EBlade::STRAIGHT, blade_types, blade_names, "Blades shapes in axial gas entry");
	up_delta       = AddConstRealParameter("delta"   , 20   , "deg", "Angle of attack of blades in axial gas entry" , 15  , 30  );
	up_lambda_0    = AddConstRealParameter("lambda_0", 0.005, "-"  , "Wall friction coefficient of pure gas"        , 0   , 1e+6);
	up_D           = AddConstRealParameter("D"       , 3    , "-"  , "Coefficient for grid efficiency curve calculation according to Muschelknautz.\n"
		"2 for long thin cyclones, 4 for wide squat cyclones"                                                       , 2   , 4   );
	up_K_main      = AddConstRealParameter("K_main"  , 0.025, "-"  , "Constant for solids loading threshold in main stream.\n"
		"Small values for fine particles with large angles of repose.\n"
		"Large values for coarse particles with small angles of repose."                                            , 0.02, 0.03);
	up_eta_adj     = AddConstRealParameter("eta_adj" , 1    , "-"  , "Separation efficiency adjustment factor"      , 0   , 1   );
	up_plot        = AddCheckBoxParameter("Plot", true, "Whether to generate plots. Turn off to increase performance.");

	AddParametersToGroup(up_entry_shape, EEntry::SLOT_RECT  , { up_b_e });
	AddParametersToGroup(up_entry_shape, EEntry::SPIRAL_FULL, { up_b_e, up_epsilon });
	AddParametersToGroup(up_entry_shape, EEntry::SPIRAL_HALF, { up_b_e, up_epsilon });
	AddParametersToGroup(up_entry_shape, EEntry::AXIAL      , { up_N_b, up_d_b, up_r_core, up_blade_shape, up_delta });
}

void CCycloneMuschelknautz::Initialize(double _time)
{
	// Check for gas and solid phases
	if (!IsPhaseDefined(EPhase::GAS))
		RaiseError("Gas phase not defined.");
	if (!IsPhaseDefined(EPhase::SOLID))
		RaiseError("Solid phase not defined.");
	// Check for size distribution
	if (!IsDistributionDefined(DISTR_SIZE))
		RaiseError("Particle size distribution not defined.");

	// Pointers to streams
	inlet    = port_inlet->GetStream();
	outlet_s = port_outlet_s->GetStream();
	outlet_g = port_outlet_g->GetStream();

	// Flowsheet parameters
	classes_num = GetClassesNumber(DISTR_SIZE);
	aver_diam   = GetClassesMeans(DISTR_SIZE);
	size_grid   = GetNumericGrid(DISTR_SIZE);

	// Create and initialize transformation matrices
	tm_i2s.Clear();
	tm_i2g.Clear();
	tm_i2s.SetDimensions(DISTR_SIZE, (unsigned)classes_num);
	tm_i2g.SetDimensions(DISTR_SIZE, (unsigned)classes_num);

	// Read unit parameters
	r_o         = 0.5 * up_d_o->GetValue();
	r_f         = 0.5 * up_d_f-> GetValue();
	r_exit      = 0.5 * up_d_exit->GetValue();
	b_e         = up_b_e->GetValue();
	h_tot       = up_h_tot->GetValue();
	h_f         = up_h_f->GetValue();
	h_cyl       = up_h_cyl->GetValue();
	h_e	        = up_h_e->GetValue();
	epsilon	    = up_epsilon->GetValue();
	N_b	        = up_N_b->GetValue();
	d_b	        = up_d_b->GetValue();
	r_core      = up_r_core->GetValue();
	delta	    = up_delta->GetValue() * MATH_PI / 180.;
	lambda_0    = up_lambda_0->GetValue();
	D           = up_D->GetValue();
	K_main      = up_K_main->GetValue();
	eta_adj     = up_eta_adj->GetValue();
	entry_shape = static_cast<EEntry>(up_entry_shape->GetValue());
	blade_shape = static_cast<EBlade>(up_blade_shape->GetValue());
	plot        = up_plot->IsChecked();

	// Calculation of geometric parameters
	switch (entry_shape)
	{
	case EEntry::SLOT_RECT:							break;
	case EEntry::SPIRAL_FULL:						break;
	case EEntry::SPIRAL_HALF:						break;
	case EEntry::AXIAL:			b_e = r_o - r_core;	break;
	}
	switch (entry_shape)
	{
	case EEntry::SLOT_RECT:		r_e = r_o - b_e / 2;	break;
	case EEntry::SPIRAL_FULL:	r_e = r_o + b_e / 2;	break;
	case EEntry::SPIRAL_HALF:	r_e = r_o;				break;
	case EEntry::AXIAL:			r_e = r_o - b_e / 2;	break;
	}
	r_con_mean = 0.5 * (r_exit + r_o);
	r_exit_eff = r_exit <= r_f ? r_f : r_exit;
	beta       = b_e / r_o;
	h_con      = h_tot - h_cyl;
	h_con_eff  = (r_o - r_exit_eff) / (r_o - r_exit) * h_con;
	h_sep      = h_cyl + h_con_eff - h_f;
	switch (entry_shape)
	{
	case EEntry::SLOT_RECT:																					break;
	case EEntry::SPIRAL_FULL:																				break;
	case EEntry::SPIRAL_HALF:																				break;
	case EEntry::AXIAL:			a = N_b != 0.0 ? sin(delta) * (MATH_PI * (r_o + r_core) / N_b) - d_b : h_e;	break;
	}

	// Calculation of areas
	A_cyl = 2 * MATH_PI * r_o * h_cyl;
	A_con = MATH_PI * (r_o + r_exit_eff) * sqrt(pow(r_o - r_exit_eff, 2) + pow(h_con_eff, 2));
	A_top = MATH_PI * pow(r_o, 2) - MATH_PI * pow(r_f, 2);
	A_f   = 2 * MATH_PI * r_f * h_f;
	switch (entry_shape)
	{
	case EEntry::SLOT_RECT:		A_tot = A_cyl + A_con + A_f + A_top;						break;
	case EEntry::SPIRAL_FULL:	A_tot = A_cyl + A_con + A_f + A_top - epsilon * r_o * h_e;	break;
	case EEntry::SPIRAL_HALF:	A_tot = A_cyl + A_con + A_f + A_top - epsilon * r_o * h_e;	break;
	case EEntry::AXIAL:			A_tot = A_cyl + A_con + A_f + A_top;						break;
	}
	A_con_2 = MATH_PI * (r_o + r_con_mean) * sqrt(pow(r_o - r_con_mean, 2) + pow(h_con / 2, 2));
	A_sed   = A_cyl + A_con_2;
	A_e1    = 2 * MATH_PI * r_o * h_e / 2;
	switch (entry_shape)
	{
	case EEntry::SLOT_RECT:		A_sp = 0;												break;
	case EEntry::SPIRAL_FULL:	A_sp = epsilon * ((b_e + 2 * r_o) / 2 * (b_e + h_e));	break;
	case EEntry::SPIRAL_HALF:	A_sp = epsilon * r_o * (b_e + h_e);						break;
	case EEntry::AXIAL:			A_sp = 0;												break;
	}

	// Check for errors in definitions of cyclone parameters
	CheckCycloneParameters();

	// Plots
	plot_sep_3d    = AddPlot("Separation"                 , "Diameter [m]", "Separation efficiency [%]", "Time [s]");
	plot_sep       = AddPlot("Total separation efficiency", "Time [s]"    , "Efficiency [%]");
	plot_main_frac = AddPlot("Main stream fraction"       , "Time [s]"    , "Fraction [-]"  );
	curve_sep       = plot_sep->AddCurve("Efficiency");
	curve_main_frac = plot_main_frac->AddCurve("Fraction");
}

void CCycloneMuschelknautz::CheckCycloneParameters()
{
	auto CheckLess = [&](double v1, double v2, const string& name1, const string& name2)
	{
		if (v1 <= v2) RaiseError(StringFunctions::Quote(name1) + " <= " + StringFunctions::Quote(name2));
	};
	auto CheckZero = [&](double v, const string& name)
	{
		if (v <= 0.0) RaiseError(StringFunctions::Quote(name) + " <= 0");
	};

	// Check radii
	CheckLess(r_o, r_f   , "r_o", "r_f"   );
	CheckLess(r_o, r_exit, "r_o", "r_exit");
	CheckLess(r_o, b_e   , "r_o", "b_e"   );
	CheckZero(r_o       , "r_o"       );
	CheckZero(r_f       , "r_f"       );
	CheckZero(r_e       , "r_e"       );
	CheckZero(r_con_mean, "r_con_mean");
	// Check heights
	CheckLess(h_tot, h_f  , "h_tot", "h_f"  );
	CheckLess(h_tot, h_cyl, "h_tot", "h_cyl");
	CheckLess(h_tot, h_e  , "h_tot", "h_e"  );
	CheckZero(h_con, "h_con");
	CheckZero(h_sep, "h_sep");
	// Check areas
	CheckZero(A_e1   , "A_e1"   );
	CheckZero(A_cyl  , "A_cyl"  );
	CheckZero(A_con  , "A_con"  );
	CheckZero(A_f    , "A_f"    );
	CheckZero(A_top  , "A_top"  );
	CheckZero(A_tot  , "A_tot"  );
	CheckZero(A_con_2, "A_con_2");
	CheckZero(A_sed  , "A_sed"  );
	// Check geometry factor
	CheckZero(beta, "beta");
}

void CCycloneMuschelknautz::Simulate(double _time)
{
	// Initialize outlets
	outlet_s->CopyFromStream(_time, inlet);
	outlet_g->CopyFromStream(_time, inlet);
	outlet_s->SetMassFlow(_time, 0);
	outlet_g->SetMassFlow(_time, 0);

	// Calculation of separation process in cyclone
	CalculateSeparationMuschelknauz(_time);
}

void CCycloneMuschelknautz::CalculateSeparationMuschelknauz(double _time)
{
	// Material parameters
	const double eta_visc = inlet->GetPhaseProperty(_time, EPhase::GAS  , VISCOSITY); // Dynamic viscosity of gas at inlet [Pa*s]
	const double rho_g    = inlet->GetPhaseProperty(_time, EPhase::GAS  , DENSITY);   // Gas density at inlet [kg/m^3]
	const double rho_s    = inlet->GetPhaseProperty(_time, EPhase::SOLID, DENSITY);   // Solids density at inlet [kg/m^3]
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
		outlet_g->SetPhaseMassFlow(_time, EPhase::GAS  , mflow_in_g);
		outlet_s->SetPhaseMassFlow(_time, EPhase::SOLID, mflow_in_s);
		return;
	}

	// Operational parameters of cyclone
	const double Vflow_in_g = mflow_in_g / rho_g;            // Gas volume flow at inlet [m^3/s]
	const double mu_in      = mflow_in_s / mflow_in_g;       // Solids loading at inlet [kg/kg]
	const double lambda_s   = WallFrictionCoeff(mu_in);      // Wall friction coefficient of solids-containing gas [-]
	const double alpha      = ContractionCoefficient(mu_in); // Contraction coefficient [-]

	// Geometrical parameters
	const double r_e_mean = r_o - 0.5 * alpha * b_e;     // Mean radius of the gas streamline at gas entry [m]
	const double r_z_mean = sqrt(r_e_mean * r_con_mean); // Reference mean radius [m]

	// Velocities
	const double v_e   = InletVelocityStreamline(Vflow_in_g);                                                                         // Inlet velocity in the middle gas streamline at gas entry [m/s]
	const double w_50  = 0.5 * 0.9 * Vflow_in_g / A_sed;                                                                              // Sinking speed at which 50% of particles are sedimented at wall [m/s]
	const double u_o   = OuterTangVelocity(Vflow_in_g, v_e, alpha, lambda_s);                                                         // Tangential velocity at outer cyclone radius [m/s]
	const double u_f   = u_o * (r_o / r_f) / (1 + lambda_s / 2 * (A_tot / Vflow_in_g) * u_o * sqrt(r_o / r_f));                       // Tangential velocity at vortex finder [m/s]
	const double u_e   = u_o * r_o / r_e_mean / (1 + lambda_s / 2 * A_e1 / (0.9 * Vflow_in_g) * u_o * sqrt(r_o / r_e_mean));          // Tangential velocity at gas streamline radius at gas entry [m/s]
	const double u_con = u_o * (r_o / r_con_mean) / (1 + lambda_s / 2 * (A_sed / (0.9 * Vflow_in_g)) * u_o * sqrt(r_o / r_con_mean)); // Tangential velocity at mean cone radius [m/s]

	// Mass separation between main and secondary streams
	const double n = log(u_f / u_o) / log(r_o / r_f);                                 // Parameter for calculating secondary stream [-]
	const double Vflow_sec = Vflow_in_g * (0.0497 + 0.0684 * n + 0.0949 * pow(n, 2)); // Gas volume flow of secondary stream [m^3/s]
	const double w_split = 1 - Vflow_sec / Vflow_in_g;                                // Fraction of material going to main stream [-]

	// PSD data
	const vector<double> psd_inflow = inlet->GetPSD(_time, PSD_MassFrac);         // PSD as mass fractions
	const vector<double> Q3_inflow  = inlet->GetPSD(_time, PSD_Q3);               // PSD as Q3
	const double d_50               = GetDistributionMedian(size_grid, Q3_inflow); // Particle size median

	// Separation at wall due to exceeding the loading limit in the main stream
	const double z_e_mean      = u_e * u_con / r_z_mean;                               // Mean centrifugal acceleration along streamline [m^2/s]
	const double d_star_main_l = sqrt(w_50 * 18 * eta_visc / (delta_rho * z_e_mean));  // Cut size of separation on the first revolution due to exceeding the loading limit [m]
	const double k             = SolidsLoadingExp(mu_in);                              // Exponent for solids loading threshold in main stream [-]
	const double mu_main       = K_main * (d_star_main_l / d_50) * pow(10 * mu_in, k); // Threshold for solids loading in main stream [kg/kg]
	const double eta_main_l    = 1 - mu_main / mu_in;                                  // Separation efficiency due to exceeding of solids loading limit in main stream (from main stream to solids output) [-]

	// Separation in the internal vortex of the main stream
	const double d_star_main_v = sqrt(18 * eta_visc * 0.9 * Vflow_in_g / (delta_rho * pow(u_f, 2) * 2 * MATH_PI * h_sep)); // Cut size of separation in internal vortex of main stream [m]
	auto eta_main_v = ReservedVector<double>(classes_num);                                                                  // Separation efficiency in internal vortex (from internal vortex to solids output) [-]
	for (const double d : aver_diam)
		eta_main_v.push_back(CalculateSeparationEff(D, d_star_main_v, d));

	// Separation at wall due to exceeding the loading limit in the secondary stream
	const double mu_sec    = mu_in >= 6 * mu_main ? 6 * mu_main : mu_in; // Threshold for solids loading in secondary stream [kg/kg]
	const double eta_sec_l = 1 - mu_sec / mu_in;				         // Separation efficiency due to exceeding of solids loading limit in secondary stream (from secondary stream to solids output) [-]

	// Separation at vortex finder of the secondary stream
	const double d_star_sec_v = sqrt(18 * eta_visc * Vflow_sec / (delta_rho * pow(2. / 3. * u_f, 2) * 2 * MATH_PI * h_f)); // Cut size of separation at vortex finder in secondary stream [m]
	auto eta_sec_v = ReservedVector<double>(classes_num);                                                                   // Separation efficiency at vortex finder (from vortex finder to solids output) [-]
	for (const double d : aver_diam)
		eta_sec_v.push_back(CalculateSeparationEff(3, d_star_sec_v, d));

	// Overall separation
	auto eta_main = ReservedVector<double>(classes_num); // Overall separation efficiency in main stream [-]
	for (const double eta : eta_main_v)
		eta_main.push_back(mu_in > mu_main ? eta_main_l + (1 - eta_main_l) * eta : eta);
	auto eta_sec = ReservedVector<double>(classes_num);  // Overall separation efficiency in secondary stream [-]
	for (const double eta : eta_sec_v)
		eta_sec.push_back(mu_in > mu_sec ? eta_sec_l + (1 - eta_sec_l) * eta : eta);
	auto eta_tot = ReservedVector<double>(classes_num);  // Total separation efficiency of cyclone [-]
	for (size_t i = 0; i < classes_num; ++i)
		eta_tot.push_back(w_split * eta_main[i] + (1 - w_split) * eta_sec[i]);

	double mass_fraction_s = 0; // Total fraction of inlet solids going to solids outlet [-]
	for (size_t i = 0; i < classes_num; ++i)
	{
		const double frac_to_s = eta_tot[i] * eta_adj; // Mass fraction of input solids going to solids outlet in this size class [-]
		const double frac_to_g = 1. - frac_to_s;       // Mass fraction of input solids going to gas outlet in this size class [-]

		tm_i2s.SetValue((unsigned)i, (unsigned)i, frac_to_s);
		tm_i2g.SetValue((unsigned)i, (unsigned)i, frac_to_g);

		mass_fraction_s += psd_inflow[i] * frac_to_s;
	}

	// Apply transformation matrices
	outlet_s->ApplyTM(_time, tm_i2s);
	outlet_g->ApplyTM(_time, tm_i2g);

	// Set output mass flows
	outlet_s->SetPhaseMassFlow(_time, EPhase::SOLID, mflow_in_s * mass_fraction_s);
	outlet_s->SetPhaseMassFlow(_time, EPhase::GAS, 0);
	outlet_g->SetPhaseMassFlow(_time, EPhase::SOLID, mflow_in_s * (1 - mass_fraction_s));
	outlet_g->SetPhaseMassFlow(_time, EPhase::GAS, mflow_in_g);

	if (plot)
	{
		vector<double> eta_tot_perc(classes_num); // Total separation efficiency in %
		std::transform(eta_tot.begin(), eta_tot.end(), eta_tot_perc.begin(), [](const double d) { return d * 100; });

		// Plotting
		plot_sep_3d->AddCurve(_time, aver_diam, eta_tot_perc);
		curve_sep->AddPoint(_time, mass_fraction_s * 100);
		curve_main_frac->AddPoint(_time, w_split);
	}
}

double CCycloneMuschelknautz::WallFrictionCoeff(double mu_in) const
{
	if (mu_in <= 1.0)
		return lambda_0 * (1 + 2 * sqrt(mu_in));
	return lambda_0 * (1 + 3 * sqrt(mu_in));
}

double CCycloneMuschelknautz::ContractionCoefficient(double mu_in) const
{
	switch (entry_shape)
	{
	case EEntry::SLOT_RECT:
	case EEntry::SPIRAL_FULL:
	case EEntry::SPIRAL_HALF:
		return (1 - sqrt(1 + 4 * (pow(beta / 2, 2) - beta / 2) * sqrt(1 - (1 - pow(beta, 2)) / (1 + mu_in) * (2 * beta - pow(beta, 2))))) / beta;
	case EEntry::AXIAL:
	{
		switch (blade_shape)
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

double CCycloneMuschelknautz::InletVelocityStreamline(double Vflow_in_g) const
{
	switch (entry_shape)
	{
	case EEntry::SLOT_RECT:
	case EEntry::SPIRAL_FULL:
	case EEntry::SPIRAL_HALF: return Vflow_in_g / (h_e * b_e);
	case EEntry::AXIAL:       return Vflow_in_g / (a * b_e * N_b);
	}
	return {};
}

double CCycloneMuschelknautz::OuterTangVelocity(double Vflow_in_g, double v_e, double alpha, double lambda_s) const
{
	switch (entry_shape)
	{
	case EEntry::SLOT_RECT:   return v_e * r_e / r_o / alpha;
	case EEntry::SPIRAL_FULL:
	case EEntry::SPIRAL_HALF: return v_e * r_e / r_o / (1 + lambda_s / 2 * A_sp / Vflow_in_g * v_e * sqrt(r_e / r_o));
	case EEntry::AXIAL:       return v_e * cos(delta) * r_e / r_o / alpha;
	}
	return {};
}

double CCycloneMuschelknautz::SolidsLoadingExp(double mu_in) const
{
	if (mu_in < 2.2e-5) return 0.81;
	if (mu_in < 0.015)  return 0.15 + 0.66 * exp(-pow((mu_in - 2.2e-5) / (0.015 - 2.2e-5), 0.6));
	if (mu_in <= 0.1)   return 0.15 + 0.66 * exp(-pow((0.1 - 0.015) / (0.1 - mu_in), 0.1) * pow(mu_in / 0.015, 0.6));
	return 0.15;
}

double CCycloneMuschelknautz::CalculateSeparationEff(double Dval, double d_star, double d) const
{
	const double d_r = d / d_star;
	if (d_r < 1 / Dval)                 return 0;
	if (1 / Dval <= d_r && d_r <= Dval) return 0.5 * (1 + cos(0.5 * MATH_PI * (1 - log(d_r) / log(Dval))));
	return 1;
}
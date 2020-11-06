/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "Crusher.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CCrusher();
}

void CCrusher::CreateBasicInfo()
{
	/// Basic unit's info ///
	m_sUnitName = "Crusher";
	m_sAuthorName = "SPE TUHH";
	m_sUniqueID = "4E2C9FB3BFA44B8E829AC393042F2BE3";
}

void CCrusher::CreateStructure()
{
		/// Add ports ///
	AddPort("Input", INPUT_PORT);
	AddPort("Output", OUTPUT_PORT);

	/// Add unit parameters ///
	AddGroupParameter("Model", BondNormal, { BondNormal, BondBimodal, Cone, Const }, { "Bond (normal distribution)", "Bond (bimodal breakage)", "Cone", "Const" }, "Crushing model");
	AddTDParameter(   "P",         0,     1000000,                            50,     "kW", "Power input");
	AddTDParameter(   "Mean",      0,     std::numeric_limits<double>::max(), 0.001,  "m",  "Mean value of the normal output distribution");
	AddTDParameter(   "Deviation", 0,     1000000,                            0.0001, "m",  "Standard deviation of the normal output distribution");
	AddConstParameter("CSS",       1e-12, 10,                                 0.04,   "m",  "Closed-side setting parameter of the King selection function");
	AddConstParameter("alpha1",    0.5,   0.95,                               0.6,    "-",  "Alpha1 parameter of the King selection function");
	AddConstParameter("alpha2",    1.7,   3.5,                                2,      "-",  "Alpha2 parameter of the King selection function");
	AddConstParameter("n",         1,     3,                                  2,      "-",  "Power parameter of the King selection function");
	AddConstParameter("d'",        0,     10e+10,                             0.003,  "m",  "Minimum fragment size of the Vogel breakage function");
	AddConstParameter("q",         0,     10e+10,                             0.55,   "-",  "Q parameter of the Vogel breakage function");

	/// Group unit parameters ///
	AddParametersToGroup("Model", "Bond (normal distribution)", { "P", "Deviation"                          });
	AddParametersToGroup("Model", "Bond (bimodal breakage)",    { "P"                                       });
	AddParametersToGroup("Model", "Cone",                       { "CSS", "alpha1", "alpha2", "n", "d'", "q" });
	AddParametersToGroup("Model", "Const",                      { "Mean", "Deviation"                       });
}

void CCrusher::Initialize(double _time)
{
	/// Check flowsheet parameters ///
	if (!IsPhaseDefined(SOA_SOLID))			RaiseError("Solid phase has not been defined.");
	if (!IsDistributionDefined(DISTR_SIZE))	RaiseError("Size distribution has not been defined.");

	/// Get pointers to streams ///
	m_inlet  = GetPortStream("Input");
	m_outlet = GetPortStream("Output");

	/// Get PSD grid parameters ///
	m_classesNumber = GetClassesNumber(DISTR_SIZE);
	m_grid          = GetNumericGrid(DISTR_SIZE);
	m_diameters     = GetClassesMeans(DISTR_SIZE);

	/// Check PSD grid parameters ///
	if (m_classesNumber == 0)					RaiseError("There are no size classes in size distribution.");

	/// Get compounds parameters ///
	m_compounds = GetCompoundsList();

	/// Get selected crushing model ///
	m_model = static_cast<EModels>(GetGroupParameterValue("Model"));

	/// Model-specific initialization ///
	switch (m_model)
	{
	case BondNormal:  InitializeBondNormal(_time);	break;
	case BondBimodal: InitializeBondBimodal(_time);	break;
	case Cone:		  InitializeCone(_time);		break;
	case Const:		  InitializeConst(_time);		break;
	}
}

void CCrusher::Simulate(double _time)
{
	/// Copy the time point from inlet to outlet ///
	m_outlet->CopyFromStream(m_inlet, _time);

	/// Model-specific simulation ///
	switch (m_model)
	{
	case BondNormal:	SimulateBondNormal(_time);	break;
	case BondBimodal:	SimulateBondBimodal(_time);	break;
	case Cone:			SimulateCone(_time);		break;
	case Const:			SimulateConst(_time);		break;
	}
}

void CCrusher::InitializeBondNormal(double _time)
{
}

void CCrusher::SimulateBondNormal(double _time)
{
	// get current unit parameters
	const double power = GetTDParameterValue("P", _time);
	const double sigma = GetTDParameterValue("Deviation", _time);

	// check unit parameters
	if (power <= 0)	RaiseError("Parameter 'P' has to be larger than 0.");
	if (sigma <= 0)	RaiseError("Parameter 'Deviation' has to be larger than 0.");

	// return if any error occured
	if (CheckError()) return;

	const double solidMassFlow = m_inlet->GetPhaseMassFlow(_time, SOA_SOLID) * 3.6;
	if (solidMassFlow == 0)
		RaiseWarning("Solid mass flow in crusher equals to 0.");

	const double workInput = solidMassFlow != 0 ? power / solidMassFlow : power;

	// component-wise application of Bond model
	for (const auto& compound : m_compounds)
	{
		if (m_inlet->GetCompoundPhaseFraction(_time, compound, SOA_SOLID) == 0) continue; // this component is not a solid

		const std::vector<double> Q3 = m_inlet->GetPSD(_time, PSD_Q3, compound);
		if (Q3.empty())
			RaiseWarning("No size distribution in input stream.");

		const double x80In = GetDistributionValue(m_grid, Q3, 0.8) * 1000;
		if (x80In <= 0)
			RaiseWarning("Characteristic distribution value of input X80 <= 0.");

		const std::vector<double> psdIn = m_inlet->GetPSD(_time, PSD_MassFrac, compound);

		const double bondIndex = m_inlet->GetCompoundConstant(compound, BOND_WORK_INDEX);
		const double x80Out = 1. / std::pow(workInput / (10 * bondIndex) + 1. / std::sqrt(x80In), 2) / 1000;
		const double meanOut = x80Out - 0.83 * sigma;

		if (meanOut <= 0)
			RaiseWarning("Unable to calculate parameters of output distribution. Try to decrease 'P' or 'Deviation' parameter.");

		// calculate output distribution
		std::vector<double> q3Out(m_classesNumber, 0);
		const double A = 1 / (sigma * std::sqrt(2 * MATH_PI));
		for (unsigned i = 0; i < m_classesNumber; ++i)
		{
			q3Out[i] = A * std::exp(-std::pow(m_diameters[i] - meanOut, 2) / (2 * sigma * sigma));
			if (q3Out[i] < 0)
				RaiseWarning("Unable to calculate output distribution. Try to decrease 'P' or 'Deviation' parameter.");
		}

		// calculate transformation matrix
		m_transform.Clear();
		const std::vector<double> psdOut = Convertq3ToMassFractions(m_grid, q3Out);
		CalculateTM(DISTR_SIZE, psdIn, psdOut, m_transform);

		// apply transformation matrix to output
		m_outlet->ApplyTM(_time, compound, m_transform);
	}

	// component-wise correction of compounds fractions
	for (const auto& compound : m_compounds)
		m_outlet->SetCompoundPhaseFraction(_time, compound, SOA_SOLID, m_inlet->GetCompoundPhaseFraction(_time, compound, SOA_SOLID));
}

void CCrusher::InitializeBondBimodal(double _time)
{
	// add internal material stream
	m_internal = AddMaterialStream("TempStream");

	// configure transformation matrix
	m_transform.Clear();
	m_transform.SetDimensions(DISTR_SIZE, m_classesNumber);
}

void CCrusher::SimulateBondBimodal(double _time)
{
	// get current unit parameters
	const double power = GetTDParameterValue("P", _time);

	// check unit parameters
	if (power <= 0)	RaiseError("Parameter 'P' has to be larger than 0.");

	// return if any error occured
	if (CheckError()) return;

	const double solidMassFlow = m_inlet->GetPhaseMassFlow(_time, SOA_SOLID) * 3.6;
	if (solidMassFlow == 0)
		RaiseWarning("Solid mass flow in crusher equals to 0.");

	const double workInput = solidMassFlow != 0 ? power / solidMassFlow : power;

	// component-wise application of Bond model
	for (const auto& compound : m_compounds)
	{
		if (m_inlet->GetCompoundPhaseFraction(_time, compound, SOA_SOLID) == 0) continue; // this component is not a solid

		const std::vector<double> Q3 = m_inlet->GetPSD(_time, PSD_Q3, compound);
		if (Q3.empty())
			RaiseWarning("No size distribution in input stream.");

		const double x80In = GetDistributionValue(m_grid, Q3, 0.8);
		if (x80In <= 0)
			RaiseWarning("Characteristic distribution value of input X80 <= 0.");

		const std::vector<double> psdIn = m_inlet->GetPSD(_time, PSD_MassFrac, compound);
		if (psdIn.empty())
			RaiseWarning("No size distribution in input stream.");

		const double bondIndex = m_inlet->GetCompoundConstant(compound, BOND_WORK_INDEX);
		const double x80Out = 1. / std::pow(workInput / (10 * bondIndex) + 1. / std::sqrt(x80In * 1000), 2) / 1000;
		double x80New = x80In;

		// iterative search for suitable transformation
		size_t iteration = 0;
		double currentFraction = 1;                        // currently assumed mass fraction of material which will be crushed
		double stepFactor = 1;                             // initial factor for increase/decrease
		while (std::fabs(x80New - x80Out) > 1e-4 * x80Out) // allowed deviation between d80's is 0.01%
		{
			m_transform.ClearData();
			m_internal->CopyFromStream(m_outlet, _time);
			ParallelFor(m_classesNumber, [&](size_t i)
			{
				// get left and right diameters of initial class on new grid
				double lDiameter = m_grid[i] / std::pow(2, 1. / 3.);
				double rDiameter = m_grid[i + 1] / std::pow(2, 1. / 3.);
				const double totalLength = rDiameter - lDiameter;

				// redistribute between classes on new grid
				unsigned iRClass = (unsigned)i + 1;
				while (iRClass > 0 && m_grid[iRClass] > rDiameter)
					iRClass--;
				while (iRClass > 0 && m_grid[iRClass] >= lDiameter)
				{
					const double fraction = (rDiameter - m_grid[iRClass]) / totalLength;
					rDiameter = m_grid[iRClass];
					m_transform.SetValue((unsigned)i, iRClass, currentFraction * fraction);
					iRClass--;
				}
				if (iRClass != 0)
				{
					const double fraction = std::min(1., (m_grid[iRClass + 1] - lDiameter) / totalLength);
					m_transform.SetValue((unsigned)i, iRClass, currentFraction * fraction);
				}
				m_transform.SetValue((unsigned)i, (unsigned)i, 1 - currentFraction);
			});
			m_internal->ApplyTM(_time, compound, m_transform);
			x80New = GetDistributionValue(m_grid, m_internal->GetPSD(_time, PSD_Q3, compound), 0.8);
			if (x80New > x80Out)
			{
				if (currentFraction == 1) // directly perform total transformation
					m_outlet->ApplyTM(_time, compound, m_transform);
				else
				{
					if (stepFactor < 0)
						stepFactor *= -0.5; // change magnitude and direction of search
					currentFraction += stepFactor;
				}
			}
			else
			{
				if (stepFactor > 0)
					stepFactor *= -0.5; // change magnitude and direction of search
				currentFraction += stepFactor;
			}

			if (iteration == 100)
				RaiseError("Cannot reach x80 = " + StringFunctions::Double2String(x80Out) + ". Current value " + StringFunctions::Double2String(x80New));
			iteration++;

			if (CheckError()) return;
		}
		// set result to output
		m_outlet->ApplyTM(_time, compound, m_transform); // apply rest
	}

	// component-wise correction of compounds fractions
	for (const auto& compound : m_compounds)
		m_outlet->SetCompoundPhaseFraction(_time, compound, SOA_SOLID, m_inlet->GetCompoundPhaseFraction(_time, compound, SOA_SOLID));
}

void CCrusher::InitializeCone(double _time)
{
	// get unit parameters
	const double CSS    = GetConstParameterValue("CSS");
	const double alpha1 = GetConstParameterValue("alpha1");
	const double alpha2 = GetConstParameterValue("alpha2");
	const double n      = GetConstParameterValue("n");
	const double FS     = GetConstParameterValue("d'");
	const double q      = GetConstParameterValue("q");

	// check unit parameters
	if (CSS <= 0)		RaiseError("Parameter 'CSS' may not be smaller or equal to 0.");
	if (alpha1 <= 0)	RaiseError("Parameter 'alpha1' may not be smaller or equal to 0.");
	if (alpha2 <= 0)	RaiseError("Parameter 'alpha2' may not be smaller or equal to 0.");
	if (FS <= 0)		RaiseError("Parameter 'd'' may not be smaller or equal to 0.");

	// return if any error occured
	if (CheckError()) return;

	// configure transformation matrix
	m_transform.Clear();
	m_transform.SetDimensions(DISTR_SIZE, m_classesNumber);

	// calculate values for King selection function
	const double minDiameter = CSS * alpha1;
	const double maxDiameter = CSS * alpha2;
	const double deltaDiameter = maxDiameter - minDiameter;
	std::vector<double> selection(m_classesNumber);
	for (size_t i = 0; i < m_classesNumber; ++i)
		if (m_diameters[i] >= maxDiameter)		selection[i] = 1;
		else if (m_diameters[i] <= minDiameter)	selection[i] = 0;
		else									selection[i] = 1 - std::pow((maxDiameter - m_diameters[i]) / deltaDiameter, n);

	// calculate values for Vogel breakage function
	std::vector<std::vector<double>> breakage(m_classesNumber, std::vector<double>(m_classesNumber, 0));
	breakage.resize(m_classesNumber, std::vector<double>(m_classesNumber, 0));
	ParallelFor(m_classesNumber, [&](size_t i)
	{
		for (size_t k = 0; k < m_classesNumber; ++k)
			if (k <= i)
				breakage[i][k] = std::pow(m_diameters[i] / m_diameters[k], q) * 0.5 * (1 + std::tanh((m_diameters[k] - FS) / FS));
		Normalize(breakage[i]);
	});

	// calculate transformation matrix
	ParallelFor(m_classesNumber, [&](size_t iSrc)
	{
		for (unsigned iDst = 0; iDst < m_classesNumber; ++iDst)
			if (iSrc == iDst)
				m_transform.SetValue(unsigned(iSrc), iDst, 1 - selection[iSrc]);
			else
				m_transform.SetValue(unsigned(iSrc), iDst, breakage[iSrc][iDst] * selection[iSrc]);
	});
}

void CCrusher::SimulateCone(double _time)
{
	// transform PSD in outlet
	m_outlet->ApplyTM(_time, m_transform);
}

void CCrusher::InitializeConst(double _time)
{
}

void CCrusher::SimulateConst(double _time)
{
	// get current unit parameters
	const double x50   = GetTDParameterValue("Mean", _time);
	const double sigma = GetTDParameterValue("Deviation", _time);

	// check unit parameters
	if (x50 <= 0)	RaiseError("Parameter 'Mean' has to be larger than 0.");
	if (sigma <= 0)	RaiseError("Parameter 'Deviation' has to be larger than 0.");

	// calculate output q3
	std::vector<double> q3(m_classesNumber, 0);
	for (unsigned i = 0; i < m_classesNumber; ++i)
		q3[i] = 1 / (sigma * std::pow(2 * MATH_PI, 0.5)) * std::exp(-std::pow(m_diameters[i] - x50, 2) / (2 * sigma * sigma));

	// get current inlet
	const std::vector<double> psdIn = m_inlet->GetPSD(_time, PSD_MassFrac);
	if (psdIn.empty())
		RaiseWarning("No size distribution in input stream.");

	// calculate transformation matrix
	m_transform.Clear();
	const std::vector<double> psdOut = Convertq3ToMassFractions(m_grid, q3);
	CalculateTM(DISTR_SIZE, psdIn, psdOut, m_transform);

	// set result to output
	m_outlet->ApplyTM(_time, m_transform);
}

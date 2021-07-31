/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#define DLL_EXPORT
#include "CrusherPBMTM.h"

extern "C" DECLDIR CBaseUnit* DYSSOL_CREATE_MODEL_FUN()
{
	return new CCrusherPBMTM();
}

//////////////////////////////////////////////////////////////////////////
/// Unit

void CCrusherPBMTM::CreateBasicInfo()
{
	/// Basic unit's info ///
	SetUnitName("Crusher PBM TM");
	SetAuthorName("SPE TUHH");
	SetUniqueID("2FD8955A80D341288031E1063B9FC822");
}

void CCrusherPBMTM::CreateStructure()
{
		/// Add ports ///
	AddPort("Input", EUnitPort::INPUT);
	AddPort("Output", EUnitPort::OUTPUT);

	/// Add unit parameters ///
	AddComboParameter("Selection", E2I(ESelection::CONSTANT),
		{ E2I(ESelection::CONSTANT), E2I(ESelection::LINEAR), E2I(ESelection::QUADRATIC), E2I(ESelection::POWER), E2I(ESelection::EXPONENTIAL),	E2I(ESelection::KING), E2I(ESelection::AUSTIN) },
		{ "Constant",                "Linear",                "Quadratic",                "Power",                "Exponential",                "King",                "Austin"                }, "Selection function");
	AddComboParameter("Breakage", E2I(EBreakage::BINARY),
		{ E2I(EBreakage::BINARY), E2I(EBreakage::DIEMER), E2I(EBreakage::VOGEL), E2I(EBreakage::AUSTIN) },
		{ "Binary",               "Diemer",               "Vogel",               "Austin"               }, "Breakage function");

	AddConstRealParameter("S_scale", 1   , "-", "Scale factor for Selection function", 0, 1   );
	AddConstRealParameter("S1"     , 3   , "-", "Parameter of Selection function"             );
	AddConstRealParameter("S2"     , 3   , "-", "Parameter of Selection function"             );
	AddConstRealParameter("S3"     , 3   , "-", "Parameter of Selection function"             );
	AddConstRealParameter("B1"     , 15  , "-", "Parameter of Breakage function"              );
	AddConstRealParameter("B2"     , 5   , "-", "Parameter of Breakage function"              );
	AddConstRealParameter("B3"     , 5   , "-", "Parameter of Breakage function"              );
	AddConstRealParameter("dt_min" , 0   , "s", "Minimum time step for integration"  , 0, 1e+9);
	AddConstRealParameter("dt_max" , 1e+9, "s", "Maximum time step for integration"  , 0, 1e+9);
	AddComboParameter("Method", E2I(EMethod::NEWTON), { E2I(EMethod::NEWTON), E2I(EMethod::KR2) }, { "Newton", "Runge-Kutta" },	"Method for calculating transformation matrices");

	AddParametersToGroup("Selection", "Constant",    { "S1" });
	AddParametersToGroup("Selection", "Linear",      { });
	AddParametersToGroup("Selection", "Quadratic",   { });
	AddParametersToGroup("Selection", "Power",       { "S1" });
	AddParametersToGroup("Selection", "Exponential", { "S1" });
	AddParametersToGroup("Selection", "King",        { "S1", "S2", "S3" });
	AddParametersToGroup("Selection", "Austin",      { "S1", "S2" });

	AddParametersToGroup("Breakage", "Binary", { });
	AddParametersToGroup("Breakage", "Diemer", { "B1", "B2" });
	AddParametersToGroup("Breakage", "Vogel",  { "B1", "B2" });
	AddParametersToGroup("Breakage", "Austin", { "B1", "B2" , "B3" });

	/// Add holdups ///
	AddHoldup("Holdup");
}

void CCrusherPBMTM::Initialize(double _time)
{
	/// Check presence of distribution by size ///
	if (!IsPhaseDefined(EPhase::SOLID))		RaiseWarning("Solid phase has not been defined.");
	if (!IsDistributionDefined(DISTR_SIZE))	RaiseWarning("Size distribution has not been defined.");

	/// Get pointers to streams ///
	m_inStream = GetPortStream("Input");
	m_outStream = GetPortStream("Output");

	/// Get pointers to holdups ///
	m_holdup = GetHoldup("Holdup");
	m_holdupMass = m_holdup->GetMass(_time);

	/// Get number of classes for PSD ///
	m_classesNum = GetClassesNumber(DISTR_SIZE);
	m_grid = GetPSDGridVolumes();
	m_means = GetPSDMeanVolumes();
	m_sizes.resize(m_classesNum);
	for (size_t i = 0; i < m_classesNum; ++i)
		m_sizes[i] = m_grid[i + 1] - m_grid[i];

	/// Configure transformation matrix ///
	m_TM.SetDimensions(DISTR_SIZE, (unsigned)m_classesNum);

	/// Unit parameters
	m_selectionFun = static_cast<ESelection>(GetComboParameterValue("Selection"));
	m_breakageFun  = static_cast<EBreakage>(GetComboParameterValue("Breakage"));
	m_sscale = GetConstRealParameterValue("S_scale");
	m_s1 = GetConstRealParameterValue("S1");
	m_s2 = GetConstRealParameterValue("S2");
	m_s3 = GetConstRealParameterValue("S3");
	m_b1 = GetConstRealParameterValue("B1");
	m_b2 = GetConstRealParameterValue("B2");
	m_b3 = GetConstRealParameterValue("B3");
	m_dtMin = GetConstRealParameterValue("dt_min");
	m_dtMax = GetConstRealParameterValue("dt_max");
	m_method = static_cast<EMethod>(GetComboParameterValue("Method"));

	/// Precalculate values
	m_S = CalculateSelectionFunction(m_means);
	m_B = CalculateBreakageFunction(m_means);
	m_nu = CalculateNu(m_means);
	m_WB = CalculateBirthWeights(m_means);
	m_WD = CalculateDeathWeights(m_means);
	m_PT = CalculateBaseTransformationMatrix();
	m_I = CMatrix2D::Identity(m_classesNum);
}

void CCrusherPBMTM::Simulate(double _timeBeg, double _timeEnd)
{
	if (_timeBeg == 0)
		m_outStream->CopyFromHoldup(0, m_holdup, m_inStream->GetMassFlow(0));

	double t1 = _timeBeg;
	while (t1 < _timeEnd)
	{
		const double dtCalc = m_dtMin == m_dtMax ? m_dtMin : MaxTimeStep(_timeEnd - t1, m_holdup->GetPSD(t1, PSD_q0, EPSDGridType::VOLUME));
		const double dtTemp = std::min(m_dtMax, std::max(m_dtMin, dtCalc));
		const double dt = t1 + dtTemp < _timeEnd ? dtTemp : _timeEnd - t1;
		const double t2 = t1 + dt;

		m_holdup->AddStream(t1, t2, m_inStream);
		switch (m_method)
		{
		case EMethod::NEWTON:	CalculateTransformationMatrixNewton(dt);	break;
		case EMethod::KR2:		CalculateTransformationMatrixRK2(dt);		break;
		}
		m_holdup->ApplyTM(t2, m_TM);
		m_holdup->SetMass(t2, m_holdupMass);
		m_outStream->CopyFromHoldup(t2, m_holdup, m_inStream->GetMassFlow(_timeEnd));
		t1 = t2;

		ShowInfo(std::to_string(t2) + "s...");
	}
}

std::vector<double> CCrusherPBMTM::CalculateSelectionFunction(const std::vector<double>& _x) const
{
	std::vector<double> res(m_classesNum, 0);
	for (size_t i = 0; i < m_classesNum; ++i)
		res[i] = Selection(_x[i]);

	for (auto& v : res)
		v *= m_sscale;

	return res;
}

std::vector<std::vector<double>> CCrusherPBMTM::CalculateBreakageFunction(const std::vector<double>& _x)
{
	std::vector<std::vector<double>> res(m_classesNum, std::vector<double>(m_classesNum, 0));
	ParallelFor(m_classesNum, [&](size_t i)
	{
		for (size_t j = i; j < m_classesNum; ++j)
			if (i == j)
				res[i][j] = AdaptiveSimpsons(m_grid[i], _x[i], _x[j], 1e-15, 10);
			else
				res[i][j] = AdaptiveSimpsons(m_grid[i], m_grid[i + 1], _x[j], 1e-15, 10);
	});
	return res;
}

std::vector<double> CCrusherPBMTM::CalculateNu(const std::vector<double>& _x) const
{
	std::vector<double> res(m_classesNum, 0);
	for (size_t i = 0; i < m_classesNum; ++i)
		res[i] = AdaptiveSimpsons(0, _x[i], _x[i], 1.e-15, 10);
	return res;
}

std::vector<double> CCrusherPBMTM::CalculateBirthWeights(const std::vector<double>& _x) const
{
	std::vector<double> res(m_classesNum, 0);
	for (size_t i = 0; i < m_classesNum; ++i)
	{
		double sum = 0.;
		for (size_t j = 0; j < i; ++j)
			sum += (_x[i] - _x[j]) * m_B[j][i];
		if (sum != 0)
			res[i] = _x[i] * (m_nu[i] - 1) / sum;
	}
	return res;
}

std::vector<double> CCrusherPBMTM::CalculateDeathWeights(const std::vector<double>& _x) const
{
	std::vector<double> res(m_classesNum, 0);
	for (size_t i = 0; i < m_classesNum; ++i)
	{
		double sum = 0.;
		for (size_t j = 0; j <= i; ++j)
			sum += _x[j] * m_B[j][i];
		res[i] = m_WB[i] / _x[i] * sum;
	}
	return res;
}

CMatrix2D CCrusherPBMTM::CalculateBaseTransformationMatrix() const
{
	CMatrix2D res(m_classesNum, m_classesNum);
	for (size_t i = 0; i < m_classesNum; ++i)
		res[i][i] = (m_WB[i] * m_B[i][i] - m_WD[i]) * m_S[i];
	for (size_t i = 1; i < m_classesNum; ++i)
		for (size_t j = 0; j < i; ++j)
			res[i][j] = m_WB[i] * m_B[j][i] * m_S[i] / (m_means[i] / m_means[j]) / (m_sizes[i] / m_sizes[j]); // scaling from numbers to mass fractions;
	return res;
}

void CCrusherPBMTM::CalculateTransformationMatrixNewton(double _dt)
{
	const CMatrix2D tm = m_PT * _dt + m_I;
	m_TM.SetMatrix(tm);
}

void CCrusherPBMTM::CalculateTransformationMatrixRK2(double _dt)
{
	const CMatrix2D M1 = m_PT * _dt + m_I;
	const CMatrix2D A2 = m_PT * _dt / 2;
	const CMatrix2D M2 = A2 + m_I;
	const CMatrix2D tm = M1 * M2 - A2;
	m_TM.SetMatrix(tm);
}

double CCrusherPBMTM::Selection(double _x) const
{
	switch (m_selectionFun)
	{
	case ESelection::CONSTANT:	  return m_s1;
	case ESelection::LINEAR:	  return _x;
	case ESelection::QUADRATIC:	  return _x * _x;
	case ESelection::POWER:		  return std::pow(_x, m_s1);
	case ESelection::EXPONENTIAL: return std::exp(m_s1 * _x);
	case ESelection::KING:		  return _x <= m_s1 ? 0 : _x >= m_s2 ? 1 : 1 - std::pow((m_s2 - _x) / (m_s2 - m_s1), m_s3);
	case ESelection::AUSTIN:	  return std::pow(_x / m_s1, m_s2);
	}
	return _x;
}

double CCrusherPBMTM::Breakage(double _x, double _y) const
{
	switch (m_breakageFun)
	{
	case EBreakage::BINARY:
		return 2. / _y;
	case EBreakage::DIEMER:
		return m_b1 * std::tgamma(m_b2 + (m_b2 + 1)*(m_b1 - 1) + 1) / (std::tgamma(m_b2 + 1)*std::tgamma(m_b2 + (m_b2 + 1)*(m_b1 - 2) + 1)) * (std::pow(_x, m_b2)*std::pow(_y - _x, m_b2 + (m_b2 + 1)*(m_b1 - 2)) / std::pow(_y, m_b1*m_b2 + m_b1 - 1));
	case EBreakage::VOGEL:
		if (_y == 0 || _x == 0) return 0;
		return 0.5 * m_b2 / _y * std::pow(_x / _y, m_b2 - 2) * (1 + std::tanh((_y - m_b1) / m_b1));
	case EBreakage::AUSTIN:
		if (_y == 0 || _x == 0) return 0;
		return m_b1 * m_b2 / _y * std::pow(_x / _y, m_b2 - 2) + (1 - m_b1) * m_b3 / _y * std::pow(_x / _y, m_b3 - 2);
	}
	return _x;
}

double CCrusherPBMTM::MaxTimeStep(double _dt, const std::vector<double>& _q0) const
{
	static CMatrix2D T1;
	T1.Resize(1, m_classesNum);
	T1.SetRow(0, _q0);
	const std::vector<double> newq0 = (T1 * (m_PT * _dt + m_I)).GetRow(0);

	std::vector<double> FC(m_classesNum);
	ParallelFor(m_classesNum, [&](size_t i)
	{
		double tempsum1 = 0;
		for (size_t k = i; k < m_classesNum; ++k)
			tempsum1 += _q0[k] * m_WB[k] * m_S[k] * m_B[i][k];

		double tempsum2 = 0;
		for (size_t k = i; k < m_classesNum; ++k)
			tempsum2 += newq0[k] * m_WB[k] * m_S[k] * m_B[i][k];

		const double Q = tempsum1 + tempsum2 - m_WD[i] * m_S[i] * (_q0[i] + newq0[i]);

		FC[i] = std::fabs(2 * _q0[i] / Q);
	});

	double res = std::numeric_limits<double>::max();
	for (size_t i = 0; i < m_classesNum; ++i)
		if (FC[i] > 0 && FC[i] < res)
			res = FC[i];

	return res;
}

double CCrusherPBMTM::AdaptiveSimpsons(double a, double b, double param, double epsilon, int recMaxDepth) const
{
	const double c = (a + b) / 2;
	const double h = b - a;
	const double fa = Breakage(a, param);
	const double fb = Breakage(b, param);
	const double fc = Breakage(c, param);
	const double S = h / 6 * (fa + 4 * fc + fb);
	return AdaptiveSimpsonsRecursive(a, b, param, epsilon, S, fa, fb, fc, recMaxDepth);
}

double CCrusherPBMTM::AdaptiveSimpsonsRecursive(double a, double b, double param, double epsilon, double S, double fa, double fb, double fc, int recDepth) const
{
	const double c = (a + b) / 2;
	const double h = (b - a) / 2;
	const double d = (a + c) / 2;
	const double e = (c + b) / 2;
	const double fd = Breakage(d, param);
	const double fe = Breakage(e, param);
	const double left = h / 6 * (fa + 4 * fd + fc);
	const double right = h / 6 * (fc + 4 * fe + fb);
	const double delta = left + right - S;
	if (recDepth <= 0 || std::fabs(delta) <= 15 * epsilon)
		return left + right + delta / 15;
	return AdaptiveSimpsonsRecursive(a, c, param, epsilon / 2, left, fa, fc, fd, recDepth - 1) + AdaptiveSimpsonsRecursive(c, b, param, epsilon / 2, right, fc, fb, fe, recDepth - 1);
}

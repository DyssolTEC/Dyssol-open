/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "Holdup.h"
#include "MaterialStream.h"
#include "DyssolUtilities.h"
#include "ContainerFunctions.h"

CHoldup::CHoldup(const std::string& _sHoldupKey/*="" */) : CStream(_sHoldupKey)
{
	initHoldup();
}

CHoldup::CHoldup(const CStream& _stream) : CStream(_stream)
{
	initHoldup();
}

void CHoldup::initHoldup()
{
	m_StreamMTP.SetDimensionLabel(0, "Mass [kg]");
	m_StreamMTP.SetDimensionLabel(1, "Temperature [K]");
	m_StreamMTP.SetDimensionLabel(2, "Pressure [Pa]");
}

void CHoldup::CopyFromHoldup(const CHoldup* _pSrcHoldup, double _dTime, bool _bDeleteDataAfter /*= true */)
{
	CopyFromStream_Base(*_pSrcHoldup, _dTime, _bDeleteDataAfter);
}

void CHoldup::CopyFromHoldup(const CHoldup* _pSrcHoldup, double _dStart, double _dEnd, bool _bDeleteDataAfter /*= true */)
{
	CopyFromStream_Base(*_pSrcHoldup, _dStart, _dEnd, _bDeleteDataAfter);
}

void CHoldup::CopyFromHoldup(double _dTimeDst, const CHoldup* _pSrcHoldup, double _dTimeSrc, bool _bDeleteDataAfter /*= true */)
{
	CopyFromStream_Base(_dTimeDst, *_pSrcHoldup, _dTimeSrc, _bDeleteDataAfter);
}

void CHoldup::AddStream(const CMaterialStream* _pStream, double _dStart, double _dEnd)
{
	if (_dStart > _dEnd) // wrong time interval
		return;

	if (!CompareStreamStructure(*_pStream)) // wrong structure
		return;

	if (m_vTimePoints.empty() || m_vTimePoints.back() < _dStart)
	{
		CopyFromStream(_pStream, _dStart, _dEnd);
		return;
	}

	// get all time points
	std::vector<double> vTimePoints = VectorsUnionSorted(_pStream->GetTimePointsForInterval(_dStart, _dEnd, true), GetTimePointsForInterval(_dStart, _dEnd, true));

	if (vTimePoints.size() == 0)
		return;

	for (size_t iTime = 1; iTime < vTimePoints.size(); ++iTime)
	{
		const double dMassSrc = _pStream->GetMassFlow((vTimePoints[iTime - 1] + vTimePoints[iTime]) / 2) * (vTimePoints[iTime] - vTimePoints[iTime - 1]);

		double dFactorTP = (vTimePoints[iTime] - vTimePoints[iTime - 1]) / std::sqrt(2.);
		if (_pStream->GetMassFlow(vTimePoints[iTime - 1]) < _pStream->GetMassFlow(vTimePoints[iTime]))
			dFactorTP = vTimePoints[iTime - 1] + dFactorTP;
		else
			dFactorTP = vTimePoints[iTime] - dFactorTP;

		std::vector<CDenseMDMatrix> vBufSrcPhasesDistrs(m_vpPhases.size());
		for (size_t iPhase = 0; iPhase < m_vpPhases.size(); ++iPhase)
			_pStream->GetPhases()->at(iPhase)->distribution.GetDistribution(vTimePoints[iTime], vBufSrcPhasesDistrs[iPhase]);

		CHoldup bufHoldup(*this);
		bufHoldup.CopyFromHoldup(this, _dStart, _dEnd);

		for (size_t j = iTime; j < vTimePoints.size(); ++j)
		{
			std::vector<double> vTempMTP(3); // MTP
			std::vector<double> vTempPhaseFrac(m_vpPhases.size()); // phase fractions
			std::vector<CDenseMDMatrix> vTempDistr(m_vpPhases.size()); // MD distributions

			AddTimePoint(vTimePoints[j]);

			//// MTP

			// get masses
			const double dMassDst = bufHoldup.m_StreamMTP.GetValue(vTimePoints[j], MTP_MASS);
			const double dMassTot = dMassDst + dMassSrc;
			if (dMassTot == 0) //  nothing to mix
				break;

			// get mass
			vTempMTP[MTP_MASS] = dMassTot;
			// get pressure
			vTempMTP[MTP_PRESSURE] = CalcMixPressure(*_pStream, dFactorTP, bufHoldup, vTimePoints[j]);
			// get temperature
			vTempMTP[MTP_TEMPERATURE] = CalcMixTemperature(*_pStream, dFactorTP, dMassSrc, bufHoldup, vTimePoints[j], dMassDst);
			// set MTP
			m_StreamMTP.SetValue(vTimePoints[j], vTempMTP);

			// get phase fractions
			std::vector<double> vTempPhaseMassSrc, vTempPhaseMassDst;
			std::tie(vTempPhaseMassSrc, vTempPhaseMassDst, vTempPhaseFrac) = CalcMixPhaseFractions(*_pStream, dFactorTP, dMassSrc, bufHoldup, vTimePoints[j], dMassDst);
			// set phase fractions
			m_PhaseFractions.SetValue(vTimePoints[j], vTempPhaseFrac);

			// get MD distributions
			vTempDistr = CalcMixMDDistributions(*_pStream, vTimePoints[iTime], dMassSrc, vTempPhaseMassSrc, bufHoldup, vTimePoints[j], dMassDst, vTempPhaseMassDst);
			// set MD distributions
			for (size_t iPhase = 0; iPhase < m_vpPhases.size(); ++iPhase)
			{
				m_vpPhases[iPhase]->distribution.AddTimePoint(vTimePoints[j]);
				m_vpPhases[iPhase]->distribution.SetDistribution(vTimePoints[j], vTempDistr[iPhase]);
			}

			// normalize MD distributions
			for (size_t iPhase = 0; iPhase < m_vpPhases.size(); ++iPhase)
				m_vpPhases[iPhase]->distribution.NormalizeMatrix(vTimePoints[j]);
		}
	}
}

void CHoldup::AddStream2(const CMaterialStream* _pStream, double _tStart, double _tEnd)
{
	if (_tStart > _tEnd) return; // wrong time interval
	if (!CompareStreamStructure(*_pStream)) return; // wrong structure

	if (m_vTimePoints.empty() || m_vTimePoints.back() < _tStart)
	{
		CopyFromStream(_pStream, _tStart, _tEnd);
		return;
	}

	const double dMassSrc = _pStream->GetMassFlow((_tStart + _tEnd) / 2) * (_tEnd - _tStart);

	double dFactorTP = (_tEnd - _tStart) / std::sqrt(2.);
	if (_pStream->GetMassFlow(_tStart) < _pStream->GetMassFlow(_tEnd))
		dFactorTP = _tStart + dFactorTP;
	else
		dFactorTP = _tEnd - dFactorTP;

	std::vector<CDenseMDMatrix> vBufSrcPhasesDistrs(m_vpPhases.size());
	for (size_t iPhase = 0; iPhase < m_vpPhases.size(); ++iPhase)
		(*_pStream->GetPhases())[iPhase]->distribution.GetDistribution(_tEnd, vBufSrcPhasesDistrs[iPhase]);

	CHoldup bufHoldup(*this);
	bufHoldup.CopyFromHoldup(this, _tStart, _tEnd);

	std::vector<double> vTempMTP(3); // mass/temperature/pressure
	std::vector<double> vTempPhaseFrac(m_vpPhases.size()); // phase fractions

	AddTimePoint(_tEnd);

	//// mass/temperature/pressure

	// get masses
	const double dMassDst = bufHoldup.m_StreamMTP.GetValue(_tEnd, MTP_MASS);
	const double dMassTot = dMassDst + dMassSrc;
	if (dMassTot == 0) //  nothing to mix
		return;

	// get mass
	vTempMTP[MTP_MASS] = dMassTot;
	// get pressure
	vTempMTP[MTP_PRESSURE] = CalcMixPressure(*_pStream, dFactorTP, bufHoldup, _tEnd);
	// get temperature
	vTempMTP[MTP_TEMPERATURE] = CalcMixTemperature(*_pStream, dFactorTP, dMassSrc, bufHoldup, _tEnd, dMassDst);
	// set mass/temperature/pressure
	m_StreamMTP.SetValue(_tEnd, vTempMTP);

	// get phase fractions
	std::vector<double> vTempPhaseMassSrc, vTempPhaseMassDst;
	std::tie(vTempPhaseMassSrc, vTempPhaseMassDst, vTempPhaseFrac) = CalcMixPhaseFractions(*_pStream, dFactorTP, dMassSrc, bufHoldup, _tEnd, dMassDst);
	// set phase fractions
	m_PhaseFractions.SetValue(_tEnd, vTempPhaseFrac);

	// get MD distributions
	std::vector<CDenseMDMatrix> vTempDistr = CalcMixMDDistributions(*_pStream, _tEnd, dMassSrc, vTempPhaseMassSrc, bufHoldup, _tEnd, dMassDst, vTempPhaseMassDst);
	// set MD distributions
	for (size_t iPhase = 0; iPhase < m_vpPhases.size(); ++iPhase)
		m_vpPhases[iPhase]->distribution.SetDistribution(_tEnd, vTempDistr[iPhase]);
}

void CHoldup::AddHoldup(const CHoldup* _Holdup, double _dTime)
{
	AddStream_Base(*_Holdup, _dTime);
}

void CHoldup::AddHoldup(const CHoldup* _Holdup, double _dStart, double _dEnd, unsigned _nTPTypes /*= BOTH_TP */)
{
	AddStream_Base(*_Holdup, _dStart, _dEnd, _nTPTypes);
}

double CHoldup::GetMass(double _dTime, unsigned _nBasis /*= BASIS_MASS */) const
{
	return GetMass_Base(_dTime, _nBasis);
}

void CHoldup::SetMass(double _dTime, double _dValue, unsigned _nBasis /*= BASIS_MASS */)
{
	SetMass_Base(_dTime, _dValue, _nBasis);
}

double CHoldup::GetCompoundMass(double _dTime, const std::string& _sCompoundKey, unsigned _nPhase, unsigned _nBasis /*= BASIS_MASS */) const
{
	return GetCompoundMass_Base(_dTime, GetCompoundIndex(_sCompoundKey), _nPhase, _nBasis);
}

double CHoldup::GetCompoundMass(double _dTime, unsigned _nCompoundIndex, unsigned _nPhase, unsigned _nBasis /*= BASIS_MASS */) const
{
	return GetCompoundMass_Base(_dTime, _nCompoundIndex, _nPhase, _nBasis);
}

double CHoldup::GetPhaseMass(double _dTime, unsigned _nPhase, unsigned _nBasis /*= BASIS_MASS */) const
{
	return GetPhaseMass_Base(_dTime, _nPhase, _nBasis);
}

void CHoldup::SetPhaseMass(double _dTime, unsigned _nPhase, double _dMassFlow, unsigned _nBasis /*= BASIS_MASS */)
{
	SetPhaseMass_Base(_dTime, _nPhase, _dMassFlow, _nBasis);
}

void CHoldup::CopyFromStream(const CMaterialStream* _pStream, double _dStart, double _dEnd)
{
	if (_dStart >= _dEnd) // wrong time interval
		return;

	//TODO: implement temperature and pressure

	if (!CompareStreamStructure(*_pStream))
		return;

	RemoveTimePointsAfter(_dStart);

	// get all time points
	std::vector<double> vTimePoints = _pStream->GetTimePointsForInterval(_dStart, _dEnd, true);

	if (vTimePoints.size() == 0)
		return;

	if (m_vTimePoints.empty())
	{
		CopyFromStream_Base(*_pStream, vTimePoints[0]);
		SetMass(vTimePoints[0], 0);
	}

	AddTimePoint(vTimePoints[0]);
	for (size_t i = 1; i < vTimePoints.size(); ++i)
	{
		std::vector<double> vTempMTP(3); // MTP
		std::vector<double> vTempPhaseFrac(m_vpPhases.size()); // phase fractions
		std::vector<CDenseMDMatrix> vTempDistr(m_vpPhases.size()); // MD distributions

		AddTimePoint(vTimePoints[i]);

		//// MTP

		// get masses
		const double dMassDst = m_StreamMTP.GetValue(vTimePoints[i], MTP_MASS);
		const double dMassSrc = _pStream->GetMassFlow((vTimePoints[i - 1] + vTimePoints[i]) / 2) * (vTimePoints[i] - vTimePoints[i - 1]);
		const double dMassTot = dMassDst + dMassSrc;
		if (dMassTot == 0) //  nothing to mix
			continue;

		double dFactorTP = (vTimePoints[i] - vTimePoints[i - 1]) / std::sqrt(2.);
		if (_pStream->GetMassFlow(vTimePoints[i - 1]) < _pStream->GetMassFlow(vTimePoints[i]))
			dFactorTP = vTimePoints[i - 1] + dFactorTP;
		else
			dFactorTP = vTimePoints[i] - dFactorTP;

		// get mass
		vTempMTP[MTP_MASS] = dMassTot;
		// get pressure
		vTempMTP[MTP_PRESSURE] = CalcMixPressure(*_pStream, dFactorTP, *this, vTimePoints[i]);
		// get temperature
		vTempMTP[MTP_TEMPERATURE] = CalcMixTemperature(*_pStream, dFactorTP, dMassSrc, *this, vTimePoints[i], dMassDst);
		// set MTP
		m_StreamMTP.SetValue(vTimePoints[i], vTempMTP);

		// get phase fractions
		std::vector<double> vTempPhaseMassSrc, vTempPhaseMassDst;
		std::tie(vTempPhaseMassSrc, vTempPhaseMassDst, vTempPhaseFrac) = CalcMixPhaseFractions(*_pStream, dFactorTP, dMassSrc, *this, vTimePoints[i], dMassDst);
		// set phase fractions
		m_PhaseFractions.SetValue(vTimePoints[i], vTempPhaseFrac);

		// get MD distributions
		vTempDistr = CalcMixMDDistributions(*_pStream, vTimePoints[i], dMassSrc, vTempPhaseMassSrc, *this, vTimePoints[i], dMassDst, vTempPhaseMassDst);
		// set MD distributions
		for (size_t iPhase = 0; iPhase < m_vpPhases.size(); ++iPhase)
		{
			m_vpPhases[iPhase]->distribution.SetDistribution(vTimePoints[i], vTempDistr[iPhase]);
			m_vpPhases[iPhase]->distribution.NormalizeMatrix(_dStart, _dEnd);
		}
	}
}

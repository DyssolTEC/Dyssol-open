/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "MaterialStream.h"
#include "Holdup.h"

CMaterialStream::CMaterialStream(const std::string& _sStreamKey/*="" */) : CStream(_sStreamKey)
{
	initMaterialStream();
}

CMaterialStream::CMaterialStream(const CStream& _stream) : CStream(_stream)
{
	initMaterialStream();
}

void CMaterialStream::initMaterialStream()
{
	m_StreamMTP.SetDimensionLabel(0, "Mass flow [kg/s]");
	m_StreamMTP.SetDimensionLabel(1, "Temperature [K]");
	m_StreamMTP.SetDimensionLabel(2, "Pressure [Pa]");
}

void CMaterialStream::CopyFromStream(const CMaterialStream* _pSrcStream, double _dTime, bool _bDeleteDataAfter /*= true */)
{
	CopyFromStream_Base(*_pSrcStream, _dTime, _bDeleteDataAfter);
}

void CMaterialStream::CopyFromStream(const CMaterialStream* _pSrcStream, double _dStart, double _dEnd, bool _bDeleteDataAfter /*= true */)
{
	CopyFromStream_Base(*_pSrcStream, _dStart, _dEnd, _bDeleteDataAfter);
}

void CMaterialStream::CopyFromStream(double _dTimeDst, const CMaterialStream* _pSrcStream, double _dTimeSrc, bool _bDeleteDataAfter /*= true */)
{
	CopyFromStream_Base(_dTimeDst, *_pSrcStream, _dTimeSrc, _bDeleteDataAfter);
}

void CMaterialStream::CopyFromHoldup(const CHoldup* _pSrcHoldup, double _dTime, double _dMassFlow, bool _bDeleteDataAfter /*= true*/)
{
	CopyFromStream_Base(*_pSrcHoldup, _dTime, _bDeleteDataAfter);
	SetMassFlow(_dTime, _dMassFlow);
}

void CMaterialStream::CopyFromHoldup(double _dTimeDst, const CHoldup* _pSrcHoldup, double _dTimeSrc, double _dMassFlow, bool _bDeleteDataAfter /*= true*/)
{
	CopyFromStream_Base(_dTimeDst, *_pSrcHoldup, _dTimeSrc, _bDeleteDataAfter);
	SetMassFlow(_dTimeDst, _dMassFlow);
}

void CMaterialStream::AddStream(const CMaterialStream* _pStream, double _dTime)
{
	AddStream_Base(*_pStream, _dTime);
}

void CMaterialStream::AddStream(const CMaterialStream* _pStream, double _dStart, double _dEnd, unsigned _nTPTypes /*= BOTH_TP */)
{
	AddStream_Base(*_pStream, _dStart, _dEnd, _nTPTypes);
}

double CMaterialStream::GetMassFlow(double _dTime, unsigned _nBasis /*= BASIS_MASS */) const
{
	return GetMass_Base(_dTime, _nBasis);
}

void CMaterialStream::SetMassFlow(double _dTime, double _dValue, unsigned _nBasis /*= BASIS_MASS */)
{
	SetMass_Base(_dTime, _dValue, _nBasis);
}

double CMaterialStream::GetCompoundMassFlow(double _dTime, const std::string& _sCompoundKey, unsigned _nPhase, unsigned _nBasis /*= BASIS_MASS */) const
{
	return GetCompoundMass_Base(_dTime, GetCompoundIndex(_sCompoundKey), _nPhase, _nBasis);
}

double CMaterialStream::GetCompoundMassFlow(double _dTime, unsigned _nCompoundIndex, unsigned _nPhase, unsigned _nBasis /*= BASIS_MASS */) const
{
	return GetCompoundMass_Base(_dTime, _nCompoundIndex, _nPhase, _nBasis);
}

double CMaterialStream::GetPhaseMassFlow(double _dTime, unsigned _nPhase, unsigned _nBasis /*= BASIS_MASS */) const
{
	return GetPhaseMass_Base(_dTime, _nPhase, _nBasis);
}

void CMaterialStream::SetPhaseMassFlow(double _dTime, unsigned _nPhase, double _dMassFlow, unsigned _nBasis /*= BASIS_MASS */)
{
	SetPhaseMass_Base(_dTime, _nPhase, _dMassFlow, _nBasis);
}

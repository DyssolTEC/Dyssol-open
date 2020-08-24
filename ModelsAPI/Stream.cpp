/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "Stream.h"
#include "DistributionsFunctions.h"
#include "DyssolStringConstants.h"
#include "DyssolUtilities.h"
#include "ContainerFunctions.h"
#include <cfloat>

const unsigned CStream::m_cnSaveVersion	= 1;

CStream::CStream(const std::string& _sKey /*= "" */)
{
	initStream( _sKey );
}

CStream::CStream(const CStream& _stream)
{
	initStream();
	SetupStream(&_stream);
}

CStream::~CStream()
{
	for( unsigned i=0; i<m_vpPhases.size(); ++i )
		delete m_vpPhases[i];
	m_vpPhases.clear();
}

void CStream::initStream(const std::string& _sKey /*= ""*/)
{
	m_sStreamKey = _sKey.empty() ? StringFunctions::GenerateRandomString() : _sKey;
	m_sStreamName = StrConst::Stream_UnspecStreamName;
	m_StreamMTP.SetDimensionsNumber(3);

	m_DistrArrays.push_back(&m_PhaseFractions);
	m_DistrArrays.push_back(&m_StreamMTP);

	m_pDistributionsGrid = nullptr;
	m_pMaterialsDB = nullptr;

	m_sCachePath = L"";
	m_bCacheEnabled = false;
	m_nCacheWindow = DEFAULT_CACHE_WINDOW;
}

void CStream::SetupStream(const CStream* _pStream)
{
	if (!_pStream) return;

	SetDistributionsGrid(_pStream->m_pDistributionsGrid);
	SetMaterialsDatabase(_pStream->m_pMaterialsDB);
	SetCompounds(_pStream->m_vCompoundsKeys);

	std::vector<std::string> vPhNames(_pStream->m_vpPhases.size());
	std::vector<unsigned> vPhSOAs(_pStream->m_vpPhases.size());
	for (size_t i = 0; i < _pStream->m_vpPhases.size(); ++i)
	{
		vPhNames[i] = _pStream->m_vpPhases[i]->sName;
		vPhSOAs[i] = _pStream->m_vpPhases[i]->nAggregationState;
	}
	SetPhases(vPhNames, vPhSOAs);

	m_StreamMTP.SetDimensionsNumber(_pStream->m_StreamMTP.GetDimensionsNumber());
	for (unsigned i = 0; i < _pStream->m_StreamMTP.GetDimensionsNumber(); ++i)
		m_StreamMTP.SetDimensionLabel(0, _pStream->m_StreamMTP.GetLabel(i));

	if (!m_vpPhases.empty())
		SetMinimalFraction(_pStream->m_vpPhases.front()->distribution.GetMinimalFraction());

	SetCachePath(_pStream->m_sCachePath);
	SetCacheParams(_pStream->m_bCacheEnabled, _pStream->m_nCacheWindow);
}

void CStream::Clear()
{
	RemoveTimePointsAfter( 0 );
	for( unsigned i=0; i<m_vpPhases.size(); ++i )
		delete m_vpPhases[i];
	m_vpPhases.clear();
	m_vCompoundsKeys.clear();
	m_vTLookupTables.clear();
	m_vPLookupTables.clear();
	m_TLookup1.Clear();
	m_TLookup2.Clear();
}

std::string CStream::GetStreamKey() const
{
	return m_sStreamKey;
}

void CStream::SetStreamKey(const std::string& _key)
{
	m_sStreamKey = _key;
}

std::string CStream::GetStreamName() const
{
	return m_sStreamName;
}

void CStream::SetStreamName(std::string _sNewName)
{
	m_sStreamName = _sNewName;
}

CDenseDistr2D* CStream::GetDistrStreamMTP()
{
	return &m_StreamMTP;
}

const CDenseDistr2D* CStream::GetDistrStreamMTP() const
{
	return &m_StreamMTP;
}

CDenseDistr2D* CStream::GetDistrPhaseFractions()
{
	return &m_PhaseFractions;
}

const CDenseDistr2D* CStream::GetDistrPhaseFractions() const
{
	return &m_PhaseFractions;
}

void CStream::AddTimePoint(double _dTime, double _dSourceTime /*= -1 */)
{
	if( _dTime < 0 ) return;

	size_t index = GetTimeIndex( _dTime, false ); // get new index to insert
	if( index < m_vTimePoints.size() )
		if( m_vTimePoints[index] == _dTime ) // time point already exists
			return;

	m_vTimePoints.insert( m_vTimePoints.begin() + index, _dTime );

	for( unsigned i=0; i < m_DistrArrays.size(); i++ )
		m_DistrArrays[i]->AddTimePoint( _dTime, _dSourceTime );
	for( unsigned i=0; i<m_vpPhases.size(); ++i )
		m_vpPhases[i]->distribution.AddTimePoint( _dTime, _dSourceTime );
}

void CStream::RemoveTimePoint(double _dTime)
{
	// check that this time point exists
	size_t nIndex = GetTimeIndex( _dTime );
	if( nIndex == -1 ) return;

	m_vTimePoints.erase( m_vTimePoints.begin() + nIndex );
	for( unsigned i=0; i < m_DistrArrays.size(); i++ )
		m_DistrArrays[i]->RemoveTimePoint( _dTime );
	for( unsigned i=0; i<m_vpPhases.size(); ++i )
		m_vpPhases[i]->distribution.RemoveTimePoint( _dTime );
}

void CStream::RemoveTimePoints( double _dStart, double _dEnd )
{
	for( unsigned i=0; i<m_DistrArrays.size(); i++ )
		m_DistrArrays[i]->RemoveTimePoints( _dStart, _dEnd );
	for( unsigned i=0; i<m_vpPhases.size(); i++ )
		m_vpPhases[i]->distribution.RemoveTimePoints( _dStart, _dEnd );

	if( m_vTimePoints.empty() ) // nothing to remove
		return;

	size_t iLast = GetTimeIndex( _dEnd, false );
	size_t iFirst = GetTimeIndex( _dStart, false );
	if( ( iLast >= m_vTimePoints.size() ) || ( m_vTimePoints[iLast] != _dEnd ) )
		iLast--;
	m_vTimePoints.erase( m_vTimePoints.begin() + iFirst, m_vTimePoints.begin() + iLast + 1 );
}

void CStream::RemoveTimePointsAfter(double _dStart, bool _bIncludeStart /*= false */)
{
	for( unsigned i=0; i<m_DistrArrays.size(); i++ )
		m_DistrArrays[i]->RemoveAllDataAfter( _dStart, _bIncludeStart );
	for( unsigned i=0; i<m_vpPhases.size(); i++ )
		m_vpPhases[i]->distribution.RemoveTimePointsAfter( _dStart, _bIncludeStart );
	if( _bIncludeStart )
		while( !m_vTimePoints.empty() )
			if( m_vTimePoints.back() >= _dStart )
				m_vTimePoints.pop_back();
			else
				break;
	else
		while ( !m_vTimePoints.empty() )
			if ( m_vTimePoints.back() > _dStart )
				m_vTimePoints.pop_back();
			else
				break;
}

std::vector<double> CStream::GetAllTimePoints() const
{
	return m_vTimePoints;
}

std::vector<double> CStream::GetTimePointsForInterval(double _dStart, double _dEnd, bool _bForceInclBoudaries /*= false */) const
{
	std::vector<double> vRes;
	for( unsigned i=0; i<m_vTimePoints.size(); i++ )
		if( ( m_vTimePoints[i] >= _dStart ) && ( m_vTimePoints[i] <= _dEnd ) )
			vRes.push_back( m_vTimePoints[i] );

	if( _bForceInclBoudaries )
	{
		if( vRes.empty() )
		{
			vRes.push_back( _dStart );
			vRes.push_back( _dEnd );
		}
		else
		{
			if(std::fabs(vRes.front() - _dStart) > 16 * DBL_EPSILON)
				vRes.insert( vRes.begin(), _dStart );
			if(std::fabs(vRes.back() - _dEnd) > 16 * DBL_EPSILON)
				vRes.push_back( _dEnd );
			//if( vRes.front() != _dStart )
			//	vRes.insert( vRes.begin(), _dStart );
			//if( vRes.back() != _dEnd )
			//	vRes.push_back( _dEnd );
		}
	}

	return vRes;
}

double CStream::GetLastTimePoint() const
{
	if( m_vTimePoints.empty() )
		return -1;
	else
		return m_vTimePoints.back();
}

double CStream::GetPreviousTimePoint(double _dTime) const
{
	if( m_vTimePoints.empty() )
		return -1;
	else
		for( long long int i=(long long)m_vTimePoints.size()-1; i>=0; --i )
			if( m_vTimePoints[(size_t)i] < _dTime )
				return m_vTimePoints[(size_t)i];
	return -1;
}

//bool CStream::ChangeTimePoint(unsigned _nTimeIndex, double _dNewTime)
//{
//	if( _nTimeIndex >= m_vTimePoints.size() )
//		return false;
//
//	if( m_vTimePoints.size() > 1 )
//	{
//		if( ( _nTimeIndex != 0 ) && ( m_vTimePoints[_nTimeIndex-1] >= _dNewTime ) ) // new time value is less than the previous time point
//			return false;
//		if( ( _nTimeIndex != m_vTimePoints.size()-1 ) && ( m_vTimePoints[_nTimeIndex+1] <= _dNewTime ) ) // new time value is bigger than the next time point
//			return false;
//	}
//
//	m_vTimePoints[_nTimeIndex] = _dNewTime;
//	for( unsigned i=0; i<m_DistrArrays.size(); i++ )
//		m_DistrArrays[i]->ChangeTimePoint( _nTimeIndex, _dNewTime );
//	for( unsigned i=0; i<m_vpPhases.size(); ++i )
//		m_vpPhases[i]->distribution.ChangeTimePoint( _nTimeIndex, _dNewTime );
//	return true;
//}

void CStream::ChangeTimePoint(unsigned _nTimeIndex, double _dNewTime)
{
	if (_dNewTime < 0)
		return;

	if( _nTimeIndex >= m_vTimePoints.size() )
		return;

	if( m_vTimePoints.size() > 1 )
	{
		bool bLess = ( _nTimeIndex != 0 ) && ( m_vTimePoints[_nTimeIndex-1] >= _dNewTime ); // new time value is less than the previous time point
		bool bBigger = ( _nTimeIndex != m_vTimePoints.size()-1 ) && ( m_vTimePoints[_nTimeIndex+1] <= _dNewTime ); // new time value is bigger than the next time point
		if( bLess || bBigger )
		{
			for( unsigned i=0; i<m_vTimePoints.size(); ++i ) // check if such time point already exists
				if( ( m_vTimePoints[i] == _dNewTime ) && ( i != _nTimeIndex ) )
					return;

			double dOldTime = m_vTimePoints[_nTimeIndex];
			//CopyFromStream_Base( _dNewTime, *this, m_vTimePoints[_nTimeIndex], false );
			AddTimePoint( _dNewTime, m_vTimePoints[_nTimeIndex] );
			RemoveTimePoint( dOldTime );
			return;
		}
	}

	m_vTimePoints[_nTimeIndex] = _dNewTime;
	for( unsigned i=0; i<m_DistrArrays.size(); i++ )
		m_DistrArrays[i]->ChangeTimePoint( _nTimeIndex, _dNewTime );
	for( unsigned i=0; i<m_vpPhases.size(); ++i )
		m_vpPhases[i]->distribution.ChangeTimePoint( _nTimeIndex, _dNewTime );
}

double CStream::GetMass_Base(double _dTime, unsigned _nBasis /*= BASIS_MASS */) const
{
	double dValue = 0;

	if( _nBasis == BASIS_MASS )
	{
		dValue = m_StreamMTP.GetValue( _dTime, MTP_MASS );
		if( dValue < 0 )  dValue = 0;
	}
	if( _nBasis == BASIS_MOLL )
	{
		for( unsigned i=0; i<m_vpPhases.size(); ++i )
			dValue += GetSinglePhaseProp( _dTime, FLOW, m_vpPhases[i]->nAggregationState, _nBasis );
	}
	return dValue;
}

void CStream::SetMass_Base(double _dTime, double _dValue, unsigned _nBasis /*= BASIS_MASS */)
{
	size_t nIndex = GetTimeIndex( _dTime );
	if( nIndex == -1 )
		return;

	if( _dValue < 0 ) _dValue = 0;

	//if( _nBasis == BASIS_MOLL )
	//{
	//	double dMolWeight = 0;
	//	for( unsigned i=0; i<m_vPhases.size(); ++i )
	//		dMolWeight += GetSinglePhaseProp( _dTime, MOLAR_MASS, i ) * m_PhaseFractions.GetValue( _dTime, i );
	//	dMolWeight /= m_StreamMTP.GetValue( _dTime, MTP_MASS );

	//	_dValue = _dValue/dMolWeight;
	//}
	if( _nBasis == BASIS_MOLL )
	{
		double dWholeMolMass = 0;
		for( unsigned i=0; i<m_vpPhases.size(); ++i )
			dWholeMolMass += GetSinglePhaseProp( _dTime, MOLAR_MASS, m_vpPhases[i]->nAggregationState ) * m_PhaseFractions.GetValue( _dTime, i );
		_dValue *= dWholeMolMass;
	}

	m_StreamMTP.SetValue( _dTime, MTP_MASS, _dValue );
}

double CStream::GetTemperature(double _dTime) const
{
	double dValue = m_StreamMTP.GetValue( _dTime, MTP_TEMPERATURE );
	return dValue < 0 ? 0 : dValue;
}

void CStream::SetTemperature(double _dTime, double _dValue)
{
	if( GetTimeIndex( _dTime ) == -1 ) // there is no such time point
		return;

	if( _dValue < 0 ) _dValue = 0;
	m_StreamMTP.SetValue( _dTime, MTP_TEMPERATURE, _dValue );
}

double CStream::GetPressure(double _dTime) const
{
	double dValue = m_StreamMTP.GetValue( _dTime, MTP_PRESSURE );
	return dValue < 0 ? 0 : dValue;
}

void CStream::SetPressure(double _dTime, double _dValue)
{
	if( GetTimeIndex( _dTime ) == -1 ) // there is no such time point
		return;

	if( _dValue < 0 ) _dValue = 0;
	m_StreamMTP.SetValue( _dTime, MTP_PRESSURE, _dValue );
}

/// For total mixture
double CStream::GetOverallProperty(double _dTime, unsigned _nProperty, unsigned _nBasis /*= BASIS_MASS */) const
{
	double dRes = 0;
	switch( _nProperty )
	{
	case FLOW:
	case TOTAL_FLOW:
		dRes = GetMass_Base( _dTime, _nBasis );
		break;
	case TEMPERATURE:
		dRes = GetTemperature( _dTime );
		break;
	case PRESSURE:
		dRes = m_StreamMTP.GetValue( _dTime, MTP_PRESSURE );
		break;
	case MOLAR_MASS:
		for( unsigned i=0; i<m_vpPhases.size(); ++i )
			dRes += GetSinglePhaseProp( _dTime, _nProperty, m_vpPhases[i]->nAggregationState ) * m_PhaseFractions.GetValue( _dTime, i );
		break;
	//case DENSITY:
	//	for( unsigned i=0; i<m_vPhases.size(); ++i )
	//		dRes += GetTPDependentProperty( _dTime, i, _nProperty ) * m_PhaseFractions.GetValue( _dTime, i );
	//	break;
	case ENTHALPY:
		for( unsigned i=0; i<m_vpPhases.size(); ++i )
			dRes += GetSinglePhaseProp( _dTime, _nProperty, m_vpPhases[i]->nAggregationState, _nBasis ) * GetSinglePhaseProp( _dTime, FRACTION, m_vpPhases[i]->nAggregationState, _nBasis );
		break;
	}
	return dRes;
}

void CStream::SetOverallProperty(double _dTime, unsigned _nProperty, double _dValue, unsigned _nBasis /*= BASIS_MASS */)
{
	switch( _nProperty )
	{
	case FLOW:
	case TOTAL_FLOW:
		SetMass_Base( _dTime, _dValue, _nBasis );
		break;
	case TEMPERATURE:
		SetTemperature( _dTime, _dValue );
		break;
	case PRESSURE:
		if( GetTimeIndex( _dTime ) == -1 ) // there is no such time point
			return;
		if( _dValue < 0 ) _dValue = 0;
		m_StreamMTP.SetValue( _dTime, MTP_PRESSURE, _dValue );
		break;
	}
}

void CStream::AddCompound(std::string _sCompoundKey)
{
	// check if this compound already exists
	if( GetCompoundIndex( _sCompoundKey ) != -1 )
		return;

	// otherwise add new compound
	m_vCompoundsKeys.push_back( _sCompoundKey );
	for( unsigned i=0; i<m_vpPhases.size(); ++i )
		m_vpPhases[i]->distribution.AddClass( DISTR_COMPOUNDS );
	m_vTLookupTables.clear();
	m_vPLookupTables.clear();
	m_TLookup1.Clear();
	m_TLookup2.Clear();
}

void CStream::RemoveCompound(std::string _sCompoundKey)
{
	int nCompIndex = GetCompoundIndex( _sCompoundKey );
	if ( nCompIndex == -1 ) // no such compound
		return;

	m_vCompoundsKeys.erase( m_vCompoundsKeys.begin() + nCompIndex );
	for( unsigned i=0; i<m_vpPhases.size(); ++i )
		m_vpPhases[i]->distribution.RemoveClass( DISTR_COMPOUNDS, nCompIndex );
	m_vTLookupTables.clear();
	m_vPLookupTables.clear();
	m_TLookup1.Clear();
	m_TLookup2.Clear();
}

void CStream::SetCompounds(const std::vector<std::string>& _vCompoundsKeys)
{
	for( unsigned i=0; i<m_vCompoundsKeys.size(); ++i )
		RemoveCompound( m_vCompoundsKeys[i] );
	for( unsigned i=0; i<_vCompoundsKeys.size(); ++i )
		AddCompound( _vCompoundsKeys[i] );
}

size_t CStream::GetCompoundsNumber() const
{
	return m_vCompoundsKeys.size();
}

std::vector<std::string> CStream::GetCompoundsList() const
{
	return m_vCompoundsKeys;
}

std::vector<std::string> CStream::GetCompoundsNames() const
{
	if (!m_pMaterialsDB) return {};
	std::vector<std::string> vNames;
	for (const auto& key : m_vCompoundsKeys)
		vNames.push_back(m_pMaterialsDB->GetCompound(key)->GetName());
	return vNames;
}

int CStream::GetCompoundIndex(const std::string& _sCompoundKey) const
{
	for (unsigned i = 0; i < m_vCompoundsKeys.size(); i++)
		if (m_vCompoundsKeys[i] == _sCompoundKey)
			return i;
	return -1;
}

double CStream::GetCompoundFraction(double _dTime, std::string _sCompoundKey, unsigned _nBasis /*= BASIS_MASS*/) const
{
	return GetCompoundFraction(_dTime, GetCompoundIndex(_sCompoundKey), _nBasis);
}

double CStream::GetCompoundFraction(double _dTime, unsigned _nCompoundIndex, unsigned _nBasis /*= BASIS_MASS*/) const
{
	if (_nCompoundIndex >= m_vCompoundsKeys.size()) // no such compound
		return 0;

	double dVal = 0;
	if (_nBasis == BASIS_MASS)
		for (size_t i = 0; i < m_vpPhases.size(); ++i)
			dVal += m_PhaseFractions.GetValue(_dTime, i) * m_vpPhases[i]->distribution.GetValue(_dTime, DISTR_COMPOUNDS, _nCompoundIndex);
	else
		for (size_t i = 0; i < m_vpPhases.size(); ++i)
			dVal += m_PhaseFractions.GetValue(_dTime, i) * GetCompoundPhaseFraction(_dTime, _nCompoundIndex, m_vpPhases[i]->nAggregationState, _nBasis);

	return dVal;
}

std::vector<double> CStream::GetCompoundsFractions(double _dTime, unsigned _nBasis /*= BASIS_MASS*/) const
{
	std::vector<double> vRes(m_vCompoundsKeys.size());
	for (unsigned i = 0; i < (unsigned)m_vCompoundsKeys.size(); ++i)
		vRes[i] = GetCompoundFraction(_dTime, i, _nBasis);
	return vRes;
}

double CStream::GetCompoundPhaseFraction(double _dTime, const std::string& _sCompoundKey, unsigned _nPhase, unsigned _nBasis /*= BASIS_MASS*/) const
{
	return GetCompoundPhaseFraction( _dTime, GetCompoundIndex( _sCompoundKey ), _nPhase, _nBasis );
}

double CStream::GetCompoundPhaseFraction(double _dTime, unsigned _nCompoundIndex, unsigned _nPhase, unsigned _nBasis /*= BASIS_MASS*/) const
{
	int nPhaseIndex = -1;
	for( unsigned i=0; i<m_vpPhases.size(); ++i )
	{
		if( m_vpPhases[i]->nAggregationState == _nPhase )
		{
			nPhaseIndex = i;
			break;
		}
	}
	if( nPhaseIndex == -1 ) // no such phase
		return 0;

	if( _nCompoundIndex >= m_vCompoundsKeys.size() ) // no such compound
		return 0;

	if( _nBasis == BASIS_MASS )
	{
		return m_vpPhases[nPhaseIndex]->distribution.GetValue( _dTime, DISTR_COMPOUNDS, _nCompoundIndex );
	}
	else
	{
		double dRes = 0;
		for (unsigned i = 0; i < m_vCompoundsKeys.size(); ++i)
			if (GetCompoundConstant(m_vCompoundsKeys[i], MOLAR_MASS) != 0)
				dRes += m_vpPhases[nPhaseIndex]->distribution.GetValue(_dTime, DISTR_COMPOUNDS, i) / GetCompoundConstant(m_vCompoundsKeys[i], MOLAR_MASS);
		if (GetCompoundConstant(m_vCompoundsKeys[_nCompoundIndex], MOLAR_MASS) != 0 && dRes != 0)
			return m_vpPhases[nPhaseIndex]->distribution.GetValue(_dTime, DISTR_COMPOUNDS, _nCompoundIndex) / GetCompoundConstant(m_vCompoundsKeys[_nCompoundIndex], MOLAR_MASS) / dRes;
		else
			return 0;
	}
	/*else
	{
		double dRes = 0;
		for( unsigned i=0; i<m_vCompoundsKeys.size(); ++i )
			dRes += m_vPhases[_nPhaseIndex]->distribution.GetValue( _dTime, DISTRIBUTION_COMPOUNDS, i ) / GetCompoundConstant( m_vCompoundsKeys[i], MOLAR_MASS );
		return m_vPhases[_nPhaseIndex]->distribution.GetValue( _dTime, DISTRIBUTION_COMPOUNDS, _nCompoundIndex ) / GetCompoundConstant( m_vCompoundsKeys[_nCompoundIndex], MOLAR_MASS ) / dRes;
	}*/
	//return m_vPhases[_nPhaseIndex]->distribution.GetValue( _dTime, DISTRIBUTION_COMPOUNDS, _nCompoundIndex );
}

void CStream::SetCompoundPhaseFraction(double _dTime, std::string _sCompoundKey, unsigned _nPhase, double _dFraction, unsigned _nBasis /*= BASIS_MASS*/)
{
	int nPhaseIndex = -1;
	for( unsigned i=0; i<m_vpPhases.size(); ++i )
	{
		if( m_vpPhases[i]->nAggregationState == _nPhase )
		{
			nPhaseIndex = i;
			break;
		}
	}
	if( nPhaseIndex == -1 ) // no such phase
		return;

	int nCompoundIndex = GetCompoundIndex( _sCompoundKey );
	if( nCompoundIndex == -1 ) // no such compound
		return;

	if( _nBasis == BASIS_MASS )
	{
		m_vpPhases[nPhaseIndex]->distribution.SetValue( _dTime, DISTR_COMPOUNDS, nCompoundIndex, _dFraction );
	}
	else
	{
		double dWholeAmount = 0;
		for (unsigned i = 0; i < m_vCompoundsKeys.size(); ++i)
			if (GetCompoundConstant(m_vCompoundsKeys[i], MOLAR_MASS) != 0)
				dWholeAmount += GetCompoundPhaseFraction(_dTime, i, _nPhase) / GetCompoundConstant(m_vCompoundsKeys[i], MOLAR_MASS);
		double dSetVal = dWholeAmount != 0 ? _dFraction * GetCompoundConstant( _sCompoundKey, MOLAR_MASS ) / dWholeAmount : 0;
		m_vpPhases[nPhaseIndex]->distribution.SetValue( _dTime, DISTR_COMPOUNDS, nCompoundIndex, dSetVal );
	}
}

bool CStream::IsCompoundsCorrect(const std::vector<std::string>& _vCompounds) const
{
	for(size_t i=0; i<_vCompounds.size(); ++i)
	{
		bool bFound = false;
		for (size_t j=0; j<m_vCompoundsKeys.size(); ++j)
		{
			if(_vCompounds[i] == m_vCompoundsKeys[j])
			{
				bFound = true;
				break;
			}
		}
		if(!bFound)
			return false;
	}
	return true;
}

double CStream::GetCompoundMass_Base(double _dTime, unsigned _nCompoundIndex, unsigned _nPhase, unsigned _nBasis /*= BASIS_MASS */) const
{
	if( _nCompoundIndex >= m_vCompoundsKeys.size() ) // no such compound
		return 0;

	int nPhaseIndex = -1;
	for( unsigned i=0; i<m_vpPhases.size(); ++i )
	{
		if( m_vpPhases[i]->nAggregationState == _nPhase )
		{
			nPhaseIndex = i;
			break;
		}
	}
	if( nPhaseIndex == -1 ) // no such phase
		return 0;

	return m_PhaseFractions.GetValue( _dTime, nPhaseIndex )	// mass fraction of phase
		* m_vpPhases[nPhaseIndex]->distribution.GetValue( _dTime, DISTR_COMPOUNDS, _nCompoundIndex )		// mass fraction of compound in phase
		* GetMass_Base( _dTime, _nBasis );						// whole mass
}

double CStream::GetCompoundConstant(const std::string& _sCompoundKey, ECompoundConstProperties _nConstProperty) const
{
	double dVal = 0;

	if(!m_pMaterialsDB) return dVal;

	switch(static_cast<ECompoundConstProperties>(_nConstProperty))
	{
	//case REACTIVITY_TYPE:
	case CRITICAL_PRESSURE:
	case CRITICAL_TEMPERATURE:
	case HEAT_OF_FUSION_AT_NORMAL_FREEZING_POINT:
	case HEAT_OF_VAPORIZATION_AT_NORMAL_BOILING_POINT:
	case MOLAR_MASS:
	case NORMAL_BOILING_POINT:
	case NORMAL_FREEZING_POINT:
	case BOND_WORK_INDEX:
	case SOA_AT_NORMAL_CONDITIONS:
	case STANDARD_FORMATION_ENTHALPY:
	case CONST_PROP_USER_DEFINED_01:
	case CONST_PROP_USER_DEFINED_02:
	case CONST_PROP_USER_DEFINED_03:
	case CONST_PROP_USER_DEFINED_04:
	case CONST_PROP_USER_DEFINED_05:
	case CONST_PROP_USER_DEFINED_06:
	case CONST_PROP_USER_DEFINED_07:
	case CONST_PROP_USER_DEFINED_08:
	case CONST_PROP_USER_DEFINED_09:
	case CONST_PROP_USER_DEFINED_10:
	case CONST_PROP_USER_DEFINED_11:
	case CONST_PROP_USER_DEFINED_12:
	case CONST_PROP_USER_DEFINED_13:
	case CONST_PROP_USER_DEFINED_14:
	case CONST_PROP_USER_DEFINED_15:
	case CONST_PROP_USER_DEFINED_16:
	case CONST_PROP_USER_DEFINED_17:
	case CONST_PROP_USER_DEFINED_18:
	case CONST_PROP_USER_DEFINED_19:
	case CONST_PROP_USER_DEFINED_20:
		dVal = m_pMaterialsDB->GetConstPropertyValue(_sCompoundKey, _nConstProperty);
		break;
	case CONST_PROP_NO_PROERTY: break;
	}
	return dVal;
}

// For pure compound depending on T or P
double CStream::GetCompoundTPDProp(const std::string& _sCompoundKey, unsigned _nProperty, double _dTemperature, double _dPressure) const
{
	double dVal = 0;

	if (!m_pMaterialsDB) return dVal;

	switch (static_cast<ECompoundTPProperties>(_nProperty))
	{
	case ENTHALPY:
		dVal = m_pMaterialsDB->GetTPPropertyValue(_sCompoundKey, static_cast<ECompoundTPProperties>(_nProperty), _dTemperature, _dPressure);
		// TO DO: Pressure correction of the fraction of the compound in liquid SOA. Therefore
		/*
		double dCompoundLiquidPhaseFraction = 0;		// (m_Comp_liquid + m_Comp_liquid2) / m_Comp
		double dLiquidDensity = GetCompoundTPDProp(_sCompoundKey, DENSITY, _dTemperature, _dPressure);	// TO DO: liquid density!
		if (dCompoundLiquidPhaseFraction > 0)
			dVal += dCompoundLiquidPhaseFraction / dLiquidDensity * (_dPressure - STANDARD_CONDITION_P);
		*/
		break;
	case THERMAL_CONDUCTIVITY:
	case VAPOR_PRESSURE:
	case VISCOSITY:
	case DENSITY:
	case PERMITTIVITY:
	case TP_PROP_USER_DEFINED_01:
	case TP_PROP_USER_DEFINED_02:
	case TP_PROP_USER_DEFINED_03:
	case TP_PROP_USER_DEFINED_04:
	case TP_PROP_USER_DEFINED_05:
	case TP_PROP_USER_DEFINED_06:
	case TP_PROP_USER_DEFINED_07:
	case TP_PROP_USER_DEFINED_08:
	case TP_PROP_USER_DEFINED_09:
	case TP_PROP_USER_DEFINED_10:
	case TP_PROP_USER_DEFINED_11:
	case TP_PROP_USER_DEFINED_12:
	case TP_PROP_USER_DEFINED_13:
	case TP_PROP_USER_DEFINED_14:
	case TP_PROP_USER_DEFINED_15:
	case TP_PROP_USER_DEFINED_16:
	case TP_PROP_USER_DEFINED_17:
	case TP_PROP_USER_DEFINED_18:
	case TP_PROP_USER_DEFINED_19:
	case TP_PROP_USER_DEFINED_20:
		dVal = m_pMaterialsDB->GetTPPropertyValue(_sCompoundKey, static_cast<ECompoundTPProperties>(_nProperty), _dTemperature, _dPressure);
		break;
	case TP_PROP_NO_PROERTY: break;
	}
	return dVal;
}

double CStream::GetCompoundTPDProp(double _dTime, const std::string& _sCompoundKey, unsigned _nProperty) const
{
	double dTemperature = GetTemperature( _dTime );
	double dPressure = GetPressure( _dTime );

	return GetCompoundTPDProp( _sCompoundKey, _nProperty, dTemperature, dPressure );
}

double CStream::GetCompoundInteractionProp(const std::string& _sCompoundKey1, const std::string& _sCompoundKey2, unsigned _nProperty, const double _dTemperature, const double _dPressure) const
{
	if (!m_pMaterialsDB) return 0;

	switch (static_cast<EInteractionProperties>(_nProperty))
	{
	case INTERFACE_TENSION:
	case INT_PROP_USER_DEFINED_01:
	case INT_PROP_USER_DEFINED_02:
	case INT_PROP_USER_DEFINED_03:
	case INT_PROP_USER_DEFINED_04:
	case INT_PROP_USER_DEFINED_05:
	case INT_PROP_USER_DEFINED_06:
	case INT_PROP_USER_DEFINED_07:
	case INT_PROP_USER_DEFINED_08:
	case INT_PROP_USER_DEFINED_09:
	case INT_PROP_USER_DEFINED_10:
	case INT_PROP_USER_DEFINED_11:
	case INT_PROP_USER_DEFINED_12:
	case INT_PROP_USER_DEFINED_13:
	case INT_PROP_USER_DEFINED_14:
	case INT_PROP_USER_DEFINED_15:
	case INT_PROP_USER_DEFINED_16:
	case INT_PROP_USER_DEFINED_17:
	case INT_PROP_USER_DEFINED_18:
	case INT_PROP_USER_DEFINED_19:
	case INT_PROP_USER_DEFINED_20:
		return m_pMaterialsDB->GetInteractionPropertyValue(_sCompoundKey1, _sCompoundKey2, static_cast<EInteractionProperties>(_nProperty), _dTemperature, _dPressure);
	case INT_PROP_NO_PROERTY: break;
	}

	return 0;
}

double CStream::GetCompoundInteractionProp(const double _dTime, const std::string _sCompoundKey1, const std::string _sCompoundKey2, const unsigned _nProperty) const
{
	const double dTemperature = GetTemperature(_dTime);
	const double dPressure = GetPressure(_dTime);
	return GetCompoundInteractionProp(_sCompoundKey1, _sCompoundKey2, _nProperty, dTemperature, dPressure);
}

void CStream::AddPhase(std::string _sName, unsigned _nAggrState)
{
	// add phase to the structure
	SPhase *pPhase = new SPhase;
	pPhase->sName = _sName;
	pPhase->nAggregationState = _nAggrState;
	pPhase->distribution.SetCachePath( m_sCachePath );
	pPhase->distribution.SetCacheParams( m_bCacheEnabled, m_nCacheWindow );
	m_vpPhases.push_back( pPhase );

	// add dimensions according to compounds
	std::vector<EDistrTypes> vTypes;
	std::vector<unsigned> vClasses;
	if( _nAggrState != SOA_SOLID )
	{
		vTypes.push_back( DISTR_COMPOUNDS );
		vClasses.push_back( static_cast<unsigned>(m_vCompoundsKeys.size()) );
	}
	else
	{
		if( m_pDistributionsGrid != NULL )
		{
			vTypes = m_pDistributionsGrid->GetDistrTypes();
			vClasses = m_pDistributionsGrid->GetClasses();
		}
	}
	m_vpPhases.back()->distribution.SetDimensions( (std::vector<unsigned>&)vTypes, vClasses );

	for( unsigned i=0; i<m_vTimePoints.size(); ++i )
		m_vpPhases.back()->distribution.AddTimePoint( m_vTimePoints[i] );

	// add phase to m_PhaseFractions
	m_PhaseFractions.AddDimension();
	m_PhaseFractions.SetDimensionLabel( m_PhaseFractions.GetDimensionsNumber()-1, _sName );
}

void CStream::RemovePhase(unsigned _nIndex)
{
	if( _nIndex >= m_vpPhases.size() ) // wrong index
		return;

	m_vpPhases.erase( m_vpPhases.begin() + _nIndex );
	m_PhaseFractions.RemoveDimension( _nIndex );
}

void CStream::ChangePhase(unsigned _nIndex, std::string _sName, unsigned _nAggrState)
{
	if( _nIndex >= m_vpPhases.size() ) // wrong index
		return;

	m_vpPhases[_nIndex]->sName = _sName;

	if( ( m_vpPhases[_nIndex]->nAggregationState == SOA_SOLID ) && ( _nAggrState != SOA_SOLID ) )
	{
		m_vpPhases[_nIndex]->distribution.SetDimension( DISTR_COMPOUNDS, static_cast<unsigned>(m_vCompoundsKeys.size()) );
		for( unsigned i=0; i<m_vTimePoints.size(); ++i )
			m_vpPhases[_nIndex]->distribution.AddTimePoint( m_vTimePoints[i] );
	}
	else if( ( m_vpPhases[_nIndex]->nAggregationState != SOA_SOLID ) && ( _nAggrState == SOA_SOLID ) )
	{
		if(m_pDistributionsGrid)
		{
			m_vpPhases[_nIndex]->distribution.SetDimensions(VectorEnumToIntegral(m_pDistributionsGrid->GetDistrTypes()), m_pDistributionsGrid->GetClasses());
			for( unsigned i=0; i<m_vTimePoints.size(); ++i )
				m_vpPhases[_nIndex]->distribution.AddTimePoint( m_vTimePoints[i] );
		}
	}

	m_vpPhases[_nIndex]->nAggregationState = _nAggrState;

	m_PhaseFractions.SetDimensionLabel( _nIndex, _sName );
}

void CStream::SetPhases(const std::vector<std::string>& _vNames, const std::vector<unsigned>& _vAggrStates)
{
	if( _vNames.size() != _vAggrStates.size() ) // wrong parameters
		return;

	for( unsigned i=0; i<m_vpPhases.size(); ++i )
		delete m_vpPhases[i];
	m_vpPhases.clear();
	m_PhaseFractions.Clear();

	for( unsigned i=0; i<_vNames.size(); ++i )
		AddPhase( _vNames[i], _vAggrStates[i] );
}

size_t CStream::GetPhasesNumber() const
{
	return m_vpPhases.size();
}

std::string CStream::GetPhaseName(unsigned _nIndex) const
{
	if( _nIndex < m_vpPhases.size() )
		return m_vpPhases[_nIndex]->sName;
	return "";
}

unsigned CStream::GetPhaseSOA(unsigned _nIndex) const
{
	if( _nIndex < m_vpPhases.size() )
		return m_vpPhases[_nIndex]->nAggregationState;
	return SOA_UNDEFINED;
}

size_t CStream::GetPhaseIndex(unsigned _phase) const
{
	for (size_t i = 0; i < m_vpPhases.size(); ++i)
		if (m_vpPhases[i]->nAggregationState == _phase)
			return i;
	return -1;
}

CMDMatrix* CStream::GetPhaseDistribution(unsigned _nIndex) const
{
	if( _nIndex < m_vpPhases.size() )
		return &m_vpPhases[_nIndex]->distribution;
	return NULL;
}

double CStream::GetPhaseMass_Base(double _dTime, unsigned _nPhase, unsigned _nBasis /*= BASIS_MASS */) const
{
	size_t nPhaseIndex = GetPhaseIndex(_nPhase);
	if (nPhaseIndex == -1) // no such phase
		return 0;

	double dWholeMass = m_StreamMTP.GetValue(_dTime, MTP_MASS);
	double dFractionFactor = m_PhaseFractions.GetValue(_dTime, nPhaseIndex);
	if (_nBasis == BASIS_MOLL)
	{
		if (GetSinglePhaseProp(_dTime, MOLAR_MASS, _nPhase) != 0)
			return dWholeMass * dFractionFactor / GetSinglePhaseProp(_dTime, MOLAR_MASS, _nPhase);
		else
			return 0;
	}
	else
	{
		return dWholeMass * dFractionFactor;
	}
}

void CStream::SetPhaseMass_Base(double _dTime, unsigned _nPhase, double _dMassFlow, unsigned _nBasis /*= BASIS_MASS */)
{
	size_t nPhaseIndex = GetPhaseIndex(_nPhase);
	if( nPhaseIndex == -1 ) // no such phase
		return;

	size_t nIndex = GetTimeIndex( _dTime );
	if( nIndex == -1 )
		return;

	if( _dMassFlow < 0 ) _dMassFlow = 0;
	if( _nBasis == BASIS_MOLL )
		_dMassFlow = _dMassFlow * GetSinglePhaseProp( _dTime, MOLAR_MASS, _nPhase );

	double dTotalMass = GetMass_Base( _dTime );
	std::vector<double> vPhaseFlows( m_vpPhases.size() );
	for( unsigned i=0; i<m_vpPhases.size(); ++i )
		vPhaseFlows[i] = GetPhaseMass_Base( _dTime, m_vpPhases[i]->nAggregationState );
	dTotalMass += _dMassFlow - vPhaseFlows[nPhaseIndex];
	vPhaseFlows[nPhaseIndex] = _dMassFlow;
	for( unsigned i=0; i<m_vpPhases.size(); ++i )
		if(dTotalMass != 0)
			m_PhaseFractions.SetValue( _dTime, i, vPhaseFlows[i]/dTotalMass );
		else
			m_PhaseFractions.SetValue( _dTime, i, 0 );
	SetMass_Base( _dTime, dTotalMass );
}

double CStream::GetSinglePhaseProp(double _dTime, unsigned _nProperty, unsigned _nPhase, unsigned _nBasis /*= BASIS_MASS */) const
{
	size_t nPhaseIndex = GetPhaseIndex(_nPhase);
	if( nPhaseIndex == -1 ) // no such phase
		return 0;

	double dRes = 0;
	switch (static_cast<ECompoundTPProperties>(_nProperty))
	{
	case FLOW:
		dRes = GetPhaseMass_Base( _dTime, _nPhase, _nBasis );
		break;
// TODO: specify algorithm
	case TEMPERATURE:
		dRes = GetTemperature( _dTime );
		break;
// TODO: specify algorithm
	case PRESSURE:
		dRes = m_StreamMTP.GetValue( _dTime, MTP_PRESSURE );
		break;
	case PHASE_FRACTION:
	case FRACTION:
		if( _nBasis == BASIS_MASS )
		{
			dRes = m_PhaseFractions.GetValue( _dTime, nPhaseIndex );
		}
		else
		{
			double dTemp = 0;
			for (unsigned i = 0; i < m_vpPhases.size(); ++i)
				if (GetSinglePhaseProp(_dTime, MOLAR_MASS, m_vpPhases[i]->nAggregationState) != 0)
					dTemp += m_PhaseFractions.GetValue(_dTime, i) / GetSinglePhaseProp(_dTime, MOLAR_MASS, m_vpPhases[i]->nAggregationState);
			if (GetSinglePhaseProp(_dTime, MOLAR_MASS, _nPhase) != 0 && dTemp != 0)
				dRes = m_PhaseFractions.GetValue(_dTime, nPhaseIndex) / GetSinglePhaseProp(_dTime, MOLAR_MASS, _nPhase) / dTemp;
		}
		break;
	case MOLAR_MASS:
		for (unsigned i = 0; i < m_vCompoundsKeys.size(); ++i)
			if (GetCompoundConstant(m_vCompoundsKeys[i], MOLAR_MASS) != 0)
				dRes += GetCompoundPhaseFraction(_dTime, i, _nPhase) / GetCompoundConstant(m_vCompoundsKeys[i], MOLAR_MASS);
		if (dRes != 0)
			dRes = 1 / dRes;
		break;
	case ENTHALPY:
		if( _nBasis == BASIS_MOLL )
		{
			dRes = GetPhaseTPDProp(_dTime, _nProperty, _nPhase) * GetSinglePhaseProp(_dTime, MOLAR_MASS, _nPhase);
		}
		else
		{
			dRes = GetPhaseTPDProp(_dTime, _nProperty, _nPhase);
		}
		break;
	case DENSITY:
	case THERMAL_CONDUCTIVITY:
	case VISCOSITY:
	case VAPOR_PRESSURE:
	case PERMITTIVITY:
	case TP_PROP_USER_DEFINED_01:
	case TP_PROP_USER_DEFINED_02:
	case TP_PROP_USER_DEFINED_03:
	case TP_PROP_USER_DEFINED_04:
	case TP_PROP_USER_DEFINED_05:
	case TP_PROP_USER_DEFINED_06:
	case TP_PROP_USER_DEFINED_07:
	case TP_PROP_USER_DEFINED_08:
	case TP_PROP_USER_DEFINED_09:
	case TP_PROP_USER_DEFINED_10:
	case TP_PROP_USER_DEFINED_11:
	case TP_PROP_USER_DEFINED_12:
	case TP_PROP_USER_DEFINED_13:
	case TP_PROP_USER_DEFINED_14:
	case TP_PROP_USER_DEFINED_15:
	case TP_PROP_USER_DEFINED_16:
	case TP_PROP_USER_DEFINED_17:
	case TP_PROP_USER_DEFINED_18:
	case TP_PROP_USER_DEFINED_19:
	case TP_PROP_USER_DEFINED_20:
		dRes = GetPhaseTPDProp(_dTime, _nProperty, _nPhase);
		break;
	case TP_PROP_NO_PROERTY: break;
	}
	return dRes;
}

void CStream::SetSinglePhaseProp(double _dTime, unsigned _nProperty, unsigned _nPhase, double _dValue, unsigned _nBasis /*= BASIS_MASS */)
{
	int nPhaseIndex = -1;
	for( unsigned i=0; i<m_vpPhases.size(); ++i )
	{
		if( m_vpPhases[i]->nAggregationState == _nPhase )
		{
			nPhaseIndex = i;
			break;
		}
	}
	if( nPhaseIndex == -1 ) // no such phase
		return;

	switch( _nProperty )
	{
	case FLOW:
		SetPhaseMass_Base( _dTime, _nPhase, _dValue, _nBasis );
		break;
// TODO: specify algorithm
	case TEMPERATURE:
		break;
// TODO: specify algorithm
	case PRESSURE:
		break;
	case PHASE_FRACTION:
	case FRACTION:
		m_PhaseFractions.SetValue( _dTime, nPhaseIndex, _dValue );
		break;
	}
}

//double CStream::GetTwoPhaseProp(double _dTime, unsigned _nProperty, unsigned _nPhaseIndex1, unsigned _nPhaseIndex2) const
//{
//	double dRes = 0;
//
//	switch( _nProperty )
//	{
//	case K_VALUE:
//		dRes = GetSinglePhaseProp( _dTime, FUGACITY_COEFFICIENT, _nPhaseIndex2 ) / GetSinglePhaseProp( _dTime, FUGACITY_COEFFICIENT, _nPhaseIndex1 );
//		break;
//	case LOG_K_VALUE:
//		dRes = std::log( GetSinglePhaseProp( _dTime, FUGACITY_COEFFICIENT, _nPhaseIndex2 ) / GetSinglePhaseProp( _dTime, FUGACITY_COEFFICIENT, _nPhaseIndex1 ) );
//		break;
//// TODO:
//	case SURFACE_TENSION:
//		dRes = -1;
//		break;
//	}
//
//	return dRes;
//}

/// For single phase mixture
double CStream::GetPhaseTPDProp(double _dTime, unsigned _nProperty, unsigned _nPhase) const
{
	int nPhaseIndex = -1;
	for( unsigned i=0; i<m_vpPhases.size(); ++i )
	{
		if( m_vpPhases[i]->nAggregationState == _nPhase )
		{
			nPhaseIndex = i;
			break;
		}
	}
	if( nPhaseIndex == -1 ) // no such phase
		return 0;

	if( !m_pMaterialsDB )
		return 0;

	double dRes = 0;
	double dT = GetSinglePhaseProp( _dTime, TEMPERATURE, _nPhase );
	double dP = GetSinglePhaseProp( _dTime, PRESSURE, _nPhase );

	switch (static_cast<ECompoundTPProperties>(_nProperty))
	{
	case VAPOR_PRESSURE:
		//dRes = m_pMaterialsDB->GetPropertyValue(m_vCompoundsKeys[0], _nProperty, m_vpPhases[nPhaseIndex]->nAggregationState, dT, dP);
		dRes = m_pMaterialsDB->GetTPPropertyValue(m_vCompoundsKeys[0], static_cast<ECompoundTPProperties>(_nProperty), dT, dP);
		for( unsigned i=1; i<m_vCompoundsKeys.size(); ++i )
		{
			//double dNewPressure = m_pMaterialsDB->GetPropertyValue(m_vCompoundsKeys[0], _nProperty, m_vpPhases[nPhaseIndex]->nAggregationState, dT, dP);
			double dNewPressure = m_pMaterialsDB->GetTPPropertyValue(m_vCompoundsKeys[0], static_cast<ECompoundTPProperties>(_nProperty), dT, dP);
			dRes = std::min( dNewPressure, dRes );
		}
		break;
	case VISCOSITY:
		switch( m_vpPhases[nPhaseIndex]->nAggregationState )
		{
		case SOA_LIQUID:
		case SOA_LIQUID2:
			for( unsigned i=0; i<m_vCompoundsKeys.size(); ++i )
			{
				//double dVisco = m_pMaterialsDB->GetPropertyValue(m_vCompoundsKeys[i], _nProperty, m_vpPhases[nPhaseIndex]->nAggregationState, dT, dP);
				double dVisco = m_pMaterialsDB->GetTPPropertyValue(m_vCompoundsKeys[i], static_cast<ECompoundTPProperties>(_nProperty), dT, dP);
				if (dVisco != 0)
					dRes += GetCompoundPhaseFraction(_dTime, i, _nPhase) * std::log(dVisco);
			}
			if( dRes != 0 )
				dRes = std::exp( dRes );
			break;
		case SOA_VAPOR:
		{
			double dNumerator = 0, dDenominator = 0;
			for( unsigned i=0; i<m_vCompoundsKeys.size(); ++i )
			{
				//double dVisco = m_pMaterialsDB->GetPropertyValue(m_vCompoundsKeys[i], _nProperty, m_vpPhases[nPhaseIndex]->nAggregationState, dT, dP);
				double dVisco = m_pMaterialsDB->GetTPPropertyValue(m_vCompoundsKeys[i], static_cast<ECompoundTPProperties>(_nProperty), dT, dP);
				double dMollMass = GetCompoundConstant( m_vCompoundsKeys[i], MOLAR_MASS );
				double dMollFrac = GetCompoundPhaseFraction( _dTime, i, _nPhase, BASIS_MOLL );
				dNumerator += dMollFrac * dVisco * std::sqrt( dMollMass );
				dDenominator += dMollFrac * std::sqrt( dMollMass );
			}
			if( dDenominator != 0 )
				dRes = dNumerator / dDenominator;
			break;
		}
		default:
			for( unsigned i=0; i<m_vCompoundsKeys.size(); ++i )
				//dRes += GetCompoundPhaseFraction(_dTime, i, _nPhase) * m_pMaterialsDB->GetPropertyValue(m_vCompoundsKeys[i], _nProperty, m_vpPhases[nPhaseIndex]->nAggregationState, dT, dP);
				dRes += GetCompoundPhaseFraction(_dTime, i, _nPhase) * m_pMaterialsDB->GetTPPropertyValue(m_vCompoundsKeys[i], static_cast<ECompoundTPProperties>(_nProperty), dT, dP);
			break;
		}
		break;
	case THERMAL_CONDUCTIVITY:
		switch( m_vpPhases[nPhaseIndex]->nAggregationState )
		{
		case SOA_LIQUID:
		case SOA_LIQUID2:
			for( unsigned i=0; i<m_vCompoundsKeys.size(); ++i )
				dRes += GetCompoundPhaseFraction( _dTime, i, _nPhase, BASIS_MOLL )
				/// std::pow(m_pMaterialsDB->GetPropertyValue(m_vCompoundsKeys[i], _nProperty, m_vpPhases[nPhaseIndex]->nAggregationState, dT, dP), 2.0);
				/ std::pow(m_pMaterialsDB->GetTPPropertyValue(m_vCompoundsKeys[i], static_cast<ECompoundTPProperties>(_nProperty), dT, dP), 2.0);
			if( dRes != 0 )
				dRes = 1/ std::sqrt( dRes );
			break;
		case SOA_VAPOR:
			for( unsigned i=0; i<m_vCompoundsKeys.size(); ++i )
			{
				double dNumerator, dDenominator = 0;
				//double dConductI = m_pMaterialsDB->GetPropertyValue(m_vCompoundsKeys[i], _nProperty, m_vpPhases[nPhaseIndex]->nAggregationState, dT, dP);
				double dConductI = m_pMaterialsDB->GetTPPropertyValue(m_vCompoundsKeys[i], static_cast<ECompoundTPProperties>(_nProperty), dT, dP);
				double dMollMassI = GetCompoundConstant( m_vCompoundsKeys[i], MOLAR_MASS );
				dNumerator = GetCompoundPhaseFraction( _dTime, i, _nPhase, BASIS_MOLL ) * dConductI;
				for( unsigned j=0; j<m_vCompoundsKeys.size(); ++j )
				{
					//double dConductJ = m_pMaterialsDB->GetPropertyValue(m_vCompoundsKeys[j], _nProperty, m_vpPhases[nPhaseIndex]->nAggregationState, dT, dP);
					double dConductJ = m_pMaterialsDB->GetTPPropertyValue(m_vCompoundsKeys[j], static_cast<ECompoundTPProperties>(_nProperty), dT, dP);
					double dMollMassJ = GetCompoundConstant( m_vCompoundsKeys[j], MOLAR_MASS );
					double dF = std::pow( ( 1 + std::sqrt( dConductI/dConductJ ) * std::pow( dMollMassJ/dMollMassI, 1/4 ) ), 2 ) / (std::sqrt( 8*( 1 + dMollMassI/dMollMassJ ) ) );
					dDenominator += GetCompoundPhaseFraction( _dTime, j, _nPhase, BASIS_MOLL ) * dF;
				}
				if( dDenominator != 0 )
					dRes += dNumerator / dDenominator;
			}
			break;
		default:
			for( unsigned i=0; i<m_vCompoundsKeys.size(); ++i )
				//dRes += GetCompoundPhaseFraction(_dTime, i, _nPhase) * m_pMaterialsDB->GetPropertyValue(m_vCompoundsKeys[i], _nProperty, m_vpPhases[nPhaseIndex]->nAggregationState, dT, dP);
				dRes += GetCompoundPhaseFraction(_dTime, i, _nPhase) * m_pMaterialsDB->GetTPPropertyValue(m_vCompoundsKeys[i], static_cast<ECompoundTPProperties>(_nProperty), dT, dP);
			break;
		}
		break;
	case DENSITY:
		if( ( m_vpPhases[nPhaseIndex]->nAggregationState == SOA_SOLID ) && ( m_pDistributionsGrid->IsDistrTypePresent( DISTR_PART_POROSITY ) ) )
		{
			CMatrix2D distr;
			m_vpPhases[nPhaseIndex]->distribution.GetDistribution( _dTime, DISTR_COMPOUNDS, DISTR_PART_POROSITY, distr );
			size_t nCompNum = m_vCompoundsKeys.size();
			size_t nPorosNum = m_pDistributionsGrid->GetClassesByDistr( DISTR_PART_POROSITY );
			std::vector<double> vPorosities = m_pDistributionsGrid->GetClassMeansByDistr( DISTR_PART_POROSITY );
			for(size_t iComp=0; iComp<nCompNum; ++iComp )
			{
				double dDensity = GetCompoundTPDProp( _dTime, m_vCompoundsKeys[iComp], DENSITY );
				for(size_t iPoros=0; iPoros<nPorosNum; ++iPoros )
				{
					//double d = distr.GetValue( iComp, iPoros );
					dRes += dDensity * ( 1 - vPorosities[iPoros] ) * distr[iComp][iPoros];
				}
			}

		}
		else
		{
			for( unsigned i=0; i<m_vCompoundsKeys.size(); ++i )
			{
				//double dComponentDensity = m_pMaterialsDB->GetPropertyValue(m_vCompoundsKeys[i], _nProperty, m_vpPhases[nPhaseIndex]->nAggregationState, dT, dP);
				double dComponentDensity = m_pMaterialsDB->GetTPPropertyValue(m_vCompoundsKeys[i], static_cast<ECompoundTPProperties>(_nProperty), dT, dP);
				if( dComponentDensity != 0 )
					dRes += GetCompoundPhaseFraction( _dTime, i, _nPhase ) / dComponentDensity;
			}
			if( dRes != 0 )
				dRes = 1/dRes;
		}
		break;
	case ENTHALPY:
		for (unsigned i = 0; i < m_vCompoundsKeys.size(); ++i)
		{
			double dTempCompFrac = GetCompoundPhaseFraction(_dTime, i, _nPhase);
			//dRes += dTempCompFrac * m_pMaterialsDB->GetPropertyValue(m_vCompoundsKeys[i], _nProperty, m_vpPhases[nPhaseIndex]->nAggregationState, dT, dP);
			dRes += dTempCompFrac * m_pMaterialsDB->GetTPPropertyValue(m_vCompoundsKeys[i], static_cast<ECompoundTPProperties>(_nProperty), dT, dP);
		}
		//dRes += CalcEnthalpyPressureCorrection(_dTime, _nPhase, MOLAR_MASS);
		break;
	case PERMITTIVITY:
	case TP_PROP_USER_DEFINED_01:
	case TP_PROP_USER_DEFINED_02:
	case TP_PROP_USER_DEFINED_03:
	case TP_PROP_USER_DEFINED_04:
	case TP_PROP_USER_DEFINED_05:
	case TP_PROP_USER_DEFINED_06:
	case TP_PROP_USER_DEFINED_07:
	case TP_PROP_USER_DEFINED_08:
	case TP_PROP_USER_DEFINED_09:
	case TP_PROP_USER_DEFINED_10:
	case TP_PROP_USER_DEFINED_11:
	case TP_PROP_USER_DEFINED_12:
	case TP_PROP_USER_DEFINED_13:
	case TP_PROP_USER_DEFINED_14:
	case TP_PROP_USER_DEFINED_15:
	case TP_PROP_USER_DEFINED_16:
	case TP_PROP_USER_DEFINED_17:
	case TP_PROP_USER_DEFINED_18:
	case TP_PROP_USER_DEFINED_19:
	case TP_PROP_USER_DEFINED_20:
		for (unsigned i = 0; i < m_vCompoundsKeys.size(); ++i)
			//dRes += GetCompoundPhaseFraction(_dTime, i, _nPhase) * m_pMaterialsDB->GetPropertyValue(m_vCompoundsKeys[i], _nProperty, m_vpPhases[nPhaseIndex]->nAggregationState, dT, dP);
			dRes += GetCompoundPhaseFraction(_dTime, i, _nPhase) * m_pMaterialsDB->GetTPPropertyValue(m_vCompoundsKeys[i], static_cast<ECompoundTPProperties>(_nProperty), dT, dP);
		break;
	case TP_PROP_NO_PROERTY: break;
	}

	return dRes;
}

std::vector<SPhase*>* CStream::GetPhases()
{
	return &m_vpPhases;
}

const std::vector<SPhase*>* CStream::GetPhases() const
{
	return &m_vpPhases;
}

void CStream::SetDistributionsGrid(const CDistributionsGrid* _pGrid)
{
	if( _pGrid != NULL )
	{
		int index = GetSolidPhaseIndex();
		if (index != -1)
			m_vpPhases[index]->distribution.UpdateDimensions(VectorEnumToIntegral(_pGrid->GetDistrTypes()), _pGrid->GetClasses());
	}

	m_pDistributionsGrid = _pGrid;
}

std::vector<unsigned> CStream::GetDistributionsTypes() const
{
	int index = GetSolidPhaseIndex();
	if( index == -1 ) // no solid phase
		return std::vector<unsigned>();
	return m_vpPhases[index]->distribution.GetDimensions();
}

std::vector<unsigned> CStream::GetDistributionsClasses() const
{
	int index = GetSolidPhaseIndex();
	if( index == -1 ) // no solid phase
		return std::vector<unsigned>();
	return m_vpPhases[index]->distribution.GetClasses();
}

size_t CStream::GetDistributionsNumber() const
{
	int index = GetSolidPhaseIndex();
	if( index == -1 ) // no solid phase
		return 0;
	return m_vpPhases[index]->distribution.GetDimensionsNumber();
}

EGridEntry CStream::GetDistributionGridType(EDistrTypes _nDimension) const
{
	if( GetSolidPhaseIndex() == -1 ) // no solid phase
		return EGridEntry::GRID_UNDEFINED;
	return m_pDistributionsGrid->GetGridEntryByDistr( _nDimension );
}

std::vector<double> CStream::GetNumericGrid(EDistrTypes _nDimension) const
{
	if( GetSolidPhaseIndex() == -1 ) // no solid phase
		return std::vector<double>();
	return m_pDistributionsGrid->GetNumericGridByDistr( _nDimension );
}

std::vector<std::string> CStream::GetSymbolicGrid(EDistrTypes _nDimension) const
{
	if( GetSolidPhaseIndex() == -1 ) // no solid phase
		return std::vector<std::string>();
	return m_pDistributionsGrid->GetSymbolicGridByDistr( _nDimension );
}

double CStream::GetFraction(double _dTime, const std::vector<unsigned>& _vCoords) const
{
	int index = GetSolidPhaseIndex();
	if( index == -1 ) // no solid phase
		return 0;
	return m_vpPhases[index]->distribution.GetValue( _dTime, _vCoords );
}

void CStream::SetFraction(double _dTime, const std::vector<unsigned>& _vCoords, double _dValue)
{
	int index = GetSolidPhaseIndex();
	if( index == -1 ) // no solid phase
		return;
	m_vpPhases[index]->distribution.SetValue( _dTime, _vCoords, _dValue );
}

CDenseMDMatrix CStream::GetDistribution(double _dTime) const
{
	const int index = GetSolidPhaseIndex();
	if (index == -1)
		return {};
	return m_vpPhases[index]->distribution.GetDistribution(_dTime);
}

bool CStream::GetDistribution(double _dTime, EDistrTypes _nDim, std::vector<double>& _vResult) const
{
	int index = GetSolidPhaseIndex();
	if( index == -1 )
		return false;
	return m_vpPhases[index]->distribution.GetDistribution( _dTime, _nDim, _vResult );
}

std::vector<double> CStream::GetDistribution(double _dTime, EDistrTypes _nDim) const
{
	const int index = GetSolidPhaseIndex();
	if (index == -1)
		return {};
	return m_vpPhases[index]->distribution.GetDistribution(_dTime, _nDim);
}

bool CStream::GetDistribution(double _dTime, EDistrTypes _nDim1, EDistrTypes _nDim2, CMatrix2D& _2DResult) const
{
	int index = GetSolidPhaseIndex();
	if( index == -1 )
		return false;
	return m_vpPhases[index]->distribution.GetDistribution( _dTime, _nDim1, _nDim2, _2DResult );
}

CMatrix2D CStream::GetDistribution(double _dTime, EDistrTypes _nDim1, EDistrTypes _nDim2) const
{
	const int index = GetSolidPhaseIndex();
	if (index == -1)
		return {};
	return m_vpPhases[index]->distribution.GetDistribution(_dTime, _nDim1, _nDim2);
}

bool CStream::GetDistribution(double _dTime, const std::vector<EDistrTypes>& _vDims, CDenseMDMatrix& _MDResult) const
{
	int index = GetSolidPhaseIndex();
	if( index == -1 )
		return false;
	return m_vpPhases[index]->distribution.GetDistribution( _dTime, (std::vector<unsigned>&)_vDims, _MDResult );
}

bool CStream::GetDistribution(double _dTime, EDistrTypes _nDim, const std::string& _sCompound, std::vector<double>& _vResult) const
{
	if(_nDim == DISTR_COMPOUNDS) return false;
	int iSolid = GetSolidPhaseIndex();
	if (iSolid == -1) return false;
	int iComp = GetCompoundIndex(_sCompound);
	if (iComp == -1) return false;
	bool bRes = m_vpPhases[iSolid]->distribution.GetVectorValue(_dTime, DISTR_COMPOUNDS, iComp, _nDim, _vResult);
	Normalize(_vResult);
	return bRes;
}

std::vector<double> CStream::GetDistribution(double _dTime, EDistrTypes _nDim, const std::string& _sCompound) const
{
	if (_nDim == DISTR_COMPOUNDS) return {};
	const int iSolid = GetSolidPhaseIndex();
	if (iSolid == -1) return {};
	const int iComp = GetCompoundIndex(_sCompound);
	if (iComp == -1) return {};
	return Normalize(m_vpPhases[iSolid]->distribution.GetVectorValue(_dTime, DISTR_COMPOUNDS, iComp, _nDim));
}

bool CStream::GetDistribution(double _dTime, EDistrTypes _nDim1, EDistrTypes _nDim2, const std::string& _sCompound, CMatrix2D& _2DResult) const
{
	if((_nDim1 == DISTR_COMPOUNDS) || (_nDim2 == DISTR_COMPOUNDS)) return false;
	int iSolid = GetSolidPhaseIndex();
	if (iSolid == -1) return false;
	int iComp = GetCompoundIndex(_sCompound);
	if (iComp == -1) return false;
	size_t nD1Size = m_pDistributionsGrid->GetClassesByDistr(_nDim1);
	std::vector<unsigned> vDims;
	vDims.push_back(DISTR_COMPOUNDS);
	vDims.push_back(_nDim1);
	vDims.push_back(_nDim2);
	std::vector<unsigned> vCoords;
	vCoords.push_back(iComp);
	vCoords.push_back(0);
	_2DResult.Resize(m_pDistributionsGrid->GetClassesByDistr(_nDim1), m_pDistributionsGrid->GetClassesByDistr(_nDim2));
	bool bRes = true;
	for (size_t i = 0; i < nD1Size; ++i)
	{
		std::vector<double> vRes;
		bool bIterRes = m_vpPhases[iSolid]->distribution.GetVectorValue(_dTime, vDims, vCoords, vRes);
		_2DResult.SetRow(i, vRes);
		bRes &= bIterRes;
		vCoords.back()++;
	}
	_2DResult.Normalize();
	return bRes;
}

bool CStream::GetDistribution(double _dTime, const std::vector<EDistrTypes>& _vDims, const std::string& _sCompound, CDenseMDMatrix& _MDResult) const
{
	if(std::find(_vDims.begin(), _vDims.end(), DISTR_COMPOUNDS) != _vDims.end()) return false;
	int iSolid = GetSolidPhaseIndex();
	if (iSolid == -1) return false;
	int iComp = GetCompoundIndex(_sCompound);
	if (iComp == -1) return false;
	std::vector<unsigned> vSizes(_vDims.size());
	for (unsigned i = 0; i < _vDims.size(); ++i)
		vSizes[i] = (unsigned)m_pDistributionsGrid->GetClassesByDistr(_vDims[i]);
	std::vector<EDistrTypes> vGetDims = _vDims;
	vGetDims.insert(vGetDims.begin(), DISTR_COMPOUNDS);
	std::vector<unsigned> vGetCoords(_vDims.size());
	vGetCoords.front() = iComp;
	std::vector<unsigned> vGetSizes = vSizes;
	vGetSizes.insert(vGetSizes.begin(), (unsigned)m_pDistributionsGrid->GetClassesByDistr(DISTR_COMPOUNDS));
	vGetSizes.pop_back();
	std::vector<EDistrTypes> vSetDims = _vDims;
	std::vector<unsigned> vSetCoords(_vDims.size() - 1);
	std::vector<unsigned> vSetSizes = vSizes;
	vSetSizes.pop_back();
	_MDResult.SetDimensions((std::vector<unsigned>&)_vDims, vSizes);
	bool bRes = true;
	bool bNotEnd;
	do
	{
		std::vector<double> vRes;
		bool bIterRes = m_vpPhases[iSolid]->distribution.GetVectorValue(_dTime, (std::vector<unsigned>&)vGetDims, vGetCoords, vRes);
		bRes &= bIterRes;
		_MDResult.SetVectorValue((std::vector<unsigned>&)vSetDims, vSetCoords, vRes);
		IncrementCoords(vGetCoords, vGetSizes);
		bNotEnd = IncrementCoords(vSetCoords, vSetSizes);
	}
	while (bNotEnd);
	_MDResult.Normalize();
	return bRes;
}

bool CStream::SetDistribution(double _dTime, EDistrTypes _nDim, const std::vector<double>& _vDistr)
{
	int index = GetSolidPhaseIndex();
	if( index == -1 )
		return false;
	return m_vpPhases[index]->distribution.SetDistribution( _dTime, _nDim, _vDistr );
}

bool CStream::SetDistribution(double _dTime, EDistrTypes _nDim1, EDistrTypes _nDim2, const CMatrix2D& _2DDistr)
{
	int index = GetSolidPhaseIndex();
	if( index == -1 )
		return false;
	return m_vpPhases[index]->distribution.SetDistribution( _dTime, _nDim1, _nDim2, _2DDistr );
}

bool CStream::SetDistribution(double _dTime, const CDenseMDMatrix& _MDDistr)
{
	int index = GetSolidPhaseIndex();
	if( index == -1 )
		return false;
	return m_vpPhases[index]->distribution.SetDistribution( _dTime, _MDDistr );
}

bool CStream::SetDistribution(double _dTime, EDistrTypes _nDim, const std::string& _sCompound, const std::vector<double>& _vDistr)
{
	if (_nDim == DISTR_COMPOUNDS) return false;
	int iSolid = GetSolidPhaseIndex();
	if (iSolid == -1) return false;
	int iComp = GetCompoundIndex(_sCompound);
	if (iComp == -1) return false;
	bool bRes = m_vpPhases[iSolid]->distribution.SetVectorValue(_dTime, DISTR_COMPOUNDS, iComp, _nDim, _vDistr, true);
	m_vpPhases[iSolid]->distribution.NormalizeMatrix(_dTime);
	return bRes;
}

bool CStream::SetDistribution(double _dTime, EDistrTypes _nDim1, EDistrTypes _nDim2, const std::string& _sCompound, const CMatrix2D& _2DDistr)
{
	// perform checks
	if ((_nDim1 == DISTR_COMPOUNDS) || (_nDim2 == DISTR_COMPOUNDS)) return false;
	int iSolid = GetSolidPhaseIndex();
	if (iSolid == -1) return false;
	int iComp = GetCompoundIndex(_sCompound);
	if (iComp == -1) return false;

	// get old distribution with compounds
	std::vector<EDistrTypes> vInDims(2);
	vInDims[0] = _nDim1;
	vInDims[1] = _nDim2;
	std::vector<EDistrTypes> vFullDims = vInDims;
	vFullDims.insert(vFullDims.begin(), DISTR_COMPOUNDS);
	CDenseMDMatrix newMDM;
	m_vpPhases[iSolid]->distribution.GetDistribution(_dTime, (std::vector<unsigned>&)vFullDims, newMDM);

	// prepare all parameters
	std::vector<unsigned> vSizes(vInDims.size());
	for (unsigned i = 0; i < vInDims.size(); ++i)
		vSizes[i] = (unsigned)m_pDistributionsGrid->GetClassesByDistr(vInDims[i]);
	// parameters for reading from _2DDistr
	size_t nD1Size = m_pDistributionsGrid->GetClassesByDistr(_nDim1);
	// parameters for writing in newMDM
	std::vector<EDistrTypes> vSetDims = vInDims;
	vSetDims.insert(vSetDims.begin(), DISTR_COMPOUNDS);
	std::vector<unsigned> vSetCoords(vInDims.size());
	vSetCoords.front() = iComp;
	std::vector<unsigned> vSetSizes = vSizes;
	vSetSizes.insert(vSetSizes.begin(), (unsigned)m_pDistributionsGrid->GetClassesByDistr(DISTR_COMPOUNDS));

	// set new values from _2DDistr to newMDM
	bool bRes = true;
	for (size_t i = 0; i < nD1Size; ++i)
	{
		std::vector<double> vRes = _2DDistr.GetRow(i);
		bRes &= newMDM.SetVectorValue((std::vector<unsigned>&)vSetDims, vSetCoords, vRes);
		IncrementCoords(vSetCoords, vSetSizes);
	}

	// set newMDM into main distribution
	newMDM.Normalize();
	bRes &= m_vpPhases[iSolid]->distribution.SetDistribution(_dTime, newMDM);
	return bRes;
}

bool CStream::SetDistribution(double _dTime, const std::string& _sCompound, const CDenseMDMatrix& _MDDistr)
{
	// perform checks
	std::vector<unsigned> vInDims = _MDDistr.GetDimensions();
	if (std::find(vInDims.begin(), vInDims.end(), DISTR_COMPOUNDS) != vInDims.end()) return false;
	int iSolid = GetSolidPhaseIndex();
	if (iSolid == -1) return false;
	int iComp = GetCompoundIndex(_sCompound);
	if (iComp == -1) return false;

	// get old distribution with compounds
	std::vector<unsigned> vFullDims = vInDims;
	vFullDims.insert(vFullDims.begin(), DISTR_COMPOUNDS);
	CDenseMDMatrix newMDM;
	m_vpPhases[iSolid]->distribution.GetDistribution(_dTime, vFullDims, newMDM);

	// prepare all parameters
	std::vector<unsigned> vSizes(vInDims.size());
	for (unsigned i = 0; i < vInDims.size(); ++i)
		vSizes[i] = (unsigned)m_pDistributionsGrid->GetClassesByDistr((EDistrTypes)vInDims[i]);
	// parameters for reading from _MDDistr
	std::vector<unsigned> vGetDims = vInDims;
	std::vector<unsigned> vGetCoords(vInDims.size() - 1);
	std::vector<unsigned> vGetSizes = vSizes;
	// parameters for writing in newMDM
	std::vector<unsigned> vSetDims = vInDims;
	vSetDims.insert(vSetDims.begin(), DISTR_COMPOUNDS);
	std::vector<unsigned> vSetCoords(vInDims.size());
	vSetCoords.front() = iComp;
	std::vector<unsigned> vSetSizes = vSizes;
	vSetSizes.insert(vSetSizes.begin(), (unsigned)m_pDistributionsGrid->GetClassesByDistr(DISTR_COMPOUNDS));

	// set new values from _MDDistr to newMDM
	bool bRes = true;
	bool bNotEnd;
	do
	{
		std::vector<double> vRes;
		bool bGetRes = _MDDistr.GetVectorValue(vGetDims, vGetCoords, vRes);
		bool bSetRes = newMDM.SetVectorValue(vSetDims, vSetCoords, vRes);
		bRes &= (bGetRes & bSetRes);
		IncrementCoords(vSetCoords, vSetSizes);
		bNotEnd = IncrementCoords(vGetCoords, vGetSizes);
	}
	while (bNotEnd);

	// set newMDM into main distribution
	newMDM.Normalize();
	bRes &= m_vpPhases[iSolid]->distribution.SetDistribution(_dTime, newMDM);
	return bRes;
}

bool CStream::ApplyTM(double _dTime, const CTransformMatrix& _TMatrix)
{
	int index = GetSolidPhaseIndex();
	if( index == -1 )
		return false;
	bool bRes = m_vpPhases[index]->distribution.Transform( _dTime, _TMatrix );
	if( bRes )
		m_vpPhases[index]->distribution.NormalizeMatrix( _dTime );
	return bRes;
}

bool CStream::ApplyTM(double _dTime, const std::string& _sCompound, const CTransformMatrix& _TMatrix)
{
	// perform checks
	int iSolid = GetSolidPhaseIndex();
	if (iSolid == -1) return false;
	int iComp = GetCompoundIndex(_sCompound);
	if (iComp == -1) return false;
	if (_TMatrix.GetDimensionsNumber() == 0) return false;
	std::vector<unsigned> vInitDims = _TMatrix.GetDimensions();
	if(std::find(vInitDims.begin(), vInitDims.end(), DISTR_COMPOUNDS) != vInitDims.end()) return false;

	// prepare all parameters
	std::vector<unsigned> vNewDims = vInitDims;
	vNewDims.insert(vNewDims.begin(), DISTR_COMPOUNDS);
	std::vector<unsigned> vOldSizes = _TMatrix.GetClasses();
	std::vector<unsigned> vOldSizesFull = vOldSizes;
	vOldSizesFull.insert(vOldSizesFull.end(), vOldSizes.begin(), vOldSizes.end() - 1);
	std::vector<unsigned> vNewSizes = vOldSizes;
	vNewSizes.insert(vNewSizes.begin(), (unsigned)m_pDistributionsGrid->GetClassesByDistr(DISTR_COMPOUNDS));
	std::vector<unsigned> vOldCoordsFull(vOldSizes.size()*2 - 1);
	std::vector<unsigned> vNewCoords(vNewSizes.size());
	vNewCoords.front() = iComp;

	// create new transformation matrix
	CTransformMatrix newTM(vNewDims, vNewSizes);

	// copy values from old to new TM
	bool bNotEnd, bRes = true;
	size_t nOldSrcSize = vNewDims.size() - 1, nOldDstSize = vNewDims.size() - 2, nNewSrcSize = vNewDims.size(), nNewDstSize = vNewDims.size() - 1;
	std::vector<unsigned> vOldCoordsSrc(nOldSrcSize), vOldCoordsDst(nOldDstSize), vNewCoordsSrc(nNewSrcSize), vNewCoordsDst(nNewDstSize);
	vNewCoordsSrc.front() = iComp;
	vNewCoordsDst.front() = iComp;
	do
	{
		std::copy(vOldCoordsFull.begin(), vOldCoordsFull.begin() + nOldSrcSize, vOldCoordsSrc.begin());
		std::copy(vOldCoordsFull.begin() + nOldSrcSize, vOldCoordsFull.end(), vOldCoordsDst.begin());
		std::copy(vOldCoordsFull.begin(), vOldCoordsFull.begin() + nOldSrcSize, vNewCoordsSrc.begin() + 1);
		std::copy(vOldCoordsFull.begin() + nOldSrcSize, vOldCoordsFull.end(), vNewCoordsDst.begin() + 1);
		std::vector<double> vRes;
		bRes &= _TMatrix.GetVectorValue(vOldCoordsSrc, vOldCoordsDst, vRes);
		bRes &= newTM.SetVectorValue(vNewCoordsSrc, vNewCoordsDst, vRes);
		bNotEnd = IncrementCoords(vOldCoordsFull, vOldSizesFull);
	}
	while (bNotEnd);

	// set values for other compounds
	std::fill(vNewCoords.begin(), vNewCoords.end(), 0);
	bNotEnd = true;
	do
	{
		if(vNewCoords.front() != iComp)
			bRes &= newTM.SetValue(vNewCoords, vNewCoords, 1);
		bNotEnd = IncrementCoords(vNewCoords, vNewSizes);
	}
	while (bNotEnd);

	// transform
	bRes &= m_vpPhases[iSolid]->distribution.Transform(_dTime, newTM);
	if(bRes)
		m_vpPhases[iSolid]->distribution.NormalizeMatrix( _dTime );

	return bRes;
}

void CStream::NormalizeDistribution(double _dTime)
{
	int index = GetSolidPhaseIndex();
	if( index == -1 )
		return;
	return m_vpPhases[index]->distribution.NormalizeMatrix( _dTime );
}

void CStream::NormalizeDistribution(double _dStart, double _dEnd)
{
	int index = GetSolidPhaseIndex();
	if( index == -1 )
		return;
	return m_vpPhases[index]->distribution.NormalizeMatrix( _dStart, _dEnd );
}

void CStream::NormalizeDistribution()
{
	int index = GetSolidPhaseIndex();
	if( index == -1 )
		return;
	return m_vpPhases[index]->distribution.NormalizeMatrix();
}

std::vector<double> CStream::GetPSD(double _dTime, EPSDTypes _PSDType, EPSDGridType _PSDGridType /*= EPSDGridType::DIAMETER*/) const
{
	return GetPSD(_dTime, _PSDType, std::vector<std::string>(), _PSDGridType);
}

std::vector<double> CStream::GetPSD(double _dTime, EPSDTypes _PSDType, const std::string& _sCompound, EPSDGridType _PSDGridType /*= EPSDGridType::DIAMETER*/) const
{
	return GetPSD(_dTime, _PSDType, std::vector<std::string>(1, _sCompound), _PSDGridType);
}

std::vector<double> CStream::GetPSD(double _dTime, EPSDTypes _PSDType, const std::vector<std::string>& _vCompounds, EPSDGridType _PSDGridType /*= EPSDGridType::DIAMETER*/) const
{
	if (!m_pDistributionsGrid) return {};
	if (GetSolidPhaseIndex() == -1) return {}; // no solid phase
	if (!m_pDistributionsGrid->IsDistrTypePresent(DISTR_SIZE)) return {}; // there is no size distribution defined in this stream
	if (!IsCompoundsCorrect(_vCompounds)) return {};
	const CDistributionsGrid& grid = *m_pDistributionsGrid; // alias

	switch(_PSDType)
	{
	case PSD_q3:		return ConvertMassFractionsToq3(grid.GetPSDGrid(_PSDGridType), p_GetPSDMassFrac(_dTime, _vCompounds));
	case PSD_Q3:		return ConvertMassFractionsToQ3(p_GetPSDMassFrac(_dTime, _vCompounds));
	case PSD_q0:		return ConvertNumbersToq0(grid.GetPSDGrid(_PSDGridType), p_GetPSDNumber(_dTime, _vCompounds, _PSDGridType));
	case PSD_Q0:		return ConvertNumbersToQ0(grid.GetPSDGrid(_PSDGridType), p_GetPSDNumber(_dTime, _vCompounds, _PSDGridType));
	case PSD_MassFrac:	return p_GetPSDMassFrac(_dTime, _vCompounds);
	case PSD_Number:	return p_GetPSDNumber(_dTime, _vCompounds, _PSDGridType);
	case PSD_q2:		return ConvertNumbersToq2(grid.GetPSDGrid(_PSDGridType), p_GetPSDNumber(_dTime, _vCompounds, _PSDGridType));
	case PSD_Q2:		return ConvertNumbersToQ2(grid.GetPSDGrid(_PSDGridType), p_GetPSDNumber(_dTime, _vCompounds, _PSDGridType));
	default: return {};
	}
}

std::vector<double> CStream::p_GetPSDMassFrac(double _dTime, const std::vector<std::string>& _vCompounds) const
{
	if (_vCompounds.empty() || _vCompounds.size() == m_vCompoundsKeys.size()) // for all available compounds
		return m_vpPhases[GetSolidPhaseIndex()]->distribution.GetDistribution(_dTime, DISTR_SIZE);
	else // only for specific compounds
	{
		std::vector<double> vDistr(m_pDistributionsGrid->GetClassesByDistr(DISTR_SIZE), 0);
		for (const auto& key : _vCompounds)
		{
			const std::vector<double> vTemp = m_vpPhases[GetSolidPhaseIndex()]->distribution.GetVectorValue(_dTime, DISTR_COMPOUNDS, GetCompoundIndex(key), DISTR_SIZE);
			for (size_t j = 0; j < vTemp.size(); ++j)
				vDistr[j] += vTemp[j];
		}
		Normalize(vDistr);
		return vDistr;
	}
}

std::vector<double> CStream::p_GetPSDNumber(double _dTime, const std::vector<std::string>& _vCompounds, EPSDGridType _PSDGridType) const
{
	const unsigned nCompoundsNum = (_vCompounds.empty() || _vCompounds.size() == m_vCompoundsKeys.size()) ? (unsigned)m_vCompoundsKeys.size() : (unsigned)_vCompounds.size();
	std::vector<std::string> vUsingComps = (_vCompounds.empty() || _vCompounds.size() == m_vCompoundsKeys.size()) ? m_vCompoundsKeys : _vCompounds;
	const bool bPorosityDefined = m_pDistributionsGrid->IsDistrTypePresent(DISTR_PART_POROSITY);
	const std::vector<double> volumes = _PSDGridType == EPSDGridType::VOLUME ? m_pDistributionsGrid->GetPSDMeans(EPSDGridType::VOLUME) : DiameterToVolume(m_pDistributionsGrid->GetPSDMeans(EPSDGridType::DIAMETER));
	const double dMtot = GetPhaseMass_Base(_dTime, SOA_SOLID);

	// single compound with no porosity, only one compound defined
	if (!bPorosityDefined && nCompoundsNum == 1 && m_vCompoundsKeys.size() == 1)
	{
		const std::vector<double> fracs = m_vpPhases[GetSolidPhaseIndex()]->distribution.GetDistribution(_dTime, DISTR_SIZE);
		const double dDensity = GetPhaseTPDProp(_dTime, DENSITY, SOA_SOLID);
		std::vector<double> res(fracs.size());
		for (size_t i = 0; i < fracs.size(); ++i)
			if (dDensity != 0 && volumes[i] != 0)
				res[i] = fracs[i] * dMtot / dDensity / volumes[i];
		return res;
	}

	// single compound with no porosity, several compounds defined
	else if (!bPorosityDefined && nCompoundsNum == 1)
	{
		const std::vector<double> fracs = Normalize(m_vpPhases[GetSolidPhaseIndex()]->distribution.GetVectorValue(_dTime, DISTR_COMPOUNDS, GetCompoundIndex(vUsingComps.front()), DISTR_SIZE));
		const double dDensity = GetPhaseTPDProp(_dTime, DENSITY, SOA_SOLID);
		std::vector<double> res(fracs.size());
		for (size_t i = 0; i < fracs.size(); ++i)
			if (dDensity != 0 && volumes[i] != 0)
				res[i] = fracs[i] * dMtot / dDensity / volumes[i];
		return res;
	}

	// porosity is defined
	else if (bPorosityDefined)
	{
		CDenseMDMatrix distr = m_vpPhases[GetSolidPhaseIndex()]->distribution.GetDistribution(_dTime, DISTR_COMPOUNDS, DISTR_SIZE, DISTR_PART_POROSITY);
		const unsigned nSizeNum = (unsigned)m_pDistributionsGrid->GetClassesByDistr(DISTR_SIZE);

		if (vUsingComps.size() != m_vCompoundsKeys.size())	// filter by compounds
		{
			std::vector<unsigned> vClasses = distr.GetClasses();
			vClasses[0] = (unsigned)_vCompounds.size(); // reduce number of compounds
			CDenseMDMatrix mdTemp;
			mdTemp.SetDimensions(distr.GetDimensions(), vClasses);
			for (unsigned i = 0; i < (unsigned)vUsingComps.size(); ++i)
				for (unsigned j = 0; j < nSizeNum; ++j)
				{
					const std::vector<double> vTmp = distr.GetVectorValue(DISTR_COMPOUNDS, GetCompoundIndex(vUsingComps[i]), DISTR_SIZE, j, DISTR_PART_POROSITY);
					mdTemp.SetVectorValue(DISTR_COMPOUNDS, i, DISTR_SIZE, j, vTmp);
				}
			mdTemp.Normalize();
			distr = mdTemp;
		}

		const unsigned nPorosNum = (unsigned)m_pDistributionsGrid->GetClassesByDistr(DISTR_PART_POROSITY);
		const std::vector<double> porosities = m_pDistributionsGrid->GetClassMeansByDistr(DISTR_PART_POROSITY);

		std::vector<double> res(nSizeNum);
		for (unsigned iComp = 0; iComp < nCompoundsNum; ++iComp)
		{
			std::vector<double> vTemp(nSizeNum);
			const double dDensity = GetCompoundTPDProp(_dTime, vUsingComps[iComp], DENSITY);
			for (unsigned iSize = 0; iSize < nSizeNum; ++iSize)
			{
				for (unsigned iPoros = 0; iPoros < nPorosNum; ++iPoros)
				{
					const double dFrac = distr.GetValue(DISTR_COMPOUNDS, iComp, DISTR_SIZE, iSize, DISTR_PART_POROSITY, iPoros);
					if (dDensity != 0)
						vTemp[iSize] += dFrac * dMtot / dDensity * (1 - porosities[iPoros]);
				}
				if (volumes[iSize] != 0)
					vTemp[iSize] /= volumes[iSize];
			}
			for (size_t iSize = 0; iSize < nSizeNum; ++iSize)
				res[iSize] += vTemp[iSize];
		}
		return res;
	}

	// several compounds without porosity
	else
	{
		CMatrix2D distr = m_vpPhases[GetSolidPhaseIndex()]->distribution.GetDistribution(_dTime, DISTR_COMPOUNDS, DISTR_SIZE);
		if (vUsingComps.size() != m_vCompoundsKeys.size())	// filter by compounds
		{
			CMatrix2D temp2D;
			temp2D.Resize(distr.Rows(), distr.Cols() - 1);
			for (size_t i = 0; i < vUsingComps.size(); ++i)
				temp2D.SetRow(i, distr.GetRow(GetCompoundIndex(vUsingComps[i])));
			temp2D.Normalize();
			distr = temp2D;
		}
		distr *= dMtot;

		const size_t nSizeNum = m_pDistributionsGrid->GetClassesByDistr(DISTR_SIZE);

		std::vector<double> res(nSizeNum);
		for (size_t iComp = 0; iComp < nCompoundsNum; ++iComp)
		{
			const double dDensity = GetCompoundTPDProp(_dTime, vUsingComps[iComp], DENSITY);
			for (size_t iSize = 0; iSize < nSizeNum; ++iSize)
				if (dDensity != 0 && volumes[iSize] != 0)
					res[iSize] += distr[iComp][iSize] / dDensity / volumes[iSize];
		}
		return res;
	}
}

void CStream::SetPSD(double _dTime, EPSDTypes _PSDType, const std::vector<double>& _vPSD, EPSDGridType _PSDGridType /*= EPSDGridType::DIAMETER*/)
{
	SetPSD(_dTime, _PSDType, "", _vPSD, _PSDGridType);
}

void CStream::SetPSD(double _dTime, EPSDTypes _PSDType, const std::string& _sCompound, const std::vector<double>& _vPSD, EPSDGridType _PSDGridType /*= EPSDGridType::DIAMETER*/)
{
	const int index = GetSolidPhaseIndex();
	if (index == -1) return; // no solid phase
	if (!m_pDistributionsGrid) return;
	if (!m_pDistributionsGrid->IsDistrTypePresent(DISTR_SIZE)) return; // there is no size distribution defined in this stream
	const CDistributionsGrid& grid = *m_pDistributionsGrid; // alias

	std::vector<double> vDistr;
	switch (_PSDType)
	{
	case PSD_q0:		vDistr = Convertq3ToMassFractions(grid.GetPSDGrid(_PSDGridType), Convertq0Toq3(grid.GetPSDGrid(), _vPSD));		                                              break;
	case PSD_Q0:		vDistr = ConvertQ0ToMassFractions(grid.GetPSDGrid(_PSDGridType), _vPSD);		                                                                              break;
	case PSD_q3:		vDistr = Convertq3ToMassFractions(grid.GetPSDGrid(_PSDGridType), _vPSD);		                                                                              break;
	case PSD_Q3:		vDistr = ConvertQ3ToMassFractions(_vPSD);										                                                                              break;
	case PSD_MassFrac:	vDistr = _vPSD;																	                                                                              break;
	case PSD_Number:	vDistr = Convertq3ToMassFractions(grid.GetPSDGrid(_PSDGridType), Convertq0Toq3(grid.GetPSDGrid(), ConvertNumbersToq0(grid.GetPSDGrid(_PSDGridType), _vPSD))); break;
	case PSD_q2:		vDistr = Convertq3ToMassFractions(grid.GetPSDGrid(_PSDGridType), Convertq0Toq3(grid.GetPSDGrid(), Convertq2Toq0(grid.GetPSDGrid(_PSDGridType), _vPSD)));	  break;
	case PSD_Q2:		vDistr = ConvertQ2ToMassFractions(grid.GetPSDGrid(_PSDGridType), _vPSD);																				      break;
	default:																																									      break;
	}
	Normalize(vDistr);

	if(_sCompound.empty())	// for the mixture
		m_vpPhases[index]->distribution.SetDistribution(_dTime, DISTR_SIZE, vDistr);
	else if(IsCompoundsCorrect(std::vector<std::string>(1, _sCompound)))	// for specific compound
		m_vpPhases[index]->distribution.SetVectorValue(_dTime, DISTR_COMPOUNDS, GetCompoundIndex(_sCompound), DISTR_SIZE, vDistr, true);
	m_vpPhases[index]->distribution.NormalizeMatrix(_dTime);
}

void CStream::SetMaterialsDatabase(const CMaterialsDatabase* _pDatabase)
{
	m_pMaterialsDB = _pDatabase;
}

void CStream::SetMinimalFraction( double _dFraction )
{
	for( unsigned i=0; i<m_vpPhases.size(); ++i )
		m_vpPhases[i]->distribution.SetMinimalFraction( _dFraction );
}

double CStream::GetMinimalFraction()
{
	if( !m_vpPhases.empty() )
		return m_vpPhases.front()->distribution.GetMinimalFraction();
	else
		return DEFAULT_MIN_FRACTION;
}

void CStream::SaveToFile(CH5Handler& _h5Saver, const std::string& _sPath)
{
	if( !_h5Saver.IsValid() )
		return;

	// current version of save procedure
	_h5Saver.WriteAttribute( _sPath, StrConst::Stream_H5AttrSaveVersion, m_cnSaveVersion );

	/// save stream name
	_h5Saver.WriteData( _sPath, StrConst::Stream_H5StreamName, m_sStreamName );

	/// save stream key
	_h5Saver.WriteData( _sPath, StrConst::Stream_H5StreamKey, m_sStreamKey );

	/// save compounds
	_h5Saver.WriteData( _sPath, StrConst::Stream_H5Compounds, m_vCompoundsKeys );

	/// save time points
	_h5Saver.WriteData( _sPath, StrConst::Stream_H5TimePoints, m_vTimePoints );

	/// save 2D distributions
	_h5Saver.WriteAttribute( _sPath, StrConst::Stream_H5Attr2DDistrNum, (int)m_DistrArrays.size() );
	_h5Saver.CreateGroup( _sPath, StrConst::Stream_H5Group2DDistrs );
	for( unsigned i=0; i<m_DistrArrays.size(); ++i )
	{
		_h5Saver.CreateGroup( _sPath + "/" + StrConst::Stream_H5Group2DDistrs, StrConst::Stream_H5Group2DDistrName + std::to_string(i) );
		std::string sPhasePath = _sPath + "/" + StrConst::Stream_H5Group2DDistrs + "/" + StrConst::Stream_H5Group2DDistrName + std::to_string(i);
		m_DistrArrays[i]->SaveToFile( _h5Saver, sPhasePath );
	}

	/// save phases
	_h5Saver.WriteAttribute( _sPath, StrConst::Stream_H5AttrPhasesNum, (int)m_vpPhases.size() );
	_h5Saver.CreateGroup( _sPath, StrConst::Stream_H5GroupPhases );
	for( unsigned i=0; i<m_vpPhases.size(); ++i )
	{
		_h5Saver.CreateGroup( _sPath + "/" + StrConst::Stream_H5GroupPhases, StrConst::Stream_H5GroupPhaseName + std::to_string(i) );
		std::string sPhasePath = _sPath + "/" + StrConst::Stream_H5GroupPhases + "/" + StrConst::Stream_H5GroupPhaseName + std::to_string(i);
		_h5Saver.WriteData( sPhasePath, StrConst::Stream_H5PhaseName, m_vpPhases[i]->sName );
		_h5Saver.WriteData( sPhasePath, StrConst::Stream_H5PhaseSOA, m_vpPhases[i]->nAggregationState );
		m_vpPhases[i]->distribution.SaveToFile( _h5Saver, sPhasePath );
	}
}

void CStream::LoadFromFile(CH5Handler& _h5Loader, const std::string& _sPath)
{
	m_vTimePoints.clear();
	m_vCompoundsKeys.clear();
	m_vTLookupTables.clear();
	m_vPLookupTables.clear();
	m_TLookup1.Clear();
	m_TLookup2.Clear();
	for( unsigned i=0; i<m_vpPhases.size(); ++i )
		delete m_vpPhases[i];
	m_vpPhases.clear();

	if( !_h5Loader.IsValid() )
		return;

	/// load stream name
	_h5Loader.ReadData( _sPath, StrConst::Stream_H5StreamName, m_sStreamName );

	/// load stream key
	_h5Loader.ReadData( _sPath, StrConst::Stream_H5StreamKey, m_sStreamKey );

	/// load compounds
	_h5Loader.ReadData( _sPath, StrConst::Stream_H5Compounds, m_vCompoundsKeys );

	/// load time points
	_h5Loader.ReadData( _sPath, StrConst::Stream_H5TimePoints, m_vTimePoints );

	/// load phases
	int nPhasesNum = _h5Loader.ReadAttribute( _sPath, StrConst::Stream_H5AttrPhasesNum );
	if( nPhasesNum != -1 )
	{
		for( unsigned i=0; i<(unsigned)nPhasesNum ; ++i )
		{
			std::string sPhasePath = _sPath + "/" + StrConst::Stream_H5GroupPhases + "/" + StrConst::Stream_H5GroupPhaseName + std::to_string(i);
			std::string sBufName;
			_h5Loader.ReadData( sPhasePath, StrConst::Stream_H5PhaseName, sBufName );
			unsigned nBufSOA;
			_h5Loader.ReadData( sPhasePath, StrConst::Stream_H5PhaseSOA, nBufSOA );
			AddPhase( sBufName, nBufSOA );
			m_vpPhases[i]->distribution.LoadFromFile( _h5Loader, sPhasePath );
		}
	}

	/// load 2D distributions
	for( unsigned i=0; i<m_DistrArrays.size(); ++i )
	{
		std::string sPhasePath = _sPath + "/" + StrConst::Stream_H5Group2DDistrs + "/" + StrConst::Stream_H5Group2DDistrName + std::to_string(i);
		m_DistrArrays[i]->LoadFromFile( _h5Loader, sPhasePath );
	}

	for( unsigned i=0; i<m_vpPhases.size(); ++i )
		m_PhaseFractions.SetDimensionLabel( i, m_vpPhases[i]->sName );
}

void CStream::ReduceTimePoints(double _dStart, double _dEnd, double _dStep)
{
	if (_dStep <= 0) return;

	std::vector<double> vTP = GetTimePointsForInterval(_dStart, _dEnd);
	if (vTP.size() <= 3) return;
	vTP.pop_back();

	size_t iT1 = 0;
	size_t iT2 = 1;
	while ((iT1 < vTP.size()) && (iT2 < vTP.size()))
	{
		if (std::fabs(vTP[iT1] - vTP[iT2]) < _dStep)
		{
			RemoveTimePoint(vTP[iT2]);
			vTP.erase(vTP.begin() + iT2);
		}
		else
		{
			iT1++;
			iT2++;
		}
	}
}

void CStream::SetCacheParams( bool _bEnabled, unsigned _nWindow )
{
	m_bCacheEnabled = _bEnabled;
	m_nCacheWindow = _nWindow;
	for( unsigned i=0; i<m_DistrArrays.size(); ++i )
		m_DistrArrays[i]->SetCacheParams( _bEnabled,_nWindow );
	for( unsigned i=0; i<m_vpPhases.size(); ++i )
		m_vpPhases[i]->distribution.SetCacheParams( _bEnabled, _nWindow );
}

void CStream::CopyFromStream_Base(const CStream& _srcStream, double _dTime, bool _bDeleteDataAfter /*= true */)
{
	// copy data
	for (size_t i = 0; i < m_DistrArrays.size(); ++i)
		m_DistrArrays[i]->CopyFrom(_srcStream.m_DistrArrays[i], _dTime);
	for (size_t i = 0; i < m_vpPhases.size(); ++i)
		m_vpPhases[i]->distribution.CopyFrom(_srcStream.m_vpPhases[i]->distribution, _dTime);

	if (_bDeleteDataAfter) // remove next time points
	{
		RemoveTimePointsAfter(_dTime);
		if (m_vTimePoints.empty() || m_vTimePoints.back() != _dTime) // add time point to the end of the current array
			m_vTimePoints.push_back(_dTime);
	}
	else
	{
		const size_t index = GetTimeIndex(_dTime, false); // get new index to insert
		if (index < m_vTimePoints.size() && m_vTimePoints[index] == _dTime) // time point already exists
			return;
		// insert time point into current array
		m_vTimePoints.insert(m_vTimePoints.begin() + index, _dTime);
	}
}

void CStream::CopyFromStream_Base(const CStream& _srcStream, double _dStart, double _dEnd, bool _bDeleteDataAfter /*= true */)
{
	std::vector<double> vTimePoints = _srcStream.GetTimePointsForInterval(_dStart, _dEnd);
	if (vTimePoints.size() == 0) // nothing to copy
		return;

	if (_bDeleteDataAfter)
	{
		RemoveTimePointsAfter(_dStart, true);
		// add time points to the end of the current array
		m_vTimePoints.insert(m_vTimePoints.end(), vTimePoints.begin(), vTimePoints.end());
	}
	else
	{
		RemoveTimePoints(_dStart, _dEnd);
		// insert time point into current array
		const size_t iInsert = GetTimeIndex(_dStart, false);
		m_vTimePoints.insert(m_vTimePoints.begin() + iInsert, vTimePoints.begin(), vTimePoints.end());
	}

	for (size_t i = 0; i < m_DistrArrays.size(); ++i)
		m_DistrArrays[i]->CopyFrom(_srcStream.m_DistrArrays[i], _dStart, _dEnd);
	for (size_t i = 0; i < m_vpPhases.size(); ++i)
		m_vpPhases[i]->distribution.CopyFrom(_srcStream.m_vpPhases[i]->distribution, _dStart, _dEnd);
}

void CStream::CopyFromStream_Base(double _dTimeDst, const CStream& _srcStream, double _dTimeSrc, bool _bDeleteDataAfter /*= true */)
{
	// copy data
	for (size_t i = 0; i < m_DistrArrays.size(); ++i)
		m_DistrArrays[i]->CopyFromTimePoint(_srcStream.m_DistrArrays[i], _dTimeSrc, _dTimeDst);
	for (size_t i = 0; i < m_vpPhases.size(); ++i)
		m_vpPhases[i]->distribution.CopyFromTimePoint(_srcStream.m_vpPhases[i]->distribution, _dTimeSrc, _dTimeDst);

	if( _bDeleteDataAfter ) // remove next time points
	{
		RemoveTimePointsAfter( _dTimeDst );
		if (m_vTimePoints.empty() || m_vTimePoints.back() != _dTimeDst)  // add time points to the end of the current array
			m_vTimePoints.push_back(_dTimeDst);
	}
	else
	{
		const size_t index = GetTimeIndex(_dTimeDst, false); // get new index to insert
		if (index < m_vTimePoints.size() && m_vTimePoints[index] == _dTimeDst) // time point already exists
			return;
		// insert time point into current array
		m_vTimePoints.insert(m_vTimePoints.begin() + index, _dTimeDst);
	}
}

void CStream::AddStream_Base(const CStream& _Stream, double _dTime)
{
	AddStream_Base(_Stream, _dTime, _dTime);
}

void CStream::AddStream_Base(const CStream& _Stream, double _dStart, double _dEnd, unsigned _nTPTypes /*= BOTH_TP */)
{
	if (!CompareStreamStructure(_Stream))
		return;

	// get all time points
	std::vector<double> vTimePoints;
	switch (static_cast<ETimePointsComposition>(_nTPTypes))
	{
	case DST_TP:
		vTimePoints = GetTimePointsForInterval(_dStart, _dEnd);
		break;
	case SRC_TP:
		vTimePoints = _Stream.GetTimePointsForInterval(_dStart, _dEnd);
		break;
	case BOTH_TP:
		VectorsUnionSorted(GetTimePointsForInterval(_dStart, _dEnd), _Stream.GetTimePointsForInterval(_dStart, _dEnd), vTimePoints);
		break;
	}

	std::vector<std::vector<double>> vResMTP(vTimePoints.size(), std::vector<double>(3));									// MTP
	std::vector<std::vector<double>> vResPhaseFrac(vTimePoints.size(), std::vector<double>(m_vpPhases.size()));				// phase fractions
	std::vector<std::vector<CDenseMDMatrix>> vResDistr(vTimePoints.size(), std::vector<CDenseMDMatrix>(m_vpPhases.size()));	// MD distributions
	for (size_t i = 0; i < vTimePoints.size(); ++i)
	{
		// get masses
		const double dMassDst = m_StreamMTP.GetValue(vTimePoints[i], MTP_MASS);
		const double dMassSrc = _Stream.m_StreamMTP.GetValue(vTimePoints[i], MTP_MASS);
		const double dMassTot = dMassDst + dMassSrc;
		if (dMassTot == 0) //  nothing to mix
			continue;

		// get mass
		vResMTP[i][MTP_MASS] = dMassTot;
		// get pressure
		vResMTP[i][MTP_PRESSURE] = CalcMixPressure(_Stream, vTimePoints[i], *this, vTimePoints[i]);
		// get temperature
		vResMTP[i][MTP_TEMPERATURE] = CalcMixTemperature(_Stream, vTimePoints[i], dMassSrc, *this, vTimePoints[i], dMassDst);

		// get phase fractions
		std::vector<double> vTempPhaseMassSrc, vTempPhaseMassDst;
		std::tie(vTempPhaseMassSrc, vTempPhaseMassDst, vResPhaseFrac[i]) = CalcMixPhaseFractions(_Stream, vTimePoints[i], dMassSrc, *this, vTimePoints[i], dMassDst);

		// get MD distributions
		vResDistr[i] = CalcMixMDDistributions(_Stream, vTimePoints[i], dMassSrc, vTempPhaseMassSrc, *this, vTimePoints[i], dMassDst, vTempPhaseMassDst);
	}

	// remove time points
	if (_nTPTypes == SRC_TP)
		RemoveTimePoints(_dStart, _dEnd);

	// set new data
	for (size_t i = 0; i < vTimePoints.size(); ++i)
	{
		// set MTP
		m_StreamMTP.SetValue(vTimePoints[i], vResMTP[i]);
		// set phase fractions
		m_PhaseFractions.SetValue(vTimePoints[i], vResPhaseFrac[i]);
		// set MD distributions
		for (size_t j = 0; j < m_vpPhases.size(); ++j)
		{
			m_vpPhases[j]->distribution.AddTimePoint(vTimePoints[i]);
			m_vpPhases[j]->distribution.SetDistribution(vTimePoints[i], vResDistr[i][j]);
		}
	}

	// normalize MD distributions
	for (size_t j = 0; j < m_vpPhases.size(); ++j)
		m_vpPhases[j]->distribution.NormalizeMatrix(_dStart, _dEnd);

	// set new time points
	if ((_nTPTypes == SRC_TP) || (_nTPTypes == BOTH_TP))
		m_vTimePoints = VectorsUnionSorted(m_vTimePoints, vTimePoints);
}

double CStream::CalcMixPressure(const CStream& _str1, double _time1, const CStream& _str2, double _time2) const
{
	const double pressure1 = _str1.m_StreamMTP.GetValue(_time1, MTP_PRESSURE);
	const double pressure2 = _str2.m_StreamMTP.GetValue(_time2, MTP_PRESSURE);
	if (pressure1 != 0 && pressure2 != 0)
		return std::min(pressure2, pressure1);	// return minimum pressure
	else
		return std::max(pressure2, pressure1);	// return nonzero pressure, if any
}

double CStream::CalcMixTemperature(const CStream& _str1, double _time1, double _mass1, const CStream& _str2, double _time2, double _mass2)
{
	// initialize lookups if not yet initialized
	if (!m_TLookup1.IsValid())	m_TLookup1.Initialize(m_pMaterialsDB, m_vCompoundsKeys, ENTHALPY, EDependencyTypes::DEPENDENCE_TEMP);
	if (!m_TLookup2.IsValid())	m_TLookup2.Initialize(m_pMaterialsDB, m_vCompoundsKeys, ENTHALPY, EDependencyTypes::DEPENDENCE_TEMP);
	// set new fractions of compounds
	m_TLookup1.SetCompoundFractions(_str1.GetCompoundsFractions(_time1));
	m_TLookup2.SetCompoundFractions(_str2.GetCompoundsFractions(_time2));
	// get enthalpy for the 1st stream
	const double enthalpy1 = m_TLookup1.GetValue(_str1.GetTemperature(_time1));
	// get enthalpy for the 2nd stream
	const double enthalpy2 = m_TLookup2.GetValue(_str2.GetTemperature(_time2));
	// calculate total mass
	const double massMix = _mass1 + _mass2;
	if (massMix == 0)							// no mass at all
		return _str1.GetTemperature(_time1);	// return some arbitrary temperature
	// calculate (specific) total enthalpy
	const double enthalpyMix = (enthalpy1 * _mass1 + enthalpy2 * _mass2) / massMix;
	// combine both enthalpy tables for mixture enthalpy table
	m_TLookup2.MultiplyTable(_mass2 / massMix);
	m_TLookup2.Add(m_TLookup1, _mass1 / massMix);
	// read out new temperature
	return m_TLookup2.GetParam(enthalpyMix);
}

std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> CStream::CalcMixPhaseFractions(const CStream& _str1, double _time1, double _mass1, const CStream& _str2, double _time2, double _mass2) const
{
	std::vector<double> phaseFractions1 = _str1.m_PhaseFractions.GetValue(_time1);
	std::vector<double> phaseFractions2 = _str2.m_PhaseFractions.GetValue(_time2);
	std::vector<double> phaseFractionsMix(phaseFractions2.size());
	for (size_t i = 0; i < phaseFractions2.size(); ++i)
		if (_mass1 + _mass2 != 0)
			phaseFractionsMix[i] = (_mass2*phaseFractions2[i] + _mass1 * phaseFractions1[i]) / (_mass1 + _mass2);
		else
			phaseFractionsMix[i] = 0;
	return std::make_tuple(phaseFractions1, phaseFractions2, phaseFractionsMix);
}

std::vector<CDenseMDMatrix> CStream::CalcMixMDDistributions(const CStream& _str1, double _time1, double _mass1, const std::vector<double>& _phaseFracs1, const CStream& _str2, double _time2, double _mass2, const std::vector<double>& _phaseFracs2) const
{
	std::vector<CDenseMDMatrix> vDistrsMix(_str2.m_vpPhases.size());
	for (size_t i = 0; i < _str2.m_vpPhases.size(); ++i)
	{
		CDenseMDMatrix distr1 = _str1.m_vpPhases[i]->distribution.GetDistribution(_time1);
		CDenseMDMatrix distr2 = _str2.m_vpPhases[i]->distribution.GetDistribution(_time2);
		vDistrsMix[i] = distr2*_mass2*_phaseFracs2[i] + distr1*_mass1*_phaseFracs1[i];

		// TODO: more effective solution
		// hack to save compounds distributions. if all compounds are set to 0, recalculate vResDistr
		std::vector<double> vCompFracs = vDistrsMix[i].GetVectorValue(DISTR_COMPOUNDS);
		if (std::all_of(vCompFracs.begin(), vCompFracs.end(), [](double d) { return d == 0; })) // all are equal to 0
			vDistrsMix[i] = distr2*_mass2 + distr1*_mass1; // TODO: all secondary dimensions will be also recalculated

		vDistrsMix[i].Normalize();
	}
	return vDistrsMix;
}

size_t CStream::GetTimeIndex(double _dTime, bool _bIsStrict /*= true */)
{
	//for( unsigned i=0; i<m_vTimePoints.size(); i++ )
	//	if( m_vTimePoints[i] == _dTime )
	//		return i;
	//	else if( m_vTimePoints[i] > _dTime )
	//		return -1;
	//return -1;

	// check if empty
	if( m_vTimePoints.empty() )
	{
		if( _bIsStrict )
			return -1;
		else
			return 0;
	}

	// check the boundaries
	if( m_vTimePoints.front() > _dTime )
	{
		if( _bIsStrict )
			return -1;
		else
			return 0;
	}
	if( m_vTimePoints.back() < _dTime )
	{
		if( _bIsStrict )
			return -1;
		else
			return m_vTimePoints.size();
	}

	// binary search
	size_t nFirst = 0;
	size_t nLast = m_vTimePoints.size();
	size_t nMid = (size_t)(nFirst + nLast) >> 1;
	while ( nFirst < nLast )
	{
		if ( _dTime <= m_vTimePoints.at(nMid) )
			nLast = nMid;
		else
			nFirst = nMid + 1;
		nMid = (size_t)(nFirst + nLast) >> 1;
	}
	if( ( m_vTimePoints[nLast] == _dTime ) || ( !_bIsStrict ) )
		return nLast;
	else
		return -1;
}

bool CStream::CompareStreamStructure(const CStream& _stream) const
{
	if (m_StreamMTP.GetDimensionsNumber() != _stream.m_StreamMTP.GetDimensionsNumber())
		return false;

	if (m_PhaseFractions.GetDimensionsNumber() != _stream.m_PhaseFractions.GetDimensionsNumber())
		return false;

	if (m_vpPhases.size() != _stream.m_vpPhases.size())
		return false;

	for (size_t i = 0; i < m_vpPhases.size(); ++i)
		if ((m_vpPhases[i]->sName != _stream.m_vpPhases[i]->sName) || (m_vpPhases[i]->nAggregationState != _stream.m_vpPhases[i]->nAggregationState))
			return false;

	const int index = GetSolidPhaseIndex();
	if (index != -1)
		if (!m_vpPhases[index]->distribution.CompareDims(_stream.m_vpPhases[index]->distribution))
			return false;

	return true;
}

int CStream::GetSolidPhaseIndex() const
{
	for( unsigned i=0; i<m_vpPhases.size(); ++i )
		if( m_vpPhases[i]->nAggregationState == SOA_SOLID )
			return i;
	return -1;
}

//int CStream::GetLiquidPhaseIndex() const
//{
//	for( unsigned i=0; i<m_vPhases.size(); ++i )
//		if( m_vPhases[i]->nAggregationState == SOA_LIQUID )
//			return i;
//	return -1;
//}
//
//int CStream::GetVaporPhaseIndex() const
//{
//	for( unsigned i=0; i<m_vPhases.size(); ++i )
//		if( m_vPhases[i]->nAggregationState == SOA_VAPOR )
//			return i;
//	return -1;
//}

bool CStream::IsPhaseDefined( unsigned _nPhaseType ) const
{
	bool bRes = false;
	for( unsigned i=0; i<m_vpPhases.size(); ++i )
		if( m_vpPhases[i]->nAggregationState == _nPhaseType )
		{
			bRes = true;
			break;
		}
		return bRes;
}

unsigned CStream::GetLiquidPhasesNumber() const
{
	unsigned nNum = 0;
	for( unsigned i=0; i<m_vpPhases.size(); ++i )
		if( m_vpPhases[i]->nAggregationState == SOA_LIQUID )
			nNum++;
	return nNum;
}

void CStream::ExtrapolateToPoint( double _dT, double _dTExtra )
{
	if( _dT >= _dTExtra ) return;

	RemoveTimePointsAfter( _dT, false );
	AddTimePoint( _dTExtra, _dT );
}

void CStream::ExtrapolateToPoint( double _dT1, double _dT2, double _dTExtra )
{
	if( _dT1 >= _dT2 ) return;
	if( _dT2 >= _dTExtra ) return;

	RemoveTimePointsAfter( _dT2, false );

	AddTimePoint( _dTExtra );

	if( m_vTimePoints.size() < 3 )
		return;

	for( unsigned i=0; i<m_DistrArrays.size(); i++ )
		m_DistrArrays[i]->ExtrapolateToPoint( _dT1, _dT2, _dTExtra );
	for( unsigned i=0; i<m_vpPhases.size(); i++ )
		m_vpPhases[i]->distribution.ExtrapolateToPoint( _dT1, _dT2, _dTExtra );
}

void CStream::ExtrapolateToPoint( double _dT0, double _dT1, double _dT2, double _dTExtra )
{
	if( _dT0 >= _dT1 ) return;
	if( _dT1 >= _dT2 ) return;
	if( _dT2 >= _dTExtra ) return;

	RemoveTimePointsAfter( _dT2, false );

	AddTimePoint( _dTExtra );

	if( m_vTimePoints.size() < 3 )
		return;

	for( unsigned i=0; i<m_DistrArrays.size(); i++ )
		m_DistrArrays[i]->ExtrapolateToPoint( _dT0, _dT1, _dT2, _dTExtra );
	for( unsigned i=0; i<m_vpPhases.size(); i++ )
		m_vpPhases[i]->distribution.ExtrapolateToPoint( _dT0, _dT1, _dT2, _dTExtra );
}

//void CStream::PSDConvert( double _dTime, unsigned _nPSDTypeIn, const std::vector<double> &_vPSDin, unsigned _nPSDTypeOut, std::vector<double> &_vPSDout )
//{
//	Convert_q3_to_q0( _dTime, _vPSDin, _vPSDout );
//}
//
//void CStream::Convert_q3_to_q0( double _dTime, const std::vector<double> &_q3, std::vector<double> &_q0 ) const
//{
//	int index = GetSolidPhaseIndex();
//	unsigned nCompoundsNum = m_vCompoundsKeys.size();
//	bool bPorosityDefined = m_pDistributionsGrid->IsTypePresent( DISTR_PART_POROSITY );
//
//	if( ( !bPorosityDefined ) && ( nCompoundsNum == 1 ) )
//	{
//		std::vector<double> vSizesGrid = m_pDistributionsGrid->GetContiniousGridByType( DISTR_SIZE );
//		ConvertFromq3Toq0( vSizesGrid, _q3, _q0 );
//	}
//	else if( bPorosityDefined )
//	{
//		unsigned nSizeNum = m_pDistributionsGrid->GetClassesNumberByType( DISTR_SIZE );
//		unsigned nPorosNum = m_pDistributionsGrid->GetClassesNumberByType( DISTR_PART_POROSITY );
//		std::vector<double> vDiameters = m_pDistributionsGrid->GetClassMediansByType( DISTR_SIZE );
//		std::vector<double> vClassSizes = m_pDistributionsGrid->GetClassSizesByType( DISTR_SIZE );
//		std::vector<double> vPorosities = m_pDistributionsGrid->GetClassMediansByType( DISTR_PART_POROSITY );
//		std::vector<double> vNumDistr( nSizeNum );
//		for( unsigned iComp=0; iComp<nCompoundsNum; ++iComp )
//		{
//			std::vector<double> vTemp( nSizeNum );
//			double dCompFrac = GetCompoundFraction( iComp, index, _dTime );
//			double dDensity = GetTPDependentProperty( _dTime, m_vCompoundsKeys[iComp], DENSITY );
//			for( unsigned iSize=0; iSize<nSizeNum; ++iSize )
//			{
//				for( unsigned iPoros=0; iPoros<nPorosNum; ++iPoros )
//				{
//					vTemp[iSize] += dCompFrac * _q3[iSize] * vClassSizes[iSize] / dDensity * ( 1 - vPorosities[iPoros] );
//				}
//				vTemp[iSize] /= std::pow( vDiameters[iSize], 3 );
//			}
//			for( unsigned iSize=0; iSize<nSizeNum; ++iSize )
//				vNumDistr[iSize] += vTemp[iSize];
//		}
//		double dNtot = 0;
//		for( unsigned iSize=0; iSize<nSizeNum; ++iSize )
//			dNtot += vNumDistr[iSize];
//		_q0.resize( nSizeNum );
//		for( unsigned iSize=0; iSize<nSizeNum; ++iSize )
//			_q0[iSize] = vNumDistr[iSize] / ( dNtot * vClassSizes[iSize] );
//	}
//	else
//	{
//		unsigned nSizeNum = m_pDistributionsGrid->GetClassesNumberByType( DISTR_SIZE );
//		std::vector<double> vDiameters = m_pDistributionsGrid->GetClassMediansByType( DISTR_SIZE );
//		std::vector<double> vClassSizes = m_pDistributionsGrid->GetClassSizesByType( DISTR_SIZE );
//		std::vector<double> vNumDistr( nSizeNum );
//		for( unsigned iComp=0; iComp<nCompoundsNum; ++iComp )
//		{
//			double dCompFrac = GetCompoundFraction( iComp, index, _dTime );
//			double dDensity = GetTPDependentProperty( _dTime, m_vCompoundsKeys[iComp], DENSITY );
//			for( unsigned iSize=0; iSize<nSizeNum; ++iSize )
//			{
//				vNumDistr[iSize] += dCompFrac * _q3[iSize] * vClassSizes[iSize] / dDensity / std::pow( vDiameters[iSize], 3 );
//			}
//		}
//		double dNtot = 0;
//		for( unsigned iSize=0; iSize<nSizeNum; ++iSize )
//			dNtot += vNumDistr[iSize];
//		_q0.resize( nSizeNum );
//		for( unsigned iSize=0; iSize<nSizeNum; ++iSize )
//			_q0[iSize] = vNumDistr[iSize] / ( dNtot * vClassSizes[iSize] );
//	}
//}

void CStream::SetCachePath(const std::wstring& _sPath)
{
	m_sCachePath = _sPath;

	for( unsigned i=0; i<m_DistrArrays.size(); ++i )
		m_DistrArrays[i]->SetCachePath( m_sCachePath );

	for( unsigned i=0; i<m_vpPhases.size(); ++i )
		m_vpPhases[i]->distribution.SetCachePath( m_sCachePath );
}

//double CStream::CalcEnthalpyPressureCorrection(double _dTime, unsigned _nBasis /*= BASIS_MASS */) const
/*{
	// Mass fraction of phase SOA_LIQUID
	double dFraction = GetSinglePhaseProp(_dTime, FRACTION, SOA_LIQUID, _nBasis);
	// Enthalpy pressure correction of phase SOA_LIQUID
	double dRes = dFraction * CalcEnthalpyPressureCorrection(_dTime, SOA_LIQUID, _nBasis);

	// Mass fraction of phase SOA_LIQUID2
	dFraction = GetSinglePhaseProp(_dTime, FRACTION, SOA_LIQUID2, _nBasis);
	// Enthalpy pressure correction of phase SOA_LIQUID2
	dRes+= dFraction * CalcEnthalpyPressureCorrection(_dTime, SOA_LIQUID2, _nBasis);

	return dRes;
}*/

//double CStream::CalcEnthalpyPressureCorrection(double _dTime, unsigned _nPhase, unsigned _nBasis /*= BASIS_MASS */) const
/*{
	// Check for pressure difference
	double dDeltaPressure = m_StreamMTP.GetValue(_dTime, MTP_PRESSURE) - STANDARD_CONDITION_P;
	if (dDeltaPressure == 0)
		return 0;

	unsigned nPhaseIndex = GetPhaseIndex(_nPhase);
	if (nPhaseIndex == -1) // no such phase
		return 0;

	if (m_vpPhases[nPhaseIndex]->nAggregationState != SOA_LIQUID && m_vpPhases[nPhaseIndex]->nAggregationState != SOA_LIQUID2)
		return 0;

	// Get density of the phase
	double dDensity = GetPhaseTPDProp(_dTime, DENSITY, _nPhase);
	if (dDensity == 0)
		return 0;

	// Return pressure correction
	return (dDeltaPressure / dDensity);
}*/


/*
double CStream::CalcTemperatureFromEnthalpyLookup(double _dTime, double _dSpecEnthalpy, double _dPressure) const
{
	double dPhaseT;
	double dTemp = 0;

	std::vector<sDependentValue> vMixtureList;
	std::vector<unsigned> vTempIndex;

	if (m_pMaterialsDB == NULL)
		return 0;

	// create mixture enthalpy lookup-table
	for (unsigned i = 0; i < m_vCompoundsKeys.size(); ++i)
	{
		// get enthalpy lookup table from material database
		std::vector<sDependentValue>* pValuesTemp = m_pMaterialsDB->GetPropertyList(m_vCompoundsKeys[i], ENTHALPY);

		double temp = GetCompoundFraction(_dTime, i);

		if (pValuesTemp != 0)
		{
			CombineValues(vMixtureList, pValuesTemp, GetCompoundFraction(_dTime, i));
		}
		// if enthalpy table is not given
		else
		{
			continue;
		}
	}

	FlipValues(vMixtureList);
	CCompoundProperty cCompoundPropertyTemp(0);
	cCompoundPropertyTemp.SetType(true);
	cCompoundPropertyTemp.SetValue(vMixtureList);
	dPhaseT = cCompoundPropertyTemp.GetValue(_dSpecEnthalpy);

	// Pressure correction of enthalpy tables for liquid phases
	bool bPresCorrection = false;
	if (_dPressure != STANDARD_CONDITION_P)
	{
		FlipValues(vMixtureList);
		for (unsigned i = 0; i < m_vCompoundsKeys.size(); ++i)
		{
			double dTempWeight = GetCompoundPhaseFraction(_dTime, m_vCompoundsKeys[i], SOA_LIQUID) * GetSinglePhaseProp(_dTime, FRACTION, SOA_LIQUID);
			dTempWeight += GetCompoundPhaseFraction(_dTime, m_vCompoundsKeys[i], SOA_LIQUID2) * GetSinglePhaseProp(_dTime, FRACTION, SOA_LIQUID2);

			if (dTempWeight != 0)
			{
				// TO DO: Density at new Temperature -> Iteration
				double dDensity = GetCompoundTPDProp(m_vCompoundsKeys[i], DENSITY, dPhaseT, _dPressure);
				double dIsothermalPart = 1. / dDensity * (_dPressure - STANDARD_CONDITION_P);
				if (dIsothermalPart != 0)
				{
					bPresCorrection = true;
					CombineValues(vMixtureList, dIsothermalPart, dTempWeight);
				}
			}
		}
		if (bPresCorrection)
		{
			FlipValues(vMixtureList);
			cCompoundPropertyTemp.SetValue(vMixtureList);
			dPhaseT = cCompoundPropertyTemp.GetValue(_dSpecEnthalpy);
		}
	}

	return dPhaseT;
}
*/


bool CStream::IsDefined(ECompoundTPProperties _nProperty, EDependencyTypes _nDependenceType) const
{
	if(_nDependenceType == EDependencyTypes::DEPENDENCE_TEMP)
		return m_vTLookupTables.find(_nProperty) != m_vTLookupTables.end();
	if (_nDependenceType == EDependencyTypes::DEPENDENCE_PRES)
		return m_vPLookupTables.find(_nProperty) != m_vPLookupTables.end();
	return false;
}

void CStream::AddPropertyTable(ECompoundTPProperties _nProperty, EDependencyTypes _nDependenceType) const
{
	if (_nDependenceType == EDependencyTypes::DEPENDENCE_TEMP)
		m_vTLookupTables.insert_or_assign(_nProperty, CLookupTable(m_pMaterialsDB, m_vCompoundsKeys, _nProperty, _nDependenceType));
	else if (_nDependenceType == EDependencyTypes::DEPENDENCE_PRES)
		m_vPLookupTables.insert_or_assign(_nProperty, CLookupTable(m_pMaterialsDB, m_vCompoundsKeys, _nProperty, _nDependenceType));
}

CLookupTable* CStream::GetLookupTable(ECompoundTPProperties _nProperty, EDependencyTypes _nDependenceType, double _dTime) const
{
	if (!IsDefined(_nProperty, _nDependenceType))
		AddPropertyTable(_nProperty, _nDependenceType);

	CLookupTable* pTable = nullptr;
	if (_nDependenceType == EDependencyTypes::DEPENDENCE_TEMP)
		pTable = &m_vTLookupTables.find(_nProperty)->second;
	else if (_nDependenceType == EDependencyTypes::DEPENDENCE_PRES)
		pTable = &m_vPLookupTables.find(_nProperty)->second;

	pTable->SetCompoundFractions(GetCompoundsFractions(_dTime));
	return pTable;
}

double CStream::CalcTemperatureFromProperty(ECompoundTPProperties _nProperty, double _dTime, double _dValue) const
{
	return GetLookupTable(_nProperty, EDependencyTypes::DEPENDENCE_TEMP, _dTime)->GetParam(_dValue);
}

double CStream::CalcPressureFromProperty(ECompoundTPProperties _nProperty, double _dTime, double _dValue) const
{
	return GetLookupTable(_nProperty, EDependencyTypes::DEPENDENCE_PRES, _dTime)->GetParam(_dValue);
}

double CStream::CalcPropertyFromTemperature(ECompoundTPProperties _nProperty, double _dTime, double _dT) const
{
	return GetLookupTable(_nProperty, EDependencyTypes::DEPENDENCE_TEMP, _dTime)->GetValue(_dT);
}

double CStream::CalcPropertyFromPressure(ECompoundTPProperties _nProperty, double _dTime, double _dP) const
{
	return GetLookupTable(_nProperty, EDependencyTypes::DEPENDENCE_PRES, _dTime)->GetValue(_dP);
}

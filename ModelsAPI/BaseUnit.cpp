/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "BaseUnit.h"
#include "Stream.h"
#include "DyssolUtilities.h"
#include "DyssolStringConstants.h"
#include "ContainerFunctions.h"
#include "Phase.h"
#include "DistributionsGrid.h"

const unsigned CBaseUnit::m_cnSaveVersion	= 2;

CBaseUnit::CBaseUnit(void):
	m_nCompilerVer(COMPILER_VERSION),
	m_sUnitName(StrConst::BUnit_UnspecValue),
	m_sUniqueID(""),
	m_sAuthorName(StrConst::BUnit_UnspecValue),
	m_dUnitVersion(1),
	m_bIsDynamic(true),
	m_bError(false),
	m_bWarning(false),
	m_bInfo(false),
	m_sErrorDescription(StrConst::BUnit_NoErrors),
	m_sWarningDescription(StrConst::BUnit_NoWarnings),
	m_sInfoDescription(StrConst::BUnit_NoInfos),
	m_pvCompoundsKeys(NULL),
	m_pvPhasesSOA(NULL),
	m_pvPhasesNames(NULL),
	m_pDistributionsGrid(NULL),
	m_pMaterialsDB(NULL),
	m_dRTol(DEFAULT_R_TOL),
	m_dATol(DEFAULT_A_TOL),
	m_sCachePath(L""),
	m_bCacheEnabled(DEFAULT_CACHE_FLAG_HOLDUPS),
	m_nCacheWindow(DEFAULT_CACHE_WINDOW),
	m_dMinFraction(DEFAULT_MIN_FRACTION),
	m_dStoreT1(0),
	m_dStoreT2(0),
	m_nPermanentHoldups(-1),
	m_nPermanentStreams(-1)
{}

CBaseUnit::~CBaseUnit(void)
{
	// delete all unit holdups
	ClearMaterialStreams();

	// clear buffer holdups
	ClearHoldups();

	// null static pointers
	m_pMaterialsDB = NULL;
	m_pDistributionsGrid = NULL;
	m_pvPhasesSOA = NULL;
	m_pvPhasesNames = NULL;
	m_pvCompoundsKeys = NULL;

	ClearPlots();
	ClearStateVariables();
}

std::string CBaseUnit::GetUnitName() const
{
	return m_sUnitName;
}

std::string CBaseUnit::GetAuthorName() const
{
	return m_sAuthorName;
}

double CBaseUnit::GetUnitVersion() const
{
	return m_dUnitVersion;
}

std::string CBaseUnit::GetUniqueID() const
{
	return m_sUniqueID;
}

std::vector<sPortStruct> CBaseUnit::GetPorts() const
{
	return m_vPorts;
}

unsigned CBaseUnit::GetPortsNumber() const
{
	return (unsigned)m_vPorts.size();
}

unsigned CBaseUnit::GetPortType(unsigned _nPortIndex) const
{
	if( _nPortIndex >= m_vPorts.size() )
		return UNDEFINED_PORT;
	return m_vPorts[_nPortIndex].nType;
}

unsigned CBaseUnit::GetPortType( const std::string &_sPortName ) const
{
	for( unsigned i=0; i<m_vPorts.size(); ++i )
		if( m_vPorts[i].sName == _sPortName )
			return GetPortType( i );
	return UNDEFINED_PORT;
}

std::string CBaseUnit::GetPortStreamKey(unsigned _nPortIndex) const
{
	if( _nPortIndex >= m_vPorts.size() )
		return "";
	return m_vPorts[_nPortIndex].sStreamKey;
}

CStream* CBaseUnit::GetPortStream(unsigned _nPortIndex) const
{
	if( _nPortIndex >= m_vPorts.size() )
		return nullptr;
	return m_vPorts[_nPortIndex].pStream;
}

CStream* CBaseUnit::GetPortStream( const std::string &_sPortName ) const
{
	for( unsigned i=0; i<m_vPorts.size(); ++i )
		if( m_vPorts[i].sName == _sPortName )
			return GetPortStream(i);
	throw std::logic_error(StrConst::BUnit_ErrGetPort(m_sUnitName, _sPortName));
	return nullptr;
}

void CBaseUnit::SetPortStreamKey(unsigned _nPortIndex, std::string _sStreamKey)
{
	if( _nPortIndex >= m_vPorts.size() )
		return;
	m_vPorts[_nPortIndex].sStreamKey = _sStreamKey;
}

void CBaseUnit::SetPortStream(unsigned _nPortIndex, CStream* _pStream)
{
	if( _nPortIndex >= m_vPorts.size() )
		return;
	m_vPorts[_nPortIndex].pStream = _pStream;
}

unsigned CBaseUnit::AddPort(const std::string& _sPortName, EPortType _nPortType)
{
	sPortStruct newPort;
	newPort.sName = _sPortName.empty() ? StrConst::BUnit_DefaultPortName + std::to_string(m_vPorts.size()) : _sPortName;
	for (size_t i = 0; i < m_vPorts.size(); ++i)
		if (m_vPorts[i].sName == newPort.sName)
			throw std::logic_error(StrConst::BUnit_ErrAddPort(m_sUnitName, newPort.sName));
	if ((_nPortType == INPUT_PORT) || (_nPortType == OUTPUT_PORT))
		newPort.nType = _nPortType;
	else
		newPort.nType = UNDEFINED_PORT;
	newPort.sStreamKey = "";
	newPort.pStream = nullptr;
	m_vPorts.push_back(newPort);
	return (unsigned)m_vPorts.size() - 1;
}

std::vector<CHoldup*> CBaseUnit::GetHoldupsInit() const
{
	return m_vHoldupsInit;
}

std::vector<CHoldup*> CBaseUnit::GetHoldups() const
{
	return m_vHoldupsWork;
}

CHoldup* CBaseUnit::AddHoldup(const std::string& _sHoldupName, const std::string& _sStreamKey /*= "" */)
{
	for (size_t i = 0; i < m_vHoldupsWork.size(); ++i)
		if (m_vHoldupsWork[i]->GetName() == _sHoldupName)
			return m_vHoldupsWork[i];

	std::string sKey = StringFunctions::GenerateUniqueKey(_sStreamKey, GetHoldupsKeys());

	CHoldup* pInitHoldup = new CHoldup(sKey);
	pInitHoldup->SetName(_sHoldupName);
	SetupStream(pInitHoldup);
	m_vHoldupsInit.push_back(pInitHoldup);

	CHoldup* pWorkHoldup = new CHoldup(sKey);
	pWorkHoldup->SetName(_sHoldupName);
	SetupStream(pWorkHoldup);
	m_vHoldupsWork.push_back(pWorkHoldup);

	CHoldup* pStoreHoldup = new CHoldup(sKey + StrConst::BUnit_StoreStreamSuffix);
	pStoreHoldup->SetName(_sHoldupName + StrConst::BUnit_StoreStreamSuffix);
	SetupStream(pStoreHoldup);
	m_vStoreHoldupsWork.push_back(pStoreHoldup);

	return pWorkHoldup;
}

CHoldup* CBaseUnit::GetHoldup( const std::string &_sHoldupName )
{
	for( unsigned i=0; i<m_vHoldupsWork.size(); ++i )
		if( m_vHoldupsWork[i]->GetName() == _sHoldupName )
			return m_vHoldupsWork[i];
	throw std::logic_error(StrConst::BUnit_ErrGetHoldup(m_sUnitName, _sHoldupName));
	return nullptr;
}

CStream* CBaseUnit::AddFeed(const std::string& _sFeedName, const std::string& _sStreamKey /*= "" */)
{
	for (size_t i = 0; i < m_vHoldupsWork.size(); ++i)
		if (m_vHoldupsWork[i]->GetName() == _sFeedName)
			throw std::logic_error(StrConst::BUnit_ErrAddFeed(m_sUnitName, _sFeedName));

	std::string sKey = StringFunctions::GenerateUniqueKey(_sStreamKey, GetHoldupsKeys());

	CStream* pInitFeed = new CStream(sKey);
	pInitFeed->SetName(_sFeedName);
	SetupStream(pInitFeed);
	m_vHoldupsInit.push_back(reinterpret_cast<CHoldup*>(pInitFeed));

	CStream* pWorkFeed = new CStream(sKey);
	pWorkFeed->SetName(_sFeedName);
	SetupStream(pWorkFeed);
	m_vHoldupsWork.push_back(reinterpret_cast<CHoldup*>(pWorkFeed));

	CStream* pStoreFeed = new CStream(sKey + StrConst::BUnit_StoreStreamSuffix);
	pStoreFeed->SetName(_sFeedName + StrConst::BUnit_StoreStreamSuffix);
	SetupStream(pStoreFeed);
	m_vStoreHoldupsWork.push_back(reinterpret_cast<CHoldup*>(pStoreFeed));

	return pWorkFeed;
}

CStream* CBaseUnit::GetFeed( const std::string &_sFeedName )
{
	for( unsigned i=0; i<m_vHoldupsWork.size(); ++i )
		if( m_vHoldupsWork[i]->GetName() == _sFeedName )
			return reinterpret_cast<CStream*>(m_vHoldupsWork[i]);
	throw std::logic_error(StrConst::BUnit_ErrGetFeed(m_sUnitName, _sFeedName));
	return nullptr;
}

void CBaseUnit::RemoveHoldup(CHoldup* _pHoldup)
{
	RemoveHoldup( _pHoldup->GetName() );
}

void CBaseUnit::RemoveHoldup(const std::string &_sName)
{
	for( unsigned i=0; i<m_vHoldupsInit.size(); ++i )
	{
		if( m_vHoldupsInit[i]->GetName() == _sName )
		{
			delete m_vHoldupsInit[i];
			delete m_vHoldupsWork[i];
			delete m_vStoreHoldupsWork[i];
			m_vHoldupsInit.erase( m_vHoldupsInit.begin() + i );
			m_vHoldupsWork.erase( m_vHoldupsWork.begin() + i );
			m_vStoreHoldupsWork.erase( m_vStoreHoldupsWork.begin() + i );
		}
	}
}

void CBaseUnit::InitializeHoldups()
{
	for( unsigned i=0; i<m_vStoreHoldupsWork.size(); ++i )
		SetupStream( m_vStoreHoldupsWork[i] );
	for( unsigned i=0; i<m_vHoldupsWork.size(); ++i )
		if( !CBaseStream::HaveSameStructure(*m_vHoldupsWork[i], *m_vHoldupsInit[i]) )	// TODO: check whether it is needed
			SetupStream( m_vHoldupsWork[i] );
	for (unsigned i = 0; i < m_vHoldupsInit.size(); ++i)
	{
		for (const auto t : m_vHoldupsWork[i]->GetAllTimePoints())
			m_vHoldupsWork[i]->RemoveTimePoint(t);
		m_vHoldupsWork[i]->CopyFromHoldup(0, m_vHoldupsInit[i]->GetLastTimePoint(), m_vHoldupsInit[i]);
	}
}

void CBaseUnit::RemoveTempHoldups()
{
	if (m_nPermanentHoldups != -1)
		while (m_vHoldupsInit.size() > (unsigned)m_nPermanentHoldups)
			RemoveHoldup(m_vHoldupsInit.back());
}

CStream* CBaseUnit::AddMaterialStream(const std::string& _sStreamName, const std::string& _sStreamKey /*= "" */)
{
	for (size_t i = 0; i < m_vStreams.size(); ++i)
		if (m_vStreams[i]->GetName() == _sStreamName)
			return m_vStreams[i];

	std::string sKey = StringFunctions::GenerateUniqueKey(_sStreamKey, GetMaterialStreamsKeys());

	CStream* pStream = new CStream(sKey);
	pStream->SetName(_sStreamName);
	SetupStream(pStream);
	m_vStreams.push_back(pStream);

	CStream* pStoreStream = new CStream(sKey + StrConst::BUnit_StoreStreamSuffix);
	pStoreStream->SetName(_sStreamName + StrConst::BUnit_StoreStreamSuffix);
	SetupStream(pStoreStream);
	m_vStoreStreams.push_back(pStoreStream);

	return pStream;
}

CStream* CBaseUnit::GetMaterialStream( const std::string &_sStreamName )
{
	for( unsigned i=0; i<m_vStreams.size(); ++i )
		if( m_vStreams[i]->GetName() == _sStreamName )
			return m_vStreams[i];
	throw std::logic_error(StrConst::BUnit_ErrGetStream(m_sUnitName, _sStreamName));
	return nullptr;
}

void CBaseUnit::RemoveMaterialStream(CStream* _pStream)
{
	RemoveMaterialStream( _pStream->GetName() );
}

void CBaseUnit::RemoveMaterialStream(const std::string &_sStreamName)
{
	for( size_t i=0; i<m_vStreams.size(); ++i )
	{
		if( m_vStreams[i]->GetName() == _sStreamName )
		{
			delete m_vStreams[i];
			delete m_vStoreStreams[i];
			m_vStreams.erase( m_vStreams.begin() + i );
			m_vStoreStreams.erase( m_vStoreStreams.begin() + i );
		}
	}
}

void CBaseUnit::InitializeMaterialStreams()
{
	for( unsigned i=0; i<m_vStoreStreams.size(); ++i )
		SetupStream( m_vStoreStreams[i] );
	for( unsigned i=0; i<m_vStreams.size(); ++i )
		if (!CBaseStream::HaveSameStructure(*m_vStreams[i], *m_vStoreStreams[i]))	// TODO: check whether it is needed
			SetupStream( m_vStreams[i] );
}

void CBaseUnit::RemoveTempMaterialStreams()
{
	if (m_nPermanentStreams != -1)
		while (m_vStreams.size() > (unsigned)m_nPermanentStreams)
			RemoveMaterialStream(m_vStreams.back());
}

void CBaseUnit::ClearHoldups()
{
	for( unsigned i=0; i<m_vHoldupsInit.size(); ++i )
	{
		delete m_vHoldupsInit[i];
		delete m_vHoldupsWork[i];
		delete m_vStoreHoldupsWork[i];
	}
	m_vHoldupsInit.clear();
	m_vHoldupsWork.clear();
	m_vStoreHoldupsWork.clear();
}

void CBaseUnit::ClearMaterialStreams()
{
	for( unsigned i=0; i<m_vStreams.size(); ++i )
	{
		delete m_vStreams[i];
		delete m_vStoreStreams[i];
	}
	m_vStreams.clear();
	m_vStoreStreams.clear();
}

void CBaseUnit::SetupStream( CBaseStream* _pStream )
{
	if( !_pStream )
		return;

	if( m_pDistributionsGrid )
		_pStream->SetGrid( m_pDistributionsGrid );
	if( m_pMaterialsDB )
		_pStream->SetMaterialsDatabase( m_pMaterialsDB );
	if (m_pvCompoundsKeys)
	{
		_pStream->ClearCompounds();
		for (const auto& c : *m_pvCompoundsKeys)
			_pStream->AddCompound(c);
	}
	if ((m_pvPhasesNames) && (m_pvPhasesSOA))
	{
		_pStream->ClearPhases();
		for (size_t i = 0; i < m_pvPhasesNames->size(); ++i)
			_pStream->AddPhase(PhaseSOA2EPhase((*m_pvPhasesSOA)[i]), (*m_pvPhasesNames)[i]);
	}
	_pStream->SetMinimumFraction( m_dMinFraction );
	_pStream->SetCacheSettings({ m_bCacheEnabled, m_nCacheWindow, m_sCachePath });
}

std::vector<std::string> CBaseUnit::GetHoldupsKeys() const
{
	std::vector<std::string> vRes;
	for( unsigned i=0; i<m_vHoldupsInit.size(); ++i )
		vRes.push_back( m_vHoldupsInit[i]->GetKey() );
	return vRes;
}

std::vector<std::string> CBaseUnit::GetMaterialStreamsKeys() const
{
	std::vector<std::string> vRes;
	for( unsigned i=0; i<m_vStreams.size(); ++i )
		vRes.push_back( m_vStreams[i]->GetKey() );
	return vRes;
}

void CBaseUnit::CopyStreamToStream(const CStream* _pSrcStream, CStream* _pDstStream, double _dTime, bool _bDeleteDataAfter /*= true */)
{
	_pDstStream->CopyFromStream(_dTime, _pSrcStream);
}

void CBaseUnit::CopyStreamToStream(const CStream* _pSrcStream, CStream* _pDstStream, double _dStartTime, double _dEndTime, bool _bDeleteDataAfter /*= true */)
{
	_pDstStream->CopyFromStream(_dStartTime, _dEndTime, _pSrcStream);
}

void CBaseUnit::CopyStreamToPort(const CStream* _pStream, unsigned _nPortIndex, double _dTime, bool _bDeleteDataAfter /*= true */)
{
	if( _nPortIndex >= m_vPorts.size() ) return;
	if( m_vPorts[_nPortIndex].nType != OUTPUT_PORT ) return;

	// copy data
	m_vPorts[_nPortIndex].pStream->CopyFromStream(_dTime, _pStream);
}

void CBaseUnit::CopyStreamToPort( const CStream* _pStream, const std::string &_sPortName, double _dTime, bool _bDeleteDataAfter /*= true*/ )
{
	for( unsigned i=0; i<m_vPorts.size(); ++i )
		if( m_vPorts[i].sName == _sPortName )
		{
			CopyStreamToPort( _pStream, i, _dTime, _bDeleteDataAfter );
			break;
		}
}

void CBaseUnit::CopyStreamToPort(const CStream* _pStream, unsigned _nPortIndex, double _dStartTime, double _dEndTime, bool _bDeleteDataAfter /*= true */)
{
	if( _nPortIndex >= m_vPorts.size() ) return;
	if( m_vPorts[_nPortIndex].nType != OUTPUT_PORT ) return;

	// copy data
	m_vPorts[_nPortIndex].pStream->CopyFromStream(_dStartTime, _dEndTime, _pStream);
}

void CBaseUnit::CopyStreamToPort( const CStream* _pStream, const std::string &_sPortName, double _dStartTime, double _dEndTime, bool _bDeleteDataAfter /*= true*/ )
{
	for( unsigned i=0; i<m_vPorts.size(); ++i )
		if( m_vPorts[i].sName == _sPortName )
		{
			CopyStreamToPort( _pStream, i, _dStartTime, _dEndTime, _bDeleteDataAfter );
			break;
		}
}

void CBaseUnit::CopyPortToStream(unsigned _nPortIndex, CStream* _pStream, double _dTime, bool _bDeleteDataAfter /*= true */)
{
	if( _nPortIndex >= m_vPorts.size() ) return;
	if( m_vPorts[_nPortIndex].nType != INPUT_PORT ) return;

	// copy data
	_pStream->CopyFromStream(_dTime, m_vPorts[_nPortIndex].pStream);
}

void CBaseUnit::CopyPortToStream( const std::string &_sPortName, CStream* _pStream, double _dTime, bool _bDeleteDataAfter /*= true*/ )
{
	for( unsigned i=0; i<m_vPorts.size(); ++i )
		if( m_vPorts[i].sName == _sPortName )
		{
			CopyPortToStream( i, _pStream, _dTime, _bDeleteDataAfter );
			break;
		}
}

void CBaseUnit::CopyPortToStream(unsigned _nPortIndex, CStream* _pStream, double _dStartTime, double _dEndTime, bool _bDeleteDataAfter /*= true */)
{
	if( _nPortIndex >= m_vPorts.size() ) return;
	if( m_vPorts[_nPortIndex].nType != INPUT_PORT ) return;

	// copy data
	_pStream->CopyFromStream(_dStartTime, _dEndTime, m_vPorts[_nPortIndex].pStream);
}

void CBaseUnit::CopyPortToStream( const std::string &_sPortName, CStream* _pStream, double _dStartTime, double _dEndTime, bool _bDeleteDataAfter /*= true*/ )
{
	for( unsigned i=0; i<m_vPorts.size(); ++i )
		if( m_vPorts[i].sName == _sPortName )
		{
			CopyPortToStream( i, _pStream, _dStartTime, _dEndTime, _bDeleteDataAfter );
			break;
		}
}

void CBaseUnit::Simulate(double _dStartTime, double _dEndTime)
{
}

void CBaseUnit::Simulate(double _dTime) {}

void CBaseUnit::Initialize(double _dTime) {}

void CBaseUnit::SaveState() {}

void CBaseUnit::LoadState() {}

void CBaseUnit::Finalize() {}

std::vector<double> CBaseUnit::GetAllDefinedTimePoints(double _dStartTime, double _dEndTime, bool _bForceStartBoundary /*= false*/, bool _bForceEndBoundary /*= false*/) const
{
	std::vector<double> vResTimePoints;
	// get time points from input ports
	for( unsigned i=0; i<m_vPorts.size(); ++i )
	{
		if( m_vPorts[i].nType == INPUT_PORT )
		{
			if( vResTimePoints.empty() ) // first input stream
				vResTimePoints = GetPortStream(i)->GetTimePoints( _dStartTime, _dEndTime );
			else // union with the last time points
				vResTimePoints = VectorsUnionSorted( vResTimePoints, GetPortStream(i)->GetTimePoints( _dStartTime, _dEndTime ) );
		}
	}
	// get time points from parameters
	vResTimePoints = VectorsUnionSorted(vResTimePoints, m_unitParameters.GetAllTimePoints(_dStartTime, _dEndTime));

	// add boundaries
	if( _bForceStartBoundary )
	{
		if( vResTimePoints.empty() )
			vResTimePoints.push_back( _dStartTime );
		else if( vResTimePoints.front() != _dStartTime )
			vResTimePoints.insert( vResTimePoints.begin(), _dStartTime );
	}
	if( _bForceEndBoundary )
	{
		if( ( vResTimePoints.empty() ) || ( vResTimePoints.back() != _dEndTime ) )
			vResTimePoints.push_back( _dEndTime );
	}

	return vResTimePoints;
}

std::vector<double> CBaseUnit::GetAllInputTimePoints(double _dStartTime, double _dEndTime, bool _bForceStartBoundary /*= false*/, bool _bForceEndBoundary /*= false*/) const
{
	std::vector<double> vResTimePoints;
	// get time points from input ports
	for( unsigned i=0; i<m_vPorts.size(); ++i )
	{
		if( m_vPorts[i].nType == INPUT_PORT )
		{
			if( vResTimePoints.empty() ) // first input stream
				vResTimePoints = GetPortStream(i)->GetTimePoints( _dStartTime, _dEndTime );
			else // union with the last time points
				vResTimePoints = VectorsUnionSorted( vResTimePoints, GetPortStream(i)->GetTimePoints( _dStartTime, _dEndTime ) );
		}
	}

	// add boundaries
	if( _bForceStartBoundary )
	{
		if( vResTimePoints.empty() )
			vResTimePoints.push_back( _dStartTime );
		else if( vResTimePoints.front() != _dStartTime )
			vResTimePoints.insert( vResTimePoints.begin(), _dStartTime );
	}
	if( _bForceEndBoundary )
	{
		if( ( vResTimePoints.empty() ) || ( vResTimePoints.back() != _dEndTime ) )
			vResTimePoints.push_back( _dEndTime );
	}

	return vResTimePoints;
}

std::vector<double> CBaseUnit::GetAllStreamsTimePoints(const std::vector<CStream*>& _vSrteams, double _dStartTime, double _dEndTime) const
{
	std::vector<double> vResTimePoints;
	// get time points from streams
	for( unsigned i=0; i<_vSrteams.size(); ++i )
	{
		if( vResTimePoints.empty() ) // first input stream
			vResTimePoints = _vSrteams[i]->GetTimePoints( _dStartTime, _dEndTime );
		else // union with the last time points
			vResTimePoints = VectorsUnionSorted( vResTimePoints, _vSrteams[i]->GetTimePoints( _dStartTime, _dEndTime ) );
	}
	return vResTimePoints;
}

void CBaseUnit::ReduceTimePoints(double _dStart, double _dEnd, double _dStep)
{
	for (auto& s : m_vHoldupsWork)		s->ReduceTimePoints(_dStart, _dEnd, _dStep);
	for (auto& s : m_vStoreHoldupsWork)	s->ReduceTimePoints(_dStart, _dEnd, _dStep);
	for (auto& s : m_vStreams)			s->ReduceTimePoints(_dStart, _dEnd, _dStep);
	for (auto& s : m_vStoreStreams)		s->ReduceTimePoints(_dStart, _dEnd, _dStep);
}

const CUnitParametersManager& CBaseUnit::GetUnitParametersManager() const
{
	return m_unitParameters;
}

CUnitParametersManager& CBaseUnit::GetUnitParametersManager()
{
	return m_unitParameters;
}

double CBaseUnit::GetConstParameterValue(const std::string& _name) const
{
	if (const CConstRealUnitParameter* param = m_unitParameters.GetConstRealParameter(_name))
		return param->GetValue();
	throw std::logic_error(StrConst::BUnit_ErrGetConstParam(m_sUnitName, _name));
}

double CBaseUnit::GetTDParameterValue(const std::string& _name, double _time) const
{
	if (const CTDUnitParameter* param = m_unitParameters.GetTDParameter(_name))
		return param->GetValue(_time);
	throw std::logic_error(StrConst::BUnit_ErrGetTDParam(m_sUnitName, _name));
}

std::string CBaseUnit::GetStringParameterValue(const std::string& _name) const
{
	if (const CStringUnitParameter* param = m_unitParameters.GetStringParameter(_name))
		return param->GetValue();
	throw std::logic_error(StrConst::BUnit_ErrGetStrParam(m_sUnitName, _name));
}

bool CBaseUnit::GetCheckboxParameterValue(const std::string& _name) const
{
	if (const CCheckBoxUnitParameter* param = m_unitParameters.GetCheckboxParameter(_name))
		return param->IsChecked();
	throw std::logic_error(StrConst::BUnit_ErrGetBoxParam(m_sUnitName, _name));
}

size_t CBaseUnit::GetComboParameterValue(const std::string& _name) const
{
	if (const CComboUnitParameter* param = m_unitParameters.GetComboParameter(_name))
		return param->GetValue();
	throw std::logic_error(StrConst::BUnit_ErrGetComboParam(m_sUnitName, _name));
}

size_t CBaseUnit::GetGroupParameterValue(const std::string& _name) const
{
	if (const CComboUnitParameter* param = m_unitParameters.GetComboParameter(_name))
		return param->GetValue();
	throw std::logic_error(StrConst::BUnit_ErrGetComboParam(m_sUnitName, _name));
}

std::string CBaseUnit::GetCompoundParameterValue(const std::string& _name) const
{
	if (const CCompoundUnitParameter* param = m_unitParameters.GetCompoundParameter(_name))
		return param->GetCompound();
	throw std::logic_error(StrConst::BUnit_ErrGetCompParam(m_sUnitName, _name));
}

CBaseSolver* CBaseUnit::GetSolverParameterValue(const std::string& _name) const
{
	if (const CSolverUnitParameter* param = m_unitParameters.GetSolverParameter(_name))
	{
		for (auto& s : m_vExternalSolvers) // find solver with the key from the found unit parameter
			if (s->GetUniqueID() == param->GetKey())
				return s;
		return nullptr;
	}
	throw std::logic_error(StrConst::BUnit_ErrGetSolver(m_sUnitName, _name));
}

CAgglomerationSolver* CBaseUnit::GetSolverAgglomeration(const std::string& _name) const
{
	CBaseSolver* pSolver = GetSolverParameterValue(_name);
	if (pSolver->GetType() != ESolverTypes::SOLVER_AGGLOMERATION_1)
		throw std::logic_error(StrConst::BUnit_ErrGetSolver1(m_sUnitName, _name));
	return dynamic_cast<CAgglomerationSolver*>(pSolver);
}

CPBMSolver* CBaseUnit::GetSolverPBM(const std::string& _name) const
{
	CBaseSolver* pSolver = GetSolverParameterValue(_name);
	if (pSolver->GetType() != ESolverTypes::SOLVER_PBM_1)
		throw std::logic_error(StrConst::BUnit_ErrGetSolver2(m_sUnitName, _name));
	return dynamic_cast<CPBMSolver*>(pSolver);
}

void CBaseUnit::AddConstParameter(const std::string& _name, double _minValue, double _maxValue, double _initValue, const std::string& _units, const std::string& _description)
{
	if (m_unitParameters.IsNameExist(_name)) throw std::logic_error(StrConst::BUnit_ErrAddParam(m_sUnitName, _name));
	m_unitParameters.AddConstRealParameter(_name, _units, _description, _minValue, _maxValue, _initValue);
}

void CBaseUnit::AddTDParameter(const std::string& _name, double _minValue, double _maxValue, double _initValue, const std::string& _units, const std::string& _description)
{
	if (m_unitParameters.IsNameExist(_name)) throw std::logic_error(StrConst::BUnit_ErrAddParam(m_sUnitName, _name));
	m_unitParameters.AddTDParameter(_name, _units, _description, _minValue, _maxValue, _initValue);
}

void CBaseUnit::AddStringParameter(const std::string& _name, const std::string& _initValue, const std::string& _description)
{
	if (m_unitParameters.IsNameExist(_name)) throw std::logic_error(StrConst::BUnit_ErrAddParam(m_sUnitName, _name));
	m_unitParameters.AddStringParameter(_name, _description, _initValue);
}

void CBaseUnit::AddCheckBoxParameter(const std::string& _name, bool _initValue, const std::string& _description)
{
	if (m_unitParameters.IsNameExist(_name)) throw std::logic_error(StrConst::BUnit_ErrAddParam(m_sUnitName, _name));
	m_unitParameters.AddCheckBoxParameter(_name, _description, _initValue);
}

void CBaseUnit::AddComboParameter(const std::string& _name, size_t _initValue, const std::vector<size_t>& _values, const std::vector<std::string>& _valuesNames, const std::string& _description)
{
	if (m_unitParameters.IsNameExist(_name)) throw std::logic_error(StrConst::BUnit_ErrAddParam(m_sUnitName, _name));

	std::vector<size_t> values = _values;
	if (values.empty())
	{
		values.resize(_valuesNames.size());
		std::iota(values.begin(), values.end(), 0);
	}

	if (values.size() != _valuesNames.size())
		throw std::logic_error(StrConst::BUnit_ErrAddComboParam(m_sUnitName, _name));

	m_unitParameters.AddComboParameter(_name, _description, _initValue, values, _valuesNames);
}

void CBaseUnit::AddGroupParameter(const std::string& _name, size_t _initValue, const std::vector<size_t>& _values, const std::vector<std::string>& _valuesNames, const std::string& _description)
{
	if (m_unitParameters.IsNameExist(_name)) throw std::logic_error(StrConst::BUnit_ErrAddParam(m_sUnitName, _name));

	std::vector<size_t> values = _values;
	if (values.empty())
	{
		values.resize(_valuesNames.size());
		std::iota(values.begin(), values.end(), 0);
	}

	if (values.size() != _valuesNames.size())
		throw std::logic_error(StrConst::BUnit_ErrAddComboParam(m_sUnitName, _name));

	m_unitParameters.AddComboParameter(_name, _description, _initValue, values, _valuesNames);
}

void CBaseUnit::AddCompoundParameter(const std::string& _name, const std::string& _description)
{
	if (m_unitParameters.IsNameExist(_name)) throw std::logic_error(StrConst::BUnit_ErrAddParam(m_sUnitName, _name));
	m_unitParameters.AddCompoundParameter(_name, _description);
}

void CBaseUnit::AddSolverParameter(const std::string& _name, ESolverTypes _solverType, const std::string& _description)
{
	if (m_unitParameters.IsNameExist(_name)) throw std::logic_error(StrConst::BUnit_ErrAddParam(m_sUnitName, _name));
	m_unitParameters.AddSolverParameter(_name, _description, _solverType);
}

void CBaseUnit::AddSolverAgglomeration(const std::string& _name, const std::string& _description)
{
	AddSolverParameter(_name, ESolverTypes::SOLVER_AGGLOMERATION_1, _description);
}

void CBaseUnit::AddSolverPBM(const std::string& _name, const std::string &_description)
{
	AddSolverParameter(_name, ESolverTypes::SOLVER_PBM_1, _description);
}

void CBaseUnit::AddParametersToGroup(const std::string& _groupParamName, const std::string& _groupName, const std::vector<std::string>& _paramNames)
{
	const auto* groupParameter = m_unitParameters.GetComboParameter(_groupParamName);
	if (!groupParameter)						// check that group parameter exists
		throw std::logic_error(StrConst::BUnit_ErrGroupParamBlock(m_sUnitName, _groupParamName, _groupName));
	if (!groupParameter->HasName(_groupName))	// check that group exists
		throw std::logic_error(StrConst::BUnit_ErrGroupParamGroup(m_sUnitName, _groupParamName, _groupName));
	for (const auto& name : _paramNames)		// check that all parameters exist
		if (!m_unitParameters.GetParameter(name))
			throw std::logic_error(StrConst::BUnit_ErrGroupParamParam(m_sUnitName, _groupParamName, _groupName, name));

	m_unitParameters.AddParametersToGroup(_groupParamName, _groupName, _paramNames);
}

unsigned CBaseUnit::GetStoredSVNumber() const
{
	unsigned nCnt = 0;
	for( std::vector<sStateVariable*>::const_iterator it = m_vStateVariables.begin(); it != m_vStateVariables.end(); ++it )
		if( (*it)->bIsSaved )
			nCnt++;
	return nCnt;
}

std::string CBaseUnit::GetStoredSVName(unsigned _nIndex) const
{
	if( _nIndex >= m_vStateVariables.size() )
		return "";

	unsigned nInternalIndex = 0;
	unsigned nTempIndex = 0;
	bool bFounded = false;
	for( unsigned i=0; i<m_vStateVariables.size(); ++i )
	{
		if( m_vStateVariables[i]->bIsSaved )
		{
			if( nTempIndex == _nIndex )
			{
				bFounded = true;
				break;
			}
			nTempIndex++;
		}
		nInternalIndex++;
	}

	if( !bFounded )
		return "";

	return m_vStateVariables[nInternalIndex]->sName;
}

bool CBaseUnit::GetStoredSVData(unsigned _nIndex, std::vector<double>& _vTimes, std::vector<double>& _vValues) const
{
	if( _nIndex >= m_vStateVariables.size() )
		return false;

	unsigned nInternalIndex = 0;
	unsigned nTempIndex = 0;
	bool bFounded = false;
	for( unsigned i=0; i<m_vStateVariables.size(); ++i )
	{
		if( m_vStateVariables[i]->bIsSaved )
		{
			if( nTempIndex == _nIndex )
			{
				bFounded = true;
				break;
			}
			nTempIndex++;
		}
		nInternalIndex++;
	}
	if( !bFounded )
		return false;

	_vTimes = m_vStateVariables[nInternalIndex]->vHistoryTimes;
	_vValues = m_vStateVariables[nInternalIndex]->vHistoryValues;

	return true;
}

unsigned CBaseUnit::AddStateVariable(const std::string &_sName, double _dInitValue, bool _bSaveHistory /*= false */)
{
	for(size_t i=0; i<m_vStateVariables.size(); ++i)
		if(m_vStateVariables[i]->sName == _sName)
			return (unsigned)i;

	sStateVariable *newVar = new sStateVariable;
	newVar->sName = _sName;
	newVar->dValue = _dInitValue;
	newVar->bIsSaved = _bSaveHistory;
	m_vStateVariables.push_back( newVar );
	return (unsigned)m_vStateVariables.size() - 1;
}

double CBaseUnit::GetStateVariable(unsigned _nIndex) const
{
	if( _nIndex >= m_vStateVariables.size() )
		return 0;

	return m_vStateVariables[_nIndex]->dValue;
}

double CBaseUnit::GetStateVariable( const std::string &_sName ) const
{
	for( unsigned i=0; i<m_vStateVariables.size(); ++i )
		if( m_vStateVariables[i]->sName == _sName )
			return GetStateVariable( i );
	throw std::logic_error(StrConst::BUnit_ErrGetSV(m_sUnitName, _sName));
	return 0;
}

void CBaseUnit::SetStateVariable(unsigned _nIndex, double _dValue)
{
	if( _nIndex >= m_vStateVariables.size() )
		return;

	m_vStateVariables[_nIndex]->dValue = _dValue;
}

void CBaseUnit::SetStateVariable( const std::string &_sName, double _dValue )
{
	for( unsigned i=0; i<m_vStateVariables.size(); ++i )
		if( m_vStateVariables[i]->sName == _sName )
			return SetStateVariable( i, _dValue );
	throw std::logic_error(StrConst::BUnit_ErrSetSV(m_sUnitName, _sName, StringFunctions::Double2String(_dValue)));
}

void CBaseUnit::ClearStateVariables(/*double _dTime *//*= -1 */)
{
	for( std::vector<sStateVariable*>::const_iterator it = m_vStateVariables.begin(); it != m_vStateVariables.end(); ++it )
		delete *it;
	m_vStateVariables.clear();
}

void CBaseUnit::SaveStateVariables(double _dTime)
{
	// delete time points which are equal or greater than _dTime
	for( unsigned i=0; i<m_vStateVariables.size(); ++i )
	{
		std::vector<double> *pTimes = &m_vStateVariables[i]->vHistoryTimes;
		if( ( m_vStateVariables[i]->bIsSaved ) && ( !pTimes->empty() ) )
		{
			unsigned index=0;
			while( index < pTimes->size() )
				if( pTimes->at(index) >= _dTime )
					break;
				else
					index++;
			if( index < pTimes->size() )
			{
				std::vector<double> *pValues = &m_vStateVariables[i]->vHistoryValues;
				pTimes->erase( pTimes->begin() + index, pTimes->end() );
				pValues->erase( pValues->begin() + index, pValues->end() );
			}
		}
	}

	// add current to a storing memory
	for( unsigned i=0; i<m_vStateVariables.size(); ++i )
	{
		if( m_vStateVariables[i]->bIsSaved )
		{
			m_vStateVariables[i]->vHistoryTimes.push_back( _dTime );
			m_vStateVariables[i]->vHistoryValues.push_back( m_vStateVariables[i]->dValue );
		}
	}
}

std::string CBaseUnit::GetStateVariableName(unsigned _nIndex) const
{
	if( _nIndex >= m_vStateVariables.size() )
		return "";

	return m_vStateVariables[_nIndex]->sName;
}

unsigned CBaseUnit::GetStateVariablesNumber() const
{
	return (unsigned)m_vStateVariables.size();
}

void CBaseUnit::SetMaterialsDatabase(const CMaterialsDatabase* _pDatabase)
{
	m_pMaterialsDB = _pDatabase;
	for( unsigned i=0; i<m_vHoldupsInit.size(); ++i )
	{
		m_vHoldupsInit[i]->SetMaterialsDatabase( m_pMaterialsDB );
		m_vHoldupsWork[i]->SetMaterialsDatabase( m_pMaterialsDB );
		m_vStoreHoldupsWork[i]->SetMaterialsDatabase( m_pMaterialsDB );
	}
	for( unsigned i=0; i<m_vStreams.size(); ++i )
	{
		m_vStreams[i]->SetMaterialsDatabase( m_pMaterialsDB );
		m_vStoreStreams[i]->SetMaterialsDatabase( m_pMaterialsDB );
	}
}

void CBaseUnit::SetCompounds(const std::vector<std::string>* _pvCompoundsKeys)
{
	m_pvCompoundsKeys = _pvCompoundsKeys;
	for( unsigned i=0; i<m_vHoldupsInit.size(); ++i )
	{
		m_vHoldupsInit[i]->ClearCompounds();
		m_vHoldupsWork[i]->ClearCompounds();
		m_vStoreHoldupsWork[i]->ClearCompounds();
		for (const auto& c : *m_pvCompoundsKeys)
		{
			m_vHoldupsInit[i]->AddCompound(c);
			m_vHoldupsWork[i]->AddCompound(c);
			m_vStoreHoldupsWork[i]->AddCompound(c);
		}
	}
	for (unsigned i = 0; i < m_vStreams.size(); ++i)
	{
		m_vStreams[i]->ClearCompounds();
		m_vStoreStreams[i]->ClearCompounds();
		for (const auto& c : *m_pvCompoundsKeys)
		{
			m_vStreams[i]->AddCompound(c);
			m_vStoreStreams[i]->AddCompound(c);
		}
	}
}

void CBaseUnit::SetCompoundsPtr( const std::vector<std::string>* _pvCompoundsKeys )
{
	m_pvCompoundsKeys = _pvCompoundsKeys;
}

void CBaseUnit::AddCompound( std::string _sCompoundKey )
{
	for( unsigned i=0; i<m_vHoldupsInit.size(); ++i )
	{
		m_vHoldupsInit[i]->AddCompound( _sCompoundKey );
		m_vHoldupsWork[i]->AddCompound( _sCompoundKey );
		m_vStoreHoldupsWork[i]->AddCompound( _sCompoundKey );
	}
	for( unsigned i=0; i<m_vStreams.size(); ++i )
	{
		m_vStreams[i]->AddCompound( _sCompoundKey );
		m_vStoreStreams[i]->AddCompound( _sCompoundKey );
	}
}

void CBaseUnit::RemoveCompound( std::string _sCompoundKey )
{
	for( unsigned i=0; i<m_vHoldupsInit.size(); ++i )
	{
		m_vHoldupsInit[i]->RemoveCompound( _sCompoundKey );
		m_vHoldupsWork[i]->RemoveCompound( _sCompoundKey );
		m_vStoreHoldupsWork[i]->RemoveCompound( _sCompoundKey );
	}
	for( unsigned i=0; i<m_vStreams.size(); ++i )
	{
		m_vStreams[i]->RemoveCompound( _sCompoundKey );
		m_vStoreStreams[i]->RemoveCompound( _sCompoundKey );
	}
}

std::vector<std::string> CBaseUnit::GetCompoundsList() const
{
	if( !m_pvCompoundsKeys )
		return std::vector<std::string>();

	return *m_pvCompoundsKeys;
}

std::vector<std::string> CBaseUnit::GetCompoundsNames() const
{
	std::vector<std::string> vNames;
	if (m_pMaterialsDB)
		for (size_t i = 0; i < m_pvCompoundsKeys->size(); ++i)
			vNames.push_back(m_pMaterialsDB->GetCompound(m_pvCompoundsKeys->at(i))->GetName());
	return vNames;
}

unsigned CBaseUnit::GetCompoundsNumber() const
{
	if( m_pvCompoundsKeys == NULL )
		return 0;

	return (unsigned)m_pvCompoundsKeys->size();
}

double CBaseUnit::GetCompoundConstant(const std::string &_sCompoundKey, unsigned _nConstant) const
{
	double dVal = 0;

	if(!m_pMaterialsDB) return dVal;

	switch (static_cast<ECompoundConstProperties>(_nConstant))
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
		dVal = m_pMaterialsDB->GetConstPropertyValue(_sCompoundKey, static_cast<ECompoundConstProperties>(_nConstant));
		break;
	case CONST_PROP_NO_PROERTY:	break;
	}
	return dVal;
}

double CBaseUnit::GetCompoundTPDProp(const std::string &_sCompoundKey, unsigned _nProperty, double _dTemperature, double _dPressure) const
{
	double dVal = 0;

	if (!m_pMaterialsDB) return dVal;

	switch (static_cast<ECompoundTPProperties>(_nProperty))
	{
	case ENTHALPY:
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

double CBaseUnit::GetCompoundsInteractionProp(const std::string& _sCompoundKey1, const std::string& _sCompoundKey2, const unsigned _nProperty, const double _dTemperature, const double _dPressure) const
{
	double dVal = 0;

	if (!m_pMaterialsDB) return dVal;

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
		dVal = m_pMaterialsDB->GetInteractionPropertyValue(_sCompoundKey1, _sCompoundKey2, static_cast<EInteractionProperties>(_nProperty), _dTemperature, _dPressure);
		break;
	case INT_PROP_NO_PROERTY: break;
	}
	return dVal;
}

bool CBaseUnit::IsCompoundNameDefined(const std::string& _sCompoundName) const
{
	const std::vector<std::string> vCompNames = GetCompoundsNames();
	return std::find(vCompNames.begin(), vCompNames.end(), _sCompoundName) != vCompNames.end();
}

bool CBaseUnit::IsCompoundKeyDefined(const std::string& _sCompoundKey) const
{
	if (!m_pvCompoundsKeys) return false;
	return std::find(m_pvCompoundsKeys->begin(), m_pvCompoundsKeys->end(), _sCompoundKey) != m_pvCompoundsKeys->end();
}

bool CBaseUnit::IsPropertyDefined(unsigned _propertyKey) const
{
	return m_pMaterialsDB->IsPropertyDefined(_propertyKey);
}

void CBaseUnit::SetPhases(const std::vector<std::string>* _pvPhasesNames, const std::vector<unsigned>* _pvPhasesSOAs)
{
	m_pvPhasesNames = _pvPhasesNames;
	m_pvPhasesSOA = _pvPhasesSOAs;
	for( unsigned i=0; i<m_vHoldupsInit.size(); ++i )
	{
		m_vHoldupsInit[i]->ClearPhases();
		m_vHoldupsWork[i]->ClearPhases();
		m_vStoreHoldupsWork[i]->ClearPhases();
		for (size_t i = 0; i < m_pvPhasesNames->size(); ++i)
		{
			m_vHoldupsInit[i]->AddPhase(PhaseSOA2EPhase((*m_pvPhasesSOA)[i]), (*m_pvPhasesNames)[i]);
			m_vHoldupsWork[i]->AddPhase(PhaseSOA2EPhase((*m_pvPhasesSOA)[i]), (*m_pvPhasesNames)[i]);
			m_vStoreHoldupsWork[i]->AddPhase(PhaseSOA2EPhase((*m_pvPhasesSOA)[i]), (*m_pvPhasesNames)[i]);
		}
	}
	for( unsigned i=0; i<m_vStreams.size(); ++i )
	{
		m_vStreams[i]->ClearPhases();
		m_vStoreStreams[i]->ClearPhases();
		for (size_t i = 0; i < m_pvPhasesNames->size(); ++i)
		{
			m_vStreams[i]->AddPhase(PhaseSOA2EPhase((*m_pvPhasesSOA)[i]), (*m_pvPhasesNames)[i]);
			m_vStoreStreams[i]->AddPhase(PhaseSOA2EPhase((*m_pvPhasesSOA)[i]), (*m_pvPhasesNames)[i]);
		}
	}
}

void CBaseUnit::SetPhasesPtr( const std::vector<std::string>* _pvPhasesNames, const std::vector<unsigned>* _pvPhasesSOAs )
{
	m_pvPhasesNames = _pvPhasesNames;
	m_pvPhasesSOA = _pvPhasesSOAs;
}

void CBaseUnit::AddPhase( std::string _sName, unsigned _nAggrState )
{
	for( unsigned i=0; i<m_vHoldupsInit.size(); ++i )
	{
		m_vHoldupsInit[i]->AddPhase(PhaseSOA2EPhase(_nAggrState), _sName);
		m_vHoldupsWork[i]->AddPhase(PhaseSOA2EPhase(_nAggrState), _sName);
		m_vStoreHoldupsWork[i]->AddPhase(PhaseSOA2EPhase(_nAggrState), _sName);
	}
	for( unsigned i=0; i<m_vStreams.size(); ++i )
	{
		m_vStreams[i]->AddPhase(PhaseSOA2EPhase(_nAggrState), _sName);
		m_vStoreStreams[i]->AddPhase(PhaseSOA2EPhase(_nAggrState), _sName);
	}
}

void CBaseUnit::RemovePhase( unsigned _nIndex )
{
	for( unsigned i=0; i<m_vHoldupsInit.size(); ++i )
	{
		m_vHoldupsInit[i]->RemovePhase(PhaseSOA2EPhase((*m_pvPhasesSOA)[_nIndex]));
		m_vHoldupsWork[i]->RemovePhase(PhaseSOA2EPhase((*m_pvPhasesSOA)[_nIndex]));
		m_vStoreHoldupsWork[i]->RemovePhase(PhaseSOA2EPhase((*m_pvPhasesSOA)[_nIndex]));
	}
	for( unsigned i=0; i<m_vStreams.size(); ++i )
	{
		m_vStreams[i]->RemovePhase(PhaseSOA2EPhase((*m_pvPhasesSOA)[_nIndex]));
		m_vStoreStreams[i]->RemovePhase(PhaseSOA2EPhase((*m_pvPhasesSOA)[_nIndex]));
	}
}

void CBaseUnit::ChangePhase( unsigned _nIndex, std::string _sName, unsigned _nAggrState )
{
	RemovePhase(_nIndex);
	AddPhase(_sName, _nAggrState);
}

bool CBaseUnit::IsPhaseDefined( EPhaseTypes _nPhaseType ) const
{
	bool bRes = false;
	for( unsigned i=0; i<m_pvPhasesSOA->size(); ++i )
		if( m_pvPhasesSOA->at(i) == _nPhaseType )
		{
			bRes = true;
			break;
		}
	return bRes;
}

unsigned CBaseUnit::GetLiquidPhasesNumber() const
{
	unsigned nNum = 0;
	for( unsigned i=0; i<m_pvPhasesSOA->size(); ++i )
		if( ( m_pvPhasesSOA->at(i) == SOA_LIQUID ) || ( m_pvPhasesSOA->at(i) == SOA_LIQUID2 ) )
			nNum++;
	return nNum;
}

unsigned CBaseUnit::GetPhasesNumber() const
{
	if( m_pvPhasesNames == NULL )
		return 0;

	return (unsigned)m_pvPhasesNames->size();
}

std::string CBaseUnit::GetPhaseName(EPhaseTypes _nPhaseType) const
{
	if( m_pvPhasesSOA != NULL )
		for( unsigned i=0; i<m_pvPhasesSOA->size(); ++i )
			if( m_pvPhasesSOA->at(i) == _nPhaseType )
				return m_pvPhasesNames->at(i);
	return "";
}

unsigned CBaseUnit::GetPhaseSOA(unsigned _nPhaseIndex) const
{
	if( ( m_pvPhasesSOA != NULL ) && ( _nPhaseIndex < m_pvPhasesSOA->size() ) )
		return m_pvPhasesSOA->at(_nPhaseIndex);
	return SOA_UNDEFINED;
}

unsigned CBaseUnit::GetPhaseIndex(unsigned _nPhaseType) const
{
	if(m_pvPhasesSOA)
		for (unsigned i = 0; i < m_pvPhasesSOA->size(); ++i)
			if (m_pvPhasesSOA->at(i) == _nPhaseType)
				return i;
	return -1;
}

void CBaseUnit::SetDistributionsGrid(const CDistributionsGrid* _pGrid)
{
	m_pDistributionsGrid = _pGrid;
	for( unsigned i=0; i<m_vHoldupsInit.size(); ++i )
	{
		m_vHoldupsInit[i]->SetGrid( m_pDistributionsGrid );
		m_vHoldupsWork[i]->SetGrid( m_pDistributionsGrid );
		m_vStoreHoldupsWork[i]->SetGrid( m_pDistributionsGrid );
		m_vHoldupsInit[i]->UpdateDistributionsGrid();
		m_vHoldupsWork[i]->UpdateDistributionsGrid();
		m_vStoreHoldupsWork[i]->UpdateDistributionsGrid();
	}
	for( unsigned i=0; i<m_vStreams.size(); ++i )
	{
		m_vStreams[i]->SetGrid( m_pDistributionsGrid );
		m_vStoreStreams[i]->SetGrid( m_pDistributionsGrid );
		m_vStreams[i]->UpdateDistributionsGrid();
		m_vStoreStreams[i]->UpdateDistributionsGrid();
	}
}

std::vector<EDistrTypes> CBaseUnit::GetDistributionsTypes() const
{
	if (!m_pDistributionsGrid) return std::vector<EDistrTypes>();
	return m_pDistributionsGrid->GetDistrTypes();
}

std::vector<unsigned> CBaseUnit::GetDistributionsClasses() const
{
	if (!m_pDistributionsGrid) return std::vector<unsigned>();
	return m_pDistributionsGrid->GetClasses();
}

unsigned CBaseUnit::GetDistributionsNumber() const
{
	if (!m_pDistributionsGrid) return 0;
	return (unsigned)m_pDistributionsGrid->GetDistributionsNumber();
}

EGridEntry CBaseUnit::GetDistributionGridType(EDistrTypes _nDistrType) const
{
	if (!m_pDistributionsGrid) return EGridEntry::GRID_UNDEFINED;
	return m_pDistributionsGrid->GetGridEntryByDistr(_nDistrType);
}

std::vector<double> CBaseUnit::GetNumericGrid(EDistrTypes _nDistrType) const
{
	if (!m_pDistributionsGrid) return std::vector<double>();
	return m_pDistributionsGrid->GetNumericGridByDistr(_nDistrType);
}

std::vector<std::string> CBaseUnit::GetSymbolicGrid(EDistrTypes _nDistrType) const
{
	if (!m_pDistributionsGrid) return std::vector<std::string>();
	return m_pDistributionsGrid->GetSymbolicGridByDistr(_nDistrType);
}

std::vector<double> CBaseUnit::GetClassesMeans(EDistrTypes _nDistrType) const
{
	if (!m_pDistributionsGrid) return {};
	return m_pDistributionsGrid->GetClassMeansByDistr(_nDistrType);
}

std::vector<double> CBaseUnit::GetPSDGridDiameters() const
{
	if (!m_pDistributionsGrid) return {};
	return m_pDistributionsGrid->GetPSDGrid(EPSDGridType::DIAMETER);
}

std::vector<double> CBaseUnit::GetPSDGridVolumes() const
{
	if (!m_pDistributionsGrid) return {};
	return m_pDistributionsGrid->GetPSDGrid(EPSDGridType::VOLUME);
}

std::vector<double> CBaseUnit::GetPSDMeanDiameters() const
{
	if (!m_pDistributionsGrid) return {};
	return m_pDistributionsGrid->GetPSDMeans(EPSDGridType::DIAMETER);
}

std::vector<double> CBaseUnit::GetPSDMeanSurfaces() const
{
	if (!m_pDistributionsGrid) return {};
	std::vector<double> d = GetPSDMeanDiameters();
	std::vector<double> res(d.size());
	for (size_t i = 0; i < d.size(); ++i)
		res[i] = MATH_PI * std::pow(d[i], 2);
	return res;
}

std::vector<double> CBaseUnit::GetPSDMeanVolumes() const
{
	if (!m_pDistributionsGrid) return {};
	return m_pDistributionsGrid->GetPSDMeans(EPSDGridType::VOLUME);
}

std::vector<double> CBaseUnit::GetClassesSizes(EDistrTypes _nDistrType) const
{
	if (!m_pDistributionsGrid) return std::vector<double>();
	return m_pDistributionsGrid->GetClassSizesByDistr(_nDistrType);
}

bool CBaseUnit::IsDistributionDefined(EDistrTypes _nDistrType) const
{
	if (!m_pDistributionsGrid) return false;
	return m_pDistributionsGrid->IsDistrTypePresent(_nDistrType);
}

unsigned CBaseUnit::GetClassesNumber(EDistrTypes _nDistrType) const
{
	if (!m_pDistributionsGrid) return 0;
	return (unsigned)m_pDistributionsGrid->GetClassesByDistr(_nDistrType);
}

void CBaseUnit::SetAbsTolerance(double _dATol)
{
	m_dATol = _dATol;
}

void CBaseUnit::SetRelTolerance(double _dRTol)
{
	m_dRTol = _dRTol;
}

double CBaseUnit::GetAbsTolerance() const
{
	return m_dATol;
}

double CBaseUnit::GetRelTolerance() const
{
	return m_dRTol;
}

void CBaseUnit::SetMinimalFraction( double _dFraction )
{
	m_dMinFraction = _dFraction;
	for( unsigned i=0; i<m_vHoldupsInit.size(); ++i )
		m_vHoldupsInit[i]->SetMinimumFraction( m_dMinFraction );

	for( unsigned i=0; i<m_vHoldupsWork.size(); ++i )
		m_vHoldupsWork[i]->SetMinimumFraction( m_dMinFraction );

	for( unsigned i=0; i<m_vStoreHoldupsWork.size(); ++i )
		m_vStoreHoldupsWork[i]->SetMinimumFraction( m_dMinFraction );

	for( unsigned i=0; i<m_vStreams.size(); ++i )
		m_vStreams[i]->SetMinimumFraction( m_dMinFraction );

	for( unsigned i=0; i<m_vStoreStreams.size(); ++i )
		m_vStoreStreams[i]->SetMinimumFraction( m_dMinFraction );
}

void CBaseUnit::SaveToFile(CH5Handler& _h5Saver, const std::string& _sPath)
{
	if (!_h5Saver.IsValid())
		return;

	std::string sGroupPath, sFullPath;

	// current version of save procedure
	_h5Saver.WriteAttribute(_sPath, StrConst::BUnit_H5AttrSaveVersion, m_cnSaveVersion);

	// save unit key
	_h5Saver.WriteData(_sPath, StrConst::BUnit_H5UnitKey, m_sUniqueID);

	// save unit ports
	_h5Saver.WriteAttribute(_sPath, StrConst::BUnit_H5AttrPortsNum, (int)m_vPorts.size());
	sGroupPath = _h5Saver.CreateGroup(_sPath, StrConst::BUnit_H5GroupPorts);
	std::vector<std::string> vPortsStreamNames, vPortsStreamKeys;
	for (size_t i = 0; i < m_vPorts.size(); ++i)
	{
		vPortsStreamNames.push_back(m_vPorts[i].sName);
		vPortsStreamKeys.push_back(m_vPorts[i].sStreamKey);
	}
	_h5Saver.WriteData(sGroupPath, StrConst::BUnit_H5UnitPortsNames, vPortsStreamNames);
	_h5Saver.WriteData(sGroupPath, StrConst::BUnit_H5UnitPortsKeys, vPortsStreamKeys);

	// save holdups
	_h5Saver.WriteAttribute(_sPath, StrConst::BUnit_H5AttrHoldupsNum, (int)m_vHoldupsInit.size());
	sGroupPath = _h5Saver.CreateGroup(_sPath, StrConst::BUnit_H5GroupHoldups);
	std::vector<std::string> vHoldupsNames;
	for (size_t i = 0; i < m_vHoldupsInit.size(); ++i)
	{
		sFullPath = _h5Saver.CreateGroup(sGroupPath, StrConst::BUnit_H5GroupHoldupName + std::to_string(i));
		m_vHoldupsInit[i]->SaveToFile(_h5Saver, sFullPath);
		vHoldupsNames.push_back(m_vHoldupsInit[i]->GetName());
	}
	_h5Saver.WriteData(sGroupPath, StrConst::BUnit_H5HoldupsNames, vHoldupsNames);

	// save working holdups
	_h5Saver.WriteAttribute(_sPath, StrConst::BUnit_H5AttrHoldupsWorkNum, (int)m_vHoldupsWork.size());
	sGroupPath = _h5Saver.CreateGroup(_sPath, StrConst::BUnit_H5GroupHoldupsWork);
	std::vector<std::string> vWorkHoldupsNames;
	for (size_t i = 0; i < m_vHoldupsWork.size(); ++i)
	{
		sFullPath = _h5Saver.CreateGroup(sGroupPath, StrConst::BUnit_H5GroupHoldupWorkName + std::to_string(i));
		m_vHoldupsWork[i]->SaveToFile(_h5Saver, sFullPath);
		vWorkHoldupsNames.push_back(m_vHoldupsWork[i]->GetName());
	}
	_h5Saver.WriteData(sGroupPath, StrConst::BUnit_H5WorkHoldupsNames, vWorkHoldupsNames);

	// save working material streams
	_h5Saver.WriteAttribute(_sPath, StrConst::BUnit_H5AttrStreamsWorkNum, (int)m_vStreams.size());
	sGroupPath = _h5Saver.CreateGroup(_sPath, StrConst::BUnit_H5GroupStreamsWork);
	std::vector<std::string> vStreamsNames;
	for (size_t i = 0; i < m_vStreams.size(); ++i)
	{
		sFullPath = _h5Saver.CreateGroup(sGroupPath, StrConst::BUnit_H5GroupStreamWorkName + std::to_string(i));
		m_vStreams[i]->SaveToFile(_h5Saver, sFullPath);
		vStreamsNames.push_back(m_vStreams[i]->GetName());
	}
	_h5Saver.WriteData(sGroupPath, StrConst::BUnit_H5WorkStreamsNames, vStreamsNames);

	// save unit parameters
	m_unitParameters.SaveToFile(_h5Saver, _h5Saver.CreateGroup(_sPath, StrConst::BUnit_H5GroupParams));

	// save unit state variables
	_h5Saver.WriteAttribute(_sPath, StrConst::BUnit_H5AttrStateVarsNum, (int)m_vStateVariables.size());
	sGroupPath = _h5Saver.CreateGroup(_sPath, StrConst::BUnit_H5GroupStateVars);
	for (unsigned i = 0; i < m_vStateVariables.size(); i++)
	{
		sFullPath = _h5Saver.CreateGroup(sGroupPath, StrConst::BUnit_H5GroupStateVarName + std::to_string(i));
		_h5Saver.WriteData(sFullPath, StrConst::BUnit_H5StateVarName, m_vStateVariables[i]->sName);
		_h5Saver.WriteData(sFullPath, StrConst::BUnit_H5StateVarValue, m_vStateVariables[i]->dValue);
		_h5Saver.WriteData(sFullPath, StrConst::BUnit_H5StateVarSavedVal, m_vStateVariables[i]->dSavedValue);
		_h5Saver.WriteData(sFullPath, StrConst::BUnit_H5StateVarIsSaved, m_vStateVariables[i]->bIsSaved);
		_h5Saver.WriteData(sFullPath, StrConst::BUnit_H5StateVarTimes, m_vStateVariables[i]->vHistoryTimes);
		_h5Saver.WriteData(sFullPath, StrConst::BUnit_H5StateVarValues, m_vStateVariables[i]->vHistoryValues);
	}

	// save plots
	_h5Saver.WriteAttribute(_sPath, StrConst::BUnit_H5AttrPlotsNum, (int)m_vPlots.size());
	sGroupPath = _h5Saver.CreateGroup(_sPath, StrConst::BUnit_H5GroupPlots);
	for (unsigned i = 0; i < m_vPlots.size(); i++)
	{
		sFullPath = _h5Saver.CreateGroup(sGroupPath, StrConst::BUnit_H5GroupPlotName + std::to_string(i));
		_h5Saver.WriteData(sFullPath, StrConst::BUnit_H5PlotName, m_vPlots[i]->sName);
		_h5Saver.WriteData(sFullPath, StrConst::BUnit_H5PlotXAxis, m_vPlots[i]->sXAxis);
		_h5Saver.WriteData(sFullPath, StrConst::BUnit_H5PlotYAxis, m_vPlots[i]->sYAxis);
		_h5Saver.WriteData(sFullPath, StrConst::BUnit_H5PlotZAxis, m_vPlots[i]->sZAxis);
		_h5Saver.WriteData(sFullPath, StrConst::BUnit_H5PlotIs2D, m_vPlots[i]->bIs2D);
		_h5Saver.WriteAttribute(sFullPath, StrConst::BUnit_H5AttrCurvesNum, (int)m_vPlots[i]->vCurves.size());
		std::string sSubGroupPath = _h5Saver.CreateGroup(sFullPath, StrConst::BUnit_H5GroupCurves);
		for (unsigned j = 0; j < m_vPlots[i]->vCurves.size(); ++j)
		{
			std::string sSubFullPath = _h5Saver.CreateGroup(sSubGroupPath, StrConst::BUnit_H5GroupCurveName + std::to_string(j));
			_h5Saver.WriteData(sSubFullPath, StrConst::BUnit_H5CurveName, m_vPlots[i]->vCurves[j].sName);
			_h5Saver.WriteData(sSubFullPath, StrConst::BUnit_H5CurveX, m_vPlots[i]->vCurves[j].vX);
			_h5Saver.WriteData(sSubFullPath, StrConst::BUnit_H5CurveY, m_vPlots[i]->vCurves[j].vY);
			_h5Saver.WriteData(sSubFullPath, StrConst::BUnit_H5CurveZ, m_vPlots[i]->vCurves[j].dZ);
		}
	}
}

void CBaseUnit::LoadFromFile(CH5Handler& _h5Loader, const std::string& _sPath)
{
	if (!_h5Loader.IsValid())
		return;

	// load version of save procedure
	int nVer = _h5Loader.ReadAttribute(_sPath, StrConst::BUnit_H5AttrSaveVersion);
	if (nVer < m_cnSaveVersion) // old version
	{
		LoadFromFileOldVer(_h5Loader, _sPath);
		return;
	}

	/// load unit key
	_h5Loader.ReadData(_sPath, StrConst::BUnit_H5UnitKey, m_sUniqueID);

	/// load unit ports
	{
		std::vector<std::string> vPortsNames, vPortsKeys;
		_h5Loader.ReadData(_sPath + "/" + StrConst::BUnit_H5GroupPorts, StrConst::BUnit_H5UnitPortsNames, vPortsNames);
		_h5Loader.ReadData(_sPath + "/" + StrConst::BUnit_H5GroupPorts, StrConst::BUnit_H5UnitPortsKeys, vPortsKeys);
		std::vector<bool> vFrUsed(vPortsNames.size(), false);
		std::vector<bool> vToUsed(m_vPorts.size(), false);
		for (size_t i = 0; i < m_vPorts.size(); ++i)	// load by names
		{
			for (size_t j = 0; j < vPortsNames.size(); ++j)
			{
				if (m_vPorts[i].sName == vPortsNames[j])
				{
					m_vPorts[i].sStreamKey = vPortsKeys[j];
					vFrUsed[j] = true;
					vToUsed[i] = true;
					break;
				}
			}
		}
		for (size_t i = 0; i < m_vPorts.size(); ++i)	// load rest by positions
			if (!vToUsed[i] && (i < vFrUsed.size()) && !vFrUsed[i])
				m_vPorts[i].sStreamKey = vPortsKeys[i];
	}

	/// load holdups
	{
		std::vector<std::string> vHoldupsNames;
		_h5Loader.ReadData(_sPath + "/" + StrConst::BUnit_H5GroupHoldups, StrConst::BUnit_H5HoldupsNames, vHoldupsNames);
		std::vector<bool> vFrUsed(vHoldupsNames.size(), false);
		std::vector<bool> vToUsed(m_vHoldupsInit.size(), false);
		for (size_t i = 0; i < m_vHoldupsInit.size(); ++i)	// load by names
		{
			for (size_t j = 0; j < vHoldupsNames.size(); ++j)
				if (m_vHoldupsInit[i]->GetName() == vHoldupsNames[j])
				{
					std::string sHoldupPath = _sPath + "/" + StrConst::BUnit_H5GroupHoldups + "/" + StrConst::BUnit_H5GroupHoldupName + std::to_string(j);
					m_vHoldupsInit[i]->SetMaterialsDatabase(m_pMaterialsDB);
					m_vHoldupsInit[i]->SetGrid(m_pDistributionsGrid);
					m_vHoldupsInit[i]->LoadFromFile(_h5Loader, sHoldupPath);
					vFrUsed[j] = true;
					vToUsed[i] = true;
					break;
				}
		}
		for (size_t i = 0; i < m_vHoldupsInit.size(); ++i)	// load rest by positions
		{
			if (!vToUsed[i] && (i < vFrUsed.size()) && !vFrUsed[i])
			{
				std::string sHoldupPath = _sPath + "/" + StrConst::BUnit_H5GroupHoldups + "/" + StrConst::BUnit_H5GroupHoldupName + std::to_string(i);
				std::string sName = m_vHoldupsInit[i]->GetName();
				m_vHoldupsInit[i]->SetMaterialsDatabase(m_pMaterialsDB);
				m_vHoldupsInit[i]->SetGrid(m_pDistributionsGrid);
				m_vHoldupsInit[i]->LoadFromFile(_h5Loader, sHoldupPath);
				m_vHoldupsInit[i]->SetName(sName);
			}
		}
	}

	/// load working holdups
	{
		std::vector<std::string> vWorkHoldupsNames;
		_h5Loader.ReadData(_sPath + "/" + StrConst::BUnit_H5GroupHoldupsWork, StrConst::BUnit_H5WorkHoldupsNames, vWorkHoldupsNames);
		std::vector<bool> vFrUsed(vWorkHoldupsNames.size(), false);
		std::vector<bool> vToUsed(m_vHoldupsWork.size(), false);
		for (size_t i = 0; i < m_vHoldupsWork.size(); ++i)	// load by names
		{
			for (size_t j = 0; j < vWorkHoldupsNames.size(); ++j)
				if (m_vHoldupsWork[i]->GetName() == vWorkHoldupsNames[j])
				{
					std::string sWorkHoldupPath = _sPath + "/" + StrConst::BUnit_H5GroupHoldupsWork + "/" + StrConst::BUnit_H5GroupHoldupWorkName + std::to_string(j);
					m_vHoldupsWork[i]->SetMaterialsDatabase(m_pMaterialsDB);
					m_vHoldupsWork[i]->SetGrid(m_pDistributionsGrid);
					m_vHoldupsWork[i]->LoadFromFile(_h5Loader, sWorkHoldupPath);
					vFrUsed[j] = true;
					vToUsed[i] = true;
					break;
				}
		}
		for (size_t i = 0; i < m_vHoldupsWork.size(); ++i)	// load rest by positions
		{
			if (!vToUsed[i] && (i < vFrUsed.size()) && !vFrUsed[i])
			{
				std::string sWorkHoldupPath = _sPath + "/" + StrConst::BUnit_H5GroupHoldupsWork + "/" + StrConst::BUnit_H5GroupHoldupWorkName + std::to_string(i);
				std::string sName = m_vHoldupsWork[i]->GetName();
				m_vHoldupsWork[i]->SetMaterialsDatabase(m_pMaterialsDB);
				m_vHoldupsWork[i]->SetGrid(m_pDistributionsGrid);
				m_vHoldupsWork[i]->LoadFromFile(_h5Loader, sWorkHoldupPath);
				m_vHoldupsWork[i]->SetName(sName);
			}
		}
	}

	// properly setup store streams
	for (size_t i = 0; i < m_vStoreHoldupsWork.size(); ++i)
		m_vStoreHoldupsWork[i]->SetupStructure(*m_vHoldupsWork[i]);

	/// load working material streams
	{
		std::vector<std::string> vWorkStreamsNames;
		_h5Loader.ReadData(_sPath + "/" + StrConst::BUnit_H5GroupStreamsWork, StrConst::BUnit_H5WorkStreamsNames, vWorkStreamsNames);
		std::vector<bool> vFrUsed(vWorkStreamsNames.size(), false);
		std::vector<bool> vToUsed(m_vStreams.size(), false);
		for (size_t i = 0; i < m_vStreams.size(); ++i)	// load by names
		{
			for (size_t j = 0; j < vWorkStreamsNames.size(); ++j)
				if (m_vStreams[i]->GetName() == vWorkStreamsNames[j])
				{
					std::string sWorkStreamPath = _sPath + "/" + StrConst::BUnit_H5GroupStreamsWork + "/" + StrConst::BUnit_H5GroupStreamWorkName + std::to_string(j);
					m_vStreams[i]->SetMaterialsDatabase(m_pMaterialsDB);
					m_vStreams[i]->SetGrid(m_pDistributionsGrid);
					m_vStreams[i]->LoadFromFile(_h5Loader, sWorkStreamPath);
					vFrUsed[j] = true;
					vToUsed[i] = true;
					break;
				}
		}
		for (size_t i = 0; i < m_vStreams.size(); ++i)	// load rest by positions
		{
			if (!vToUsed[i] && (i < vFrUsed.size()) && !vFrUsed[i])
			{
				std::string sWorkStreamPath = _sPath + "/" + StrConst::BUnit_H5GroupStreamsWork + "/" + StrConst::BUnit_H5GroupStreamWorkName + std::to_string(i);
				std::string sName = m_vStreams[i]->GetName();
				m_vStreams[i]->SetMaterialsDatabase(m_pMaterialsDB);
				m_vStreams[i]->SetGrid(m_pDistributionsGrid);
				m_vStreams[i]->LoadFromFile(_h5Loader, sWorkStreamPath);
				m_vStreams[i]->SetName(sName);
			}
		}
	}

	/// load unit parameters
	m_unitParameters.LoadFromFile(_h5Loader, _sPath + "/" + StrConst::BUnit_H5GroupParams);

	/// load unit state variables
	{
		ClearStateVariables();
		int nStateVarsNum = _h5Loader.ReadAttribute(_sPath, StrConst::BUnit_H5AttrStateVarsNum);
		if (nStateVarsNum != -1)
		{
			for (size_t i = 0; i < static_cast<size_t>(nStateVarsNum); ++i)
			{
				std::string sSVPath = _sPath + "/" + StrConst::BUnit_H5GroupStateVars + "/" + StrConst::BUnit_H5GroupStateVarName + std::to_string(i);
				m_vStateVariables.push_back(new sStateVariable());
				_h5Loader.ReadData(sSVPath, StrConst::BUnit_H5StateVarName, m_vStateVariables[i]->sName);
				_h5Loader.ReadData(sSVPath, StrConst::BUnit_H5StateVarValue, m_vStateVariables[i]->dValue);
				_h5Loader.ReadData(sSVPath, StrConst::BUnit_H5StateVarSavedVal, m_vStateVariables[i]->dSavedValue);
				_h5Loader.ReadData(sSVPath, StrConst::BUnit_H5StateVarIsSaved, m_vStateVariables[i]->bIsSaved);
				_h5Loader.ReadData(sSVPath, StrConst::BUnit_H5StateVarTimes, m_vStateVariables[i]->vHistoryTimes);
				_h5Loader.ReadData(sSVPath, StrConst::BUnit_H5StateVarValues, m_vStateVariables[i]->vHistoryValues);
			}
		}
	}

	// load plots
	{
		ClearPlots();
		int nPlotsNum = _h5Loader.ReadAttribute(_sPath, StrConst::BUnit_H5AttrPlotsNum);
		if (nPlotsNum != -1)
		{
			std::string sPlotsPath = _sPath + "/" + StrConst::BUnit_H5GroupPlots;
			for (size_t i = 0; i < static_cast<size_t>(nPlotsNum); i++)
			{
				std::string sCurrPlotPath = sPlotsPath + "/" + StrConst::BUnit_H5GroupPlotName + std::to_string(i);
				m_vPlots.push_back(new SPlot());
				_h5Loader.ReadData(sCurrPlotPath, StrConst::BUnit_H5PlotName, m_vPlots[i]->sName);
				_h5Loader.ReadData(sCurrPlotPath, StrConst::BUnit_H5PlotXAxis, m_vPlots[i]->sXAxis);
				_h5Loader.ReadData(sCurrPlotPath, StrConst::BUnit_H5PlotYAxis, m_vPlots[i]->sYAxis);
				_h5Loader.ReadData(sCurrPlotPath, StrConst::BUnit_H5PlotZAxis, m_vPlots[i]->sZAxis);
				_h5Loader.ReadData(sCurrPlotPath, StrConst::BUnit_H5PlotIs2D, m_vPlots[i]->bIs2D);
				int nCurvesNum = _h5Loader.ReadAttribute(sCurrPlotPath, StrConst::BUnit_H5AttrCurvesNum);
				if (nCurvesNum != -1)
				{
					std::string sCurvesPath = sCurrPlotPath + "/" + StrConst::BUnit_H5GroupCurves;
					for (size_t j = 0; j < static_cast<size_t>(nCurvesNum); ++j)
					{
						std::string sCurrPlotPath = sCurvesPath + "/" + StrConst::BUnit_H5GroupCurveName + std::to_string(j);
						m_vPlots[i]->vCurves.push_back(SCurve());
						_h5Loader.ReadData(sCurrPlotPath, StrConst::BUnit_H5CurveName, m_vPlots[i]->vCurves[j].sName);
						_h5Loader.ReadData(sCurrPlotPath, StrConst::BUnit_H5CurveX, m_vPlots[i]->vCurves[j].vX);
						_h5Loader.ReadData(sCurrPlotPath, StrConst::BUnit_H5CurveY, m_vPlots[i]->vCurves[j].vY);
						_h5Loader.ReadData(sCurrPlotPath, StrConst::BUnit_H5CurveZ, m_vPlots[i]->vCurves[j].dZ);
					}
				}
			}
		}
	}
}

void CBaseUnit::LoadFromFileOldVer( CH5Handler& _h5Loader, const std::string& _sPath )
{
	if( !_h5Loader.IsValid() )
		return;

	// load version of save procedure
	int nVer = _h5Loader.ReadAttribute( _sPath, StrConst::BUnit_H5AttrSaveVersion );
	if(nVer != 1) return; // wrong version

	/// load unit key
	_h5Loader.ReadData( _sPath, StrConst::BUnit_H5UnitKey, m_sUniqueID );

	/// load unit ports
	std::vector<std::string> vsBuf;
	_h5Loader.ReadData( _sPath, StrConst::BUnit_H5UnitPorts, vsBuf );
	for( unsigned j=0; j<vsBuf.size(); ++j )
		m_vPorts[j].sStreamKey = vsBuf[j];

	/// load holdups
	for( unsigned i=0; i<m_vHoldupsInit.size(); i++ )
	{
		std::string sHoldupPath = _sPath + "/" + StrConst::BUnit_H5GroupHoldups + "/" + StrConst::BUnit_H5GroupHoldupName + std::to_string(i);
		m_vHoldupsInit[i]->SetMaterialsDatabase(m_pMaterialsDB);
		m_vHoldupsInit[i]->SetGrid(m_pDistributionsGrid);
		m_vHoldupsInit[i]->LoadFromFile( _h5Loader, sHoldupPath );
	}

	/// load working holdups
	int nWorkHoldupsNum = _h5Loader.ReadAttribute( _sPath, StrConst::BUnit_H5AttrHoldupsWorkNum );
	if( nWorkHoldupsNum > 0 )
	{
		for( int i=0; i<nWorkHoldupsNum; i++ )
		{
			std::string sHoldupWorkPath = _sPath + "/" + StrConst::BUnit_H5GroupHoldupsWork + "/" + StrConst::BUnit_H5GroupHoldupWorkName + std::to_string(i);
			m_vHoldupsWork[i]->SetMaterialsDatabase(m_pMaterialsDB);
			m_vHoldupsWork[i]->SetGrid(m_pDistributionsGrid);
			m_vHoldupsWork[i]->LoadFromFile( _h5Loader, sHoldupWorkPath );
		}
	}

	/// load working material streams
	ClearMaterialStreams();
	int nStreamsNum = _h5Loader.ReadAttribute( _sPath, StrConst::BUnit_H5AttrStreamsWorkNum );
	if( nStreamsNum > 0 )
	{
		for( int i=0; i<nStreamsNum; i++ )
		{
			AddMaterialStream( "TempName", "TempKey" );
			std::string sStreamWorkPath = _sPath + "/" + StrConst::BUnit_H5GroupStreamsWork + "/" + StrConst::BUnit_H5GroupStreamWorkName + std::to_string(i);
			m_vStreams[i]->SetMaterialsDatabase(m_pMaterialsDB);
			m_vStreams[i]->SetGrid(m_pDistributionsGrid);
			m_vStreams[i]->LoadFromFile( _h5Loader, sStreamWorkPath );
		}
	}

	/// load unit parameters
	m_unitParameters.LoadFromFile(_h5Loader, _sPath + "/" + StrConst::BUnit_H5GroupParams);

	/// load unit state variables
	ClearStateVariables();
	int nStateVarsNum = _h5Loader.ReadAttribute( _sPath, StrConst::BUnit_H5AttrStateVarsNum );
	if( nStateVarsNum != -1 )
	{
		for( unsigned i=0; i<static_cast<unsigned>(nStateVarsNum); i++ )
		{
			std::string sSVPath = _sPath + "/" + StrConst::BUnit_H5GroupStateVars + "/" + StrConst::BUnit_H5GroupStateVarName + std::to_string(i);
			m_vStateVariables.push_back( new sStateVariable() );
			_h5Loader.ReadData( sSVPath, StrConst::BUnit_H5StateVarName, m_vStateVariables[i]->sName );
			_h5Loader.ReadData( sSVPath, StrConst::BUnit_H5StateVarValue, m_vStateVariables[i]->dValue );
			_h5Loader.ReadData( sSVPath, StrConst::BUnit_H5StateVarSavedVal, m_vStateVariables[i]->dSavedValue );
			_h5Loader.ReadData( sSVPath, StrConst::BUnit_H5StateVarIsSaved, m_vStateVariables[i]->bIsSaved );
			_h5Loader.ReadData( sSVPath, StrConst::BUnit_H5StateVarTimes, m_vStateVariables[i]->vHistoryTimes );
			_h5Loader.ReadData( sSVPath, StrConst::BUnit_H5StateVarValues, m_vStateVariables[i]->vHistoryValues );
		}
	}

	// load plots
	ClearPlots();
	int nPlotsNum = _h5Loader.ReadAttribute( _sPath, StrConst::BUnit_H5AttrPlotsNum );
	if(nPlotsNum != -1)
	{
		std::string sPlotsPath = _sPath + "/" + StrConst::BUnit_H5GroupPlots;
		for( unsigned i=0; i<static_cast<unsigned>(nPlotsNum); i++ )
		{
			std::string sCurrPlotPath = sPlotsPath + "/" + StrConst::BUnit_H5GroupPlotName + std::to_string(i);
			m_vPlots.push_back(new SPlot());
			_h5Loader.ReadData( sCurrPlotPath, StrConst::BUnit_H5PlotName, m_vPlots[i]->sName );
			_h5Loader.ReadData( sCurrPlotPath, StrConst::BUnit_H5PlotXAxis, m_vPlots[i]->sXAxis );
			_h5Loader.ReadData( sCurrPlotPath, StrConst::BUnit_H5PlotYAxis, m_vPlots[i]->sYAxis );
			_h5Loader.ReadData( sCurrPlotPath, StrConst::BUnit_H5PlotZAxis, m_vPlots[i]->sZAxis );
			_h5Loader.ReadData( sCurrPlotPath, StrConst::BUnit_H5PlotIs2D, m_vPlots[i]->bIs2D );
			int nCurvesNum = _h5Loader.ReadAttribute( sCurrPlotPath, StrConst::BUnit_H5AttrCurvesNum );
			if(nCurvesNum != -1)
			{
				std::string sCurvesPath = sCurrPlotPath + "/" + StrConst::BUnit_H5GroupCurves;
				for( unsigned j=0; j<static_cast<unsigned>(nCurvesNum); ++j )
				{
					std::string sCurrPlotPath = sCurvesPath + "/" + StrConst::BUnit_H5GroupCurveName + std::to_string(j);
					m_vPlots[i]->vCurves.push_back(SCurve());
					_h5Loader.ReadData( sCurrPlotPath, StrConst::BUnit_H5CurveName, m_vPlots[i]->vCurves[j].sName );
					_h5Loader.ReadData( sCurrPlotPath, StrConst::BUnit_H5CurveX, m_vPlots[i]->vCurves[j].vX );
					_h5Loader.ReadData( sCurrPlotPath, StrConst::BUnit_H5CurveY, m_vPlots[i]->vCurves[j].vY );
					_h5Loader.ReadData( sCurrPlotPath, StrConst::BUnit_H5CurveZ, m_vPlots[i]->vCurves[j].dZ );
				}
			}
		}
	}
}

bool CBaseUnit::CheckError() const
{
	return m_bError;
}

bool CBaseUnit::CheckWarning() const
{
	return m_bWarning;
}

bool CBaseUnit::CheckInfo() const
{
	return m_bInfo;
}

std::string CBaseUnit::GetErrorDescription() const
{
	return m_sErrorDescription;
}

std::string CBaseUnit::GetWarningDescription() const
{
	return m_sWarningDescription;
}

std::string CBaseUnit::GetInfoDescription() const
{
	return m_sInfoDescription;
}

void CBaseUnit::RaiseError(const std::string &_sDescription /*= "" */)
{
	m_bError = true;
	if( !_sDescription.empty() )
		m_sErrorDescription = _sDescription;
	else
		m_sErrorDescription = StrConst::BUnit_UnknownError;
}

void CBaseUnit::RaiseWarning(const std::string& _sDescription /*= "" */)
{
	m_bWarning = true;
	std::string sTemp;
	if (!_sDescription.empty())
		sTemp = _sDescription;
	else
		sTemp = StrConst::BUnit_UnknownWarning;

	if (m_sWarningDescription.empty())
		m_sWarningDescription = sTemp;
	else
		m_sWarningDescription += ("\n" + sTemp);
}

void CBaseUnit::ShowInfo(const std::string& _sDescription)
{
	if (_sDescription.empty())
		return;

	m_bInfo = true;
	std::string sTemp = _sDescription;

	if (m_sInfoDescription.empty())
		m_sInfoDescription = sTemp;
	else
		m_sInfoDescription += ("\n" + sTemp);
}

void CBaseUnit::ClearError()
{
	m_bError = false;
	m_sErrorDescription.clear();
}

void CBaseUnit::ClearWarning()
{
	m_bWarning = false;
	m_sWarningDescription.clear();
}

void CBaseUnit::ClearInfo()
{
	m_bInfo = false;
	m_sInfoDescription.clear();
}

void CBaseUnit::SetDynamicUnit(bool _bIsDynamic)
{
	m_bIsDynamic = _bIsDynamic;
}

void CBaseUnit::InitializeUnit(double _dTime)
{
	m_bError = false;
	m_bWarning = false;
	ClearStateVariables();
	ClearPlots();
	m_nPermanentHoldups = (int)m_vHoldupsInit.size();
	m_nPermanentStreams = (int)m_vStreams.size();
	InitializeHoldups();
	InitializeMaterialStreams();
	InitializeExternalSolvers();
	Initialize(_dTime);
	SaveStateUnit(_dTime);
}

void CBaseUnit::FinalizeUnit()
{
	Finalize();
	FinalizeExternalSolvers();
	RemoveTempHoldups();
	RemoveTempMaterialStreams();
}

void CBaseUnit::SaveStateUnit(double _dT1, double _dT2 /*= -1*/)
{
	// call internal saving procedure of unit
	SaveState();
	// call saving procedures for solvers
	for (auto& s : m_vExternalSolvers)
		s->SaveState();
	// save state variables
	for (auto& v : m_vStateVariables)
		v->dSavedValue = v->dValue;
	// save material streams and holdups
	if(_dT2 != -1)
	{
		for (size_t i = 0; i < m_vStreams.size(); ++i)
			m_vStoreStreams[i]->CopyFromStream(_dT1, _dT2, m_vStreams[i]);
		for (size_t i = 0; i < m_vStoreHoldupsWork.size(); ++i)
			m_vStoreHoldupsWork[i]->CopyFromHoldup(_dT1, _dT2, m_vHoldupsWork[i]);
	}
	else
	{
		for (size_t i = 0; i < m_vStreams.size(); ++i)
			m_vStoreStreams[i]->CopyFromStream(_dT1, m_vStreams[i]->GetLastTimePoint(), m_vStreams[i]);
		for (size_t i = 0; i < m_vStoreHoldupsWork.size(); ++i)
			m_vStoreHoldupsWork[i]->CopyFromHoldup(_dT1, m_vHoldupsWork[i]->GetLastTimePoint(), m_vHoldupsWork[i]);
	}
	// save time points
	m_dStoreT1 = _dT1;
	m_dStoreT2 = _dT2;
	// save state of plots
	SavePlots();
}

void CBaseUnit::LoadStateUnit()
{
	// call internal loading procedure of unit
	LoadState();
	// call loading procedures for solvers
	for (auto& s : m_vExternalSolvers)
		s->LoadState();
	// load state variables
	for(auto& v : m_vStateVariables)
		v->dValue = v->dSavedValue;
	// load material streams and holdups
	if (m_dStoreT2 != -1)
	{
		for (size_t i = 0; i < m_vStreams.size(); ++i)
			m_vStreams[i]->CopyFromStream(m_dStoreT1, m_dStoreT2, m_vStoreStreams[i]);
		for (size_t i = 0; i < m_vHoldupsWork.size(); ++i)
			m_vHoldupsWork[i]->CopyFromHoldup(m_dStoreT1, m_dStoreT2, m_vStoreHoldupsWork[i]);
	}
	else
	{
		for (size_t i = 0; i < m_vStreams.size(); ++i)
			m_vStreams[i]->CopyFromStream(m_dStoreT1, m_vStoreStreams[i]->GetLastTimePoint(), m_vStoreStreams[i]);
		for (size_t i = 0; i < m_vHoldupsWork.size(); ++i)
			m_vHoldupsWork[i]->CopyFromHoldup(m_dStoreT1, m_vStoreHoldupsWork[i]->GetLastTimePoint(), m_vStoreHoldupsWork[i]);
	}
	// load state of plots
	LoadPlots();
}

bool CBaseUnit::IsDynamicUnit() const
{
	return m_bIsDynamic;
}

void CBaseUnit::CalculateTM( EDistrTypes _nDistrType, std::vector<double> _vInDistr, std::vector<double> _vOutDistr, CTransformMatrix &_outTM )
{
	if( _vInDistr.size() != _vOutDistr.size() )	return;
	if( _vInDistr.size() == 0 )	return;

	// normalize
	double dInSum = std::accumulate( _vInDistr.begin(), _vInDistr.end(), .0 );
	double dOutSum = std::accumulate( _vOutDistr.begin(), _vOutDistr.end(), .0 );
	if( dInSum == 0 || dOutSum == 0 ) return;
	if( dInSum != 1 || dOutSum != 1 )
		for( unsigned i=0; i<_vInDistr.size(); ++i )
		{
			_vInDistr[i] /= dInSum;
			_vOutDistr[i] /= dOutSum;
		}

	_outTM.SetDimensions( _nDistrType, (unsigned)_vInDistr.size() );
	unsigned iIn = 0;
	unsigned iOut = 0;
	double dInVal = _vInDistr.front();
	double dInInit = _vInDistr.front();
	double dOutVal = _vOutDistr.front();
	double dTrVal = 0;
	bool bInReady = false;
	bool bOutReady = false;

	while( !bInReady && !bOutReady )
	{
		if( dInVal >= dOutVal )
		{
			if( dInVal == dInInit )
			{
				if (dInVal != 0)
					dTrVal = dOutVal/dInVal;
				else
					dTrVal = 0;
				dInVal -= dOutVal;
				dOutVal = 0;
			}
			else
			{
				if (dInInit != 0)
					dTrVal = dOutVal / dInInit;
				else
					dTrVal = 0;
				dInVal -= dOutVal;
				dOutVal = 0;
			}
		}
		else
		{
			if( dInInit != 0 )
				dTrVal = dInVal/dInInit;
			else
				dTrVal = 0;
			dOutVal -= dInVal;
			dInVal = 0;
		}

		_outTM.SetValue( iIn, iOut, dTrVal );

		if( dOutVal == 0 )
		{
			iOut++;
			if( iOut < _vOutDistr.size() )
				dOutVal = _vOutDistr[iOut];
			else
				bOutReady = true;
		}

		if( dInVal == 0 )
		{
			iIn++;
			if( iIn < _vInDistr.size() )
				dInVal = dInInit = _vInDistr[iIn];
			else
				bInReady = true;
		}
	}
}

void CBaseUnit::SetCachePath(const std::wstring& _sPath)
{
	m_sCachePath = _sPath;

	for( unsigned i=0; i<m_vHoldupsInit.size(); ++i )
		m_vHoldupsInit[i]->SetCacheSettings({m_bCacheEnabled, m_nCacheWindow, m_sCachePath});

	for( unsigned i=0; i<m_vHoldupsWork.size(); ++i )
		m_vHoldupsWork[i]->SetCacheSettings({ m_bCacheEnabled, m_nCacheWindow, m_sCachePath });

	for( unsigned i=0; i<m_vStoreHoldupsWork.size(); ++i )
		m_vStoreHoldupsWork[i]->SetCacheSettings({ m_bCacheEnabled, m_nCacheWindow, m_sCachePath });

	for( unsigned i=0; i<m_vStreams.size(); ++i )
		m_vStreams[i]->SetCacheSettings({ m_bCacheEnabled, m_nCacheWindow, m_sCachePath });

	for( unsigned i=0; i<m_vStoreStreams.size(); ++i )
		m_vStoreStreams[i]->SetCacheSettings({ m_bCacheEnabled, m_nCacheWindow, m_sCachePath });
}

void CBaseUnit::SetCacheParams( bool _bEnabled, unsigned _nWindow )
{
	m_bCacheEnabled = _bEnabled;
	m_nCacheWindow = _nWindow;

	for( unsigned i=0; i<m_vHoldupsInit.size(); ++i )
		m_vHoldupsInit[i]->SetCacheSettings({ m_bCacheEnabled, m_nCacheWindow, m_sCachePath });

	for( unsigned i=0; i<m_vHoldupsWork.size(); ++i )
		m_vHoldupsWork[i]->SetCacheSettings({ m_bCacheEnabled, m_nCacheWindow, m_sCachePath });

	for( unsigned i=0; i<m_vStoreHoldupsWork.size(); ++i )
		m_vStoreHoldupsWork[i]->SetCacheSettings({ m_bCacheEnabled, m_nCacheWindow, m_sCachePath });

	for( unsigned i=0; i<m_vStreams.size(); ++i )
		m_vStreams[i]->SetCacheSettings({ m_bCacheEnabled, m_nCacheWindow, m_sCachePath });

	for( unsigned i=0; i<m_vStoreStreams.size(); ++i )
		m_vStoreStreams[i]->SetCacheSettings({ m_bCacheEnabled, m_nCacheWindow, m_sCachePath });
}

void CBaseUnit::ClearSimulationResults()
{
	RemoveTempHoldups();
	RemoveTempMaterialStreams();

	for( unsigned i=0; i<m_vHoldupsWork.size(); ++i )
		m_vHoldupsWork[i]->RemoveAllTimePoints();

	for( unsigned i=0; i<m_vStoreHoldupsWork.size(); ++i )
		m_vStoreHoldupsWork[i]->RemoveAllTimePoints();

	for( unsigned i=0; i<m_vStreams.size(); ++i )
		m_vStreams[i]->RemoveAllTimePoints();

	for( unsigned i=0; i<m_vStoreStreams.size(); ++i )
		m_vStoreStreams[i]->RemoveAllTimePoints();

	ClearStateVariables();
	ClearPlots();
}

int CBaseUnit::p_AddPlot(const std::string& _sPlotName, const std::string& _sXAxisName, const std::string& _sYAxisName, const std::string& _sZAxisName, bool _bIs2D)
{
	for (size_t i = 0; i < m_vPlots.size(); ++i)
		if (m_vPlots[i]->sName == _sPlotName)
			if((m_vPlots[i]->sXAxis == _sXAxisName) && (m_vPlots[i]->sYAxis == _sYAxisName) && (m_vPlots[i]->sZAxis == _sZAxisName))
				return (int)i;
			else
				throw std::logic_error(StrConst::BUnit_ErrAddPlot(m_sUnitName, _sPlotName));

	SPlot* pPlot = new SPlot();
	pPlot->sName = _sPlotName;
	pPlot->sXAxis = _sXAxisName;
	pPlot->sYAxis = _sYAxisName;
	pPlot->sZAxis = _sZAxisName;
	pPlot->bIs2D = _bIs2D;
	m_vPlots.push_back(pPlot);

	SPlot* pStorePlot = new SPlot();
	m_vStorePlots.push_back(pStorePlot);

	return (int)m_vPlots.size() - 1;
}

int CBaseUnit::AddPlot(const std::string& _sPlotName, const std::string& _sXAxisName, const std::string& _sYAxisName)
{
	return p_AddPlot(_sPlotName, _sXAxisName, _sYAxisName, "", true);
}

int CBaseUnit::AddPlot(const std::string& _sPlotName, const std::string& _sXAxisName, const std::string& _sYAxisName, const std::string& _sZAxisName)
{
	return p_AddPlot(_sPlotName, _sXAxisName, _sYAxisName, _sZAxisName, false);
}

int CBaseUnit::AddCurveOnPlot(unsigned _nPlotIndex, const std::string& _sCurveName)
{
	if (_nPlotIndex >= m_vPlots.size()) return -1;
	if (!m_vPlots[_nPlotIndex]->bIs2D)
	{
		RaiseWarning(StrConst::BUnit_WarnAddCurve(m_sUnitName, m_vPlots[_nPlotIndex]->sName, _sCurveName));
		return -1;
	}
	m_vPlots[_nPlotIndex]->vCurves.push_back(SCurve());
	m_vPlots[_nPlotIndex]->vCurves.back().sName = _sCurveName;
	return (int)m_vPlots[_nPlotIndex]->vCurves.size() - 1;
}

int CBaseUnit::AddCurveOnPlot(const std::string& _sPlotName, const std::string& _sCurveName)
{
	for (unsigned i = 0; i < (unsigned)m_vPlots.size(); ++i)
		if (m_vPlots[i]->sName == _sPlotName)
			return AddCurveOnPlot(i, _sCurveName);
	throw std::logic_error(StrConst::BUnit_ErrAddCurve(m_sUnitName, _sPlotName, _sCurveName));
	return -1;
}

int CBaseUnit::AddCurveOnPlot(unsigned _nPlotIndex, double _dZValue)
{
	if (_nPlotIndex >= m_vPlots.size()) return -1;
	m_vPlots[_nPlotIndex]->vCurves.push_back(SCurve());
	m_vPlots[_nPlotIndex]->vCurves.back().sName = m_vPlots[_nPlotIndex]->sZAxis + ":" + StringFunctions::Double2String(_dZValue);
	m_vPlots[_nPlotIndex]->vCurves.back().dZ = _dZValue;
	return (int)m_vPlots[_nPlotIndex]->vCurves.size() - 1;
}

int CBaseUnit::AddCurveOnPlot(const std::string& _sPlotName, double _dZValue)
{
	for (unsigned i = 0; i < (unsigned)m_vPlots.size(); ++i)
		if (m_vPlots[i]->sName == _sPlotName)
			return AddCurveOnPlot(i, _dZValue);
	throw std::logic_error(StrConst::BUnit_ErrAddCurve(m_sUnitName, _sPlotName, StringFunctions::Double2String(_dZValue)));
	return -1;
}

void CBaseUnit::AddPointOnCurve(unsigned _nPlotIndex, unsigned _nCurveIndex, double _dX, double _dY)
{
	if( _nPlotIndex >= m_vPlots.size() ) return;
	if( _nCurveIndex >= m_vPlots[_nPlotIndex]->vCurves.size() ) return;
	m_vPlots[_nPlotIndex]->vCurves[_nCurveIndex].vX.push_back(_dX);
	m_vPlots[_nPlotIndex]->vCurves[_nCurveIndex].vY.push_back(_dY);
}

void CBaseUnit::AddPointOnCurve(const std::string& _sPlotName, const std::string& _sCurveName, double _dX, double _dY)
{
	for(unsigned i=0; i<(unsigned)m_vPlots.size(); ++i)
		if((m_vPlots[i]->sName == _sPlotName) && (m_vPlots[i]->bIs2D))
			for(unsigned j=0; j<(unsigned)m_vPlots[i]->vCurves.size(); ++j)
				if(m_vPlots[i]->vCurves[j].sName == _sCurveName)
				{	AddPointOnCurve(i, j, _dX, _dY); return; }
	throw std::logic_error(StrConst::BUnit_ErrAddPoint(m_sUnitName, _sPlotName, _sCurveName, StringFunctions::Double2String(_dX), StringFunctions::Double2String(_dY)));
}

void CBaseUnit::AddPointOnCurve(const std::string& _sPlotName, double _dZValue, double _dX, double _dY)
{
	for(unsigned i=0; i<(unsigned)m_vPlots.size(); ++i)
		if((m_vPlots[i]->sName == _sPlotName) && (!m_vPlots[i]->bIs2D))
			for(unsigned j=0; j<(unsigned)m_vPlots[i]->vCurves.size(); ++j)
				if(m_vPlots[i]->vCurves[j].dZ == _dZValue)
				{	AddPointOnCurve(i, j, _dX, _dY); return; }
	throw std::logic_error(StrConst::BUnit_ErrAddPoint(m_sUnitName, _sPlotName, StringFunctions::Double2String(_dZValue), StringFunctions::Double2String(_dX), StringFunctions::Double2String(_dY)));
}

void CBaseUnit::AddPointOnCurve(unsigned _nPlotIndex, unsigned _nCurveIndex, const std::vector<double>& _vX, const std::vector<double>& _vY)
{
	if(_vX.size() != _vY.size())
	{
		RaiseWarning(StrConst::BUnit_WarnAddPoint(m_sUnitName, std::to_string(_nPlotIndex), std::to_string(_nCurveIndex), "", ""));
		return;
	}
	for(size_t i=0; i<_vX.size(); ++i)
		AddPointOnCurve(_nPlotIndex, _nCurveIndex, _vX[i], _vY[i]);
}

void CBaseUnit::AddPointOnCurve(const std::string& _sPlotName, const std::string& _sCurveName, const std::vector<double>& _vX, const std::vector<double>& _vY)
{
	if(_vX.size() != _vY.size())
	{
		RaiseWarning(StrConst::BUnit_WarnAddPoint(m_sUnitName, _sPlotName, _sCurveName, "", ""));
		return;
	}
	for(size_t i=0; i<_vX.size(); ++i)
		AddPointOnCurve(_sPlotName, _sCurveName, _vX[i], _vY[i]);
}

void CBaseUnit::AddPointOnCurve(const std::string& _sPlotName, double _dZValue, const std::vector<double>& _vX, const std::vector<double>& _vY)
{
	if(_vX.size() != _vY.size())
	{
		RaiseWarning(StrConst::BUnit_WarnAddPoint(m_sUnitName, _sPlotName, StringFunctions::Double2String(_dZValue), "", ""));
		return;
	}
	for(size_t i=0; i<_vX.size(); ++i)
		AddPointOnCurve(_sPlotName, _dZValue, _vX[i], _vY[i]);
}

unsigned CBaseUnit::GetPlotsNumber() const
{
	return (unsigned)m_vPlots.size();
}

unsigned CBaseUnit::GetCurvesNumber(size_t _nPlot) const
{
	if(_nPlot < m_vPlots.size())
		return (unsigned)m_vPlots[_nPlot]->vCurves.size();
	else
		return 0;
}

std::string CBaseUnit::GetPlotName(size_t _nPlot) const
{
	if(_nPlot < m_vPlots.size())
		return m_vPlots[_nPlot]->sName;
	return "";
}

std::string CBaseUnit::GetPlotXAxisName(unsigned _nPlot) const
{
	if(_nPlot < m_vPlots.size())
		return m_vPlots[_nPlot]->sXAxis;
	return "";
}

std::string CBaseUnit::GetPlotYAxisName(unsigned _nPlot) const
{
	if(_nPlot < m_vPlots.size())
		return m_vPlots[_nPlot]->sYAxis;
	return "";
}

std::string CBaseUnit::GetPlotZAxisName(unsigned _nPlot) const
{
	if(_nPlot < m_vPlots.size())
		return m_vPlots[_nPlot]->sZAxis;
	return "";
}

std::string CBaseUnit::GetCurveName(unsigned _nPlot, unsigned _nCurve) const
{
	if((_nPlot < m_vPlots.size()) && (_nCurve < m_vPlots[_nPlot]->vCurves.size()))
		return m_vPlots[_nPlot]->vCurves[_nCurve].sName;
	return "";
}

std::vector<double> CBaseUnit::GetCurveX(unsigned _nPlot, unsigned _nCurve) const
{
	if((_nPlot < m_vPlots.size()) && (_nCurve < m_vPlots[_nPlot]->vCurves.size()))
		return m_vPlots[_nPlot]->vCurves[_nCurve].vX;
	return std::vector<double>();
}

std::vector<double> CBaseUnit::GetCurveY(unsigned _nPlot, unsigned _nCurve) const
{
	if((_nPlot < m_vPlots.size()) && (_nCurve < m_vPlots[_nPlot]->vCurves.size()))
		return m_vPlots[_nPlot]->vCurves[_nCurve].vY;
	return std::vector<double>();
}

double CBaseUnit::GetCurveZ(size_t _nPlot, size_t _nCurve) const
{
	if((_nPlot < m_vPlots.size()) && (_nCurve < m_vPlots[_nPlot]->vCurves.size()))
		return m_vPlots[_nPlot]->vCurves[_nCurve].dZ;
	return 0;
}

bool CBaseUnit::IsPlot2D(unsigned _nPlot)
{
	if(_nPlot < m_vPlots.size())
		return m_vPlots[_nPlot]->bIs2D;
	return true;
}

void CBaseUnit::ClearPlots()
{
	for(size_t i=0; i<m_vPlots.size(); ++i)
		delete m_vPlots[i];
	m_vPlots.clear();

	for(size_t i=0; i<m_vStorePlots.size(); ++i)
		delete m_vStorePlots[i];
	m_vStorePlots.clear();
}

void CBaseUnit::SavePlots()
{
	for(size_t i=0; i<m_vPlots.size(); ++i)
		*m_vStorePlots[i] = *m_vPlots[i];
}

void CBaseUnit::LoadPlots()
{
	for(size_t i=0; i<m_vStorePlots.size(); ++i)
		*m_vPlots[i] = *m_vStorePlots[i];
}

void CBaseUnit::InitializeExternalSolvers() const
{
	for (auto& s : m_vExternalSolvers) s->Initialize();
}

void CBaseUnit::FinalizeExternalSolvers() const
{
	for (auto& s : m_vExternalSolvers) s->Finalize();
}

void CBaseUnit::SetSolversPointers(const std::vector<CBaseSolver*>& _vPointers)
{
	m_vExternalSolvers = _vPointers;
}

void CBaseUnit::HeatExchange(CStream* _pStream1, CStream* _pStream2, double _dTime, double _dEfficiency)
{
	// No heat transfer if _dEfficiency bigger 1 or smaller/equal 0
	if (_dEfficiency <= 0. || _dEfficiency > 1.)
		return;

	// Mass flows and temperature of both streams
	double dMassSrc1 = _pStream1->GetMassFlow(_dTime);
	double dMassSrc2 = _pStream2->GetMassFlow(_dTime);
	double dMassSrcTot = dMassSrc1 + dMassSrc2;
	if (dMassSrcTot == 0)
		return;

	double dTemperatureSrc1 = _pStream1->GetTemperature(_dTime);
	double dTemperatureSrc2 = _pStream2->GetTemperature(_dTime);

	// Calculate enthalpy lookup tables for both streams
	CLookupTable lookupSrc1(m_pMaterialsDB, GetCompoundsList(), ENTHALPY, EDependencyTypes::DEPENDENCE_TEMP);
	std::vector<double> vCompoundFractionsSrc1 = _pStream1->GetCompoundsFractions(_dTime);
	lookupSrc1.SetCompoundFractions(vCompoundFractionsSrc1);
	CLookupTable lookupSrc2(m_pMaterialsDB, GetCompoundsList(), ENTHALPY, EDependencyTypes::DEPENDENCE_TEMP);
	std::vector<double> vCompoundFractionsSrc2 = _pStream2->GetCompoundsFractions(_dTime);
	lookupSrc2.SetCompoundFractions(vCompoundFractionsSrc2);


	// Specific enthalpy of both streams and combination
	double dEnthalpySrc1 = lookupSrc1.GetValue(dTemperatureSrc1);
	double dEnthalpySrc2 = lookupSrc2.GetValue(dTemperatureSrc2);
	double dEnthalpySrcTot = (dMassSrc1 * dEnthalpySrc1 + dMassSrc2 * dEnthalpySrc2) / dMassSrcTot;

	// Add up both enthalpy tables weighted with their respective mass fraction of total mass flow
	CLookupTable lookupMix(m_pMaterialsDB, GetCompoundsList(), ENTHALPY, EDependencyTypes::DEPENDENCE_TEMP);
	lookupMix.Add(lookupSrc1, dMassSrc1 / dMassSrcTot);
	lookupMix.Add(lookupSrc2, dMassSrc2 / dMassSrcTot);

	// Get ideal heat exchange temperature, i.e. temperature for maximum heat exchange between both streams (here: mixing temperature)
	double dTemperatureMix = lookupMix.GetParam(dEnthalpySrcTot);

	// If efficiency is 1 use ideal heat exchange temperature for both streams
	if (_dEfficiency == 1.)
	{
		// Set stream temperatures
		_pStream1->SetTemperature(_dTime, dTemperatureMix);
		_pStream2->SetTemperature(_dTime, dTemperatureMix);
		return;
	}
	// Else calculate heat transfer with respective efficiency, i.e. maximum heat exchange multiplied with efficiency
	else
	{
		// Actual heat exchange between both streams
		double dEnthalpyDst1 = _dEfficiency * dMassSrc1 * (lookupSrc1.GetValue(dTemperatureMix) - dEnthalpySrc1);
		double dEnthalpyDst2 = -dEnthalpyDst1;

		// New enthalpies of both streams
		dEnthalpyDst1 += dMassSrc1 * dEnthalpySrc1;
		dEnthalpyDst2 += dMassSrc2 * dEnthalpySrc2;

		// New specific enthalpies of both streams
		if (dMassSrc1 != 0)
			dEnthalpyDst1 /= dMassSrc1;
		if (dMassSrc2 != 0)
			dEnthalpyDst2 /= dMassSrc2;

		// Read out temperatures for new specific enthalpies from enthalpy lookup tables
		double dTemperatureDst1 = lookupSrc1.GetParam(dEnthalpyDst1);
		double dTemperatureDst2 = lookupSrc2.GetParam(dEnthalpyDst2);

		// Set stream temperatures
		_pStream1->SetTemperature(_dTime, dTemperatureDst1);
		_pStream2->SetTemperature(_dTime, dTemperatureDst2);
	}
}

bool CBaseUnit::IsDefined(ECompoundTPProperties _nProperty, EDependencyTypes _nDependenceType)
{
	return (m_vLookupTables.find(_nProperty) != m_vLookupTables.end());
}

void CBaseUnit::AddPropertyTable(ECompoundTPProperties _nProperty, EDependencyTypes _nDependenceType)
{
	if (IsDefined(_nProperty, _nDependenceType))
		return;

	m_vLookupTables.insert_or_assign(_nProperty, CLookupTable(m_pMaterialsDB, GetCompoundsList(), _nProperty, _nDependenceType));
}

double CBaseUnit::GetParamFromLookup(ECompoundTPProperties _nProperty, EDependencyTypes _nDependenceType, const std::vector<double>& _vCompoundFractions, double _dValue)
{
	if (!IsDefined(_nProperty, _nDependenceType))
		AddPropertyTable(_nProperty, _nDependenceType);

	m_vLookupTables.find(_nProperty)->second.SetCompoundFractions(_vCompoundFractions);

	return m_vLookupTables.find(_nProperty)->second.GetParam(_dValue);
}

double CBaseUnit::CalcTemperatureFromProperty(ECompoundTPProperties _nProperty, const std::vector<double>& _vCompoundFractions, double _dValue)
{
	return GetParamFromLookup(_nProperty, EDependencyTypes::DEPENDENCE_TEMP, _vCompoundFractions, _dValue);
}

double CBaseUnit::CalcPressureFromProperty(ECompoundTPProperties _nProperty, const std::vector<double>& _vCompoundFractions, double _dValue)
{
	return GetParamFromLookup(_nProperty, EDependencyTypes::DEPENDENCE_PRES, _vCompoundFractions, _dValue);
}

EPhase CBaseUnit::PhaseSOA2EPhase(unsigned _soa)
{
	switch (_soa)
	{
	case SOA_SOLID:		return EPhase::SOLID;
	case SOA_LIQUID:	return EPhase::LIQUID;
	case SOA_VAPOR:		return EPhase::VAPOR;
	default:			return EPhase::UNDEFINED;
	}
}
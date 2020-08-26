/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "BaseModel.h"
#include "DyssolStringConstants.h"

const unsigned CBaseModel::m_cnSaveVersion	= 1;

CBaseModel::CBaseModel(CModelsManager* _pModelsManager, const std::string& _sModelKey /*= ""*/):
	m_sModelKey(_sModelKey),
	m_sModelName("Unspecified"),
	m_sCachePath(L""),
	m_bCacheEnabled(DEFAULT_CACHE_FLAG_HOLDUPS),
	m_nCacheWindow(DEFAULT_CACHE_WINDOW),
	m_pModelsManager(_pModelsManager),
	m_pUnit(nullptr)
{
	if (m_sModelKey.empty())
		m_sModelKey = StringFunctions::GenerateRandomKey();
}

CBaseModel::~CBaseModel()
{
	ClearExternalSolvers();
	m_pModelsManager->FreeUnit(m_pUnit);
}

void CBaseModel::SetModelKey( const std::string &_sNewModelKey )
{
	m_sModelKey = _sNewModelKey;
}

void CBaseModel::SetModelName( std::string _sModelName )
{
	m_sModelName = _sModelName;
}

std::string CBaseModel::GetUnitKey() const
{
	if ( m_pUnit == NULL )
		return "";
	else
		return m_pUnit->GetUniqueID();
}

std::string CBaseModel::GetUnitName() const
{
	if ( m_pUnit == NULL )
		return "";
	else
		return m_pUnit->GetUnitName();
}

void CBaseModel::SetUnit(const std::string& _sUnitKey)
{
	if(m_pUnit)	m_pModelsManager->FreeUnit(m_pUnit);
	m_pUnit = m_pModelsManager->InstantiateUnit(_sUnitKey);
	if(m_pUnit)
	{
		m_pUnit->SetCachePath(m_sCachePath);
		m_pUnit->SetCacheParams(m_bCacheEnabled, m_nCacheWindow);
	}
}

const CBaseUnit* CBaseModel::GetUnit() const
{
	return m_pUnit;
}

std::vector<sPortStruct> CBaseModel::GetUnitPorts()
{
	std::vector<sPortStruct> vResultVector;
	if ( m_pUnit != NULL )
		vResultVector = m_pUnit->GetPorts();
	return vResultVector;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

size_t CBaseModel::GetHoldupsCount() const
{
	return m_pUnit ? m_pUnit->GetHoldupsInit().size() : 0;
}

const CHoldup* CBaseModel::GetHoldup(size_t _index) const
{
	if (!m_pUnit || _index >= m_pUnit->GetHoldups().size()) return nullptr;
	return m_pUnit->GetHoldups()[_index];
}

CHoldup* CBaseModel::GetHoldup(size_t _index)
{
	return const_cast<CHoldup*>(static_cast<const CBaseModel&>(*this).GetHoldup(_index));
}

const CHoldup* CBaseModel::GetHoldup(const std::string& _sKey) const
{
	for (auto& h : m_pUnit->GetHoldups())
		if (h->GetKey() == _sKey)
			return h;
	return nullptr;
}

CHoldup* CBaseModel::GetHoldup(const std::string& _sKey)
{
	return const_cast<CHoldup*>(static_cast<const CBaseModel&>(*this).GetHoldup(_sKey));
}

const CHoldup* CBaseModel::GetHoldupInit(size_t _index) const
{
	if (!m_pUnit || _index >= m_pUnit->GetHoldupsInit().size()) return nullptr;
	return m_pUnit->GetHoldupsInit()[_index];
}

CHoldup* CBaseModel::GetHoldupInit(size_t _index)
{
	return const_cast<CHoldup*>(static_cast<const CBaseModel&>(*this).GetHoldupInit(_index));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CBaseModel::ReduceTimePoints(double _dStart, double _dEnd, double _dStep)
{
	if (m_pUnit)
		m_pUnit->ReduceTimePoints(_dStart, _dEnd, _dStep);
}

void CBaseModel::SetCompounds( const std::vector<std::string>* _pvCompoundsKeys )
{
	if ( m_pUnit != NULL )
		m_pUnit->SetCompounds( _pvCompoundsKeys );
}

void CBaseModel::SetCompoundsPtr( const std::vector<std::string>* _pvCompoundsKeys )
{
	if ( m_pUnit != NULL )
		m_pUnit->SetCompoundsPtr( _pvCompoundsKeys );
}

void CBaseModel::SetPhases( const std::vector<std::string>* _pvPhasesNames, const std::vector<unsigned>* _pvPhasesSOAs )
{
	if ( m_pUnit != NULL )
		m_pUnit->SetPhases( _pvPhasesNames, _pvPhasesSOAs );
}

void CBaseModel::SetPhasesPtr( const std::vector<std::string>* _pvPhasesNames, const std::vector<unsigned>* _pvPhasesSOAs )
{
	if ( m_pUnit != NULL )
		m_pUnit->SetPhasesPtr( _pvPhasesNames, _pvPhasesSOAs );
}

void CBaseModel::SetAbsTolerance( double _dATol )
{
	if ( m_pUnit != NULL )
		m_pUnit->SetAbsTolerance( _dATol );
}

void CBaseModel::SetRelTolerance( double _dRTol )
{
	if ( m_pUnit != NULL )
		m_pUnit->SetRelTolerance( _dRTol );
}

void CBaseModel::SetDistributionsGrid( const CDistributionsGrid* _pGrid )
{
	if ( m_pUnit != NULL )
		m_pUnit->SetDistributionsGrid( _pGrid );
}

void CBaseModel::SetMaterialsDatabase( const CMaterialsDatabase* _pDatabase )
{
	if ( m_pUnit != NULL )
		m_pUnit->SetMaterialsDatabase( _pDatabase );
}

void CBaseModel::AddPhase( std::string _sName, unsigned _nAggrState )
{
	if ( m_pUnit != NULL )
		m_pUnit->AddPhase( _sName, _nAggrState );
}

void CBaseModel::RemovePhase( unsigned _nIndex )
{
	if ( m_pUnit != NULL )
		m_pUnit->RemovePhase( _nIndex );
}

void CBaseModel::ChangePhase( unsigned _nIndex, std::string _sName, unsigned _nAggrState )
{
	if ( m_pUnit )
		m_pUnit->ChangePhase( _nIndex, _sName, _nAggrState );
}

void CBaseModel::AddCompound( std::string _sCompoundKey )
{
	if ( m_pUnit )
		m_pUnit->AddCompound( _sCompoundKey );
}

void CBaseModel::RemoveCompound( std::string _sCompoundKey )
{
	if ( m_pUnit )
		m_pUnit->RemoveCompound( _sCompoundKey );
}

void CBaseModel::SetPortStreamKey( unsigned _nPortIndex, std::string _sStreamKey )
{
	if ( m_pUnit != NULL )
		m_pUnit->SetPortStreamKey( _nPortIndex, _sStreamKey );
}

void CBaseModel::SetPortStream( unsigned _nPortIndex, CStream* _pMaterialStream )
{
	if ( m_pUnit != NULL )
		m_pUnit->SetPortStream( _nPortIndex, _pMaterialStream );
}

void CBaseModel::Simulate( double _dStartTime, double _dEndTime )
{
	if ( m_pUnit == NULL ) return;
	m_pUnit->Simulate( _dStartTime, _dEndTime );
}

void CBaseModel::Simulate( double _dTime )
{
	if ( m_pUnit == NULL ) return;
	m_pUnit->Simulate( _dTime );
}

void CBaseModel::SaveInternalState(double _dT1, double _dT2)
{
	if ( m_pUnit == NULL ) return;
	m_pUnit->SaveStateUnit(_dT1, _dT2);
}

void CBaseModel::LoadInternalState()
{
	if ( m_pUnit == NULL ) return;
	m_pUnit->LoadStateUnit();
}

void CBaseModel::Initialize( double _dTime )
{
	if ( m_pUnit == NULL ) return;
	m_pUnit->InitializeUnit( _dTime );
}

void CBaseModel::Finalize()
{
	if(!m_pUnit) return;
	m_pUnit->FinalizeUnit();
}

void CBaseModel::SaveToFile( CH5Handler& _h5Saver, const std::string& _sPath )
{
	if( !_h5Saver.IsValid() )
		return;

	// current version of save procedure
	_h5Saver.WriteAttribute( _sPath, StrConst::BModel_H5AttrSaveVersion, m_cnSaveVersion );

	_h5Saver.WriteData( _sPath, StrConst::BModel_H5ModelKey, m_sModelKey );
	_h5Saver.WriteData( _sPath, StrConst::BModel_H5ModelName, m_sModelName );

	if ( m_pUnit != NULL )
	{
		_h5Saver.CreateGroup( _sPath, StrConst::BModel_H5GroupUnit );
		m_pUnit->SaveToFile( _h5Saver, _sPath + "/" + StrConst::BModel_H5GroupUnit );
	}
}

void CBaseModel::LoadFromFile( CH5Handler& _h5Loader, const std::string& _sPath )
{
	if( !_h5Loader.IsValid() )
		return;

	_h5Loader.ReadData( _sPath, StrConst::BModel_H5ModelKey, m_sModelKey );
	_h5Loader.ReadData( _sPath, StrConst::BModel_H5ModelName, m_sModelName );

	if ( m_pUnit != NULL )
	{
		m_pUnit->LoadFromFile( _h5Loader, _sPath + "/" + StrConst::BModel_H5GroupUnit );
		m_pUnit->SetCachePath( m_sCachePath );
		m_pUnit->SetCacheParams( m_bCacheEnabled, m_nCacheWindow );
	}
}

CUnitParametersManager* CBaseModel::GetUnitParametersManager() const
{
	if (!m_pUnit) return nullptr;
	return &m_pUnit->GetUnitParametersManager();
}

unsigned CBaseModel::GetStoredStateVariablesNumber()
{
	if( m_pUnit == NULL ) return 0;
	return m_pUnit->GetStoredSVNumber();
}

std::string CBaseModel::GetStoredStateVariableName( unsigned _nIndex )
{
	if( m_pUnit == NULL ) return "";
	return m_pUnit->GetStoredSVName( _nIndex );
}

bool CBaseModel::GetStoredStateVariableData( unsigned _nIndex, std::vector<double>* _pvValues, std::vector<double>* _pvTimes )
{
	if( m_pUnit == NULL ) return false;
	return m_pUnit->GetStoredSVData( _nIndex, *_pvValues, *_pvTimes );
}

bool CBaseModel::IsDynamic()
{
	if( m_pUnit == NULL ) return 0;
	return m_pUnit->IsDynamicUnit();
}

std::vector<double> CBaseModel::GetAllInletTimePoints( double _dStartTime, double _dEndTime, bool _bForceStartBoundary /*= false*/, bool _bForceEndBoundary /*= false*/ )
{
	if( m_pUnit == NULL ) return std::vector<double>();
	return m_pUnit->GetAllDefinedTimePoints( _dStartTime, _dEndTime, _bForceStartBoundary, _bForceEndBoundary );
}

void CBaseModel::ClearSimulationResults()
{
	if( !m_pUnit ) return;
		m_pUnit->ClearSimulationResults();
}

bool CBaseModel::CheckError()
{
	if( m_pUnit == NULL ) return false;
	return m_pUnit->CheckError();
}

std::string CBaseModel::GetErrorDescription()
{
	if( m_pUnit == NULL ) return "";
	return m_pUnit->GetErrorDescription();
}

void CBaseModel::ClearError()
{
	if( m_pUnit )
		m_pUnit->ClearError();
}

bool CBaseModel::CheckWarning()
{
	if( m_pUnit == NULL ) return false;
	return m_pUnit->CheckWarning();
}

std::string CBaseModel::GetWarningDescription()
{
	if( m_pUnit == NULL ) return "";
	return m_pUnit->GetWarningDescription();
}

void CBaseModel::ClearWarning()
{
	if( m_pUnit )
		m_pUnit->ClearWarning();
}

bool CBaseModel::CheckInfo()
{
	if (m_pUnit == NULL) return false;
	return m_pUnit->CheckInfo();
}

std::string CBaseModel::GetInfoDescription()
{
	if (m_pUnit == NULL) return "";
	return m_pUnit->GetInfoDescription();
}

void CBaseModel::ClearInfo()
{
	if (m_pUnit)
		m_pUnit->ClearInfo();
}

void CBaseModel::SetCachePath(const std::wstring& _sPath)
{
	m_sCachePath = _sPath;
	if( m_pUnit )
		m_pUnit->SetCachePath( m_sCachePath );
}

void CBaseModel::SetCacheParams( bool _bEnabled, unsigned _nWindow )
{
	m_bCacheEnabled = _bEnabled;
	m_nCacheWindow = _nWindow;
	if( m_pUnit )
		m_pUnit->SetCacheParams( _bEnabled, _nWindow );
}

void CBaseModel::SetMinimalFraction( double _dFraction )
{
	if( m_pUnit )
		m_pUnit->SetMinimalFraction( _dFraction );
}

unsigned CBaseModel::GetPlotsNumber() const
{
	if( m_pUnit )
		return m_pUnit->GetPlotsNumber();
	return 0;
}

unsigned CBaseModel::GetCurvesNumber(unsigned _nPlot) const
{
	if( m_pUnit )
		return m_pUnit->GetCurvesNumber( _nPlot );
	return 0;
}

std::string CBaseModel::GetPlotName(unsigned _nPlot) const
{
	if( m_pUnit )
		return m_pUnit->GetPlotName( _nPlot );
	return "";
}

std::string CBaseModel::GetPlotXAxisName(unsigned _nPlot) const
{
	if( m_pUnit )
		return m_pUnit->GetPlotXAxisName( _nPlot );
	return "";
}

std::string CBaseModel::GetPlotYAxisName(unsigned _nPlot) const
{
	if( m_pUnit )
		return m_pUnit->GetPlotYAxisName( _nPlot );
	return "";
}

std::string CBaseModel::GetPlotZAxisName(unsigned _nPlot) const
{
	if( m_pUnit )
		return m_pUnit->GetPlotZAxisName( _nPlot );
	return "";
}

std::string CBaseModel::GetCurveName(unsigned _nPlot, unsigned _nCurve) const
{
	if( m_pUnit )
		return m_pUnit->GetCurveName( _nPlot, _nCurve );
	return "";
}

std::vector<double> CBaseModel::GetCurveX(unsigned _nPlot, unsigned _nCurve) const
{
	if( m_pUnit )
		return m_pUnit->GetCurveX( _nPlot, _nCurve );
	return std::vector<double>();
}

std::vector<double> CBaseModel::GetCurveY(unsigned _nPlot, unsigned _nCurve) const
{
	if( m_pUnit )
		return m_pUnit->GetCurveY( _nPlot, _nCurve );
	return std::vector<double>();
}

double CBaseModel::GetCurveZ(unsigned _nPlot, unsigned _nCurve) const
{
	if( m_pUnit )
		return m_pUnit->GetCurveZ( _nPlot, _nCurve );
	return 0;
}

bool CBaseModel::IsPlot2D(unsigned _nPlot)
{
	if( m_pUnit )
		return m_pUnit->IsPlot2D( _nPlot );
	return true;
}

std::string CBaseModel::InitializeExternalSolvers()
{
	if (!m_pUnit) return StrConst::Flow_ErrEmptyUnit(m_sModelName);

	ClearExternalSolvers();

	for (const auto s : m_pUnit->GetUnitParametersManager().GetAllSolverParameters())	// for all solver parameters
	{
		if (s->GetKey().empty())
		{
			ClearExternalSolvers();
			return StrConst::Flow_ErrSolverKeyEmpty(s->GetName(), m_sModelName);
		}

		CExternalSolver* pSolver = m_pModelsManager->InstantiateSolver(s->GetKey());
		if (!pSolver)
		{
			ClearExternalSolvers();
			return StrConst::Flow_ErrCannotLoadSolverLib(s->GetName(), m_sModelName);
		}

		m_vExternalSolvers.push_back(pSolver);
	}

	m_pUnit->SetSolversPointers(m_vExternalSolvers);

	return "";
}

void CBaseModel::ClearExternalSolvers()
{
	for (auto& s : m_vExternalSolvers)
		m_pModelsManager->FreeSolver(s);
	m_vExternalSolvers.clear();
}

/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "DAESolver.h"
#include <ida/ida.h>
#include <ida/ida_ls_impl.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <nvector/nvector_serial.h>
#include <cstring>

#ifdef _MSC_VER
#else
// To get rid of ErrorHandler() function, which must take char*  as a parameter.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

CDAESolver::CDAESolver():
	m_dMaxStep(0),
	m_pIDAmem(nullptr),
	m_pModel(nullptr),
	m_dLastTime(0),
	m_vectorVars(nullptr),
	m_vectorDers(nullptr),
	m_vectorATols(nullptr),
	m_vectorId(nullptr),
	m_pStoreIDAmem(nullptr),
	m_StoreVectorVars(nullptr),
	m_StoreVectorDers(nullptr),
	m_sErrorDescription(""),
	m_nMaxIter(500)
{
}

CDAESolver::~CDAESolver()
{
	ClearMemory();
}

void CDAESolver::SetSolverMaxIter(size_t _nMaxIter)
{
	m_nMaxIter = _nMaxIter;
}

size_t CDAESolver::GetSolverMaxIter()
{
	return m_nMaxIter;
}

bool CDAESolver::SetModel( CDAEModel* _pModel )
{
	ClearMemory();

	m_pModel = _pModel;

	// Create IDA memory
	m_pIDAmem = IDACreate();
	if( m_pIDAmem == NULL )
	{
		ErrorHandler(-1, "IDA", "IDACreate", "Cannot allocate memory for solver.", &m_sErrorDescription);
		return false;
	}
	if( IDASetErrHandlerFn( m_pIDAmem, &CDAESolver::ErrorHandler, &m_sErrorDescription ) != IDA_SUCCESS )
	{
		ErrorHandler( -1, "IDA", "IDASetErrHandlerFn", "Cannot setup error handler function.", &m_sErrorDescription );
		return false;
	}

	const sunindextype nVarsCnt = static_cast<sunindextype>(_pModel->GetVariablesNumber());

	// Allocate N-vectors
	m_vectorVars  = N_VNew_Serial(nVarsCnt);
	m_vectorDers  = N_VNew_Serial(nVarsCnt);
	m_vectorATols = N_VNew_Serial(nVarsCnt);
	m_vectorId    = N_VNew_Serial(nVarsCnt);

	if (!m_vectorVars || !m_vectorDers || !m_vectorATols || !m_vectorId)
	{
		ErrorHandler( -1, "IDA", "N_VNew_Serial", "Cannot allocate memory for solver.", &m_sErrorDescription );
		return false;
	}

	// Create and initialize  y, y', tolerances, states
	for (size_t i = 0; i < static_cast<size_t>(nVarsCnt); ++i)
	{
		NV_DATA_S(m_vectorVars)[i]  = _pModel->GetVarInitValue(i);	// values of variables
		NV_DATA_S(m_vectorDers)[i]  = _pModel->GetDerInitValue(i);	// values of derivatives
		NV_DATA_S(m_vectorATols)[i] = _pModel->GetATol(i);			// values of absolute tolerances
		NV_DATA_S(m_vectorId)[i]    = _pModel->GetVarType(i);		// values of states
	}

	// Setup states of variables
	if( IDASetId( m_pIDAmem, m_vectorId ) != IDA_SUCCESS )
		return false;

	// Set constraints
	N_Vector vConstrVars = N_VNew_Serial(nVarsCnt);
	bool bAllZero = true;
	for (size_t i = 0; i < static_cast<size_t>(nVarsCnt); ++i)
	{
		NV_DATA_S(vConstrVars)[i] = _pModel->GetConstraintValue(i);
		if (NV_DATA_S(vConstrVars)[i] != 0)
			bAllZero = false;
	}
	if (!bAllZero)
		if (IDASetConstraints(m_pIDAmem, vConstrVars) != IDA_SUCCESS)
		{
			N_VDestroy_Serial(vConstrVars);
			return false;
		}
	N_VDestroy_Serial(vConstrVars);

	// Set solver constants
	IDASetMaxNumSteps(m_pIDAmem, static_cast<long>(m_nMaxIter));

	// Initialize IDA memory
	if( IDAInit( m_pIDAmem, &CDAESolver::ResidualFunction, ZERO, m_vectorVars, m_vectorDers ) != IDA_SUCCESS )
		return false;

	// Set model as user data
	if( IDASetUserData( m_pIDAmem, _pModel ) != IDA_SUCCESS )
		return false;

	// Set tolerances
	if( IDASVtolerances( m_pIDAmem, _pModel->GetRTol(), m_vectorATols ) != IDA_SUCCESS )
		return false;

	// Set linear solver
	/* Create dense SUNMatrix for use in linear solves */
	m_A = SUNDenseMatrix(nVarsCnt, nVarsCnt);
	/* Create dense SUNMatrix for use in linear solves */
	m_LS = SUNLinSol_Dense(m_vectorVars, m_A);
	/* Attach the matrix and linear solver */
	if (IDASetLinearSolver(m_pIDAmem, m_LS, m_A) != IDALS_SUCCESS)
		return false;

	if( !InitStoringMemory() )
		return false;

	SaveState();

	return true;
}

bool CDAESolver::Calculate( realtype _dStartTime, realtype _dEndTime )
{
	if( _dStartTime == _dEndTime )
	{
		ErrorHandler( -1, "CDAESolver", "Calculate", "Start and end time are equal. Cannot perform calculations for dynamic unit.", &m_sErrorDescription );
		return false;
	}

	if( _dStartTime == 0 )
	{
		if( IDACalcIC( m_pIDAmem, IDA_YA_YDP_INIT, 0.001 ) != IDA_SUCCESS )
			return false;
		N_Vector vConsistVars, vConsistDers;
		vConsistVars = N_VNew_Serial( static_cast<sunindextype>(m_pModel->GetVariablesNumber()) );
		vConsistDers = N_VNew_Serial( static_cast<sunindextype>(m_pModel->GetVariablesNumber()) );
		if( IDAGetConsistentIC( m_pIDAmem, vConsistVars, vConsistDers ) != IDA_SUCCESS )
			return false;
		m_pModel->HandleResults( 0, NV_DATA_S( vConsistVars ), NV_DATA_S( vConsistDers ) );
		N_VDestroy_Serial( vConsistVars );
		N_VDestroy_Serial( vConsistDers );
	}

	realtype dStep = (_dEndTime - _dStartTime) / 2.0;
	if((m_dMaxStep != 0) && (dStep > m_dMaxStep))
		dStep = m_dMaxStep;
	if( IDASetMaxStep( m_pIDAmem, RCONST( dStep ) ) != IDA_SUCCESS )
		return false;

	if( IDASetStopTime( m_pIDAmem, RCONST(_dEndTime) ) != IDA_SUCCESS )
		return false;

	int bRes;
	do
	{
		bRes = IDASolve( m_pIDAmem, _dEndTime, &m_dLastTime, m_vectorVars, m_vectorDers, IDA_ONE_STEP );
		if( bRes < 0 )
			return false;
		m_pModel->HandleResults( m_dLastTime, NV_DATA_S( m_vectorVars ), NV_DATA_S( m_vectorDers ) );
	}
	while( bRes != IDA_TSTOP_RETURN );

	return true;
}

bool CDAESolver::Calculate( realtype _dTime )
{
	if( _dTime == 0 )
	{
		if( IDACalcIC( m_pIDAmem, IDA_YA_YDP_INIT, 0.001 ) != IDA_SUCCESS )
			return false;
		m_pModel->HandleResults( 0, NV_DATA_S( m_vectorVars ), NV_DATA_S( m_vectorDers ) );
	}
	else
	{
		if( IDASetStopTime( m_pIDAmem, RCONST(_dTime) ) != IDA_SUCCESS )
			return false;

		int bRes;
		do
		{
			bRes = IDASolve( m_pIDAmem, _dTime, &m_dLastTime, m_vectorVars, m_vectorDers, IDA_ONE_STEP );
			if( bRes < 0 )
				return false;
		}
		while( bRes != IDA_TSTOP_RETURN );
		m_pModel->HandleResults( m_dLastTime, NV_DATA_S( m_vectorVars ), NV_DATA_S( m_vectorDers ) );
	}

	return true;
}

void CDAESolver::SaveState()
{
	CopyIDAmem( m_pStoreIDAmem, m_pIDAmem );
	CopyNVector( m_StoreVectorVars, m_vectorVars );
	CopyNVector( m_StoreVectorDers, m_vectorDers );
}

void CDAESolver::LoadState()
{
	CopyIDAmem( m_pIDAmem, m_pStoreIDAmem );
	CopyNVector( m_vectorVars, m_StoreVectorVars );
	CopyNVector( m_vectorDers, m_StoreVectorDers );
}

std::string CDAESolver::GetError()
{
	return m_sErrorDescription;
}

bool CDAESolver::SetMaxStep( double _dStep )
{
	m_dMaxStep = _dStep;
	if( IDASetMaxStep( m_pIDAmem, RCONST( m_dMaxStep ) ) != IDA_SUCCESS )
		return false;
	return true;
}

int CDAESolver::ResidualFunction( realtype _dTime, N_Vector _value, N_Vector _deriv, N_Vector _res, void *_pModel )
{
	realtype *pValue = NV_DATA_S( _value );
	realtype *pDeriv = NV_DATA_S( _deriv );
	realtype *pResult = NV_DATA_S( _res );

	bool bRes = (static_cast<CDAEModel*>(_pModel))->GetResiduals( _dTime, pValue, pDeriv, pResult );

	return bRes ? 0 : -1;
}

bool CDAESolver::InitStoringMemory()
{
	// Allocate IDA memory for storing
	m_pStoreIDAmem = IDACreate();
	if (!m_pStoreIDAmem)
	{
		ErrorHandler( -1, "IDA", "IDACreate", "Cannot allocate memory for solver.", &m_sErrorDescription );
		return false;
	}
	if( IDASetErrHandlerFn( m_pStoreIDAmem, &CDAESolver::ErrorHandler, &m_sErrorDescription ) != IDA_SUCCESS )
	{
		ErrorHandler( -1, "IDA", "IDASetErrHandlerFn", "Cannot setup error handler function.", &m_sErrorDescription );
		return false;
	}

	const sunindextype nVarsCnt = static_cast<sunindextype>(m_pModel->GetVariablesNumber());
	m_StoreVectorVars = N_VNew_Serial( nVarsCnt );
	m_StoreVectorDers = N_VNew_Serial( nVarsCnt );
	if (!m_StoreVectorVars || !m_StoreVectorDers)
	{
		ErrorHandler( -1, "IDA", "N_VNew_Serial", "Cannot allocate memory for solver.", &m_sErrorDescription );
		return false;
	}

	// Initialize IDA memory for storing
	if (IDAInit(m_pStoreIDAmem, &CDAESolver::ResidualFunction, ZERO, m_StoreVectorVars, m_StoreVectorDers) != IDA_SUCCESS)
		return false;

	// Set linear solver
	/* Create dense SUNMatrix for use in linear solves */
	m_A_store = SUNDenseMatrix(nVarsCnt, nVarsCnt);
	/* Create dense SUNMatrix for use in linear solves */
	m_LS_store = SUNDenseLinearSolver(m_vectorVars, m_A_store);
	/* Attach the matrix and linear solver */
	if (IDASetLinearSolver(m_pStoreIDAmem, m_LS_store, m_A_store) != IDALS_SUCCESS)
		return false;

	auto* originMem = static_cast<IDAMemRec*>( m_pIDAmem );
	auto* storeMem = static_cast<IDAMemRec*>( m_pStoreIDAmem );

	// Allocate memory in m_pStoreIDAmem
	if (!storeMem->ida_ewt)		storeMem->ida_ewt   = N_VNew_Serial(nVarsCnt);
	if (!storeMem->ida_yy)		storeMem->ida_yy    = N_VNew_Serial(nVarsCnt);
	if (!storeMem->ida_yp)		storeMem->ida_yp    = N_VNew_Serial(nVarsCnt);
	if (!storeMem->ida_delta)	storeMem->ida_delta = N_VNew_Serial(nVarsCnt);

	return true;
}

void CDAESolver::ClearMemory()
{
	m_sErrorDescription.clear();

	// free vectors
	if (m_vectorVars)		{ N_VDestroy_Serial(m_vectorVars);		m_vectorVars = nullptr; }
	if (m_StoreVectorVars)	{ N_VDestroy_Serial(m_StoreVectorVars);	m_StoreVectorVars = nullptr; }
	if (m_vectorDers)		{ N_VDestroy_Serial(m_vectorDers);		m_vectorDers = nullptr; }
	if (m_StoreVectorDers)	{ N_VDestroy_Serial(m_StoreVectorDers);	m_StoreVectorDers = nullptr; }
	if (m_vectorATols)		{ N_VDestroy_Serial(m_vectorATols);		m_vectorATols = nullptr; }
	if (m_vectorId)			{ N_VDestroy_Serial(m_vectorId);		m_vectorId = nullptr; }
	SUNMatDestroy(m_A);
	SUNLinSolFree(m_LS);
	// free IDA memory
	if (m_pIDAmem)			{ IDAFree(&m_pIDAmem);					m_pIDAmem = nullptr; }
	// free store IDA memory
	if (m_pStoreIDAmem)
	{
		auto* storeMem = static_cast<IDAMemRec*>(m_pStoreIDAmem);
		if (storeMem->ida_ewt)		{ N_VDestroy_Serial(storeMem->ida_ewt);		storeMem->ida_ewt = nullptr; }
		if (storeMem->ida_yy)		{ N_VDestroy_Serial(storeMem->ida_yy);		storeMem->ida_yy = nullptr; }
		if (storeMem->ida_yp)		{ N_VDestroy_Serial(storeMem->ida_yp);		storeMem->ida_yp = nullptr; }
		if (storeMem->ida_delta)	{ N_VDestroy_Serial(storeMem->ida_delta);	storeMem->ida_delta = nullptr; }
		SUNMatDestroy(m_A_store);
		SUNLinSolFree(m_LS_store);
		IDAFree(&m_pStoreIDAmem);	m_pStoreIDAmem = nullptr;
	}
}

void CDAESolver::CopyNVector( N_Vector _dst, N_Vector _src )
{
	if ((_dst == NULL) || (_src == NULL))	return;
	std::memcpy(NV_DATA_S(_dst), NV_DATA_S(_src), sizeof(realtype)*static_cast<size_t>(NV_LENGTH_S(_src)));
}

void CDAESolver::CopyIDAmem( void* _pDst, void* _pSrc )
{
	if( ( _pSrc == NULL ) || ( _pDst == NULL ) )
		return;

	auto* src = static_cast<IDAMemRec*>( _pSrc );
	auto* dst = static_cast<IDAMemRec*>( _pDst );

	// Divided differences array and associated minor arrays
	for(size_t i=0; i<MXORDP1; ++i )
		std::memcpy( NV_DATA_S( dst->ida_phi[i] ), NV_DATA_S( src->ida_phi[i] ), sizeof(realtype)*static_cast<size_t>(NV_LENGTH_S(src->ida_phi[i])) );
	std::memcpy( dst->ida_psi, src->ida_psi, sizeof(realtype)*MXORDP1 );
	std::memcpy( dst->ida_alpha, src->ida_alpha, sizeof(realtype)*MXORDP1 );
	std::memcpy( dst->ida_beta, src->ida_beta, sizeof(realtype)*MXORDP1 );
	std::memcpy( dst->ida_sigma, src->ida_sigma, sizeof(realtype)*MXORDP1 );
	std::memcpy( dst->ida_gamma, src->ida_gamma, sizeof(realtype)*MXORDP1 );

	/// N_Vectors
	if( ( src->ida_ewt != NULL ) && ( dst->ida_ewt != NULL) )
		std::memcpy( NV_DATA_S( dst->ida_ewt ), NV_DATA_S( src->ida_ewt ), sizeof(realtype)*static_cast<size_t>(NV_LENGTH_S(src->ida_ewt)) );
	if( ( src->ida_yy != NULL ) && ( dst->ida_yy != NULL) )
		std::memcpy( NV_DATA_S( dst->ida_yy ), NV_DATA_S( src->ida_yy ), sizeof(realtype)*static_cast<size_t>(NV_LENGTH_S(src->ida_yy)) );
	if( ( src->ida_yp != NULL ) && ( dst->ida_yp != NULL) )
		std::memcpy( NV_DATA_S( dst->ida_yp ), NV_DATA_S( src->ida_yp ), sizeof(realtype)*static_cast<size_t>(NV_LENGTH_S(src->ida_yp)) );
	if( ( src->ida_delta != NULL ) && ( dst->ida_delta != NULL) )
		std::memcpy( NV_DATA_S( dst->ida_delta ), NV_DATA_S( src->ida_delta ), sizeof(realtype)*static_cast<size_t>(NV_LENGTH_S(src->ida_delta)) );

	/// Tstop information
	dst->ida_tstopset = src->ida_tstopset;
	dst->ida_tstop = src->ida_tstop;

	/// Step Data
	dst->ida_kk = src->ida_kk;
	dst->ida_kused = src->ida_kused;
	dst->ida_knew = src->ida_knew;
	dst->ida_phase = src->ida_phase;
	dst->ida_ns = src->ida_ns;

	dst->ida_hin = src->ida_hin;
	dst->ida_h0u = src->ida_h0u;
	dst->ida_hh = src->ida_hh;
	dst->ida_hused = src->ida_hused;
	dst->ida_rr = src->ida_rr;
	dst->ida_tn = src->ida_tn;
	dst->ida_tretlast = src->ida_tretlast;
	dst->ida_cj = src->ida_cj;
	dst->ida_cjlast = src->ida_cjlast;
	dst->ida_cjold = src->ida_cjold;
	dst->ida_cjratio = src->ida_cjratio;
	dst->ida_ss = src->ida_ss;
	dst->ida_epsNewt = src->ida_epsNewt;
	dst->ida_epcon = src->ida_epcon;
	dst->ida_toldel = src->ida_toldel;

	/// Limits
	dst->ida_hmax_inv = src->ida_hmax_inv;

	/// Counters
	dst->ida_nst = src->ida_nst;
	dst->ida_nre = src->ida_nre;
	dst->ida_ncfn = src->ida_ncfn;
	dst->ida_netf = src->ida_netf;
	dst->ida_nni = src->ida_nni;
	dst->ida_nsetups = src->ida_nsetups;

	/// Space requirements for IDA
	dst->ida_tolsf = src->ida_tolsf;

	// Rootfinding Data
	dst->ida_nrtfn = src->ida_nrtfn;
	dst->ida_trout = src->ida_trout;
	dst->ida_toutc = src->ida_toutc;
	dst->ida_taskc = src->ida_taskc;

	/// Linear Solver specific memory
	((IDALsMemRec*)dst->ida_lmem)->nje = ((IDALsMemRec*)src->ida_lmem)->nje;
	((IDALsMemRec*)dst->ida_lmem)->nreDQ = ((IDALsMemRec*)src->ida_lmem)->nreDQ;

	 SUNMatCopy(((IDALsMemRec*)src->ida_lmem)->J, ((IDALsMemRec*)dst->ida_lmem)->J);
	if ((((IDALsMemRec*)dst->ida_lmem)->x != NULL) && (((IDALsMemRec*)src->ida_lmem)->x != NULL))
		std::memcpy(NV_DATA_S(((IDALsMemRec*)dst->ida_lmem)->x), NV_DATA_S(((IDALsMemRec*)src->ida_lmem)->x), sizeof(realtype)*static_cast<size_t>(NV_LENGTH_S(((IDALsMemRec*)src->ida_lmem)->x)));
}

void CDAESolver::ErrorHandler( int _nErrorCode, const char *_pModule, const char *_pFunction, char *_pMsg, void *_sOutString )
{
	if( _sOutString == NULL ) return;

	if( _nErrorCode < 0 )
	{
		std::string sDescr = "[";
		sDescr += _pModule;
		sDescr += " ERROR] in ";
		sDescr += _pFunction;
		sDescr += ": ";
		sDescr += _pMsg;
		*static_cast<std::string*>(_sOutString) = sDescr;
	}
}

#ifdef _MSC_VER
#else
#pragma GCC diagnostic pop
#endif

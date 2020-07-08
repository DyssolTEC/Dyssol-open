/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "Stream.h"

class CMaterialStream;

class CHoldup : public CStream
{
public:
	/** Basic constructor.
	 *	\param _sHoldupKey Unique key of this stream*/
	CHoldup(const std::string& _sHoldupKey = "");
	/** Copy constructor.
	*	\param _stream Initial stream. Only the structure (phases, compounds, etc.) will be copied from _stream, but not the data itself.*/
	CHoldup(const CStream& _stream);

private:
	/** Initialize Holdup-specific data.*/
	void initHoldup();

public:
	// ============= Functions to work with OTHER STREAMS and HOLDUPS

	/** Copies all holdup data for specified time point.
	 *	Before copying all data after the time point _dTime in the destination holdup can be removed if flag _bDeleteDataAfter is set to true.
	 *	\param _pSrcHoldup Pointer to a holdup that should be copied
	 *	\param _dTime Time point to copy
	 *	\param _bDeleteDataAfter Before copying the all data in the destination holdup is removed after _dTime*/
	void CopyFromHoldup(const CHoldup* _pSrcHoldup, double _dTime, bool _bDeleteDataAfter = true);
	/** Copies all data from another holdup for specified time interval.
	 *	Before copying all data after the time point _dStart in the destination holdup can be removed if flag _bDeleteDataAfter is set to true.
	 *	\param _pSrcHoldup Pointer to a holdup that should be copied
	 *	\param _dStart Start of the time interval to copy
	 *	\param _dEnd End of the time interval to copy
	 *	\param _bDeleteDataAfter Before copying the all data in the destination holdup is removed after _dTime*/
	void CopyFromHoldup(const CHoldup* _pSrcHoldup, double _dStart, double _dEnd, bool _bDeleteDataAfter = true);
	/** Copies all data from one time point of _pSrcHoldup to another time point in this holdup.
	 *	Before copying all data after the time point _dTimeDst in the destination holdup can be removed if flag _bDeleteDataAfter is set to true.
	 *	\param _dTimeDst Time point in this stream where data should be copied
	 *	\param _pSrcHoldup Pointer to a holdup that should be copied
	 *	\param _dTimeSrc Time point of the source holdup to copy
	 *	\param _bDeleteDataAfter Before copying the all data in the destination holdup is removed after _dTimeDst*/
	void CopyFromHoldup(double _dTimeDst, const CHoldup* _pSrcHoldup, double _dTimeSrc, bool _bDeleteDataAfter = true);

	/** Performs mixing of the holdup with the stream _pStream for the specified time interval. Boundary points are included into this interval.
	*	Only streams with the same data structure (MD dimensions, phases, etc) can be mixed.
	*	\param _pStream Pointer to a stream
	*	\param _dStart Start of the time interval
	*	\param _dEnd End of the time interval*/
	void AddStream(const CMaterialStream* _pStream, double _dStart, double _dEnd);
	void AddStream2(const CMaterialStream* _pStream, double _tStart, double _tEnd);

	/** Performs a mixing of the holdup with holdup _pHoldup at the specified time point.
	 *	Only holdups with the same data structure (MD dimensions, phases, etc) can be mixed.
	 *	\param _pHoldup Pointer to a holdup
	 *	\param _dTime Time point that should be mixed*/
	void AddHoldup(const CHoldup* _pHoldup, double _dTime);
	/** Performs a mixing of the holdup with the holdup _pHoldup for specified time interval. Boundary points are included into this interval.
	 *	Parameter _nTPTypes specifies which time points will be present in the resulting holdup (combining points from two holdup (BOTH_TP),
	 *	only from the first holdup (DST_TP), or only from the second holdup (SRC_TP)). Data for non-existent points are obtained by linear approximation.
	 *	Only holdups with the same data structure (MD dimensions, phases, etc) can be mixed.
	 *	\param _pHoldup Pointer to a holdup
	 *	\param _dStart Start of the time interval
	 *	\param _dEnd End of the time interval
	 *	\param _nTPTypes Specifies which time points will be present in the resulting holdup*/
	void AddHoldup(const CHoldup* _pHoldup, double _dStart, double _dEnd, unsigned _nTPTypes = BOTH_TP);

	///** Performs subtraction of the stream*/
	//bool SubHoldup( CHoldup* _pHoldup, double _dTime );
	//bool SubHoldup( CHoldup* _pHoldup, double _dStart, double _dEnd );
	//bool SubStream( CMaterialStream* _pStream, double _dStart, double _dEnd );


	// ============= Functions to work with OVERALL PROPERTIES

	/** Returns mass of the holdup. If there is no specified time point, the value will be interpolated
	 *	\param _dTime Time point
	 *	\param _nBasis Basis of results (BASIS_MASS [kg] or BASIS_MOLL [mol])*/
	double GetMass(double _dTime, unsigned _nBasis = BASIS_MASS) const;
	/** Sets mass of the holdup. If there is no specified time point, the value will not be set
	 *	\param _dTime Time point
	 *	\param _dValue New mass value
	 *	\param _nBasis Basis of value (BASIS_MASS [kg] or BASIS_MOLL [mol])*/
	void SetMass(double _dTime, double _dValue, unsigned _nBasis = BASIS_MASS);


	// ============= Functions to work with COMPOUNDS

	/** Returns mass of the compound with key _sCompoundKey in phase _nPhase at the time point _dTime.
	 *	_nBasis is a basis of value (BASIS_MASS [kg] or BASIS_MOLL [mol]).*/
	double GetCompoundMass(double _dTime, const std::string& _sCompoundKey, unsigned _nPhase, unsigned _nBasis = BASIS_MASS) const;
	/** Returns mass of the compound with index _nCompoundIndex in phase _nPhase at the time point _dTime.
	 *	_nBasis is a basis of value (BASIS_MASS [kg] or BASIS_MOLL [mol]).*/
	double GetCompoundMass(double _dTime, unsigned _nCompoundIndex, unsigned _nPhase, unsigned _nBasis = BASIS_MASS) const;


	// ============= Functions to work with PHASES

	/** Returns mass of the specified phase in the holdup. If there is no specified time point, the value will be interpolated.
	 *	_nBasis is a basis of value (BASIS_MASS [kg] or BASIS_MOLL [mol]).
	 *	\param _dTime Time point
	 *	\param _nPhase Key of the phase
	 *	\param _nBasis Basis of value (BASIS_MASS [kg] or BASIS_MOLL [mol]).*/
	double GetPhaseMass(double _dTime, unsigned _nPhase, unsigned _nBasis = BASIS_MASS) const;

	/** Sets mass of the specified phase in the holdup for time point. Negative values before setting will be converted to zero.
	 *	If there is no specified time point or phase in holdup, the value will not be set.
	 *	_nBasis is a basis of value (BASIS_MASS [kg] or BASIS_MOLL [mol]).*/
	void SetPhaseMass(double _dTime, unsigned _nPhase, double _dMassFlow, unsigned _nBasis = BASIS_MASS);

private:
	void CopyFromStream(const CMaterialStream* _pStream, double _dStart, double _dEnd);
};

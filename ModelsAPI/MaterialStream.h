/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "Stream.h"

class CHoldup;

/**	Class describes the material flow.*/
class CMaterialStream : public CStream
{
	friend CHoldup;

public:
	/** Basic constructor.
	 *	\param _sMaterialKey Unique key of this stream.*/
	CMaterialStream(const std::string& _sMaterialKey = "");
	/** Copy constructor.
	*	\param _stream Initial stream. Only the structure (phases, compounds, etc.) will be copied from _stream, but not the data itself.*/
	CMaterialStream(const CStream& _stream);

private:
	/** Initialize MaterialStream-specific data.*/
	void initMaterialStream();

public:
	// ============= Functions to work with OTHER STREAMS

	/** Copies all stream data for specified time point.
	 *	Before copying all data after the time point _dTime in the destination stream can be removed if flag _bDeleteDataAfter is set to true.
	 *	\param _pSrcStream Pointer to a stream that should be copied
	 *	\param _dTime Time point to copy
	 *	\param _bDeleteDataAfter Before copying the all data in the destination stream is removed after _dTime*/
	void CopyFromStream(const CMaterialStream* _pSrcStream, double _dTime, bool _bDeleteDataAfter = true);
	/** Copies all stream data for specified time interval.
	 *	Before copying all data after the time point _dStart in the destination stream can be removed if flag _bDeleteDataAfter is set to true.
	 *	\param _pSrcStream Pointer to a stream that should be copied
	 *	\param _dStart Start of the time interval to copy
	 *	\param _dEnd End of the time interval to copy
	 *	\param _bDeleteDataAfter Before copying the all data in the destination stream is removed after _dStart*/
	void CopyFromStream(const CMaterialStream* _pSrcStream, double _dStart, double _dEnd, bool _bDeleteDataAfter = true);
	/** Copies all stream data from one time point of _pSrcStream to another time point in this stream.
	 *	Before copying all data after the time point _dTimeDst in the destination stream can be removed if flag _bDeleteDataAfter is set to true.
	 *	\param _dTimeDst Time point in this stream where data should be copied
	 *	\param _pSrcStream Pointer to a stream that should be copied
	 *	\param _dTimeSrc Time point of the source stream to copy
	 *	\param _bDeleteDataAfter Before copying the all data in the destination stream is removed after _dTimeDst*/
	void CopyFromStream(double _dTimeDst, const CMaterialStream* _pSrcStream, double _dTimeSrc, bool _bDeleteDataAfter = true);

	/** Copies all data for specified time point from the holdup and sets specified mass flow.
	*	Before copying all data after the time point _dTime in the destination stream can be removed if flag _bDeleteDataAfter is set to true.
	*	\param _pSrcHoldup Pointer to a holdup that should be copied
	*	\param _dTime Time point to copy
	*	\param _dMassFlow Mass flow to set to this stream after copy
	*	\param _bDeleteDataAfter Before copying the all data in the destination stream is removed after _dTime*/
	void CopyFromHoldup(const CHoldup* _pSrcHoldup, double _dTime, double _dMassFlow, bool _bDeleteDataAfter = true);
	/** Copies all holdup data from one time point of _pSrcHoldup to another time point in this stream.
	*	Before copying all data after the time point _dTimeDst in the destination stream can be removed if flag _bDeleteDataAfter is set to true.
	*	\param _dTimeDst Time point in this stream where data should be copied
	*	\param _pSrcHoldup Pointer to a holdup that should be copied
	*	\param _dTimeSrc Time point of the source holdup to copy
	*	\param _dMassFlow Mass flow to set to this stream after copy
	*	\param _bDeleteDataAfter Before copying the all data in the destination stream is removed after _dTimeDst*/
	void CopyFromHoldup(double _dTimeDst, const CHoldup* _pSrcHoldup, double _dTimeSrc, double _dMassFlow, bool _bDeleteDataAfter = true);

	/** Performs mixing of the stream with the stream _pStream at specified time point.
	 *	Only streams with the same data structure (MD dimensions, phases, etc) can be mixed.
	 *	\param _pStream Pointer to a stream
	 *	\param _dTime Time point that should be mixed*/
	void AddStream(const CMaterialStream* _pStream, double _dTime);
	/** Performs mixing of the stream with stream _pStream for the specified time interval. Boundary points are included into this interval.
	 *	Parameter _nTPTypes specifies which time points will be present in the resulting stream (combining points from two streams (BOTH_TP),
	 *	only from the first stream (DST_TP), or only from the second stream (SRC_TP)). Data for non-existing points are obtained by linear interpolation.
	 *	Only streams with the same data structure (MD dimensions, phases, etc) can be mixed.
	 *	\param _pStream Pointer to a stream
	 *	\param _dStart Start of the time interval
	 *	\param _dEnd End of the time interval
	 *	\param _nTPTypes Specifies which time points will be present in the resulting stream*/
	void AddStream(const CMaterialStream* _pStream, double _dStart, double _dEnd, unsigned _nTPTypes = BOTH_TP);

	///** Performs subtraction of the stream*/
	//bool SubStream( CMaterialStream* _pStream, double _dTime );
	//bool SubStream( CMaterialStream* _pStream, double _dStart, double _dEnd );


	// ============= Functions to work with OVERALL PROPERTIES

	/** Returns mass flow of the stream. If there is no specified time point, the value will be interpolated.
	 *	\param _dTime Time point
	 *	\param _nBasis Basis of results (BASIS_MASS [kg/s] or BASIS_MOLL [mol/s])*/
	double GetMassFlow(double _dTime, unsigned _nBasis = BASIS_MASS) const;
	/** Sets mass flow of the stream. If there is no specified time point, the value will not be set.
	 *	\param _dTime Time point
	 *	\param _dValue New mass flow value
	 *	\param _nBasis Basis of value (BASIS_MASS [kg/s] or BASIS_MOLL [mol/s])*/
	void SetMassFlow(double _dTime, double _dValue, unsigned _nBasis = BASIS_MASS);


	// ============= Functions to work with COMPOUNDS

	/** Returns mass flow of the compound with key _sCompoundKey in phase _nPhase at the time point _dTime.
	 *	_nBasis is a basis of value (BASIS_MASS [kg/s] or BASIS_MOLL [mol/s]).*/
	double GetCompoundMassFlow(double _dTime, const std::string& _sCompoundKey, unsigned _nPhase, unsigned _nBasis = BASIS_MASS) const;
	/** Returns mass flow of the compound with index _nCompoundIndex in phase _nPhase at the time point _dTime.
	 *	_nBasis is a basis of value (BASIS_MASS [kg/s] or BASIS_MOLL [mol/s]).*/
	double GetCompoundMassFlow(double _dTime, unsigned _nCompoundIndex, unsigned _nPhase, unsigned _nBasis = BASIS_MASS) const;


	// ============= Functions to work with PHASES

	/** Returns mass flow of the specified phase in the stream. If there is no specified time point, the value will be interpolated.
	 *	\param _dTime Time point
	 *	\param _nPhase Index of the phase
	 *	\param _nBasis Basis of value (BASIS_MASS [kg/s] or BASIS_MOLL [mol/s]).*/
	double GetPhaseMassFlow(double _dTime, unsigned _nPhase, unsigned _nBasis = BASIS_MASS) const;
	/** Sets mass flow of the specified phase in the stream for time point. Negative values before setting will be converted to zero.
	 *	If there is no specified time point or phase in stream, the value will not be set.
	 *	_nBasis is a basis of value (BASIS_MASS [kg/s] or BASIS_MOLL [mol/s]).*/
	void SetPhaseMassFlow(double _dTime, unsigned _nPhase, double _dMassFlow, unsigned _nBasis = BASIS_MASS);
};

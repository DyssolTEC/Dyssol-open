/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "Stream.h"

class CStream;

class CHoldup : public CBaseStream
{
public:
	// Basic constructor.
	CHoldup(const std::string& _key = "");
	// Copy constructor.
	CHoldup(const CBaseStream& _other);

	// Copies all data at the given time point from another holdup. All data after the time point are removed from this holdup.
	void CopyFromHoldup(double _time, const CHoldup* _source);
	// Copies all data at the given time interval from another holdup. All data after the end time point are removed from this holdup.
	void CopyFromHoldup(double _timeBeg, double _timeEnd, const CHoldup* _source);
	// Copies all data from the given time point of another holdup to another time point of this stream. All data after the time point are removed from this holdup.
	void CopyFromHoldup(double _timeDst, const CHoldup* _source, double _timeSrc);

	/* Mixes the content of the specified stream at the given time interval with the current holdup. Before mixing, all data after the first time point are removed.
	 * The beginning of the time interval as the first defined time point preceding the given end time point, i.e. [source.PreviousTime(timeEnd); timeEnd].
	 * Takes into account only two given time points, regardless of whether they were defined in the stream. All possible time points within the interval are discarded.*/
	void AddStream(double _timeEnd, const CStream* _source);
	/* Mixes the content of the specified stream at the given time interval with the current holdup. Before mixing, all data after the first time point are removed.
	 * Takes into account only two given time points, regardless of whether they were defined in the stream. All possible time points within the interval are discarded.*/
	void AddStream(double _timeBeg, double _timeEnd, const CStream* _source);
	/* Removes all data after the first time point and adds content of the specified stream at the given time interval to the current holdup.
	 * Takes into account both the given boundary time points and all the time points within the interval defined in the stream.*/
	void AddStreamInterval(double _timeBeg, double _timeEnd, const CStream* _source);

	// Mixes the specified holdup with the current holdup at the given time point.
	void AddHoldup(double _time, const CHoldup* _source);
	// Mixes the specified holdup with the current holdup for each time point from the given time interval.
	void AddHoldup(double _timeBeg, double _timeEnd, const CHoldup* _source);

	// TODO: move it somewhere
	////////////////////////////////////////////////////////////////////////////////
	/// Deprecated functions
	[[deprecated("WARNING! AddStream(const CStream*, double, double) is deprecated. Use AddStream(double, const CStream*), AddStream(double, double, const CStream*) or AddStreamInterval(double, double, const CStream*) instead.")]]
	void AddStream(const CStream* _source, double _timeBeg, double _timeEnd);
	[[deprecated("WARNING! AddStream2(const CStream*, double, double) is deprecated. Use AddStream(double, double, const CStream*) instead.")]]
	void AddStream2(const CStream* _source, double _timeBeg, double _timeEnd);
};

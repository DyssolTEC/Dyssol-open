/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolTypes.h"
#include "H5Handler.h"

class CTimeDependentValue
{
	inline static const double m_eps{ 16 * std::numeric_limits<double>::epsilon() };

	static const uint16_t m_saveVersion{ 0 }; // Version of the saving procedure.

	std::vector<STDValue> m_data; // Time-dependent data.

	std::string m_name;
	std::string m_units;

public:
	CTimeDependentValue() = default;							// Creates a new empty dependent value.
	CTimeDependentValue(std::string _name, std::string _units);	// Creates a new empty dependent value with the specified name and units.

	void SetName(const std::string& _name);		// Sets new name of the dependent value.
	std::string GetName() const;				// Returns the name of the dependent value.
	void SetUnits(const std::string& _units);	// Sets new measurement units of the dependent value.
	std::string GetUnits() const;				// Returns the measurement units of the dependent value.

	// TODO: remove this function
	void AddTimePoint(double _time);							// Adds a new temp point _time if it doesn't already exist and fills it with the data of previous time point.
	void CopyTimePoint(double _timeDst, double _timeSrc);		// Adds a new temp point _timeDst if it doesn't already exist and fills it with the data of existing time point _timeSrc.
	void RemoveTimePoint(double _time);							// Removes the specified time point if it does already exist.
	void RemoveTimePoints(double _timeBeg, double _timeEnd);	// Removes all existing time points in the specified interval.
	void RemoveAllTimePoints();									// Removes all existing time points.

	// TODO: remove
	size_t GetTimePointsNumber() const;		// Returns the number of defined time points.
	//std::vector<double> GetAllTimePoints() const;

	void SetValue(double _time, double _value);	// Sets new value at the given time point. Creates a new time point if needed.
	double GetValue(double _time) const;		// Returns the value at the given time point.

	// TODO: work with two vectors
	// Sets new values in form of two vectors: times and values, removing all previously defined data.
	void SetRawData(const std::vector<std::vector<double>>& _data);
	// Returns all data in form of two vectors: times and values.
	std::vector<std::vector<double>> GetRawData() const;

	void CopyFrom(double _time, const CTimeDependentValue& _source);						// Copies data from another dependent value at the given time point.
	void CopyFrom(double _timeDst, const CTimeDependentValue& _source, double _timeSrc);	// Copies data to the given time point from another time point of the source dependent value.
	void CopyFrom(double _timeBeg, double _timeEnd, const CTimeDependentValue& _source);	// Copies data from another dependent value at the given time interval.

	// Performs nearest-neighbor extrapolation of data.
	void Extrapolate(double _timeExtra, double _time);
	// Performs linear extrapolation of data.
	void Extrapolate(double _timeExtra, double _time1, double _time2);
	// Performs cubic spline extrapolation of data.
	void Extrapolate(double _timeExtra, double _time1, double _time2, double _time3);

	// Sets new caching parameters.
	void SetCacheSettings(const SCacheSettings& _cache);

	//void CrearData();

	// Saves data to file.
	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	// Loads data from file
	void LoadFromFile(CH5Handler& _h5File, const std::string& _path);

private:
	// Performs linear interpolation. If the parameter is outside of the defined limits, performs nearest-neighbor extrapolation of data. Returns zero if the data vector is empty.
	double Interpolate(double _time) const;
	// Checks whether the given time point exists.
	bool HasTime(double _time) const;
	// Returns the nearest time point before _time.
	double PreviousTime(double _time) const;
	// Returns iterators pointing on values between the specified interval. If the values cannot be found, return two iterators to the end.
	std::pair<std::vector<STDValue>::iterator, std::vector<STDValue>::iterator> Interval(double _timeBeg, double _timeEnd);
	// Returns const iterators pointing on values between the specified interval. If the values cannot be found, return two iterators to the end.
	std::pair<std::vector<STDValue>::const_iterator, std::vector<STDValue>::const_iterator> Interval(double _timeBeg, double _timeEnd) const;
};

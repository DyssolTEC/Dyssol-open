#pragma once

#include "DyssolTypes.h"
#include "H5Handler.h"

class CTimeDependentValue
{
	static const uint16_t m_saveVersion; // Version of the saving procedure.

	std::vector<STDValue> m_data; // Time-dependent data.

	std::string m_name;
	std::string m_units;

	// TODO: make it accessible or global
	double m_eps{ 1e-20 };

public:
	CTimeDependentValue() = default;							// Creates a new empty dependent value.
	CTimeDependentValue(std::string _name, std::string _units);	// Creates a new empty dependent value with the specified name and units.

	// TODO: remove this function
	void AddTimePoint(double _time);							// Adds a new temp point _time if it doesn't already exist and fills it with the data of previous time point.
	void CopyTimePoint(double _timeDst, double _timeSrc);		// Adds a new temp point _timeDst if it doesn't already exist and fills it with the data of existing time point _timeSrc.
	void RemoveTimePoint(double _time);							// Removes the specified time point if it does already exist.
	void RemoveTimePoints(double _timeBeg, double _timeEnd);	// Removes all existing time points in the specified interval.

	void SetValue(double _time, double _value);
	double GetValue(double _time) const;

	//size_t GetTimePointsNumber() const;
	//std::vector<double> GetAllTimePoints() const;

	//void RemoveTimePoint(double _time);
	//void RemoveTimePoints(double _timeBeg, double _timeEnd);

	//void CopyFrom(const CTimeDependentValue& _source, double _time);
	//void CopyFrom(const CTimeDependentValue& _source, double _timeBeg, double _timeEnd);

	//void CrearData();

	void SetName(const std::string& _name);
	std::string GetName() const;
	void SetUnits(const std::string& _units);
	std::string GetUnits() const;

	///** Save data to file.*/
	//void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	///** Load data from file*/
	//void LoadFromFile(CH5Handler& _h5File, const std::string& _path) const;

private:
	// Performs linear interpolation. If the parameter is outside of the defined limits, performs nearest-neighbor extrapolation of data. Returns zero if the data vector is empty.
	double Interpolate(double _time) const;
	// Checks whether the given time point exists.
	bool HasTime(double _time) const;
	// Returns the nearest time point before _time.
	double PreviousTime(double _time) const;
};

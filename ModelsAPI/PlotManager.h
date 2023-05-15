/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolTypes.h"
#include <memory>
#include <vector>

class CH5Handler;

////////////////////////////////////////////////////////////////////////////////
// CCurve
//

class CCurve
{
	static const unsigned m_saveVersion{ 1 };	// Current version of the saving procedure.

	std::string m_name;				// Name of the curve.
	double m_valueZ{};				// Value of Z axis for which this curve is defined.

	std::vector<CPoint> m_values;	// Values stored in the curve.

public:
	CCurve(std::string _name);
	CCurve(double _z);

	// Returns the name of the curve.
	std::string GetName() const;
	// Sets new name of the curve.
	void SetName(const std::string& _name);

	// Returns the value of Z axis for which this curve is defined.
	double GetZValue() const;
	// Sets a new value of Z axis for which this curve is defined.
	void SetZValue(double _z);

	// Adds a new point to the curve.
	void AddPoint(double _x, double _y);
	// Adds a new point to the curve.
	void AddPoint(const CPoint& _point);

	// Adds a new points to the curve. X and Y must have the same length.
	void AddPoints(const std::vector<double>& _x, const std::vector<double>& _y);
	// Adds a new points to the curve.
	void AddPoints(const std::vector<CPoint>& _points);

	// Returns X values of all points defined in the curve.
	std::vector<double> GetXValues() const;
	// Returns Y values of all points defined in the curve.
	std::vector<double> GetYValues() const;
	// Returns all points defined in the curve.
	std::vector<CPoint> GetPoints() const;

	// Removes all defined points from the curve.
	void ClearData();
	// Removes all data from the curve.
	void Clear();

	// Saves data to file.
	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	// Loads data from file.
	void LoadFromFile(CH5Handler& _h5File, const std::string& _path);
};

////////////////////////////////////////////////////////////////////////////////
// CPlot
//

class CPlot
{
	static const unsigned m_saveVersion{ 1 };	// Current version of the saving procedure.

protected:
	std::string m_name;		// Name of the plot.
	std::string m_labelX;	// Label of the X axis.
	std::string m_labelY;	// Label of the Y axis.
	std::string m_labelZ;	// Label of the Z axis.

	std::vector<std::unique_ptr<CCurve>> m_curves;	// Curves in this plot.

public:
	CPlot(std::string _name);
	CPlot(std::string _name, std::string _labelX, std::string _labelY);
	CPlot(std::string _name, std::string _labelX, std::string _labelY, std::string _labelZ);
	~CPlot() = default;

	CPlot(const CPlot& _other);
	CPlot(CPlot&& _other) = default;
	CPlot& operator=(const CPlot& _other);
	CPlot& operator=(CPlot&& _other) = default;

	// Returns the name of the plot.
	std::string GetName() const;
	// Sets new name of the plot.
	void SetName(const std::string& _name);

	// Returns a label of the X axis.
	std::string GetLabelX() const;
	// Returns a label of the Y axis.
	std::string GetLabelY() const;
	// Returns a label of the Z axis.
	std::string GetLabelZ() const;
	// Sets a label of the X axis.
	void SetLabelX(const std::string& _label);
	// Sets a label of the Y axis.
	void SetLabelY(const std::string& _label);
	// Sets a label of the Z axis.
	void SetLabelZ(const std::string& _label);
	// Sets labels of all axes.
	void SetLabels(const std::string& _labelX, const std::string& _labelY);
	// Sets labels of all axes.
	void SetLabels(const std::string& _labelX, const std::string& _labelY, const std::string& _labelZ);

	// Adds a new curve with the specified name to the plot. If a curve with the given name already exists, does nothing, and returns nullptr.
	CCurve* AddCurve(const std::string& _name);
	// Adds a new curve with the specified name to the plot and fills it with the given values. X and Y must have the same length. If a curve with the given name already exists, does nothing, and returns nullptr.
	CCurve* AddCurve(const std::string& _name, const std::vector<double>& _x, const std::vector<double>& _y);
	// Adds a new curve with the specified name to the plot and fills it with the given values. If a curve with the given name already exists, does nothing, and returns nullptr.
	CCurve* AddCurve(const std::string& _name, const std::vector<CPoint>& _points);
	// Adds several curves with the specified names to the plot. If any curve with the given name value already exists, does nothing, and returns empty vector.
	std::vector<CCurve*> AddCurves(const std::vector<std::string>& _names);
	// Adds a new curve with the specified Z value to the plot. If a curve with the given Z value already exists, does nothing, and returns nullptr.
	CCurve* AddCurve(double _z);
	// Adds a new curve with the specified Z value to the plot and fills it with the given values. X and Y must have the same length. If a curve with the given Z value already exists, does nothing, and returns nullptr.
	CCurve* AddCurve(double _z, const std::vector<double>& _x, const std::vector<double>& _y);
	// Adds a new curve with the specified Z value to the plot and fills it with the given values. If a curve with the given Z value already exists, does nothing, and returns nullptr.
	CCurve* AddCurve(double _z, const std::vector<CPoint>& _points);
	// Adds several curves with the specified Z values to the plot. If any curve with the given Z value already exists, does nothing, and returns empty vector.
	std::vector<CCurve*> AddCurves(const std::vector<double>& _z);

	// Returns a curve with the specified index.
	const CCurve* GetCurve(size_t _index) const;
	// Returns a curve with the specified index.
	CCurve* GetCurve(size_t _index);
	// Returns a curve with the specified name.
	const CCurve* GetCurve(const std::string& _name) const;
	// Returns a curve with the specified name.
	CCurve* GetCurve(const std::string& _name);
	// Returns a curve with the specified Z value.
	const CCurve* GetCurve(double _z) const;
	// Returns a curve with the specified Z value.
	CCurve* GetCurve(double _z);

	// Removes a curve with the specified name from the plot.
	void RemoveCurve(const std::string& _name);
	// Removes a curve with the specified Z value from the plot.
	void RemoveCurve(double _z);

	// Returns all curves defined in the plot.
	std::vector<const CCurve*> GetAllCurves() const;
	// Returns all curves defined in the plot.
	std::vector<CCurve*> GetAllCurves();

	// Returns Z values of all defined curves.
	std::vector<double> GetZValues() const;

	// Checks if the plot is a 2D plot, based on the presence of Z values.
	bool Is2D() const;

	// Returns a number of defined curves.
	size_t GetCurvesNumber() const;

	// Removes all defined curves from the plot.
	void ClearData();
	// Removes all data from the plot.
	void Clear();

	// Saves data to file.
	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	// Loads data from file.
	void LoadFromFile(CH5Handler& _h5File, const std::string& _path);

private:
	// Tries to add a new curve with the specified Z value to the plot, without setting any data, omitting name clashes. Returns nullptr on failure.
	CCurve* AddEmptyCurve(double _z);
};

////////////////////////////////////////////////////////////////////////////////
// CPlotManager
//

class CPlotManager
{
	static const unsigned m_saveVersion{ 1 };	// Current version of the saving procedure.

	std::vector<std::unique_ptr<CPlot>> m_plots;		// Plots.
	std::vector<std::unique_ptr<CPlot>> m_plotsStored;	// A copy of plots used to store data during cyclic recalculations.

public:
	/**
	 * \internal
	 * \brief Copies user-defined data from _plots.
	 * \details Copies information about generated plots. Assumes the corresponding plots structure is the same.
	 * \param _plots Reference to source plots manager.
	 */
	void CopyUserData(const CPlotManager& _plots) const;

	// Adds a new 2D plot with the specified name. If a plot with the given name already exists, does nothing, and returns nullptr.
	CPlot* AddPlot(const std::string& _name);
	// Adds a new 2D plot with the specified name and axes labels. If a plot with the given name already exists, does nothing, and returns nullptr.
	CPlot* AddPlot(const std::string& _name, const std::string& _labelX, const std::string& _labelY);
	// Adds a new 3D plot with the specified name and axes labels. If a plot with the given name already exists, does nothing, and returns nullptr.
	CPlot* AddPlot(const std::string& _name, const std::string& _labelX, const std::string& _labelY, const std::string& _labelZ);
	// Returns a 2D plot with the specified index.
	const CPlot* GetPlot(size_t _index) const;
	// Returns a 2D plot with the specified index.
	CPlot* GetPlot(size_t _index);
	// Returns a 2D plot with the specified name.
	const CPlot* GetPlot(const std::string& _name) const;
	// Returns a 2D plot with the specified name.
	CPlot* GetPlot(const std::string& _name);
	// Removes a 2D plot with the specified name.
	void RemovePlot(const std::string& _name);

	// Returns all defined 2D plots.
	std::vector<const CPlot*> GetAllPlots() const;
	// Returns all defined 2D plots.
	std::vector<CPlot*> GetAllPlots();

	// Returns a number of defined plots.
	size_t GetPlotsNumber() const;

	// Removes all defined plots.
	void ClearData();
	// Removes all data.
	void Clear();

	// Stores the current state of all plots.
	void SaveState();
	// Restores previously stored state of all plots.
	void LoadState();

	// Saves data to file.
	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	// Loads data from file.
	void LoadFromFile(CH5Handler& _h5File, const std::string& _path);
	// Loads data from file. A compatibility version.
	void LoadFromFile_v0(const CH5Handler& _h5File, const std::string& _path);
};


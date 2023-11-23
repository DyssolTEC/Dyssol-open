/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolTypes.h"
#include <memory>
#include <vector>

class CH5Handler;

////////////////////////////////////////////////////////////////////////////////
// CCurve
//

/**
 * \brief Curve on the plot.
 */
class CCurve
{
	static const unsigned m_saveVersion{ 1 };	///< Current version of the saving procedure.

	std::string m_name;				///< Name of the curve.
	double m_valueZ{};				///< Value of Z axis for which this curve is defined.

	std::vector<CPoint> m_values;	///< Values stored in the curve.

public:
	/**
	 * \private
	 * \brief Constructor.
	 * \param _name Name of the curve.
	 */
	CCurve(std::string _name);
	/**
	 * \private
	 * \brief Constructor.
	 * \param _z Z-value of the curve.
	 */
	CCurve(double _z);

	/**
	 * \brief Returns the name of the curve.
	 * \return Curve's name.
	 */
	std::string GetName() const;
	/**
	 * \brief Sets new name of the curve.
	 * \param _name Curve's name.
	 */
	void SetName(const std::string& _name);

	/**
	 * \brief Returns the value of Z axis for which this curve is defined.
	 * \return Z-value of the curve.
	 */
	double GetZValue() const;
	/**
	 * \brief Sets a new value of Z axis for which this curve is defined.
	 * \param _z Z-value of the curve.
	 */
	void SetZValue(double _z);

	/**
	 * \brief Adds a new point to the curve.
	 * \param _x X-value of the point.
	 * \param _y Y-value of the point.
	 */
	void AddPoint(double _x, double _y);
	/**
	 * \brief Adds a new point to the curve.
	 * \param _point New point.
	 */
	void AddPoint(const CPoint& _point);

	/**
	 * \brief Adds new points to the curve.
	 * \details Vectors must have the same length.
	 * \param _x X-values of the points.
	 * \param _y Y-values of the points.
	 */
	void AddPoints(const std::vector<double>& _x, const std::vector<double>& _y);
	/**
	 * \brief Adds new points to the curve.
	 * \param _points New points.
	 */
	void AddPoints(const std::vector<CPoint>& _points);

	/**
	 * \brief Returns X values of all points defined in the curve.
	 * \return X-values of all points.
	 */
	std::vector<double> GetXValues() const;
	/**
	 * \brief Returns Y values of all points defined in the curve.
	 * \return Y-values of all points.
	 */
	std::vector<double> GetYValues() const;
	/**
	 * \brief Returns all points defined in the curve.
	 * \return All points.
	 */
	std::vector<CPoint> GetPoints() const;

	/**
	 * \brief Removes all defined points from the curve.
	 */
	void ClearData();
	/**
	 * \brief Removes all data from the curve.
	 */
	void Clear();

	/**
	 * \private
	 * \brief Saves data to file.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	/**
	 * \private
	 * \brief Loads data from file.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data in the file.
	 */
	void LoadFromFile(CH5Handler& _h5File, const std::string& _path);
};

////////////////////////////////////////////////////////////////////////////////
// CPlot
//

/**
 * \brief Plot in the unit.
 */
class CPlot
{
	static const unsigned m_saveVersion{ 1 };	///< Current version of the saving procedure.

protected:
	std::string m_name;		///< Name of the plot.
	std::string m_labelX;	///< Label of the X axis.
	std::string m_labelY;	///< Label of the Y axis.
	std::string m_labelZ;	///< Label of the Z axis.

	std::vector<std::unique_ptr<CCurve>> m_curves;	///< Curves in this plot.

public:
	/**
	 * \private
	 * \brief Constructor.
	 * \param _name Name of the plot.
	 */
	CPlot(std::string _name);
	/**
	 * \private
	 * \brief Constructor.
	 * \param _name Name of the plot.
	 * \param _labelX Name of X label.
	 * \param _labelY Name of Y label.
	 */
	CPlot(std::string _name, std::string _labelX, std::string _labelY);
	/**
	 * \private
	 * \brief Constructor.
	 * \param _name Name of the plot.
	 * \param _labelX Name of X label.
	 * \param _labelY Name of Y label.
	 * \param _labelZ Name of Z label.
	 */
	CPlot(std::string _name, std::string _labelX, std::string _labelY, std::string _labelZ);
	/**
	 * \private
	 * \brief Destructor.
	 */
	~CPlot() = default;

	/**
	 * \private
	 * \brief Copy constructor.
	 * \param _other Target plot.
	 */
	CPlot(const CPlot& _other);
	/**
	 * \private
	 * \brief Move constructor.
	 * \param _other Target plot.
	 */
	CPlot(CPlot&& _other) = default;
	/**
	 * \private
	 * \brief Copy assignment operator.
	 * \param _other Target plot.
	 * \return Reference to the plot.
	 */
	CPlot& operator=(const CPlot& _other);
	/**
	 * \private
	 * \brief Move assignment operator.
	 * \param _other Target plot.
	 * \return Reference to the plot.
	 */
	CPlot& operator=(CPlot&& _other) = default;

	/**
	 * \brief Returns the name of the plot.
	 * \return Plot's name.
	 */
	std::string GetName() const;
	/**
	 * \brief Sets new name of the plot.
	 * \param _name Plot's name.
	 */
	void SetName(const std::string& _name);

	/**
	 * \brief Returns a label of the X axis.
	 * \return X label of the plot.
	 */
	std::string GetLabelX() const;
	/**
	 * \brief Returns a label of the Y axis.
	 * \return Y label of the plot.
	 */
	std::string GetLabelY() const;
	/**
	 * \brief Returns a label of the Z axis.
	 * \return Z label of the plot.
	 */
	std::string GetLabelZ() const;
	/**
	 * \brief Sets a label of the X axis.
	 * \param _label X label of the plot.
	 */
	void SetLabelX(const std::string& _label);
	/**
	 * \brief Sets a label of the Y axis.
	 * \param _label Y label of the plot.
	 */
	void SetLabelY(const std::string& _label);
	/**
	 * \brief Sets a label of the Z axis.
	 * \param _label Z label of the plot.
	 */
	void SetLabelZ(const std::string& _label);
	/**
	 * \brief Sets labels of all axes.
	 * \param _labelX X label of the plot.
	 * \param _labelY Y label of the plot.
	 */
	void SetLabels(const std::string& _labelX, const std::string& _labelY);
	/**
	 * \brief Sets labels of all axes.
	 * \param _labelX X label of the plot.
	 * \param _labelY Y label of the plot.
	 * \param _labelZ Z label of the plot.
	 */
	void SetLabels(const std::string& _labelX, const std::string& _labelY, const std::string& _labelZ);

	/**
	 * \brief Adds a new curve with the specified name to the plot.
	 * \details If a curve with the given name already exists, does nothing, and returns nullptr.
	 * \param _name Plot's name.
	 * \return Pointer to the added curve.
	 */
	CCurve* AddCurve(const std::string& _name);
	/**
	 * \brief Adds a new curve with the specified name to the plot and fills it with the given values.
	 * \details X and Y must have the same length. If a curve with the given name already exists, does nothing, and returns nullptr.
	 * \param _name Plot's name.
	 * \param _x X-values of the points.
	 * \param _y Y-values of the points.
	 * \return Pointer to the added curve.
	 */
	CCurve* AddCurve(const std::string& _name, const std::vector<double>& _x, const std::vector<double>& _y);
	/**
	 * \brief Adds a new curve with the specified name to the plot and fills it with the given values.
	 * \details If a curve with the given name already exists, does nothing, and returns nullptr.
	 * \param _name Plot's name.
	 * \param _points Points on the curve.
	 * \return Pointer to the added curve.
	 */
	CCurve* AddCurve(const std::string& _name, const std::vector<CPoint>& _points);
	/**
	 * \brief Adds several curves with the specified names to the plot.
	 * \details If any curve with the given name value already exists, does nothing, and returns empty vector.
	 * \param _names Names of the curves.
	 * \return Pointers to the added curves.
	 */
	std::vector<CCurve*> AddCurves(const std::vector<std::string>& _names);
	/**
	 * \brief Adds a new curve with the specified Z value to the plot.
	 * \details If a curve with the given Z value already exists, does nothing, and returns nullptr.
	 * \param _z Z-value of the curve.
	 * \return Pointer to the added curve.
	 */
	CCurve* AddCurve(double _z);
	/**
	 * \brief Adds a new curve with the specified Z value to the plot and fills it with the given values.
	 * \details X and Y must have the same length. If a curve with the given Z value already exists, does nothing, and returns nullptr.
	 * \param _z Z-value of the curve.
	 * \param _x X-values of the points.
	 * \param _y Y-values of the points.
	 * \return Pointer to the added curve.
	 */
	CCurve* AddCurve(double _z, const std::vector<double>& _x, const std::vector<double>& _y);
	/**
	 * \brief Adds a new curve with the specified Z value to the plot and fills it with the given values.
	 * \details If a curve with the given Z value already exists, does nothing, and returns nullptr.
	 * \param _z Z-value of the curve.
	 * \param _points New points.
	 * \return Pointer to the added curve.
	 */
	CCurve* AddCurve(double _z, const std::vector<CPoint>& _points);
	/**
	 * \brief Adds several curves with the specified Z values to the plot.
	 * \details If any curve with the given Z value already exists, does nothing, and returns empty vector.
	 * \param _z Z-values of the curves.
	 * \return Pointers to the added curves.
	 */
	std::vector<CCurve*> AddCurves(const std::vector<double>& _z);

	/**
	 * \brief Returns a curve with the specified index.
	 * \param _index Index of the curve.
	 * \return Const pointer to the curve.
	 */
	const CCurve* GetCurve(size_t _index) const;
	/**
	 * \brief Returns a curve with the specified index.
	 * \param _index Index of the curve.
	 * \return Pointer to the curve.
	 */
	CCurve* GetCurve(size_t _index);
	/**
	 * \brief Returns a curve with the specified name.
	 * \param _name Name of the curve.
	 * \return Const pointer to the curve.
	 */
	const CCurve* GetCurve(const std::string& _name) const;
	/**
	 * \brief Returns a curve with the specified name.
	 * \param _name Name of the curve.
	 * \return Pointer to the curve.
	 */
	CCurve* GetCurve(const std::string& _name);
	/**
	 * \brief Returns a curve with the specified Z value.
	 * \param _z Z-value of the curve.
	 * \return Const pointer to the curve.
	 */
	const CCurve* GetCurve(double _z) const;
	/**
	 * \brief Returns a curve with the specified Z value.
	 * \param _z Z-value of the curve.
	 * \return Pointer to the curve.
	 */
	CCurve* GetCurve(double _z);

	/**
	 * \brief Removes a curve with the specified name from the plot.
	 * \param _name Name of the curve.
	 */
	void RemoveCurve(const std::string& _name);
	/**
	 * \brief Removes a curve with the specified Z value from the plot.
	 * \param _z Z-value of the curve.
	 */
	void RemoveCurve(double _z);

	/**
	 * \brief Returns all curves defined in the plot.
	 * \return Const pointers to the curves.
	 */
	std::vector<const CCurve*> GetAllCurves() const;
	/**
	 * \brief Returns all curves defined in the plot.
	 * \return Pointers to the curves.
	 */
	std::vector<CCurve*> GetAllCurves();

	/**
	 * \brief Returns Z values of all defined curves.
	 * \return Z values of the curves.
	 */
	std::vector<double> GetZValues() const;

	/**
	 * \brief Checks if the plot is a 2D plot, based on the presence of Z values.
	 * \return Whether the curve is 2D.
	 */
	bool Is2D() const;

	/**
	 * \brief Returns a number of defined curves.
	 * \return Number of curves.
	 */
	size_t GetCurvesNumber() const;

	/**
	 * \brief Removes all defined curves from the plot.
	 */
	void ClearData();
	/**
	 * \brief Removes all data from the plot.
	 */
	void Clear();

	/**
	 * \private
	 * \brief Saves data to file.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data.
	 */
	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	/**
	 * \private
	 * \brief Loads data from file.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data.
	 */
	void LoadFromFile(CH5Handler& _h5File, const std::string& _path);

private:
	/**
	 * \brief Tries to add a new curve with the specified Z value to the plot, without setting any data, omitting name clashes.
	 * \details Returns nullptr on failure.
	 */
	CCurve* AddEmptyCurve(double _z);
};

////////////////////////////////////////////////////////////////////////////////
// CPlotManager
//

/**
 * \brief Plot manager to access all plots and curves on them.
 */
class CPlotManager
{
	static const unsigned m_saveVersion{ 1 };	///< Current version of the saving procedure.

	std::vector<std::unique_ptr<CPlot>> m_plots;		///< Plots.
	std::vector<std::unique_ptr<CPlot>> m_plotsStored;	///< A copy of plots used to store data during cyclic recalculations.

public:
	/**
	 * \private
	 * \brief Copies user-defined data from _plots.
	 * \details Copies information about generated plots. Assumes the corresponding plots structure is the same.
	 * \param _plots Reference to source plots manager.
	 */
	void CopyUserData(const CPlotManager& _plots) const;

	/**
	 * \brief Adds a new 2D plot with the specified name.
	 * \details If a plot with the given name already exists, does nothing, and returns nullptr.
	 * \param _name Plot name.
	 * \return Pointer to the added plot.
	 */
	CPlot* AddPlot(const std::string& _name);
	/**
	 * \brief Adds a new 2D plot with the specified name and axes labels.
	 * \details If a plot with the given name already exists, does nothing, and returns nullptr.
	 * \param _name Plot name.
	 * \param _labelX X label of the plot.
	 * \param _labelY Y label of the plot.
	 * \return Pointer to the added plot.
	 */
	CPlot* AddPlot(const std::string& _name, const std::string& _labelX, const std::string& _labelY);
	/**
	 * \brief Adds a new 3D plot with the specified name and axes labels.
	 * \details If a plot with the given name already exists, does nothing, and returns nullptr.
	 * \param _name Plot name.
	 * \param _labelX X label of the plot.
	 * \param _labelY Y label of the plot.
	 * \param _labelZ Z label of the plot.
	 * \return Pointer to the added plot.
	 */
	CPlot* AddPlot(const std::string& _name, const std::string& _labelX, const std::string& _labelY, const std::string& _labelZ);
	/**
	 * \brief Returns a 2D plot with the specified index.
	 * \param _index Index of the plot.
	 * \return Const pointer to the plot.
	 */
	const CPlot* GetPlot(size_t _index) const;
	/**
	 * \brief Returns a 2D plot with the specified index.
	 * \param _index Index of the plot.
	 * \return Pointer to the plot.
	 */
	CPlot* GetPlot(size_t _index);
	/**
	 * \brief Returns a 2D plot with the specified name.
	 * \param _name Plot name.
	 * \return Const pointer to the plot.
	 */
	const CPlot* GetPlot(const std::string& _name) const;
	/**
	 * \brief Returns a 2D plot with the specified name.
	 * \param _name Plot name.
	 * \return Pointer to the plot.
	 */
	CPlot* GetPlot(const std::string& _name);
	/**
	 * \private
	 * \brief Removes a 2D plot with the specified name.
	 * \param _name Plot name.
	 */
	void RemovePlot(const std::string& _name);

	/**
	 * \brief Returns all defined 2D plots.
	 * \return Const pointers to all plots.
	 */
	std::vector<const CPlot*> GetAllPlots() const;
	/**
	 * \brief Returns all defined 2D plots.
	 * \return Pointers to all plots.
	 */
	std::vector<CPlot*> GetAllPlots();

	/**
	 * \brief Returns a number of defined plots.
	 * \return Number of all plots.
	 */
	size_t GetPlotsNumber() const;

	/**
	 * \private
	 * \brief Removes all defined plots.
	 */
	void ClearData();
	/**
	 * \private
	 * \brief Removes all data.
	 */
	void Clear();

	/**
	 * \private
	 * \brief Stores the current state of all plots.
	 */
	void SaveState();
	/**
	 * \private
	 * \brief Restores previously stored state of all plots.
	 */
	void LoadState();

	/**
	 * \private
	 * \brief Saves data to file.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data.
	 */
	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	/**
	 * \private
	 * \brief Loads data from file.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data.
	 */
	void LoadFromFile(CH5Handler& _h5File, const std::string& _path);
	/**
	 * \private
	 * \brief Loads data from file.
	 * \details A compatibility version.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data.
	 */
	void LoadFromFile_v0(const CH5Handler& _h5File, const std::string& _path);
};


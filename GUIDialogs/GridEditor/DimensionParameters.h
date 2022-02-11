/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_DimensionParameters.h"
#include "DyssolDefines.h"
#include <memory>

class CGridDimension;
class CMaterialsDatabase;

class CDimensionParameters : public QWidget
{
	// Types of grid units for PSD.
	enum class EGridUnit : uint32_t
	{
		UNIT_M, UNIT_MM, UNIT_UM, UNIT_M3, UNIT_MM3, UNIT_UM3, UNIT_DEFAULT
	};

	Q_OBJECT
	Ui::CDimensionParameters ui{};

	const CMaterialsDatabase& m_materialsDB;	// Reference to a global database of materials.
	std::unique_ptr<CGridDimension> m_grid;		// Current grid dimension.
	mutable std::string m_message;				// Last error message.

public:
	CDimensionParameters(const CGridDimension& _grid, const CMaterialsDatabase& _materialsDB, QWidget* _parent = nullptr);

	// Returns current distribution type.
	EDistrTypes GetDistributionType() const;
	// Checks whether the grid is valid.
	bool IsValid() const;
	// Returns last error message.
	QString LastMessage() const;
	// Returns the defined grid.
	CGridDimension& GetGrid() const;

private:
	// Creates the distribution type combobox.
	void SetupComboBoxDistribution() const;
	// Creates the grid entry combobox.
	void SetupComboBoxEntry() const;
	// Creates the grid function combobox.
	void SetupComboBoxFunction() const;
	// Creates the units combobox.
	void SetupComboBoxUnits() const;

	// Updates all widgets.
	void UpdateWholeView() const;
	// Updates distribution types combobox.
	void UpdateDistribution() const;
	// Updates number of classes widget.
	void UpdateClasses() const;
	// Updates grid entry combobox.
	void UpdateEntry() const;
	// Updates grid function combobox.
	void UpdateFunction() const;
	// Updates limits widgets.
	void UpdateLimits() const;
	// Updates units combobox.
	void UpdateUnits() const;
	// Updates grid table.
	void UpdateGrid() const;

	// Updates visibility of widgets according to the grid settings.
	void UpdateWidgetsVisibility() const;

	// Called when distribution type changes.
	void DistributionChanged() const;
	// Called when number of classes changes.
	void ClassesChanged() const;
	// Called when grid entry type changes.
	void EntryChanged();
	// Called when grid function type changes.
	void FunctionChanged() const;
	// Called when grid limits change.
	void LimitsChanged() const;
	// Called when units change.
	void UnitsChanged() const;
	// Called when values in the values table change.
	void GridChanged() const;

	// Sets the values to the grid according to the current settings.
	void SetGrid() const;
	// Calculates the numeric grid according to the currently selected settings.
	std::vector<double> CalculateGridNumeric() const;
	// Calculates the symbolic grid according to the currently selected settings.
	std::vector<std::string> CalculateGridSymbolic() const;
	// Determines a used distribution grid function from the grid values.
	EGridFunction DetermineGridFunction() const;
	// Determines the most suitable units from the grid values.
	EGridUnit DetermineUnits() const;
	// Checks if the vector corresponds the function in given units.
	bool IsOfFunction(const std::vector<double>& _v, EGridFunction _fun, EGridUnit _units = EGridUnit::UNIT_DEFAULT) const;

	// Converts the value in given units to meter, it the conversion is required. Either uses the given units or takes the currently selected ones.
	double ToM(double _val, EGridUnit _units = EGridUnit::UNIT_DEFAULT) const;
	// Converts the value in meter to given units, it the conversion is required. Either uses the given units or takes the currently selected ones.
	double FromM(double _val, EGridUnit _units = EGridUnit::UNIT_DEFAULT) const;
	// Converts the values in given units to meter, it the conversion is required. Either uses the given units or takes the currently selected ones.
	std::vector<double> ToM(const std::vector<double>& _vals, EGridUnit _units = EGridUnit::UNIT_DEFAULT) const;
	// Converts the values in meter to given units, it the conversion is required. Either uses the given units or takes the currently selected ones.
	std::vector<double> FromM(const std::vector<double>& _vals, EGridUnit _units = EGridUnit::UNIT_DEFAULT) const;
	// Converts the size value given in provided units to meter.
	double SizeToM(double _val, EGridUnit _units) const;
	// Converts the size value given in meter to provided units.
	double SizeFromM(double _val, EGridUnit _units) const;

	// Sets the current item of the combobox euqal to the item with the given user data.
	static void SelectComboboxValue(QComboBox* _combo, uint32_t _data);
	// Helper function - sets the string to error message and returns false.
	bool SetMessageAndReturn(const std::string& _message) const;
};

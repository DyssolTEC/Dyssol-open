/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolTypes.h"
#include <vector>

class CH5Handler;

/*
 * Base for grid dimensions.
 */
class CGridDimension
{
	static const unsigned m_saveVersion{ 1 }; // Current version of the saving procedure.

	// TODO: rename type (dimension or ->distribution)
	EDistrTypes m_type{ DISTR_SIZE };				// Distribution type.
	// TODO: rename type
	// TODO: maybe remove
	EGridEntry m_entry{ EGridEntry::GRID_NUMERIC };	// Entry type.

public:
	CGridDimension() = default;
	explicit CGridDimension(EGridEntry _entry);
	explicit CGridDimension(EDistrTypes _type, EGridEntry _entry);
	CGridDimension(const CGridDimension& _other) = default;
	CGridDimension(CGridDimension&& _other) = default;
	CGridDimension& operator=(const CGridDimension& _other) = default;
	CGridDimension& operator=(CGridDimension&& _other) = default;
	virtual ~CGridDimension() = default;

	// Returns a non-managed pointer to a copy of this.
	virtual CGridDimension* Clone() const;
	// Compares two objects.
	friend bool operator==(const CGridDimension& _lhs, const CGridDimension& _rhs);
	// Compares two objects.
	friend bool operator!=(const CGridDimension& _lhs, const CGridDimension& _rhs);

	// Returns the type of the dimension.
	[[nodiscard]] EDistrTypes DimensionType() const;
	// Returns the type of the grid for this dimension.
	[[nodiscard]] EGridEntry GridType() const;
	// Returns the number of classes defined in this grid dimension.
	[[nodiscard]] virtual size_t ClassesNumber() const;

	// Sets new distribution type.
	void SetType(EDistrTypes _type);

	// Saves grid to a HDF5 file.
	//virtual void SaveToFile(CH5Handler& _h5File, const std::string& _path);
	// Loads grid from a HDF5 file.
	//virtual void LoadFromFile(const CH5Handler& _h5File, const std::string& _path);

protected:
	// Compares for equality with another object.
	virtual bool Equal(const CGridDimension& _other) const;
};

/*
 * Numeric grid.
 */
class CGridDimensionNumeric : public CGridDimension
{
	static const unsigned m_saveVersion{ 1 }; // Current version of the saving procedure.

	// TODO: remove
	EGridFunction m_function{ EGridFunction::GRID_FUN_MANUAL };	// Function to define grid.
	std::vector<double> m_grid{ 0 , 1 };						// Grid itself.

public:
	CGridDimensionNumeric();
	explicit CGridDimensionNumeric(EDistrTypes _type);
	CGridDimensionNumeric(EDistrTypes _type, std::vector<double> _grid, EGridFunction _function = EGridFunction::GRID_FUN_MANUAL);

	// Returns a non-managed pointer to a copy of this.
	CGridDimensionNumeric* Clone() const override;

	// Returns the number of classes defined in this grid dimension.
	[[nodiscard]] size_t ClassesNumber() const override;
	// Returns current numerical grid.
	[[nodiscard]] std::vector<double> Grid() const;
	// Returns the function that was used to define the grid.
	[[nodiscard]] EGridFunction Function() const;

	// Sets new grid.
	void SetGrid(const std::vector<double>& _grid);
	// Sets new function.
	void SetFunction(EGridFunction _fun);

	// Returns mean values for each class.
	[[nodiscard]] std::vector<double> GetClassesMeans() const;
	// Returns sizes of classes.
	[[nodiscard]] std::vector<double> GetClassesSizes() const;

	// Saves grid to a HDF5 file.
	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	// Loads grid from a HDF5 file.
	void LoadFromFile(const CH5Handler& _h5File, const std::string& _path);

private:
	// Compares for equality with another object.
	[[nodiscard]] bool Equal(const CGridDimension& _other) const override;
};

/*
 * Symbolic grid.
 */
class CGridDimensionSymbolic : public CGridDimension
{
	static const unsigned m_saveVersion{ 1 }; // Current version of the saving procedure.

	std::vector<std::string> m_grid;	// Grid itself.

public:
	CGridDimensionSymbolic();
	explicit CGridDimensionSymbolic(EDistrTypes _type);
	CGridDimensionSymbolic(EDistrTypes _type, const std::vector<std::string>& _grid);

	// Returns a non-managed pointer to a copy of this.
	CGridDimensionSymbolic* Clone() const override;

	// Returns the number of classes defined in this grid dimension.
	[[nodiscard]] size_t ClassesNumber() const override;
	// Returns current symbolic grid.
	[[nodiscard]] std::vector<std::string> Grid() const;

	// Sets new grid.
	void SetGrid(const std::vector<std::string>& _grid);

	// Adds new class to the grid.
	void AddClass(const std::string& _entry);
	// Removes the class from the grid.
	void RemoveClass(const std::string& _entry);

	// Saves grid to a HDF5 file.
	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	// Loads grid from a HDF5 file.
	void LoadFromFile(const CH5Handler& _h5File, const std::string& _path);

private:
	// Compares for equality with another object.
	[[nodiscard]] bool Equal(const CGridDimension& _other) const override;
};

/*
 * Describes a multidimensional grid consisting of numeric and symbolic grids.
 * There is always a main grid defined in the flowsheet, it is used for all new units.
 * Each unit obtains a copy of the main grid, and can modify it if required.
 */
class CMultidimensionalGrid
{
	static const unsigned m_saveVersion{ 3 }; // Current version of the saving procedure.

	std::vector<std::unique_ptr<CGridDimension>> m_grids;	// All defined dimensions.

public:
	CMultidimensionalGrid() = default;
	CMultidimensionalGrid(const CMultidimensionalGrid& _other);
	CMultidimensionalGrid& operator=(const CMultidimensionalGrid& _other);
	CMultidimensionalGrid(CMultidimensionalGrid&& _other) = default;
	CMultidimensionalGrid& operator=(CMultidimensionalGrid && _other) = default;
	~CMultidimensionalGrid() = default;

	// Removes all.
	void Clear();

	// Returns number of classes for each defined dimension.
	std::vector<size_t> GetClassesNumbers() const;
	// Returns types of each defined dimension.
	std::vector<EDistrTypes> GetDimensionsTypes() const;
	// Returns types of the grids of each defined dimension.
	std::vector<EGridEntry> GetGridsTypes() const;

	// Returns the number of defined dimensions in the grid.
	size_t GetDimensionsNumber() const;

	// Adds a new dimension with the given parameters to the grid if it does not exist yet.
	void AddDimension(const CGridDimension& _gridDimension);

	// Adds a new empty numerical dimension to the grid.
	CGridDimensionNumeric* AddNumericDimension();
	// Adds a new numerical dimension to the grid if it does not exist yet.
	CGridDimensionNumeric* AddNumericDimension(EDistrTypes _type);
	// Adds a new numerical dimension to the grid if it does not exist yet.
	CGridDimensionNumeric* AddNumericDimension(EDistrTypes _type, const std::vector<double>& _grid, EGridFunction _function = EGridFunction::GRID_FUN_MANUAL);
	// TODO: maybe remove this version
	// Adds a new numerical dimension with the given parameters to the grid if it does not exist yet.
	CGridDimensionNumeric* AddNumericDimension(const CGridDimensionNumeric& _gridDimension);

	// Adds a new empty symbolic dimension to the grid.
	CGridDimensionSymbolic* AddSymbolicDimension();
	// Adds a new symbolic dimension to the grid if it does not exist yet.
	CGridDimensionSymbolic* AddSymbolicDimension(EDistrTypes _type);
	// Adds a new symbolic dimension to the grid if it does not exist yet.
	CGridDimensionSymbolic* AddSymbolicDimension(EDistrTypes _type, const std::vector<std::string>& _grid);
	// TODO: maybe remove this version
	// Adds a new numerical dimension with the given parameters to the grid if it does not exist yet.
	CGridDimensionSymbolic* AddSymbolicDimension(const CGridDimensionSymbolic& _gridDimension);

	// Returns grid dimension by its type. Returns nullptr if such dimension does not exist.
	const CGridDimension* GetGridDimension(EDistrTypes _type) const;
	// Returns grid dimension by its type. Returns nullptr if such dimension does not exist.
	CGridDimension* GetGridDimension(EDistrTypes _type);
	// Returns numeric grid dimension by its type. Returns nullptr if such dimension does not exist.
	const CGridDimensionNumeric* GetGridDimensionNumeric(EDistrTypes _type) const;
	// Returns numeric grid dimension by its type. Returns nullptr if such dimension does not exist.
	CGridDimensionNumeric* GetGridDimensionNumeric(EDistrTypes _type);
	// Returns symbolic grid dimension by its type. Returns nullptr if such dimension does not exist.
	const CGridDimensionSymbolic* GetGridDimensionSymbolic(EDistrTypes _type) const;
	// Returns symbolic grid dimension by its type. Returns nullptr if such dimension does not exist.
	CGridDimensionSymbolic* GetGridDimensionSymbolic(EDistrTypes _type);

	// Returns all defined grid dimensions.
	std::vector<const CGridDimension*> GetGridDimensions() const;
	// Returns all defined grid dimensions.
	std::vector<CGridDimension*> GetGridDimensions();

	// Returns a grid defined for PSD if it exists.
	std::vector<double> GetPSDGrid(EPSDGridType _type = EPSDGridType::DIAMETER) const;
	// Returns mean values of each class of the grid defined for PSD if it exists.
	std::vector<double> GetPSDMeans(EPSDGridType _type = EPSDGridType::DIAMETER) const;

	// Checks if the grid for this dimension is already defined.
	bool HasDimension(EDistrTypes _type) const;

	// Equality operator
	bool operator==(const CMultidimensionalGrid& _other) const;
	// Inequality operator
	bool operator!=(const CMultidimensionalGrid& _other) const;

	// Saves grid to a HDF5 file.
	void SaveToFile(CH5Handler& _h5File, const std::string& _path);
	// Loads grid from a HDF5 file.
	void LoadFromFile(const CH5Handler& _h5File, const std::string& _path);
	// Loads grid from a HDF5 file. Compatibility version.
	void LoadFromFile_v2(const CH5Handler& _h5File, const std::string& _path);
	// Loads grid from a HDF5 file. Compatibility version.
	void LoadFromFile_v1(const CH5Handler& _h5File, const std::string& _path);
private:
};

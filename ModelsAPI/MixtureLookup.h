/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "TwoWayMap.h"

/* Bidirectional lookup table to find the correspondence between two lists of values in a mixture of components.
 * The values are denoted as [left] and [right].
 * Each component has its weight and is defined for a set of [left:right] values. All weights are summed up to 1.
 * Each set of components - [left] and [right] - must contain only unique values.
 * After all the components are filled with values, it is possible to read 'Table[left] = right' and 'Table[right] = left' of the weighted mixture.
 * For values that do not directly defined, interpolation is applied.
 * If two components C1 and C2 with weights W_1 and W_2 are filled as follows
 * C1 = [X1_1:Y1_1, X1_2:Y1_2], W_1
 * C2 = [X2_1:Y2_1, X2_2:Y2_2], W_2
 * the resulting mixture table is calculated as
 * M  = [X1_1:Y1(X1_M)*W_1+Y2(X1_1)*W_1,
 *		 X1_2:Y1(X1_M)*W_2+Y2(X1_2)*W_2,
 *		 X2_1:Y1(X1_M)*W_1+Y2(X2_1)*W_1,
 *		 X2_2:Y1(X1_M)*W_2+Y2(X2_2)*W_2]
 */
class CMixtureLookup
{
	// A version of CTwoWayMap extended with required arithmetic operations.
	class CTwoWayMapExt : public CTwoWayMap
	{
	public:
		// Adds a _value to all _right elements, so that [_left:_right+_value].
		void Add(double _value);
		// Adds a _table to all _right elements, so that [_left:_right+_table(_left)].
		void Add(const CDependentValues& _table);
		// Adds a weighted _table to all _right elements, so that [_left:_right+T(_left)*_value].
		void AddMult(const CDependentValues& _table, double _value);
		// Multiplies a _value with all _right elements, so that [_left:_right*_value].
		void Mult(double _value);
	};

protected:
	CTwoWayMapExt m_table;						// Main lookup table.
	std::vector<CDependentValues> m_componets;	// All components.
	std::vector<double> m_weights;				// Weights of all components.

public:
	// Creates an empty mixture lookup table.
	CMixtureLookup() = default;
	// Creates a mixture lookup table with defined _components and equal weights.
	CMixtureLookup(std::vector<CDependentValues> _components);
	// Creates a mixture lookup table with defined _components and their _weights. Both vectors must have the same size.
	CMixtureLookup(std::vector<CDependentValues> _components, std::vector<double> _weights);

	// Adds a new _component with _weight.
	void AddComponent(const CDependentValues& _component, double _weight = 1.);
	// Removes a component and its weight entry.
	void RemoveComponent(size_t _index);
	// Returns the number of defined components.
	[[nodiscard]] size_t ComponentsNumber() const;

	// Sets new _weights for all components. _weights must have the same size as the number of defined components.
	void SetWeights(const std::vector<double>& _weights);
	// Returns weights of all components.
	[[nodiscard]] std::vector<double> GetWeights() const;

	// Returns _left element of the [_left:_right] pair. Performs linear interpolation is _right is not defined explicitly. Returns 0, if there are no data at all.
	[[nodiscard]] double GetLeft(double _right) const;
	// Returns _right element of the [_left:_right] pair. Performs linear interpolation is _left is not defined explicitly. Returns 0, if there are no data at all.
	[[nodiscard]] double GetRight(double _left) const;

	// Sets a _component with some _weight as a mixture.
	void Set(const CDependentValues& _component, double _weight = 1.);
	// Adds _value to each _right (dependent) entry of the mixture table.
	void Add(double _value);
	// Adds a _component with some _weight to each _right (dependent) entry of the mixture table.
	void Add(const CDependentValues& _component, double _weight = 1.);
	// Adds another mixture _table with some _weight to each _right (dependent) entry of the mixture table. _table must have the same number of components.
	void Add(const CMixtureLookup& _table, double _weight = 1.);
	// Multiplies each _right (dependent) entry of the mixture table with a _value.
	void Multiply(double _value);

	// Removes all data.
	void Clear();

private:
	// Updates the values in lookup table according to set components and weights.
	void Update();
};


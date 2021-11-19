/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DependentValues.h"

/* Container that implements a bidirectional map of elements [left:right].
 * Each set - [left] and [right] - must contain only unique values.
 * Provides interpolation between existing values. */
class CTwoWayMap
{
protected:
	CDependentValues m_direct;	// Table for direct access, T[left] = right.
	CDependentValues m_revert;	// Table for revert access, T[right] = left.

public:
	// Sets a new element [_left:_right]. If _left of _right is not unique withing their sets, does nothing.
	void Set(double _left, double _right);
	// Sets a new table of pairs in form [_left:_right], removing all previous data. If _left of _right set is not unique, keeps only unique elements.
	void SetLeftToRight(const CDependentValues& _table);
	// Sets a new table of pairs in form [_right:_left], removing all previous data. If _left of _right set is not unique, keeps only unique elements.
	void SetRightToLeft(const CDependentValues& _table);

	// Removes the element [_left:_right] if it exists.
	void Remove(double _left, double _right);
	// Removes the element [_left:X] if it exists.
	void RemoveLeft(double _left);
	// Removes the element [X:_right] if it exists.
	void RemoveRight(double _right);

	// Returns _left element of the [_left:_right] pair. Performs linear interpolation is _right is not defined explicitly. Returns 0, if there are no data at all.
	[[nodiscard]] double GetLeft(double _right) const;
	// Returns _right element of the [_left:_right] pair. Performs linear interpolation is _left is not defined explicitly. Returns 0, if there are no data at all.
	[[nodiscard]] double GetRight(double _left) const;

	// Checks if a pair [_left:X] is defined.
	[[nodiscard]] bool HasLeft(double _left) const;
	// Checks if a pair [X:_right] is defined.
	[[nodiscard]] bool HasRight(double _right) const;

	// Returns an unmodifiable table of pairs in form [_left:_right].
	[[nodiscard]] const CDependentValues& GetLeftToRightTable() const;
	// Returns an unmodifiable table of pairs in form [_right:_left].
	[[nodiscard]] const CDependentValues& GetRightToLeftTable() const;

	// Returns the number of defined pairs.
	[[nodiscard]] size_t Size() const;
	// Checks if the table is empty.
	[[nodiscard]] bool IsEmpty() const;
	// Removes all data.
	void Clear();

	bool operator==(const CTwoWayMap& _other) const;

protected:
	// Calculates a reverted table [X:Y] -> [Y:X].
	static CDependentValues Reverted(const CDependentValues& _table);
	// Returns a table with entries consisting of only unique X and Y.
	static CDependentValues Unique(const CDependentValues& _table);
};


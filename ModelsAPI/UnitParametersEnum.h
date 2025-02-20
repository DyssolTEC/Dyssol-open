/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2024, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <cstdint>

/**
 * Types of unit parameters.
 */
enum class EUnitParameter : uint8_t
{
	UNKNOWN = 0,          ///< Type is undefined.
	TIME_DEPENDENT = 1,   ///< Time-dependent unit parameter.
	CONSTANT = 2,         ///< Real constant unit parameter.
	STRING = 3,           ///< String unit parameter.
	CHECKBOX = 4,         ///< Check box unit parameter.
	SOLVER = 5,           ///< Solver unit parameter.
	COMBO = 6,            ///< Combo-box unit parameter.
	GROUP = 7,            ///< Combo-box unit parameter.
	COMPOUND = 8,         ///< Compound unit parameter.
	CONSTANT_DOUBLE = 9,  ///< Real constant unit parameter.
	CONSTANT_INT64 = 10,  ///< Signed integer constant unit parameter.
	CONSTANT_UINT64 = 11, ///< Unsigned integer constant unit parameter.
	REACTION = 12,        ///< Reaction unit parameter.
	LIST_DOUBLE = 13,     ///< List of real unit parameters.
	LIST_INT64 = 14,      ///< List of signed integer unit parameters.
	LIST_UINT64 = 15,     ///< List of unsigned integer unit parameters.
	MDB_COMPOUND = 16,    ///< Compound from the materials database.
	PARAM_DEPENDENT = 17, ///< Dependent unit parameter.
};

/* Copyright (c) 2023, DyssolTEC. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "H5Handler.h"

class CFlowsheet;

struct SSaveLoadData
{
	CFlowsheet* flowsheet{};                        /// Pointer to global flowsheet.
};

/**
 * This class is responsible for saving and loading of data about the flowsheet and all modules into an HDF5 file.
 */
class CSaveLoadManager
{
	static constexpr unsigned m_saveVersion{ 6 }; /// Current version of the saving procedure.

	SSaveLoadData m_data{};

	CH5Handler m_fileHandler{}; /// Handler of data file in HDF5 format.

public:
	CSaveLoadManager() = default;
	CSaveLoadManager(const SSaveLoadData& _data);

	/**
	 * Returns current file name.
	 * \details Returns possibly transformed file name that was really used during saving/loading.
	 * \return Transformed full path to the file.
	 */
	[[nodiscard]] std::filesystem::path GetFileName() const;

	/**
	 * Saves all data into the HDF5 file.
	 * \param _fileName Full path to the file.
	 * \return Operation success flag.
	 */
	bool SaveToFile(const std::filesystem::path& _fileName);
	/**
	 * Loads all data from the HDF5 file.
	 * \param _fileName Full path to the file.
	 * \return Operation success flag.
	 */
	bool LoadFromFile(const std::filesystem::path& _fileName);
};


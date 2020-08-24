/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <ios>
#include <map>
#include <vector>

//#define CHUNK_LENGTH 100

/* List of [param:value].
 * Chunks. Descriptor for each chunk.
 * Split into files with continuous numbering. */
class CCachedMap
{
//	struct SChunkDescriptor
//	{
//		double paramBeg{ 0.0 };		// Begin of the parameter interval within this chunk.
//		double paramEnd{ 0.0 };		// End of the parameter interval within this chunk.
//		size_t size{ 0 };			// Number of values stored in chunk.
//		size_t fileNumber{ 0 };		// Number of the cache file.
//		std::streamoff pos{ 0 };	// Data position in the file.
//	};
//
//public:
//	using size_type      = std::map<double, double>::size_type;
//	using iterator       = std::map<double, double>::iterator;
//	using const_iterator = std::map<double, double>::const_iterator;
//
//private:
//	std::map<double, double> m_data;				// The data itself.
//	std::vector<SChunkDescriptor> m_descriptors;	// List of file descriptors.
//
//public:
//	[[nodiscard]] bool empty() const noexcept;
//	[[nodiscard]] size_type size() const noexcept;
};


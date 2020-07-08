/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "H5Cpp.h"
#include "H5PacketTable.h"

class CH5PacketTable : public FL_PacketTable
{
private:
	static H5I_type_t H5PT_ptable_id_type;

public:
	CH5PacketTable( hid_t fileID, char* name, hid_t dtypeID, hsize_t chunkSize, int compression = -1 );
	CH5PacketTable( H5::Group& _h5Group, const std::string& _sName, hid_t _dtypeID, hsize_t _chunkSize, int _compression = -1 );
	CH5PacketTable( H5::H5File& _h5File, const std::string& _sName, hid_t _dtypeID, hsize_t _chunkSize, int _compression = -1 );
	CH5PacketTable( hid_t fileID, char* name );
	CH5PacketTable( H5::Group& _h5Group, const std::string& _sName );
	CH5PacketTable( H5::H5File& _h5File, const std::string& _sName );
	~CH5PacketTable(void);

	hid_t getId();

	void close();
};

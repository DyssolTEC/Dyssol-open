/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "PacketTable.h"

H5I_type_t CH5PacketTable::H5PT_ptable_id_type = H5I_UNINIT;

CH5PacketTable::CH5PacketTable(hid_t fileID, char* name, hid_t dtypeID, hsize_t chunkSize, int compression)
	: FL_PacketTable(fileID, name, dtypeID, chunkSize, compression)
{
}

CH5PacketTable::CH5PacketTable(H5::Group& _h5Group, const std::string& _sName, hid_t _dtypeID, hsize_t _chunkSize, int _compression)
	: FL_PacketTable(_h5Group.getId(), const_cast<char*>(_sName.c_str()), _dtypeID, _chunkSize, _compression)
{
	//table_id = H5PTcreate_fl( _h5Group.getId(), _sName.c_str(), _dtypeID, _chunkSize, _compression );
}

CH5PacketTable::CH5PacketTable(H5::H5File& _h5File, const std::string& _sName, hid_t _dtypeID, hsize_t _chunkSize, int _compression)
	: FL_PacketTable(_h5File.getId(), const_cast<char*>(_sName.c_str()), _dtypeID, _chunkSize, _compression)
{
	//table_id = H5PTcreate_fl( _h5Group.getId(), _sName.c_str(), _dtypeID, _chunkSize, _compression );
}

CH5PacketTable::CH5PacketTable(hid_t fileID, char* name)
	: FL_PacketTable(fileID, name)
{
	table_id = H5PTopen(fileID, name);
}

CH5PacketTable::CH5PacketTable(H5::Group& _h5Group, const std::string& _sName)
	: FL_PacketTable(_h5Group.getId(), const_cast<char*>(_sName.c_str()))
{
	table_id = H5PTopen(_h5Group.getId(), _sName.c_str());
}

CH5PacketTable::CH5PacketTable(H5::H5File& _h5File, const std::string& _sName)
	: FL_PacketTable(_h5File.getId(), const_cast<char*>(_sName.c_str()))
{
	table_id = H5PTopen(_h5File.getId(), _sName.c_str());
}

CH5PacketTable::~CH5PacketTable()
{
	H5PTclose(table_id);
}

hid_t CH5PacketTable::getId()
{
	return table_id;
}

void CH5PacketTable::close()
{
	H5PTclose(table_id);
}

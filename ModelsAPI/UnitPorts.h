/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DyssolDefines.h"
#include <memory>
#include <string>
#include <vector>

class CStream;
class CH5Handler;

////////////////////////////////////////////////////////////////////////////////
// CUnitPort
//

// Description of the unit port for connecting material streams.
class CUnitPort
{
	static const unsigned m_saveVersion{ 1 };	// Current version of the saving procedure.

private:
	std::string m_name;							// The name of the port, should be unique for the unit.
	EUnitPort m_type{ EUnitPort::UNDEFINED };	// Port type: INPUT or OUTPUT.
	std::string m_streamKey;					// The key of the stream which is connected to this port.
	CStream* m_stream{ nullptr };				// Pointer to the connected material stream.

public:
	CUnitPort(std::string _name, EUnitPort _type);

	std::string GetName() const;				// Returns port's name.
	void SetName(const std::string& _name);		// Sets port's name.

	EUnitPort GetType() const;					// Returns port's type.
	void SetType(EUnitPort _type);				// Sets port's type.

	std::string GetStreamKey() const;			// Returns key of the stream, connected to this port.
	void SetStreamKey(const std::string& _key);	// Sets key of the stream, connected to this port.

	CStream* GetStream() const;					// Returns stream, connected to this port.
	void SetStream(CStream* _stream);			// Sets stream, connected to this port.

	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;	// Saves data to file.
	void LoadFromFile(CH5Handler& _h5File, const std::string& _path);		// Loads data from file.
};

////////////////////////////////////////////////////////////////////////////////
// CPortsManager
//

class CPortsManager
{
	static const unsigned m_saveVersion{ 1 };	// Current version of the saving procedure.

	std::vector<std::unique_ptr<CUnitPort>> m_ports;	// All defined ports.

public:
	// Adds a port and returns a pointer to it. If a port with this name already exist, does nothing and return nullptr.
	CUnitPort* AddPort(const std::string& _name, EUnitPort _type);
	// Returns a port with the specified name.
	[[nodiscard]] const CUnitPort* GetPort(const std::string& _name) const;
	// Returns a port with the specified name.
	CUnitPort* GetPort(const std::string& _name);
	// Returns a port with the specified index.
	[[nodiscard]] const CUnitPort* GetPort(size_t _index) const;
	// Returns a port with the specified index.
	CUnitPort* GetPort(size_t _index);
	// Returns pointers to all defined ports.
	std::vector<CUnitPort*> GetAllPorts();
	// Returns const pointers to all defined ports.
	std::vector<const CUnitPort*> GetAllPorts() const;
	// Returns pointers to all defined input ports.
	std::vector<CUnitPort*> GetAllInputPorts();
	// Returns const pointers to all defined input ports.
	std::vector<const CUnitPort*> GetAllInputPorts() const;
	// Returns pointers to all defined output ports.
	std::vector<CUnitPort*> GetAllOutputPorts();
	// Returns const pointers to all defined output ports.
	std::vector<const CUnitPort*> GetAllOutputPorts() const;

	// Returns a number of defined ports.
	size_t GetPortsNumber() const;

	// Saves data to file.
	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	// Loads data from file.
	void LoadFromFile(CH5Handler& _h5File, const std::string& _path);
	// Loads data from file. A compatibility version.
	void LoadFromFile_v0(const CH5Handler& _h5File, const std::string& _path);
	// Loads data from file. A compatibility version.
	void LoadFromFile_v00(const CH5Handler& _h5File, const std::string& _path);

private:
	// Returns the names of all defined ports.
	std::vector<std::string> AllPortNames() const;
	// Returns keys of connected streams for all defined ports.
	std::vector<std::string> AllPortStreamKeys() const;
};
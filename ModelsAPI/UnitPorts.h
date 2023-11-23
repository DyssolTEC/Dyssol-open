/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

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

/**
 * \brief Description of the unit port.
 * \details The main purpose is connecting to material streams.
 */
class CUnitPort
{
	static const unsigned m_saveVersion{ 1 };	///< Current version of the saving procedure.

private:
	std::string m_name;							///< The name of the port, should be unique for the unit.
	EUnitPort m_type{ EUnitPort::UNDEFINED };	///< Port type: INPUT or OUTPUT.
	std::string m_streamKey;					///< The key of the stream which is connected to this port.
	CStream* m_stream{ nullptr };				///< Pointer to the connected material stream.

public:
	/**
	 * \brief Constructs unit port with the given name and type.
	 * \param _name Name of the port.
	 * \param _type Type of the port.
	 */
	CUnitPort(std::string _name, EUnitPort _type);

	/**
	 * Returns port's name.
	 * \return Name of the port.
	 */
	std::string GetName() const;
	/**
	 * Sets port's name.
	 * \param _name New name of the port.
	 */
	void SetName(const std::string& _name);

	/**
	 * Returns port's type.
	 * \return Type of the port.
	 */
	EUnitPort GetType() const;
	/**
	 * Sets port's type.
	 * \param _type New type of the port.
	 */
	void SetType(EUnitPort _type);

	/**
	 * Returns key of the stream, connected to this port.
	 * \return Key of the stream connected to the port.
	 */
	std::string GetStreamKey() const;
	/**
	 * Sets key of the stream, connected to this port.
	 * \param _key New key of the stream.
	 */
	void SetStreamKey(const std::string& _key);

	/**
	 * Returns stream, connected to this port.
	 * \return Pointer to the stream, currently connected to this port.
	 */
	CStream* GetStream() const;
	/**
	 * Sets stream, connected to this port.
	 * \param _stream Pointer no a new stream.
	 */
	void SetStream(CStream* _stream);

	/**
	 * \private
	 * \brief Saves data to file.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data.
	 */
	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	/**
	 * \private
	 * \brief Loads data from file.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data.
	 */
	void LoadFromFile(CH5Handler& _h5File, const std::string& _path);
};

////////////////////////////////////////////////////////////////////////////////
// CPortsManager
//

/**
 * \brief Manager of unit ports.
 */
class CPortsManager
{
	static const unsigned m_saveVersion{ 1 };	// Current version of the saving procedure.

	std::vector<std::unique_ptr<CUnitPort>> m_ports;	// All defined ports.

public:
	/**
	 * \private
	 * \brief Copies user-defined data from _ports.
	 * \details Copies information about selected streams. Assumes the corresponding ports structure is the same.
	 * \param _ports Reference to source ports manager.
	 */
	void CopyUserData(const CPortsManager& _ports) const;

	/**
	 * Adds a port and returns a pointer to it. If a port with this name already exist, does nothing and return nullptr.
	 * \param _name Name of the port. Must be unique. If a port with this name already exist, does nothing.
	 * \param _type Type of the port.
	 * \return Pointer to the created port. If a port with this name already exist, returns nullptr.
	 */
	CUnitPort* AddPort(const std::string& _name, EUnitPort _type);
	/**
	 * Returns a port with the specified name.
	 * \param _name Name of the port.
	 * \return const pointer to the port. nullptr if such port does not exist.
	 */
	[[nodiscard]] const CUnitPort* GetPort(const std::string& _name) const;
	/**
	 * Returns a port with the specified name.
	 * \param _name Name of the port.
	 * \return Pointer to the port. nullptr if such port does not exist.
	 */
	CUnitPort* GetPort(const std::string& _name);
	/**
	 * Returns a port with the specified index.
	 * \param _index Index of the port.
	 * \return Const pointer to the port. nullptr if such port does not exist.
	 */
	[[nodiscard]] const CUnitPort* GetPort(size_t _index) const;
	/**
	 * Returns a port with the specified index.
	 * \param _index Index of the port.
	 * \return Pointer to the port. nullptr if such port does not exist.
	 */
	CUnitPort* GetPort(size_t _index);
	/**
	 * Returns pointers to all defined ports.
	 * \return Pointers to all defined ports.
	 */
	std::vector<CUnitPort*> GetAllPorts();
	/**
	 * Returns const pointers to all defined ports.
	 * \return Const pointers to all defined ports.
	 */
	std::vector<const CUnitPort*> GetAllPorts() const;
	/**
	 * Returns pointers to all defined input ports.
	 * \return Pointers to all input ports.
	 */
	std::vector<CUnitPort*> GetAllInputPorts();
	/**
	 * Returns const pointers to all defined input ports.
	 * \return Const pointers to all input ports.
	 */
	std::vector<const CUnitPort*> GetAllInputPorts() const;
	/**
	 * Returns pointers to all defined output ports.
	 * \return Pointers to all output ports.
	 */
	std::vector<CUnitPort*> GetAllOutputPorts();
	/**
	 * Returns const pointers to all defined output ports.
	 * \return Const pointers to all output ports.
	 */
	std::vector<const CUnitPort*> GetAllOutputPorts() const;

	/**
	 * Returns a number of defined ports.
	 * \return Number of ports.
	 */
	size_t GetPortsNumber() const;

	/**
	 * \private
	 * \brief Saves data to file.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data.
	 */
	void SaveToFile(CH5Handler& _h5File, const std::string& _path) const;
	/**
	 * \private
	 * \brief Loads data from file.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data.
	 */
	void LoadFromFile(CH5Handler& _h5File, const std::string& _path);
	/**
	 * \private
	 * \brief Loads data from file.
	 * \details A compatibility version.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data.
	 */
	void LoadFromFile_v0(const CH5Handler& _h5File, const std::string& _path);
	/**
	 * \private
	 * \brief Loads data from file.
	 * \details A compatibility version.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data.
	 */
	void LoadFromFile_v00(const CH5Handler& _h5File, const std::string& _path);

private:
	// Returns the names of all defined ports.
	std::vector<std::string> AllPortNames() const;
	// Returns keys of connected streams for all defined ports.
	std::vector<std::string> AllPortStreamKeys() const;
};
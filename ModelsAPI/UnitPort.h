/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <string>

class CStream;

// Description of the unit port for connecting material streams.
class CUnitPort
{
public:
	// TODO: rename
	// Types of unit ports.
	enum class EPortType2
	{
		INPUT = 0,
		OUTPUT = 1,
		UNDEFINED = 2
	};

private:
	std::string m_name;							// The name of the port, should be unique for the unit.
	EPortType2 m_type{ EPortType2::UNDEFINED };	// Port type: INPUT or OUTPUT.
	std::string m_streamKey;					// The key of the stream which is connected to this port.
	CStream* m_stream{ nullptr };				// Pointer to the connected material stream.

public:
	CUnitPort(std::string _name, EPortType2 _type);

	std::string GetName() const;				// Returns port's name.
	void SetName(const std::string& _name);		// Sets port's name.

	EPortType2 GetType() const;					// Returns port's type.
	void SetType(EPortType2 _type);				// Sets port's type.

	std::string GetStreamKey() const;			// Returns key of the stream, connected to this port.
	void SetStreamKey(const std::string& _key);	// Sets key of the stream, connected to this port.

	CStream* GetStream() const;					// Returns stream, connected to this port.
	void SetStream(CStream* _stream);			// Sets stream, connected to this port.
};


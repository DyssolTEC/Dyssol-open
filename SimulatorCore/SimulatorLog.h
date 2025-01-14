/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <string>
#include <vector>

/** Describes simulation log. Implements circular storage of messages by maintaining separate read and write positions for message buffering.
 *	Assumes that the distance between read and write positions never will be larger as MAX_LOG_SIZE. */
class CSimulatorLog
{
public:
	enum class ELogColor
	{
		DEFAULT = 0,
		RED = 1,
		ORANGE = 2
	};

private:
	struct SColorLog
	{
		ELogColor color;
		std::string text;
	};

	const size_t MAX_LOG_SIZE = 500;

	std::vector<SColorLog> m_log;
	size_t m_iReadPos;
	size_t m_iWritePos;

public:
	CSimulatorLog();
	~CSimulatorLog();

	// Removes all messages and resets read and write positions.
	void Clear();

	// Writes a message with the specified color to the current write position and advances this position. If _console is set, the message will be additionally written into std::out.
	void Write(const std::string& _text, ELogColor _color, bool _console);
	// Writes an info message with the pre-defined color to the current write position and advances this position. If _console is set, the message will be additionally written into std::out.
	void WriteInfo(const std::string& _text, bool _console = false);
	// Writes a warning message with the pre-defined color to the current write position and advances this position. If _console is set, the message will be additionally written into std::out.
	void WriteWarning(const std::string& _text, bool _console = true);
	// Writes an error message with the pre-defined color to the current write position and advances this position. If _console is set, the message will be additionally written into std::out.
	void WriteError(const std::string& _text, bool _console = true);

	// Returns message from the current read position and advances this position. Returns empty string if the end of log is reached.
	std::string Read();
	// Returns the complete log as a single string, containing all valid messages from start to current write position
	std::string GetFullLog() const;
	// Returns color from the current read position.
	ELogColor GetReadColor() const;

	// Returns true if the end of the log file is reached (the read position is equal to the write position).
	bool EndOfLog() const;
};


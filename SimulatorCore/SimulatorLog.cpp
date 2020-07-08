/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "SimulatorLog.h"
#include <iostream>

CSimulatorLog::CSimulatorLog()
{
	m_log.resize(MAX_LOG_SIZE);
	m_iReadPos = 0;
	m_iWritePos = 0;
}

CSimulatorLog::~CSimulatorLog()
{
}

void CSimulatorLog::Clear()
{
	for(auto l : m_log)
	{
		l.color = ELogColor::DEFAULT;
		l.text.clear();
	}
	m_iReadPos = m_iWritePos = 0;
}

void CSimulatorLog::Write(const std::string& _text, ELogColor _color, bool _console)
{
	const size_t iPos = m_iWritePos % MAX_LOG_SIZE; // up to MAX_LOG_SIZE and then again cyclic from 0
	m_log[iPos].text = _text;
	m_log[iPos].color = _color;
	m_iWritePos++;
	if(_console)
		std::cout << _text << std::endl;
}

void CSimulatorLog::WriteInfo(const std::string& _text, bool _console /*= false*/)
{
	Write(_text, ELogColor::DEFAULT, _console);
}

void CSimulatorLog::WriteWarning(const std::string& _text, bool _console /*= true*/)
{
	Write("Warning! " + _text, ELogColor::ORANGE, _console);
}

void CSimulatorLog::WriteError(const std::string& _text, bool _console /*= true*/)
{
	Write("Error! " + _text, ELogColor::RED, _console);
}

std::string CSimulatorLog::Read()
{
	if (EndOfLog()) return "";
	const size_t iPos = m_iReadPos % MAX_LOG_SIZE; // up to MAX_LOG_SIZE and then again cyclic from 0
	m_iReadPos++;
	return m_log[iPos].text;
}

CSimulatorLog::ELogColor CSimulatorLog::GetReadColor() const
{
	return m_log[m_iReadPos % MAX_LOG_SIZE].color;
}

bool CSimulatorLog::EndOfLog() const
{
	return m_iReadPos == m_iWritePos;
}

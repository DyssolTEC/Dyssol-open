/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "UnitPort.h"

CUnitPort::CUnitPort(std::string _name, EPortType2 _type) :
	m_name{ std::move(_name) },
	m_type{ _type }
{
}

std::string CUnitPort::GetName() const
{
	return m_name;
}

void CUnitPort::SetName(const std::string& _name)
{
	m_name = _name;
}

CUnitPort::EPortType2 CUnitPort::GetType() const
{
	return m_type;
}

void CUnitPort::SetType(EPortType2 _type)
{
	m_type = _type;
}

std::string CUnitPort::GetStreamKey() const
{
	return m_streamKey;
}

void CUnitPort::SetStreamKey(const std::string& _key)
{
	m_streamKey = _key;
}

CStream* CUnitPort::GetStream() const
{
	return m_stream;
}

void CUnitPort::SetStream(CStream* _stream)
{
	m_stream = _stream;
}
/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "UnitPorts.h"
#include "H5Handler.h"
#include "DyssolStringConstants.h"
#include "DyssolUtilities.h"

////////////////////////////////////////////////////////////////////////////////
// CUnitPort
//

CUnitPort::CUnitPort(std::string _name, EUnitPort _type) :
	m_name{ std::move(_name) },
	m_type{ _type }
{
}

const std::string& CUnitPort::GetName() const
{
	return m_name;
}

void CUnitPort::SetName(const std::string& _name)
{
	m_name = _name;
}

EUnitPort CUnitPort::GetType() const
{
	return m_type;
}

void CUnitPort::SetType(EUnitPort _type)
{
	m_type = _type;
}

const std::string& CUnitPort::GetStreamKey() const
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

void CUnitPort::SaveToFile(CH5Handler& _h5File, const std::string& _path) const
{
	if (!_h5File.IsValid()) return;

	// current version of save procedure
	_h5File.WriteAttribute(_path, StrConst::H5AttrSaveVersion, m_saveVersion);

	_h5File.WriteData(_path, StrConst::UPort_H5Name,      m_name);
	_h5File.WriteData(_path, StrConst::UPort_H5Type,      E2I(m_type));
	_h5File.WriteData(_path, StrConst::UPort_H5StreamKey, m_streamKey);
}

void CUnitPort::LoadFromFile(CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;

	// current version of save procedure
	//const int version = _h5File.ReadAttribute(_path, StrConst::H5AttrSaveVersion);

	_h5File.ReadData(_path, StrConst::UPort_H5Name,      m_name);
	_h5File.ReadData(_path, StrConst::UPort_H5StreamKey, m_streamKey);
}

////////////////////////////////////////////////////////////////////////////////
// CPortsManager
//

void CPortsManager::CopyUserData(const CPortsManager& _ports) const
{
	if (m_ports.size() != _ports.m_ports.size()) return;
	for (size_t i = 0; i < m_ports.size(); ++i)
		m_ports[i]->SetStreamKey(_ports.m_ports[i]->GetStreamKey());
}

CUnitPort* CPortsManager::AddPort(const std::string& _name, EUnitPort _type)
{
	if (GetPort(_name)) return nullptr;
	m_ports.emplace_back(new CUnitPort{ _name, _type });
	return m_ports.back().get();
}

const CUnitPort* CPortsManager::GetPort(const std::string& _name) const
{
	for (const auto& p : m_ports)
		if (p->GetName() == _name)
			return p.get();
	return nullptr;
}

CUnitPort* CPortsManager::GetPort(const std::string& _name)
{
	return const_cast<CUnitPort*>(const_cast<const CPortsManager&>(*this).GetPort(_name));
}

const CUnitPort* CPortsManager::GetPort(size_t _index) const
{
	if (_index >= m_ports.size()) return {};
	return m_ports[_index].get();
}

CUnitPort* CPortsManager::GetPort(size_t _index)
{
	return const_cast<CUnitPort*>(const_cast<const CPortsManager&>(*this).GetPort(_index));
}

std::vector<CUnitPort*> CPortsManager::GetAllPorts()
{
	std::vector<CUnitPort*> res;
	for (auto& p : m_ports)
		res.push_back(p.get());
	return res;
}

std::vector<const CUnitPort*> CPortsManager::GetAllPorts() const
{
	std::vector<const CUnitPort*> res;
	for (const auto& p : m_ports)
		res.push_back(p.get());
	return res;
}

std::vector<CUnitPort*> CPortsManager::GetAllInputPorts()
{
	std::vector<CUnitPort*> res;
	for (auto& p : m_ports)
		if (p->GetType() == EUnitPort::INPUT)
			res.push_back(p.get());
	return res;
}

std::vector<const CUnitPort*> CPortsManager::GetAllInputPorts() const
{
	std::vector<const CUnitPort*> res;
	for (const auto& p : m_ports)
		if (p->GetType() == EUnitPort::INPUT)
			res.push_back(p.get());
	return res;
}

std::vector<CUnitPort*> CPortsManager::GetAllOutputPorts()
{
	std::vector<CUnitPort*> res;
	for (auto& p : m_ports)
		if (p->GetType() == EUnitPort::OUTPUT)
			res.push_back(p.get());
	return res;
}

std::vector<const CUnitPort*> CPortsManager::GetAllOutputPorts() const
{
	std::vector<const CUnitPort*> res;
	for (const auto& p : m_ports)
		if (p->GetType() == EUnitPort::OUTPUT)
			res.push_back(p.get());
	return res;
}

size_t CPortsManager::GetPortsNumber() const
{
	return m_ports.size();
}

std::vector<std::string> CPortsManager::GetInputPortsNames() const
{
	std::vector<std::string> res;
	for (const auto& p : m_ports)
		if (p->GetType() == EUnitPort::INPUT)
			res.push_back(p->GetName());
	return res;
}

std::vector<std::string> CPortsManager::GetOutputPortsNames() const
{
	std::vector<std::string> res;
	for (const auto& p : m_ports)
		if (p->GetType() == EUnitPort::OUTPUT)
			res.push_back(p->GetName());
	return res;
}

void CPortsManager::Clear()
{
	m_ports.clear();
}

void CPortsManager::SaveToFile(CH5Handler& _h5File, const std::string& _path) const
{
	if (!_h5File.IsValid()) return;

	// current version of save procedure
	_h5File.WriteAttribute(_path, StrConst::H5AttrSaveVersion, m_saveVersion);

	_h5File.WriteData(_path, StrConst::UPort_H5PortsNames, AllPortNames());
	_h5File.WriteData(_path, StrConst::UPort_H5PortsKeys, AllPortStreamKeys());

	_h5File.WriteAttribute(_path, StrConst::UPort_H5AttrPortsNum, static_cast<int>(m_ports.size()));
	for (size_t i = 0; i < m_ports.size(); ++i)
	{
		const std::string portPath = _h5File.CreateGroup(_path, StrConst::UPort_H5GroupPortName + std::to_string(i));
		m_ports[i]->SaveToFile(_h5File, portPath);
	}
}

void CPortsManager::LoadFromFile(CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;

	// version of save procedure
	//const int version = _h5File.ReadAttribute(_path, StrConst::H5AttrSaveVersion);

	/* complex loading procedure with names, while users can change ports during the development of a unit.
	 * this approach allows to properly load even if the order or names of ports are changed by a developer. */
	std::vector<std::string> names, keys;
	_h5File.ReadData(_path, StrConst::UPort_H5PortsNames, names);
	_h5File.ReadData(_path, StrConst::UPort_H5PortsKeys, keys);
	std::vector<bool> portsLoaded(m_ports.size(), false);	// whether an existing port is already loaded
	std::vector<bool> portsReaded(names.size(), false);		// whether a saved port is already used to load an existing port
	const std::string portPath = _path + "/" + StrConst::UPort_H5GroupPortName;
	// try to load by names
	for (size_t iExist = 0; iExist < m_ports.size(); ++iExist)
		for (size_t iSaved = 0; iSaved < names.size(); ++iSaved)
			if (m_ports[iExist]->GetName() == names[iSaved])
			{
				m_ports[iExist]->LoadFromFile(_h5File, portPath + std::to_string(iSaved));
				portsReaded[iSaved] = true;
				portsLoaded[iExist] = true;
				break;
			}
	// load rest by positions
	for (size_t i = 0; i < m_ports.size(); ++i)
		if (!portsLoaded[i] && i < portsReaded.size() && !portsReaded[i])
		{
			const std::string name = m_ports[i]->GetName();
			m_ports[i]->LoadFromFile(_h5File, portPath + std::to_string(i));
			m_ports[i]->SetName(name);
		}
}

void CPortsManager::LoadFromFile_v0(const CH5Handler& _h5File, const std::string& _path)
{
	if (!_h5File.IsValid()) return;

	/* complex loading procedure with names, while users can change ports during the development of a unit.
	* this approach allows to properly load even if the order or names of ports are changed by a developer. */
	std::vector<std::string> names, keys;
	_h5File.ReadData(_path, StrConst::UPort_H5PortsNames, names);
	_h5File.ReadData(_path, StrConst::UPort_H5PortsKeys, keys);
	std::vector<bool> portsLoaded(m_ports.size(), false);	// whether an existing port is already loaded
	std::vector<bool> portsReaded(names.size(), false);		// whether a saved port is already used to load an existing port
	// try to load by names
	for (size_t iExist = 0; iExist < m_ports.size(); ++iExist)
		for (size_t iSaved = 0; iSaved < names.size(); ++iSaved)
			if (m_ports[iExist]->GetName() == names[iSaved])
			{
				m_ports[iExist]->SetStreamKey(keys[iSaved]);
				portsReaded[iSaved] = true;
				portsLoaded[iExist] = true;
				break;
			}
	// load rest by positions
	for (size_t i = 0; i < m_ports.size(); ++i)
		if (!portsLoaded[i] && i < portsReaded.size() && !portsReaded[i])
			m_ports[i]->SetStreamKey(keys[i]);
}

void CPortsManager::LoadFromFile_v00(const CH5Handler& _h5File, const std::string& _path)
{
	std::vector<std::string> keys;
	_h5File.ReadData(_path, StrConst::BUnit_H5UnitPorts, keys);
	for (size_t i = 0; i < keys.size() && i < m_ports.size(); ++i)
		m_ports[i]->SetStreamKey(keys[i]);
}

std::vector<std::string> CPortsManager::AllPortNames() const
{
	std::vector<std::string> res;
	for (const auto& p : m_ports)
		res.push_back(p->GetName());
	return res;
}

std::vector<std::string> CPortsManager::AllPortStreamKeys() const
{
	std::vector<std::string> res;
	for (const auto& p : m_ports)
		res.push_back(p->GetStreamKey());
	return res;
}

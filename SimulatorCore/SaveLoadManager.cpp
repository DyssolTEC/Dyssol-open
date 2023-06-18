/* Copyright (c) 2023, DyssolTEC. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "SaveLoadManager.h"
#include "DyssolStringConstants.h"
#include "Flowsheet.h"

CSaveLoadManager::CSaveLoadManager(CFlowsheet* _flowsheet)
	: m_flowsheet{ _flowsheet }
{
}

std::filesystem::path CSaveLoadManager::GetFileName() const
{
	return m_fileHandler.FileName();
}

bool CSaveLoadManager::SaveToFile(const std::filesystem::path& _fileName)
{
	if (_fileName.empty()) return false;

	// TODO: m_parameters.fileSingleFlag
	m_fileHandler.Create(_fileName);

	if (!m_fileHandler.IsValid()) return false;

	const std::string root = "/";

	// current version of save procedure
	m_fileHandler.WriteAttribute(root, StrConst::H5AttrSaveVersion, m_saveVersion);

	bool success = true;

	// save flowsheet
	if (m_flowsheet && success)
	{
		const std::string flowsheetGroup = m_fileHandler.CreateGroup(root, StrConst::SLM_H5GroupFlowsheet);
		success &= m_flowsheet->SaveToFile(m_fileHandler, flowsheetGroup);
		if (success) m_flowsheet->SetFileName(_fileName);
	}

	m_fileHandler.Close();

	return success;
}

bool CSaveLoadManager::LoadFromFile(const std::filesystem::path& _fileName)
{
	if (_fileName.empty()) return false;

	m_fileHandler.Open(_fileName);

	if (!m_fileHandler.IsValid()) return false;

	const std::string root = "/";

	// version of save procedure
	const int version = m_fileHandler.ReadAttribute(root, StrConst::H5AttrSaveVersion);

	// compatibility with older versions
	if (m_flowsheet && version <= 5)
	{
		const bool res = m_flowsheet->LoadFromFile(m_fileHandler, root);
		m_flowsheet->SetFileName(_fileName);
		m_fileHandler.Close();
		return res;
	}

	bool success = true;

	// load flowsheet
	if (m_flowsheet && success) success &= m_flowsheet->LoadFromFile(m_fileHandler, StrConst::SLM_H5GroupFlowsheet);
	if (m_flowsheet && success) m_flowsheet->SetFileName(_fileName);

	m_fileHandler.Close();

	return success;
}

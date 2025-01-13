/* Copyright (c) 2023, DyssolTEC. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "SaveLoadManager.h"
#include "DyssolStringConstants.h"
#include "Flowsheet.h"

CSaveLoadManager::CSaveLoadManager(const SSaveLoadData& _data)
	: m_data{ _data }
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
	if (m_data.flowsheet && success)
	{
		const std::string flowsheetGroup = m_fileHandler.CreateGroup(root, StrConst::SLM_H5GroupFlowsheet);
		success &= m_data.flowsheet->SaveToFile(m_fileHandler, flowsheetGroup);
		if (success) m_data.flowsheet->SetFileName(_fileName);
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

	bool success = true;

	// compatibility with older versions
	const auto rootPath = version <= 5 ? root : StrConst::SLM_H5GroupFlowsheet;

	// load flowsheet
	if (m_data.flowsheet && success)
	{
		success &= m_data.flowsheet->LoadFromFile(m_fileHandler, rootPath);

		if(success)
			m_data.flowsheet->SetFileName(_fileName);
	}

	m_fileHandler.Close();

	return success;
}

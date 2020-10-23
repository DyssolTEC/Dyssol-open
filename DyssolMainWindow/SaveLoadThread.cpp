/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "SaveLoadThread.h"
#include "Flowsheet.h"

CSaveLoadThread::CSaveLoadThread(CFlowsheet* _pFlowsheet, bool _bSaver, QObject* parent /*= nullptr*/)
	: CBasicThread(parent),
	m_pFlowsheet{ _pFlowsheet },
	m_bSuccess{ false },
	m_bSaverFlag{ _bSaver },
	m_blocked{ false }
{
}

void CSaveLoadThread::SetFileName(const QString &_sFileName)
{
	m_sFileName = _sFileName;
}

QString CSaveLoadThread::GetFileName() const
{
	return m_sFileName;
}

QString CSaveLoadThread::GetFinalFileName() const
{
	return QString::fromStdWString(m_fileHandler.FileName());
}

bool CSaveLoadThread::IsSuccess() const
{
	return m_bSuccess;
}

void CSaveLoadThread::Block()
{
	m_blocked = true;
}

void CSaveLoadThread::StartTask()
{
	if (!m_blocked)
		if (m_bSaverFlag)
			m_bSuccess = m_pFlowsheet->SaveToFile(m_fileHandler, m_sFileName.toStdWString());
		else
			m_bSuccess = m_pFlowsheet->LoadFromFile(m_fileHandler, m_sFileName.toStdWString());
	else
		m_bSuccess = false;
	emit Finished();
}

void CSaveLoadThread::StopTask() {}

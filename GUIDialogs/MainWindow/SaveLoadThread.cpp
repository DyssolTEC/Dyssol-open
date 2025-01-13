/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2024, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "SaveLoadThread.h"
#include "Flowsheet.h"

CSaveLoadThread::CSaveLoadThread(const SSaveLoadData& _data, bool _saver, QObject* _parent /*= nullptr*/)
	: CBasicThread{ _parent }
	, m_fileHandler{ _data }
	, m_isSaver{ _saver }
{
}

void CSaveLoadThread::SetFileName(const QString& _fileName)
{
	m_fileName = _fileName;
}

QString CSaveLoadThread::GetFileName() const
{
	return m_fileName;
}

QString CSaveLoadThread::GetFinalFileName() const
{
	return QString::fromStdWString(m_fileHandler.GetFileName().wstring());
}

bool CSaveLoadThread::IsSuccess() const
{
	return m_isSuccess;
}

void CSaveLoadThread::Block()
{
	m_isBlocked = true;
}

void CSaveLoadThread::StartTask()
{
	if (!m_isBlocked)
	{
		if(m_isSaver)
		{
			m_isSuccess = m_fileHandler.SaveToFile(m_fileName.toStdWString());
		}
		else
		{
			m_isSuccess = m_fileHandler.LoadFromFile(m_fileName.toStdWString());
		}
	}
	else
	{
		m_isSuccess = false;
	}

	emit Finished();
}

void CSaveLoadThread::RequestStop() {}

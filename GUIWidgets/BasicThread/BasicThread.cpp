/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "BasicThread.h"

CBasicThread::CBasicThread(QObject *parent /*= nullptr*/)
	: QObject(parent)
{
	connect(&m_Thread, &QThread::started, this, &CBasicThread::StartTask);
}

void CBasicThread::Run()
{
	this->moveToThread(&m_Thread);
	m_Thread.start();
}

void CBasicThread::Stop()
{
	m_Thread.exit();
}

bool CBasicThread::IsRunning() const
{
	return m_Thread.isRunning();
}

bool CBasicThread::IsFinished() const
{
	return m_Thread.isFinished();
}

bool CBasicThread::Wait(unsigned long _nTime /*= ULONG_MAX */)
{
	return m_Thread.wait(_nTime);
}

void CBasicThread::StartTask()
{
	emit Finished();
}

void CBasicThread::RequestStop()
{

}

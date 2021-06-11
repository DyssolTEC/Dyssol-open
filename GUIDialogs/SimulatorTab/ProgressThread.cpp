/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ProgressThread.h"

CProgressThread::CProgressThread(CSimulator* _pSimulator, QObject* _parent /*= nullptr*/) :
	CBasicThread(_parent)
{
	m_pSimulator = _pSimulator;
}

bool CProgressThread::WasAborted() const
{
	return m_aborted;
}

void CProgressThread::StartTask()
{
	m_aborted = false;	// reset flag
	m_pSimulator->Simulate();
	emit Finished();
}

void CProgressThread::RequestStop()
{
	if (m_pSimulator->GetCurrentStatus() != ESimulatorStatus::SIMULATOR_IDLE)
		m_pSimulator->SetCurrentStatus(ESimulatorStatus::SIMULATOR_SHOULD_BE_STOPPED);
	m_aborted = true;
}

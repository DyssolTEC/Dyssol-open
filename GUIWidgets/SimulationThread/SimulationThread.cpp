/* Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "SimulationThread.h"

CSimulationThread::CSimulationThread(std::function<void()> _run, std::function<void()> _stop, QObject* _parent) :
	CBasicThread(_parent)
{
	m_run = _run;
	m_stop = _stop;
}

bool CSimulationThread::WasAborted() const
{
	return m_aborted;
}

void CSimulationThread::StartTask()
{
	m_aborted = false;	// reset flag
	m_run();
	emit Finished();
}

void CSimulationThread::RequestStop()
{
	m_stop();
	m_aborted = true;
}

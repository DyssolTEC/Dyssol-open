/* Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "SimulationThread.h"

CSimulationThread::CSimulationThread(const RunFunc& _run, const StopFunc& _stop, QObject* _parent)
	: CBasicThread{ _parent }
	, m_run{ _run }
	, m_stop{ _stop }
{
}

CSimulationThread::CSimulationThread(QObject* _parent)
	: CSimulationThread{ RunFunc{}, StopFunc{}, _parent }
{
}

void CSimulationThread::SetFunctions(const RunFunc& _run, const StopFunc& _stop)
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

	if (m_run)
		m_run();

	emit Finished();
}

void CSimulationThread::RequestStop()
{
	if (m_stop)
		m_stop();

	m_aborted = true;
}

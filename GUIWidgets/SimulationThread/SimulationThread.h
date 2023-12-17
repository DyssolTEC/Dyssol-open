/* Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BasicThread.h"

class CSimulationThread : public CBasicThread
{
	Q_OBJECT

	bool m_aborted{ false }; // Whether the simulation was aborted.
	std::function<void()> m_run; // Function to run simulation
	std::function<void()> m_stop; // Function to abort simulation

public:
	CSimulationThread(std::function<void()> _run, std::function<void()> _stop, QObject* _parent = nullptr);

	bool WasAborted() const;	// Returns whether the simulation was aborted by user.

public slots:
	void StartTask() override;
	void RequestStop() override;
};

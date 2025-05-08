/* Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BasicThread.h"

class CSimulationThread : public CBasicThread
{
	Q_OBJECT
public:
	using RunFunc = std::function<void()>;
	using StopFunc = std::function<void()>;

public:
	CSimulationThread(const RunFunc& _run, const StopFunc& _stop, QObject* _parent = nullptr);
	CSimulationThread(QObject* _parent = nullptr);

	void SetFunctions(const RunFunc& _run, const StopFunc& _stop);

	bool WasAborted() const; // Returns whether the simulation was aborted by user.

	void StartTask() override;
	void RequestStop() override;

private:
	bool m_aborted{ false }; // Whether the simulation was aborted.
	RunFunc m_run;           // Function to run simulation
	StopFunc m_stop;         // Function to abort simulation
};

/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BasicThread.h"
#include "Simulator.h"

class CProgressThread : public CBasicThread
{
	Q_OBJECT

	CSimulator* m_pSimulator;
	bool m_aborted{ false }; // Whether the simulation was aborted.

public:
	CProgressThread(CSimulator* _pSimulator, QObject* _parent = nullptr);

	bool WasAborted() const;	// Returns whether the simulation was aborted by user.

public slots:
	void StartTask() override;
	void RequestStop() override;
};

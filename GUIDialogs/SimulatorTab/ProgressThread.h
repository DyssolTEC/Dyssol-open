/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BasicThread.h"
#include "Simulator.h"

class CProgressThread : public CBasicThread
{
	Q_OBJECT

	CSimulator* m_pSimulator;

public:
	CProgressThread(CSimulator* _pSimulator, QObject* _parent = nullptr);

public slots:
	void StartTask() override;
	void StopTask() override;
};

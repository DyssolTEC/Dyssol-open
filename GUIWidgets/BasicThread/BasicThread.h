/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <QThread>

class CBasicThread : public QObject
{
	Q_OBJECT

protected:
	QThread m_Thread;

public:
	CBasicThread(QObject *parent = nullptr);

	void Run();
	void Stop();

	bool IsRunning() const;
	bool IsFinished() const;

	bool Wait(unsigned long _nTime = ULONG_MAX);

public slots:
	virtual void StartTask();
	virtual void RequestStop();

signals:
	void Finished();
};

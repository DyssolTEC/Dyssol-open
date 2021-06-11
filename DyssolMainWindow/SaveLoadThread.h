/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BasicThread.h"
#include "H5Handler.h"

class CFlowsheet;

/** Flowsheet saver/loader thread */
class CSaveLoadThread : public CBasicThread
{
	Q_OBJECT

public:

private:
	CH5Handler m_fileHandler;
	QString m_sFileName;
	CFlowsheet *m_pFlowsheet;
	bool m_bSuccess;
	bool m_bSaverFlag;	// true for saving, false for loading
	bool m_blocked;

public:
	CSaveLoadThread(CFlowsheet* _pFlowsheet, bool _bSaver, QObject* parent = nullptr);
	void SetFileName(const QString& _sFileName);
	QString GetFileName() const;
	QString GetFinalFileName() const;	// Returns possibly transformed file name that was really used during saving/loading.
	bool IsSuccess() const;				// Returns true if saving/loading operation succeed.
	void Block();						// Blocks saving/loading operation.

public slots:
	void StartTask() override;
	void RequestStop() override;
};

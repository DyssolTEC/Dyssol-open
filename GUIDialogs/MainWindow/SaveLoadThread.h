/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2024, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BasicThread.h"
#include "SaveLoadManager.h"

class CFlowsheet;

/** Flowsheet saver/loader thread */
class CSaveLoadThread : public CBasicThread
{
	Q_OBJECT

	QString m_fileName;
	CSaveLoadManager m_fileHandler{}; /// Data saver/loader.
	bool m_isSuccess{};
	bool m_isSaver{};	              /// true for saving, false for loading
	bool m_isBlocked{};

public:
	CSaveLoadThread(const SSaveLoadData& _data, bool _saver, QObject* _parent = nullptr);
	void SetFileName(const QString& _fileName);
	[[nodiscard]] QString GetFileName() const;
	[[nodiscard]] QString GetFinalFileName() const; // Returns possibly transformed file name that was really used during saving/loading.
	[[nodiscard]] bool IsSuccess() const;           // Returns true if saving/loading operation succeed.
	void Block();                                   // Blocks saving/loading operation.

public slots:
	void StartTask() override;
	void RequestStop() override;
};

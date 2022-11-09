/* Copyright (c) 2023, DyssolTEC. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <QDialog>
#include "ModelsManager.h"

class CQtDialog :
	public QDialog
{
	Q_OBJECT

public:
	CModelsManager* m_modelsManager{}; // Pointer to models manager.

private:
	QString m_helpLink;

public:
	CQtDialog(CModelsManager* _modelsManager, QWidget* _parent);
	CQtDialog(CModelsManager* _modelsManager, QWidget* _parent, Qt::WindowFlags _flags);

	void SetHelpLink(const QString& _helpLink);
	QString GetHelpLink();

	/**
	 * Opens help file.
	 */
	void OpenHelp(const QString& _link);
};

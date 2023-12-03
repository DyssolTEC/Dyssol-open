/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_SettingsEditor.h"
#include "QtDialog.h"
#include <QSettings>

class CSettingsEditor
	: public CQtDialog
{
	Q_OBJECT

	Ui::CSettingsEditorClass ui;

	QString m_currCachePath; // Currently used cache path.

public:
	CSettingsEditor(QWidget* _parent = nullptr);

	void SetPointers(CModelsManager* _modelsManager, QSettings* _settings) override;
	void InitializeConnections() const;

public slots:
	void UpdateWholeView() const;
	void setVisible(bool _bVisible) override;

private:
	void UpdateWarningsVisible() const;

private slots:
	void OpenConfigFile() const;
	void ChangeCachePath();
	void ClearCacheClicked();

	void ApplyChanges();

signals:
	void NeedRestart();
	void NeedCacheClear();
};

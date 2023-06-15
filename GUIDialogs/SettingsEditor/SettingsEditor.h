/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_SettingsEditor.h"
#include "QtDialog.h"
#include <QSettings>

class CSettingsEditor : public CQtDialog
{
	Q_OBJECT

	Ui::CSettingsEditorClass ui;

	QSettings *m_pSettings;
	QString m_currCachePath; // Currently used cache path.

public:
	CSettingsEditor(QSettings* _pSettings, CModelsManager* _modelsManager, QWidget* _parent = nullptr);

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

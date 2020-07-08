/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_ModulesManagerTab.h"
#include "ModelsManager.h"
#include <QSettings>
#include <QCheckBox>

class CModulesManagerTab: public QDialog
{
	Q_OBJECT
private:
	Ui::CModulesManagerClass ui;
	CModelsManager* m_pModelsManager;
	QSettings *m_pSettings;

public:
	CModulesManagerTab(CModelsManager* _pModelsManager, QSettings* _pSettings, QWidget* parent = nullptr);

	void InitializeConnections() const;

public slots:
	void setVisible(bool _bVisible) override;

	void UpdateWholeView() const;

private:
	void UpdateModels();
	void UpdateDirsView() const;
	void UpdateModelsView() const;
	void SetModelInfoToTable(const SModelDescriptor& _model, const QString& _type) const;

	void UpdateConfigFile() const;

private slots:
	void AddDir();
	void RemoveDir();
	void UpDir();
	void DownDir();
	void ChangeDirActivity(int _iRow, int _iCol, QCheckBox* _pCheckBox);

signals:
	void ModelsListWasChanged(); // when models list has been changed
};

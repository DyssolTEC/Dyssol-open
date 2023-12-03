/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_ModulesManagerTab.h"
#include "ModelsManager.h"
#include "QtDialog.h"
#include <QSettings>
#include <QCheckBox>

class CModulesManagerTab
	: public CQtDialog
{
	Q_OBJECT
private:
	Ui::CModulesManagerClass ui;

public:
	CModulesManagerTab(QWidget* _parent = nullptr);

	void SetPointers(CModelsManager* _modelsManager, QSettings* _settings) override;

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

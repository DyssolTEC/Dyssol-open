/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "SettingsEditor.h"
#include "SignalBlocker.h"
#include "DyssolStringConstants.h"
#include <QMessageBox>
#include <QDesktopServices>
#include <QFileDialog>

CSettingsEditor::CSettingsEditor(QWidget* _parent)
	: CQtDialog{ _parent }
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	ui.labelWarningCachePath->setVisible(false);
	ui.labelWarning->setVisible(false);

	SetHelpLink("001_ui/gui.html#sec-gui-menu-tools-settings");
}

void CSettingsEditor::SetPointers(CModelsManager* _modelsManager, QSettings* _settings)
{
	CDyssolBaseWidget::SetPointers(_modelsManager, _settings);

	m_currCachePath = m_settings->value(StrConst::Dyssol_ConfigCachePath).toString();
}

void CSettingsEditor::InitializeConnections() const
{
	connect(ui.buttonBox,             &QDialogButtonBox::accepted, this, &CSettingsEditor::ApplyChanges);
	connect(ui.buttonBox,             &QDialogButtonBox::rejected, this, &CSettingsEditor::close);
	connect(ui.pushButtonOpenConfig,  &QPushButton::clicked,       this, &CSettingsEditor::OpenConfigFile);
	connect(ui.pushButtonChangeCache, &QPushButton::clicked,       this, &CSettingsEditor::ChangeCachePath);
	connect(ui.pushButtonClearCache,  &QPushButton::clicked,       this, &CSettingsEditor::ClearCacheClicked);
}

void CSettingsEditor::UpdateWholeView() const
{
	ui.checkBoxLoadLast->setChecked(m_settings->value(StrConst::Dyssol_ConfigLoadLastFlag).toBool());
	ui.lineEditCachePath->setText(m_settings->value(StrConst::Dyssol_ConfigCachePath).toString());

	UpdateWarningsVisible();
}

void CSettingsEditor::setVisible(bool _bVisible)
{
	if (_bVisible && !isVisible())
		UpdateWholeView();
	QDialog::setVisible(_bVisible);
}

void CSettingsEditor::UpdateWarningsVisible() const
{
	const bool bVisible = ui.lineEditCachePath->text() != m_currCachePath;
	ui.labelWarningCachePath->setVisible(bVisible);
	ui.labelWarning->setVisible(bVisible);
}

void CSettingsEditor::OpenConfigFile() const
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(m_settings->fileName()));
}

void CSettingsEditor::ChangeCachePath()
{
	const QString dir = QFileDialog::getExistingDirectory(this, "Select new path for cache", ui.lineEditCachePath->text(), QFileDialog::ShowDirsOnly);
	if (dir.isEmpty()) return;

	if (dir == m_currCachePath)
		ui.lineEditCachePath->setText(dir);
	else
	{
		const auto reply = QMessageBox::question(this, "Confirm restart",
			"This parameter will be applied after the program restart. Restart now?",
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if (reply != QMessageBox::Cancel)
			ui.lineEditCachePath->setText(dir);
		if (reply == QMessageBox::Yes)
		{
			ApplyChanges();
			emit NeedRestart();
		}
	}

	UpdateWarningsVisible();
}

void CSettingsEditor::ClearCacheClicked()
{
	const auto reply = QMessageBox::question(this, "Clear cache",
		"To apply, Dyssol will be restarted. All other instances of Dyssol must be previously manually closed. Proceed now?",
		QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	if (reply == QMessageBox::Yes)
		emit NeedCacheClear();
}

void CSettingsEditor::ApplyChanges()
{
	m_settings->setValue(StrConst::Dyssol_ConfigLoadLastFlag, ui.checkBoxLoadLast->isChecked());
	m_settings->setValue(StrConst::Dyssol_ConfigCachePath, ui.lineEditCachePath->text());

	QDialog::accept();
}

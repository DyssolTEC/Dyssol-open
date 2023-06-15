/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ModulesManagerTab.h"
#include "ModelsManager.h"
#include "DyssolStringConstants.h"
#include <QFileDialog>
#include <QDateTime>

CModulesManagerTab::CModulesManagerTab(CModelsManager* _pModelsManager, QSettings* _pSettings, QWidget* _parent)
	: CQtDialog{ _pModelsManager, _parent }
	, m_pSettings{ _pSettings }
{
	ui.setupUi(this);
	ui.tableModels->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.tableDirs->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	ui.tableDirs->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);

	// clear models manager
	m_modelsManager->Clear();

	// initialize units dirs
	const QStringList dirs = m_pSettings->value(StrConst::MM_ConfigModelsParamName).toStringList();
	const QList<bool> dirsFlags = m_pSettings->value(StrConst::MM_ConfigModelsFlagsParamName).value<QList<bool>>();
	for (int i = 0; i < dirs.size(); ++i)
		m_modelsManager->AddDir(dirs[i].toStdWString(), dirsFlags.size() < dirs.size() ? true : dirsFlags[i]);

	SetHelpLink("001_ui/gui.html#sec-gui-menu-tools-models-manager");
}

void CModulesManagerTab::InitializeConnections() const
{
	connect(ui.buttonAddDir,  &QPushButton::clicked,		   this, &CModulesManagerTab::AddDir);
	connect(ui.buttonRemDir,  &QPushButton::clicked,		   this, &CModulesManagerTab::RemoveDir);
	connect(ui.buttonUpDir,	  &QPushButton::clicked,		   this, &CModulesManagerTab::UpDir);
	connect(ui.buttonDownDir, &QPushButton::clicked,		   this, &CModulesManagerTab::DownDir);
	connect(ui.tableDirs,	  &CQtTable::CheckBoxStateChanged, this, &CModulesManagerTab::ChangeDirActivity);
}

void CModulesManagerTab::setVisible(bool _bVisible)
{
	QDialog::setVisible(_bVisible);
	if (_bVisible)
		UpdateWholeView();
}

void CModulesManagerTab::UpdateWholeView() const
{
	UpdateDirsView();
	UpdateModelsView();
}

void CModulesManagerTab::UpdateModels()
{
	UpdateWholeView();
	UpdateConfigFile();
	emit ModelsListWasChanged();
}

void CModulesManagerTab::UpdateDirsView() const
{
	QSignalBlocker blocker(ui.tableDirs);

	ui.tableDirs->clearContents();
	ui.tableDirs->setRowCount(static_cast<int>(m_modelsManager->DirsNumber()));
	for (int i = 0; i < static_cast<int>(m_modelsManager->DirsNumber()); ++i)
	{
		ui.tableDirs->SetItemNotEditable(i, 0, m_modelsManager->GetDirPath(i));
		ui.tableDirs->SetCheckBox(i, 1, m_modelsManager->GetDirActivity(i));
	}
}

void CModulesManagerTab::UpdateModelsView() const
{
	QSignalBlocker blocker(ui.tableModels);
	ui.tableModels->setSortingEnabled(false);

	ui.tableModels->clearContents();
	ui.tableModels->setRowCount(0);

	// go through all units
	for (const auto& u : m_modelsManager->GetAvailableUnits())
		SetModelInfoToTable(u, u.isDynamic ? QString(StrConst::MMT_Dynamic) : StrConst::MMT_SteadyState);

	// go through all solvers
	for (const auto& s : m_modelsManager->GetAvailableSolvers())
		SetModelInfoToTable(s, QStringList{ SOLVERS_TYPE_NAMES }[static_cast<unsigned>(s.solverType)] + StrConst::MMT_Solver);

	ui.tableModels->setSortingEnabled(true);
}

void CModulesManagerTab::SetModelInfoToTable(const SModelDescriptor& _model, const QString& _type) const
{
	const QDateTime date = QFileInfo(QString::fromStdWString(_model.fileLocation.wstring())).lastModified();
	const int row = ui.tableModels->rowCount();
	ui.tableModels->insertRow(ui.tableModels->rowCount());
	ui.tableModels->SetItemNotEditable(row, 0, _model.name        , QString::fromStdString(_model.uniqueID)); // set name
	ui.tableModels->SetItemNotEditable(row, 1, _model.fileLocation, QString::fromStdString(_model.uniqueID)); // set path
	ui.tableModels->SetItemNotEditable(row, 2, _type              , QString::fromStdString(_model.uniqueID)); // set type
	ui.tableModels->SetItemNotEditable(row, 3, _model.version     , QString::fromStdString(_model.uniqueID)); // set interface version
	ui.tableModels->SetItemNotEditable(row, 4, _model.author      , QString::fromStdString(_model.uniqueID)); // set author name
	ui.tableModels->SetItemNotEditable(row, 5, date.toString()    , QString::fromStdString(_model.uniqueID)); // set creation date
}

void CModulesManagerTab::UpdateConfigFile() const
{
	QStringList unitDirs;
	QList<bool> unitDirsFlags;
	for (size_t i = 0; i < m_modelsManager->DirsNumber(); ++i)
	{
		unitDirs.push_back(QString::fromStdWString(m_modelsManager->GetDirPath(i).wstring()));
		unitDirsFlags.push_back(m_modelsManager->GetDirActivity(i));
	}
	m_pSettings->setValue(StrConst::MM_ConfigModelsParamName, unitDirs);
	m_pSettings->setValue(StrConst::MM_ConfigModelsFlagsParamName, QVariant::fromValue(unitDirsFlags));
}

void CModulesManagerTab::AddDir()
{
	const QString folder = QFileDialog::getExistingDirectory(this, "Select directory", ".", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (folder.isEmpty()) return;
	QApplication::setOverrideCursor(Qt::WaitCursor);
	if (m_modelsManager->AddDir(folder.toStdWString()))
	{
		UpdateModels();
		ui.tableDirs->RestoreSelectedCell(ui.tableDirs->rowCount() - 1, 0);
	}
	QApplication::restoreOverrideCursor();
}

void CModulesManagerTab::RemoveDir()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	const int iRow = ui.tableDirs->currentRow();
	if(m_modelsManager->RemoveDir(iRow))
	{
		UpdateModels();
		ui.tableDirs->RestoreSelectedCell(iRow, 0);
	}
	QApplication::restoreOverrideCursor();
}

void CModulesManagerTab::UpDir()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	const int iRow = ui.tableDirs->currentRow();
	if (m_modelsManager->UpDir(iRow))
	{
		UpdateModels();
		ui.tableDirs->RestoreSelectedCell(iRow - 1, 0);
	}
	QApplication::restoreOverrideCursor();
}

void CModulesManagerTab::DownDir()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	const int iRow = ui.tableDirs->currentRow();
	if (m_modelsManager->DownDir(iRow))
	{
		UpdateModels();
		ui.tableDirs->RestoreSelectedCell(iRow + 1, 0);
	}
	QApplication::restoreOverrideCursor();
}

void CModulesManagerTab::ChangeDirActivity(int _iRow, int _iCol, QCheckBox* _pCheckBox)
{
	m_modelsManager->SetDirActivity(_iRow, _pCheckBox->isChecked());
	UpdateModelsView();
	UpdateConfigFile();
	emit ModelsListWasChanged();
}

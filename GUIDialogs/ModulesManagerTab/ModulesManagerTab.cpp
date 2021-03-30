/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ModulesManagerTab.h"
#include "ModelsManager.h"
#include "DyssolStringConstants.h"
#include <QFileDialog>
#include <QDateTime>

CModulesManagerTab::CModulesManagerTab(CModelsManager* _pModelsManager, QSettings* _pSettings, QWidget* parent) :
	QDialog(parent),
	m_pModelsManager(_pModelsManager),
	m_pSettings(_pSettings)
{
	ui.setupUi(this);
	ui.tableModels->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.tableDirs->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	ui.tableDirs->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);

	// clear models manager
	m_pModelsManager->Clear();

	// initialize units dirs
	const QStringList dirs = m_pSettings->value(StrConst::MM_ConfigModelsParamName).toStringList();
	const QList<bool> dirsFlags = m_pSettings->value(StrConst::MM_ConfigModelsFlagsParamName).value<QList<bool>>();
	for (int i = 0; i < dirs.size(); ++i)
		m_pModelsManager->AddDir(dirs[i].toStdWString(), dirsFlags.size() < dirs.size() ? true : dirsFlags[i]);
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
	ui.tableDirs->setRowCount(static_cast<int>(m_pModelsManager->DirsNumber()));
	for (int i = 0; i < static_cast<int>(m_pModelsManager->DirsNumber()); ++i)
	{
		ui.tableDirs->SetItemNotEditable(i, 0, m_pModelsManager->GetDirPath(i));
		ui.tableDirs->SetCheckBox(i, 1, m_pModelsManager->GetDirActivity(i));
	}
}

void CModulesManagerTab::UpdateModelsView() const
{
	QSignalBlocker blocker(ui.tableModels);
	ui.tableModels->setSortingEnabled(false);

	ui.tableModels->clearContents();
	ui.tableModels->setRowCount(0);

	// go through all units
	for (const auto& u : m_pModelsManager->GetAvailableUnits())
		SetModelInfoToTable(u, u.isDynamic ? QString(StrConst::MMT_Dynamic) : StrConst::MMT_SteadyState);

	// go through all solvers
	for (const auto& s : m_pModelsManager->GetAvailableSolvers())
		SetModelInfoToTable(s, QStringList{ SOLVERS_TYPE_NAMES }[static_cast<unsigned>(s.solverType)] + StrConst::MMT_Solver);

	ui.tableModels->setSortingEnabled(true);
}

void CModulesManagerTab::SetModelInfoToTable(const SModelDescriptor& _model, const QString& _type) const
{
	const QDateTime date = QFileInfo(QString::fromStdWString(_model.fileLocation)).lastModified();
	const int row = ui.tableModels->rowCount();
	ui.tableModels->insertRow(ui.tableModels->rowCount());
	ui.tableModels->SetItemNotEditable(row, 0, _model.name, QString::fromStdString(_model.name));		          // set name
	ui.tableModels->SetItemNotEditable(row, 1, _model.fileLocation, QString::fromStdWString(_model.fileLocation)); // set path
	ui.tableModels->SetItemNotEditable(row, 2, _type, _type);                                                     // set type
	ui.tableModels->SetItemNotEditable(row, 3, _model.version, static_cast<uint>(_model.version));	                                  // set interface version
	ui.tableModels->SetItemNotEditable(row, 4, _model.author, QString::fromStdString(_model.author));		      // set author name
	ui.tableModels->SetItemNotEditable(row, 5, date.toString(), date);	                                          // set creation date
}

void CModulesManagerTab::UpdateConfigFile() const
{
	QStringList unitDirs;
	QList<bool> unitDirsFlags;
	for (size_t i = 0; i < m_pModelsManager->DirsNumber(); ++i)
	{
		unitDirs.push_back(QString::fromStdWString(m_pModelsManager->GetDirPath(i)));
		unitDirsFlags.push_back(m_pModelsManager->GetDirActivity(i));
	}
	m_pSettings->setValue(StrConst::MM_ConfigModelsParamName, unitDirs);
	m_pSettings->setValue(StrConst::MM_ConfigModelsFlagsParamName, QVariant::fromValue(unitDirsFlags));
}

void CModulesManagerTab::AddDir()
{
	const QString folder = QFileDialog::getExistingDirectory(this, "Select directory", ".", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (folder.isEmpty()) return;
	QApplication::setOverrideCursor(Qt::WaitCursor);
	if (m_pModelsManager->AddDir(folder.toStdWString()))
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
	if(m_pModelsManager->RemoveDir(iRow))
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
	if (m_pModelsManager->UpDir(iRow))
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
	if (m_pModelsManager->DownDir(iRow))
	{
		UpdateModels();
		ui.tableDirs->RestoreSelectedCell(iRow + 1, 0);
	}
	QApplication::restoreOverrideCursor();
}

void CModulesManagerTab::ChangeDirActivity(int _iRow, int _iCol, QCheckBox* _pCheckBox)
{
	m_pModelsManager->SetDirActivity(_iRow, _pCheckBox->isChecked());
	UpdateModelsView();
	UpdateConfigFile();
	emit ModelsListWasChanged();
}

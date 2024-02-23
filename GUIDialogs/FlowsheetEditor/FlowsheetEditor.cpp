/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "FlowsheetEditor.h"
#include "Flowsheet.h"
#include "BaseUnit.h"
#include "Stream.h"
#include "MaterialsDatabase.h"
#include "DyssolStringConstants.h"
#include "ReactionsEditor.h"
#include "ContainerFunctions.h"
#include "FlowsheetViewer.h"
#include "QtUtilities.h"
#include <QMessageBox>
#include <cmath>
#include <sstream>


CFlowsheetEditor::CFlowsheetEditor(CFlowsheet* _pFlowsheet, const CMaterialsDatabase* _matrialsDB, QWidget* _parent)
	: CQtDialog{ _parent }
	, m_pFlowsheet{ _pFlowsheet }
	, m_materialsDB{ _matrialsDB }
	, m_pSelectedModel{ nullptr }
	, m_pSelectedStream{ nullptr }
	, m_viewer{ new CFlowsheetViewer{ _pFlowsheet } }
{
	ui.setupUi(this);
	ui.tableListValues->EnablePasting(false);
	ui.tableListValues->setSelectionMode(QAbstractItemView::ExtendedSelection);
	// set focus policy to enable focus to switch back to flowsheet editor
	this->setFocusPolicy(Qt::ClickFocus);

	SetHelpLink("001_ui/gui.html#sec-gui-tabs-flowsheet");
}

CFlowsheetEditor::~CFlowsheetEditor()
{
	delete m_viewer;
}

void CFlowsheetEditor::SetPointers(CModelsManager* _modelsManager, QSettings* _settings)
{
	CQtDialog::SetPointers(_modelsManager, _settings);
	m_viewer->SetPointers(_modelsManager, _settings);
}

void CFlowsheetEditor::InitializeConnections()
{
	connect(ui.buttonAddModel,		&QPushButton::clicked,					this, &CFlowsheetEditor::AddModel);
	connect(ui.buttonDeleteModel,	&QPushButton::clicked,					this, &CFlowsheetEditor::DeleteModel);
	connect(ui.buttonUpModel,		&QPushButton::clicked,					this, &CFlowsheetEditor::UpModel);
	connect(ui.buttonDownModel,		&QPushButton::clicked,					this, &CFlowsheetEditor::DownModel);
	connect(ui.listModels,			&QTableWidget::cellChanged,				this, &CFlowsheetEditor::ChangeModelName);
	connect(ui.listModels,			&QTableWidget::itemSelectionChanged,	this, &CFlowsheetEditor::ChangeSelectedModel);

	connect(ui.buttonAddStream,		&QPushButton::clicked,					this, &CFlowsheetEditor::AddStream);
	connect(ui.buttonDeleteStream,	&QPushButton::clicked,					this, &CFlowsheetEditor::DeleteStream);
	connect(ui.buttonUpStream,		&QPushButton::clicked,					this, &CFlowsheetEditor::UpStream);
	connect(ui.buttonDownStream,	&QPushButton::clicked,					this, &CFlowsheetEditor::DownStream);
	connect(ui.listStreams,			&QTableWidget::cellChanged,				this, &CFlowsheetEditor::ChangeStreamName);
	connect(ui.listStreams,			&QTableWidget::itemSelectionChanged,	this, &CFlowsheetEditor::ChangeSelectedStream);

	connect(ui.comboUnits,			QOverload<int>::of(&QComboBox::currentIndexChanged),	this, &CFlowsheetEditor::ChangeUnitInModel);

	connect(ui.tablePorts,          &CQtTable::ComboBoxIndexChanged,        this, &CFlowsheetEditor::NewPortStreamSelected);

	connect(ui.buttonAddListValue,		&QPushButton::clicked,				this, &CFlowsheetEditor::AddUnitParamListItem);
	connect(ui.buttonRemoveListValue,	&QPushButton::clicked,				this, &CFlowsheetEditor::DeleteUnitParamListItem);
	connect(ui.tableListValues,			&QTableWidget::itemChanged,			this, &CFlowsheetEditor::ListValueChanged);
	connect(ui.tableListValues,			&CQtTable::PasteInitiated,			this, &CFlowsheetEditor::PasteParamTable);


	connect(ui.tableUnitParams,		&QTableWidget::itemSelectionChanged,	this, &CFlowsheetEditor::NewUnitParameterSelected);
	connect(ui.tableUnitParams,		&QTableWidget::cellChanged,				this, &CFlowsheetEditor::UnitParamValueChanged);
	connect(ui.tableUnitParams,		&CQtTable::ComboBoxIndexChanged,		this, &CFlowsheetEditor::UnitParamValueChanged);
	connect(ui.tableUnitParams,		&CQtTable::CheckBoxStateChanged,		this, &CFlowsheetEditor::UnitParamValueChanged);
	connect(ui.tableUnitParams,		&CQtTable::PushButtonClicked,		    this, &CFlowsheetEditor::UnitParamValueChanged);

	connect(ui.buttonShowFlowsheet, &QPushButton::clicked         , m_viewer, &QDialog::show);
	connect(this                  , &CFlowsheetEditor::DataChanged, m_viewer, [&](){ m_viewer->Update(); });
}

void CFlowsheetEditor::setVisible(bool _bVisible)
{
	QWidget::setVisible(_bVisible);
	if (_bVisible)
		UpdateWholeView();
}

void CFlowsheetEditor::UpdateWholeView()
{
	UpdateModelsView();
	UpdateStreamsView();
	UpdateAvailableUnits();
	m_viewer->Update();
}

void CFlowsheetEditor::UpdateAvailableUnits() const
{
	QSignalBlocker blocker(ui.comboUnits);

	ui.comboUnits->clear();

	const std::vector<SUnitDescriptor> units = m_modelsManager->GetAvailableUnits();
	for (int i = 0; i < static_cast<int>(units.size()); ++i)
		ui.comboUnits->insertItem(i, QString::fromStdString(units[i].name), QString::fromStdString(units[i].uniqueID));

	// sort units in combobox alphabetically
	ui.comboUnits->model()->sort(0);

	UpdateUnitCombo();
}

void CFlowsheetEditor::UpdateAvailableSolvers() const
{
	UpdateUnitParamTable();
}

void CFlowsheetEditor::AddModel()
{
	CUnitContainer* model = m_pFlowsheet->AddUnit();
	model->SetName(StrConst::FE_UnitDefaultName + std::to_string(m_pFlowsheet->GetUnitsNumber()));

	UpdateModelsView();
	QSignalBlocker blocker(ui.listModels);
	ui.listModels->setCurrentItem(ui.listModels->item(ui.listModels->rowCount() - 1, 0));
	ChangeSelectedModel();

	emit ModelsChanged();
	emit DataChanged();
}

void CFlowsheetEditor::DeleteModel()
{
	if (!m_pSelectedModel) return;

	m_pFlowsheet->DeleteUnit(m_pSelectedModel->GetKey());

	UpdateModelsView();
	UpdateUnitCombo();
	UpdatePortsView();
	UpdateUnitParamTable();
	UpdateUnitParamDescr();
	UpdateListValuesTable();

	emit ModelsChanged();
	emit DataChanged();
}

void CFlowsheetEditor::UpModel()
{
	if (!m_pSelectedModel || ui.listModels->currentRow() <= 0) return;
	QSignalBlocker blocker(ui.listModels);
	m_pFlowsheet->ShiftUnit(m_pSelectedModel->GetKey(), EDirection::UP);
	ui.listModels->setCurrentItem(ui.listModels->item(ui.listModels->currentRow() - 1, 0));
	UpdateModelsView();
	emit DataChanged();
}

void CFlowsheetEditor::DownModel()
{
	if (!m_pSelectedModel || ui.listModels->currentRow() >= ui.listModels->rowCount() - 1) return;
	QSignalBlocker blocker(ui.listModels);
	m_pFlowsheet->ShiftUnit(m_pSelectedModel->GetKey(), EDirection::DOWN);
	ui.listModels->setCurrentItem(ui.listModels->item(ui.listModels->currentRow() + 1, 0));
	UpdateModelsView();
	emit DataChanged();
}

void CFlowsheetEditor::AddStream()
{
	CStream*stream = m_pFlowsheet->AddStream();
	stream->SetName(StrConst::FE_StreamDefaultName + std::to_string(m_pFlowsheet->GetStreamsNumber()));

	UpdateStreamsView();
	QSignalBlocker blocker(ui.listStreams);
	ui.listStreams->setCurrentItem(ui.listStreams->item(ui.listStreams->rowCount() - 1, 0));
	ChangeSelectedStream();
	UpdatePortsView();

	emit StreamsChanged();
	emit DataChanged();
}

void CFlowsheetEditor::DeleteStream()
{
	if (!m_pSelectedStream) return;

	m_pFlowsheet->DeleteStream(m_pSelectedStream->GetKey());

	UpdateStreamsView();
	UpdatePortsView();

	emit StreamsChanged();
	emit DataChanged();
}

void CFlowsheetEditor::UpStream()
{
	if (!m_pSelectedStream || ui.listStreams->currentRow() <= 0) return;
	m_pFlowsheet->ShiftStream(m_pSelectedStream->GetKey(), EDirection::UP);
	UpdateStreamsView();
	UpdatePortsView();
	ui.listStreams->setCurrentItem(ui.listStreams->item(ui.listStreams->currentRow() - 1, 0));
	emit DataChanged();
}

void CFlowsheetEditor::DownStream()
{
	if (!m_pSelectedStream || ui.listStreams->currentRow() >= ui.listStreams->rowCount() - 1) return;
	m_pFlowsheet->ShiftStream(m_pSelectedStream->GetKey(), EDirection::DOWN);
	UpdateStreamsView();
	UpdatePortsView();
	ui.listStreams->setCurrentItem(ui.listStreams->item(ui.listStreams->currentRow() + 1, 0));
	emit DataChanged();
}

void CFlowsheetEditor::ChangeSelectedStream()
{
	const QTableWidgetItem *pItem = ui.listStreams->currentItem();
	m_pSelectedStream = pItem ? m_pFlowsheet->GetStream(pItem->data(Qt::UserRole).toString().toStdString()) : nullptr;
}

void CFlowsheetEditor::ChangeSelectedModel()
{
	const QTableWidgetItem *pItem = ui.listModels->currentItem();
	m_pSelectedModel = pItem ? m_pFlowsheet->GetUnit(pItem->data(Qt::UserRole).toString().toStdString()) : nullptr;
	m_pModelParams = m_pSelectedModel && m_pSelectedModel->GetModel() ? &m_pSelectedModel->GetModel()->GetUnitParametersManager() : nullptr;

	EnableGUIElements();
	UpdateUnitCombo();
	UpdatePortsView();
	UpdateUnitParamDescr();
	UpdateListValuesTable();
	UpdateUnitParamTable();
}

void CFlowsheetEditor::ChangeUnitInModel(int _index)
{
	if (!m_pSelectedModel) return;

	// set new unit
	m_pSelectedModel->SetModel(ui.comboUnits->itemData(_index).toString().toStdString());
	m_pFlowsheet->SetTopologyModified(true);
	m_pModelParams = m_pSelectedModel->GetModel() ? &m_pSelectedModel->GetModel()->GetUnitParametersManager() : nullptr;

	UpdatePortsView();
	UpdateUnitParamDescr();
	UpdateListValuesTable();
	UpdateUnitParamTable();

	emit UnitChanged();
	emit DataChanged();
}

void CFlowsheetEditor::ChangeModelName(int _iRow, int _iCol)
{
	if (!m_pSelectedModel) return;

	const auto item = ui.listModels->item(_iRow, _iCol);
	if (!item) return;
	m_pSelectedModel->SetName(item->text().simplified().toStdString());

	UpdateModelsView();

	emit ModelsChanged();
	emit DataChanged();
}

void CFlowsheetEditor::ChangeStreamName(int _iRow, int _iCol)
{
	if (!m_pSelectedStream) return;

	m_pSelectedStream->SetName(ui.listStreams->item(_iRow, _iCol)->text().simplified().toStdString());

	UpdateStreamsView();
	UpdatePortsView();

	emit StreamsChanged();
	emit DataChanged();
}

void CFlowsheetEditor::NewPortStreamSelected(int _iRow, int _iCol, QComboBox* _comboBox)
{
	if (!m_pSelectedModel || !m_pSelectedModel->GetModel()) return;
	m_pSelectedModel->GetModel()->GetPortsManager().GetPort(ui.tablePorts->GetItemUserData(_iRow, 0).toStdString())->SetStreamKey(_comboBox->currentData(Qt::UserRole).toString().toStdString());
	m_pFlowsheet->SetTopologyModified(true);
	emit DataChanged();
}

void CFlowsheetEditor::NewUnitParameterSelected() const
{
	UpdateUnitParamDescr();
	UpdateListValuesTable();
}

void CFlowsheetEditor::AddUnitParamListItem()
{
	if (!m_pModelParams) return;

	// get selected parameter
	auto* param = m_pModelParams->GetParameter(ui.tableUnitParams->GetItemUserData(ui.tableUnitParams->currentRow(), 0).toInt());
	if (!param) return;

	if (param->GetType() == EUnitParameter::TIME_DEPENDENT || param->GetType() == EUnitParameter::PARAM_DEPENDENT)
	{
		auto* paramD = dynamic_cast<CDependentUnitParameter*>(param);

		// add new dependent value
		if (paramD->IsEmpty()) // add the first dependent value
			paramD->SetValue(0, 0);
		else                  // add next as (lastParam + 1, lastValue)
			paramD->SetValue(paramD->GetParams().back() + 1, paramD->GetValues().back());
	}
	else if (param->GetType() == EUnitParameter::LIST_DOUBLE)
	{
		auto* paramL = dynamic_cast<CListRealUnitParameter*>(param);
		if (paramL->IsEmpty())	paramL->AddValue(0);
		else					paramL->AddValue(paramL->GetValues().back());
	}
	else if (param->GetType() == EUnitParameter::LIST_UINT64)
	{
		auto* paramL = dynamic_cast<CListUIntUnitParameter*>(param);
		if (paramL->IsEmpty())	paramL->AddValue(0);
		else					paramL->AddValue(paramL->GetValues().back());
	}
	else if (param->GetType() == EUnitParameter::LIST_INT64)
	{
		auto* paramL = dynamic_cast<CListIntUnitParameter*>(param);
		if (paramL->IsEmpty())	paramL->AddValue(0);
		else					paramL->AddValue(paramL->GetValues().back());
	}

	UpdateUnitParamTable();
	UpdateListValuesTable();

	emit DataChanged();
}

void CFlowsheetEditor::DeleteUnitParamListItem()
{
	if (!m_pModelParams) return;

	// get selected parameter
	auto* param = m_pModelParams->GetParameter(ui.tableUnitParams->GetItemUserData(ui.tableUnitParams->currentRow(), 0).toInt());
	if (!param) return;

	if (param->GetType() == EUnitParameter::TIME_DEPENDENT || param->GetType() == EUnitParameter::PARAM_DEPENDENT)
	{
		auto* paramD = dynamic_cast<CDependentUnitParameter*>(param);
		if (paramD->Size() < 2)
		{
			QMessageBox::information(this, StrConst::Dyssol_ApplicationName, StrConst::FE_DRemoveLast);
			return;
		}

		// get selected dependent value
		const int iTP = ui.tableListValues->currentRow();

		if (iTP != -1) // remove selected
			paramD->RemoveValue(paramD->GetParams()[iTP]);
		else           // remove last
			paramD->RemoveValue(paramD->GetParams().back());

	}
	else if (param->GetType() == EUnitParameter::LIST_DOUBLE)
	{
		auto* paramL = dynamic_cast<CListRealUnitParameter*>(param);
		if (paramL->Size() < 2) return;
		const int index = ui.tableListValues->currentRow();
		if (index != -1)	paramL->RemoveValue(index);					// remove selected
		else           		paramL->RemoveValue(paramL->Size() - 1);	// remove last
	}
	else if (param->GetType() == EUnitParameter::LIST_UINT64)
	{
		auto* paramL = dynamic_cast<CListUIntUnitParameter*>(param);
		if (paramL->Size() < 2) return;
		const int index = ui.tableListValues->currentRow();
		if (index != -1)	paramL->RemoveValue(index);					// remove selected
		else           		paramL->RemoveValue(paramL->Size() - 1);	// remove last

	}
	else if (param->GetType() == EUnitParameter::LIST_INT64)
	{
		auto* paramL = dynamic_cast<CListIntUnitParameter*>(param);
		if (paramL->Size() < 2) return;
		const int index = ui.tableListValues->currentRow();
		if (index != -1)	paramL->RemoveValue(index);					// remove selected
		else           		paramL->RemoveValue(paramL->Size() - 1);	// remove last
	}

	UpdateUnitParamTable();
	UpdateListValuesTable();

	emit DataChanged();
}

void CFlowsheetEditor::ListValueChanged()
{
	if (!m_pModelParams) return;

	// get selected parameter
	auto* param = m_pModelParams->GetParameter(ui.tableUnitParams->GetItemUserData(ui.tableUnitParams->currentRow(), 0).toInt());
	if (!param) return;

	const int iRow = ui.tableListValues->currentRow();

	if (param->GetType() == EUnitParameter::TIME_DEPENDENT || param->GetType() == EUnitParameter::PARAM_DEPENDENT)
	{
		auto* paramD = dynamic_cast<CDependentUnitParameter*>(param);
		if (ui.tableListValues->currentColumn() == 0) // change dependent value
			paramD->RemoveValue(paramD->GetParams()[iRow]);
		paramD->SetValue(ui.tableListValues->GetItem(iRow, 0).toDouble(), ui.tableListValues->GetItem(iRow, 1).toDouble());
	}
	else if (param->GetType() == EUnitParameter::LIST_DOUBLE)
	{
		auto* paramL = dynamic_cast<CListRealUnitParameter*>(param);
		paramL->SetValue(iRow, ui.tableListValues->GetItem(iRow, 1).toDouble());
	}
	else if (param->GetType() == EUnitParameter::LIST_UINT64)
	{
		auto* paramL = dynamic_cast<CListUIntUnitParameter*>(param);
		paramL->SetValue(iRow, ui.tableListValues->GetItem(iRow, 1).toUInt());

	}
	else if (param->GetType() == EUnitParameter::LIST_INT64)
	{
		auto* paramL = dynamic_cast<CListIntUnitParameter*>(param);
		paramL->SetValue(iRow, ui.tableListValues->GetItem(iRow, 1).toInt());
	}


	UpdateUnitParamTable();
	UpdateListValuesTable();

	emit DataChanged();
}

void CFlowsheetEditor::UnitParamValueChanged(int _row, int _col)
{
	if (!m_pModelParams) return;

	// get selected parameter
	auto* param = m_pModelParams->GetParameter(ui.tableUnitParams->GetItemUserData(_row, 0).toInt());
	if (!param) return;

	bool dataChanged = true; // whether the data were changed here
	switch (param->GetType())
	{
	case EUnitParameter::CONSTANT: [[fallthrough]];
	case EUnitParameter::CONSTANT_DOUBLE:
		dynamic_cast<CConstRealUnitParameter*>(param)->SetValue(ui.tableUnitParams->GetItem(_row, _col).toDouble());
		break;
	case EUnitParameter::CONSTANT_INT64:
		dynamic_cast<CConstIntUnitParameter*>(param)->SetValue(ui.tableUnitParams->GetItem(_row, _col).toInt());
		break;
	case EUnitParameter::CONSTANT_UINT64:
		dynamic_cast<CConstUIntUnitParameter*>(param)->SetValue(ui.tableUnitParams->GetItem(_row, _col).toUInt());
		break;
	case EUnitParameter::TIME_DEPENDENT:
		dynamic_cast<CTDUnitParameter*>(param)->SetValue(dynamic_cast<CTDUnitParameter*>(param)->GetParams().front(), ui.tableUnitParams->GetItem(_row, _col).toDouble());
		break;
	case EUnitParameter::PARAM_DEPENDENT:
		dynamic_cast<CDependentUnitParameter*>(param)->SetValue(dynamic_cast<CDependentUnitParameter*>(param)->GetParams().front(), ui.tableUnitParams->GetItem(_row, _col).toDouble());
		break;
	case EUnitParameter::STRING:
		dynamic_cast<CStringUnitParameter*>(param)->SetValue(ui.tableUnitParams->GetItem(_row, _col).toStdString());
		break;
	case EUnitParameter::CHECKBOX:
	{
		const QCheckBox* checkbox = ui.tableUnitParams->GetCheckBox(_row, _col);
		dynamic_cast<CCheckBoxUnitParameter*>(param)->SetChecked(checkbox->isChecked());
		break;
	}
	case EUnitParameter::SOLVER:
	{
		const QComboBox* combo = ui.tableUnitParams->GetComboBox(_row, _col);
		const QString solverKey = combo->itemData(combo->currentIndex()).toString();
		dynamic_cast<CSolverUnitParameter*>(param)->SetKey(solverKey.toStdString());
		break;
	}
	case EUnitParameter::COMBO:
	{
		const QComboBox* combo = ui.tableUnitParams->GetComboBox(_row, _col);
		const size_t value = combo->itemData(combo->currentIndex()).toUInt();
		dynamic_cast<CComboUnitParameter*>(param)->SetValue(value);
		break;
	}
	case EUnitParameter::GROUP:
	{
		const QComboBox* combo = ui.tableUnitParams->GetComboBox(_row, _col);
		const size_t value = combo->itemData(combo->currentIndex()).toUInt();
		dynamic_cast<CComboUnitParameter*>(param)->SetValue(value);
		break;
	}
	case EUnitParameter::COMPOUND:
	{
		const QComboBox* combo = ui.tableUnitParams->GetComboBox(_row, _col);
		const QString key = combo->itemData(combo->currentIndex()).toString();
		dynamic_cast<CCompoundUnitParameter*>(param)->SetCompound(key.toStdString());
		break;
	}
	case EUnitParameter::MDB_COMPOUND:
	{
		const QComboBox* combo = ui.tableUnitParams->GetComboBox(_row, _col);
		const QString key = combo->itemData(combo->currentIndex()).toString();
		dynamic_cast<CMDBCompoundUnitParameter*>(param)->SetCompound(key.toStdString());
		break;
	}
	case EUnitParameter::REACTION:
	{
		auto* reactionParam = dynamic_cast<CReactionUnitParameter*>(param);
		const auto compounds = m_pFlowsheet->GetCompounds();
		CReactionsEditor editor{ reactionParam->GetReactions(), &compounds, m_materialsDB, this };
		if (editor.exec() == QDialog::Accepted)
			reactionParam->SetReactions(editor.GetReactions());
		else
			dataChanged = false;
		break;
	}
	case EUnitParameter::LIST_DOUBLE:
		dynamic_cast<CListRealUnitParameter*>(param)->SetValue(0, ui.tableUnitParams->GetItem(_row, _col).toDouble());
		break;
	case EUnitParameter::LIST_UINT64:
		dynamic_cast<CListUIntUnitParameter*>(param)->SetValue(0, ui.tableUnitParams->GetItem(_row, _col).toUInt());
		break;
	case EUnitParameter::LIST_INT64:
		dynamic_cast<CListIntUnitParameter*>(param)->SetValue(0, ui.tableUnitParams->GetItem(_row, _col).toInt());
		break;
	case EUnitParameter::UNKNOWN:
		break;
	}

	UpdateUnitParamTable();
	UpdateListValuesTable();

	if (dataChanged)
		emit DataChanged();
}

template<typename T>
void PasteListValues(CListUnitParameter<T>* _listParam, const std::vector<std::vector<double>>& _data, int _row)
{
	if (!_listParam) return;
	const size_t listSize = _listParam->GetValues().size();
	// insert data row by row
	for (size_t i = 0; i < _data.size(); ++i)
	{
		size_t index = i + _row;
		T value = static_cast<T>(_data[i][0]);
		if (i >= listSize)
			_listParam->AddValue(value);
		else
			_listParam->SetValue(index, value);
	}
}

void CFlowsheetEditor::PasteParamTable(int _row, int _col)
{
	if (!m_pModelParams) return;

	auto* up = m_pModelParams->GetParameter(ui.tableUnitParams->GetItemUserData(ui.tableUnitParams->currentRow(), 0).toInt());
	if (!up) return;

	// parse pasted text
	const auto data = ParseClipboardAsDoubles();

	// proceed based on parameter type
	switch (up->GetType())
	{
	case EUnitParameter::PARAM_DEPENDENT: [[fallthrough]];
	case EUnitParameter::TIME_DEPENDENT:
	{
		auto* paramTD = dynamic_cast<CDependentUnitParameter*>(up);

		// new parameters (and possibly values) pasted
		if (_col == 0)
		{
			const std::vector<double> valuesOld = paramTD->GetValues();

			// existing parameters that are going to be overwritten are deleted first
			const std::vector<double> paramsOld = paramTD->GetParams();
			for (size_t i = _row; i < paramsOld.size() && i < data.size() + _row; ++i)
				paramTD->RemoveValue(paramsOld[i]);

			// insert data line by line
			std::set<double> duplicates;
			int overwrite = QMessageBox::NoButton;
			for (size_t iData = 0; iData < data.size(); ++iData)
			{
				const double param = data[iData][0];
				const double value = data[iData].size() >= 2 ? data[iData][1] : iData + _row < valuesOld.size() ? valuesOld[iData + _row] : 0.0;
				// check if time point already exists
				if (paramTD->HasParam(param))
					duplicates.insert(param);
				if (paramTD->HasParam(param) && paramTD->GetValue(param) != value)
				{
					// ask user which value to keep
					if (overwrite == QMessageBox::NoButton)
					{
						const auto reply = AskYesAllNoAllCancel(this, StrConst::FE_InvalidClipboard, StrConst::FE_ParamExists(param, paramTD->GetValue(param), value));
						if (reply == QMessageBox::Cancel) break;
						if (reply == QMessageBox::YesToAll || reply == QMessageBox::NoToAll) overwrite = reply;
						if (reply == QMessageBox::No || reply == QMessageBox::NoToAll) continue;
					}
					if (overwrite == QMessageBox::NoToAll) continue;
				}
				paramTD->SetValue(param, value);
			}

			// notify user about duplicates
			if (!duplicates.empty())
				Notify(this, StrConst::FE_InvalidClipboard, StrConst::FE_DuplicateParams(duplicates));
		}
		// only values pasted
		else
		{
			// all currently defined parameters
			const std::vector<double> params = paramTD->GetParams();
			// check there is enough defined parameters to paste all the values
			if (_row + data.size() > paramTD->Size())
				if (AskYesNoCancel(this, StrConst::FE_InvalidClipboard, StrConst::FE_NotEnoughParameters) != QMessageBox::Yes)
					return;
			// set new data
			for (size_t iData = 0; iData < data.size() && iData + _row < params.size(); ++iData)
				paramTD->SetValue(params[iData + _row], data[iData][0]);
		}

		break;
	}
	case EUnitParameter::LIST_DOUBLE: PasteListValues(dynamic_cast<CListRealUnitParameter*>(up), data, _row);	break;
	case EUnitParameter::LIST_UINT64: PasteListValues(dynamic_cast<CListUIntUnitParameter*>(up), data, _row);	break;
	case EUnitParameter::LIST_INT64:  PasteListValues(dynamic_cast<CListIntUnitParameter*>(up) , data, _row);	break;
	case EUnitParameter::CONSTANT:
	case EUnitParameter::STRING:
	case EUnitParameter::CHECKBOX:
	case EUnitParameter::SOLVER:
	case EUnitParameter::COMBO:
	case EUnitParameter::GROUP:
	case EUnitParameter::COMPOUND:
	case EUnitParameter::CONSTANT_DOUBLE:
	case EUnitParameter::CONSTANT_INT64:
	case EUnitParameter::CONSTANT_UINT64:
	case EUnitParameter::REACTION:
	case EUnitParameter::MDB_COMPOUND:
	case EUnitParameter::UNKNOWN:
		break;
	}

	UpdateUnitParamTable();
	UpdateListValuesTable();

	emit DataChanged();
}

void CFlowsheetEditor::UpdateModelsView()
{
	QSignalBlocker blocker(ui.listModels);

	// save last selected model
	const auto oldPos = ui.listModels->CurrentCellPos();

	// update list of models
	ui.listModels->setRowCount(static_cast<int>(m_pFlowsheet->GetUnitsNumber()));
	int iRow = 0;
	for (const auto& unit : m_pFlowsheet->GetAllUnits())
		ui.listModels->SetItemEditable(iRow++, 0, unit->GetName(), QString::fromStdString(unit->GetKey()));

	// restore selection
	ui.listModels->RestoreSelectedCell(oldPos);
	ChangeSelectedModel();
}

void CFlowsheetEditor::UpdateStreamsView()
{
	QSignalBlocker blocker(ui.listStreams);
	const auto oldPos = ui.listStreams->CurrentCellPos();

	ui.listStreams->setRowCount(static_cast<int>(m_pFlowsheet->GetStreamsNumber()));
	int iRow = 0;
	for (const auto& stream : m_pFlowsheet->GetAllStreams())
		ui.listStreams->SetItemEditable(iRow++, 0, stream->GetName(), QString::fromStdString(stream->GetKey()));

	// restore selection
	ui.listStreams->RestoreSelectedCell(oldPos);
	ChangeSelectedStream();
}

void CFlowsheetEditor::UpdateUnitCombo() const
{
	QSignalBlocker blocker(ui.comboUnits);
	ui.comboUnits->setCurrentIndex(-1);

	if (!m_pSelectedModel) return;

	// set selected unit
	const QString unitKey = QString::fromStdString(m_pSelectedModel->GetModel() ? m_pSelectedModel->GetModel()->GetUniqueID() : "");
	for (int i = 0; i < ui.comboUnits->count(); ++i)
		if (ui.comboUnits->itemData(i).toString() == unitKey)
		{
			ui.comboUnits->setCurrentIndex(i);
			break;
		}
}

void CFlowsheetEditor::UpdatePortsView() const
{
	QSignalBlocker blocker(ui.tablePorts);

	// clear elements
	ui.tablePorts->setRowCount(0);
	ui.labelPorts->setText("Ports");

	if (!m_pSelectedModel || !m_pSelectedModel->GetModel()) return;

	// update label
	ui.labelPorts->setText("Ports of " + QString::fromStdString(m_pSelectedModel->GetName()));

	// get list of ports for current unit
	std::vector<CUnitPort*> ports = m_pSelectedModel->GetModel()->GetPortsManager().GetAllPorts();

	// resize the table
	ui.tablePorts->setRowCount(static_cast<int>(ports.size()));

	// prepare data for combo boxes
	std::vector<std::string> streamNames;
	std::vector<std::string> streamKeys;
	for (const auto& stream : m_pFlowsheet->GetAllStreams())
	{
		streamNames.push_back(stream->GetName());
		streamKeys.emplace_back(stream->GetKey());
	}

	// create table with total ports of the unit
	for (int i = 0; i < static_cast<int>(ports.size()); ++i)
	{
		ui.tablePorts->SetItemNotEditable(i, 0, ports[i]->GetName(), QString::fromStdString(ports[i]->GetName()));
		ui.tablePorts->SetItemNotEditable(i, 1, ports[i]->GetType() == EUnitPort::INPUT ? QString(StrConst::FE_PortTypeInput) : QString(StrConst::FE_PortTypeOutput));
		ui.tablePorts->SetRowBackgroundColor(i, ports[i]->GetType() == EUnitPort::INPUT ? QColor(0, 255, 0, 50) : QColor(255, 216, 0, 50));
		QComboBox* combo = ui.tablePorts->SetComboBox(i, 2, streamNames, streamKeys, ports[i]->GetStreamKey());
		combo->setMaxVisibleItems(20);
	}
}

void CFlowsheetEditor::UpdateUnitParamTable() const
{
	QSignalBlocker blocker(ui.tableUnitParams);
	const auto oldPos = ui.tableUnitParams->CurrentCellPos();

	// clear some GUI elements
	ui.tableUnitParams->setRowCount(0);

	if (!m_pModelParams) return;

	int iRow = 0;
	for (int i = 0; i < static_cast<int>(m_pModelParams->ParametersNumber()); ++i)
	{
		if (!m_pModelParams->IsParameterActive(i)) continue; // do not show this parameter

		const CBaseUnitParameter* param = m_pModelParams->GetParameter(i);
		ui.tableUnitParams->insertRow(iRow);
		ui.tableUnitParams->SetItemNotEditable(iRow, 0, param->GetName(), i);

		switch (param->GetType())
		{
		case EUnitParameter::CONSTANT: [[fallthrough]];
		case EUnitParameter::CONSTANT_DOUBLE:
		{
			const auto* p = dynamic_cast<const CConstRealUnitParameter*>(param);
			ui.tableUnitParams->SetItemEditable(iRow, 2, p->GetValue());
			if (!p->IsInBounds())
				ui.tableUnitParams->SetItemBackgroundColor(iRow, 2, Qt::red);
			ui.tableUnitParams->SetItemNotEditable(iRow, 1, !p->GetUnits().empty() ? "[" + p->GetUnits() + "]" : std::string{});
			break;
		}
		case EUnitParameter::CONSTANT_INT64:
		{
			const auto* p = dynamic_cast<const CConstIntUnitParameter*>(param);
			ui.tableUnitParams->SetItemEditable(iRow, 2, static_cast<double>(p->GetValue()));
			if (!p->IsInBounds())
				ui.tableUnitParams->SetItemBackgroundColor(iRow, 2, Qt::red);
			ui.tableUnitParams->SetItemNotEditable(iRow, 1, !p->GetUnits().empty() ? "[" + p->GetUnits() + "]" : std::string{});
			break;
		}
		case EUnitParameter::CONSTANT_UINT64:
		{
			const auto* p = dynamic_cast<const CConstUIntUnitParameter*>(param);
			ui.tableUnitParams->SetItemEditable(iRow, 2, static_cast<double>(p->GetValue()));
			if (!p->IsInBounds())
				ui.tableUnitParams->SetItemBackgroundColor(iRow, 2, Qt::red);
			ui.tableUnitParams->SetItemNotEditable(iRow, 1, !p->GetUnits().empty() ? "[" + p->GetUnits() + "]" : std::string{});
			break;
		}
		case EUnitParameter::TIME_DEPENDENT: [[fallthrough]];
		case EUnitParameter::PARAM_DEPENDENT:
		{
			const auto* p = dynamic_cast<const CDependentUnitParameter*>(param);
			if (p->Size() == 1)
			{
				ui.tableUnitParams->SetItemEditable(iRow, 2, p->GetValue(p->GetParams().front()));
				if (!p->IsInBounds())
					ui.tableUnitParams->SetItemBackgroundColor(iRow, 2, Qt::red);
			}
			else
			{
				ui.tableUnitParams->SetItemNotEditable(iRow, 2, QString(StrConst::FE_TDParamMessage));
				ui.tableUnitParams->SetItemBackgroundColor(iRow, 2, Qt::gray);
				ui.tableUnitParams->SetItemFontItalic(iRow, 2);
			}
			ui.tableUnitParams->SetItemNotEditable(iRow, 1, !p->GetUnits().empty() ? "[" + p->GetUnits() + "]" : std::string{});
			break;
		}
		case EUnitParameter::STRING:
		{
			const auto* p = dynamic_cast<const CStringUnitParameter*>(param);
			ui.tableUnitParams->SetItemEditable(iRow, 2, p->GetValue());
			ui.tableUnitParams->SetItemNotEditable(iRow, 1, QString{});
			break;
		}
		case EUnitParameter::CHECKBOX:
		{
			const auto* p = dynamic_cast<const CCheckBoxUnitParameter*>(param);
			ui.tableUnitParams->SetCheckBox(iRow, 2, p->IsChecked());
			ui.tableUnitParams->SetItemNotEditable(iRow, 1, QString{});
			break;
		}
		case EUnitParameter::SOLVER:
		{
			const auto* p = dynamic_cast<const CSolverUnitParameter*>(param);
			// create combo with total list of possible solvers with the specified type
			const std::vector<SSolverDescriptor> solvers = m_modelsManager->GetAvailableSolvers();
			std::vector<std::string> names;
			std::vector<std::string> keys;
			for (const auto& s : solvers)
				if (s.solverType == p->GetSolverType()) // selected solver type
				{
					names.push_back(s.name);
					keys.push_back(s.uniqueID);
				}
			ui.tableUnitParams->SetComboBox(iRow, 2, names, keys, p->GetKey());
			ui.tableUnitParams->SetItemNotEditable(iRow, 1, QString{});
			break;
		}
		case EUnitParameter::COMBO: [[fallthrough]];
		case EUnitParameter::GROUP:
		{
			const auto* p = dynamic_cast<const CComboUnitParameter*>(param);
			// create combo with the list of possible values
			ui.tableUnitParams->SetComboBox(iRow, 2, p->GetNames(), p->GetItems(), p->GetValue());
			ui.tableUnitParams->SetItemNotEditable(iRow, 1, QString{});
			break;
		}
		case EUnitParameter::COMPOUND:
		{
			const auto* p = dynamic_cast<const CCompoundUnitParameter*>(param);
			// create combo with total list of possible compounds
			ui.tableUnitParams->SetComboBox(iRow, 2, m_materialsDB->GetCompoundsNames(m_pFlowsheet->GetCompounds()), m_pFlowsheet->GetCompounds(), p->GetCompound());
			ui.tableUnitParams->SetItemNotEditable(iRow, 1, QString{});
			break;
		}
		case EUnitParameter::MDB_COMPOUND:
		{
			const auto* p = dynamic_cast<const CMDBCompoundUnitParameter*>(param);
			// create combo with total list of possible compounds
			ui.tableUnitParams->SetComboBox(iRow, 2, m_materialsDB->GetCompoundsNames(), m_materialsDB->GetCompoundsKeys(), p->GetCompound());
			ui.tableUnitParams->SetItemNotEditable(iRow, 1, QString{});
			break;
		}
		case EUnitParameter::REACTION:
		{
			ui.tableUnitParams->SetPushButton(iRow, 2, "Reactions");
			ui.tableUnitParams->SetItemNotEditable(iRow, 1, QString{});
			break;
		}
		case EUnitParameter::LIST_DOUBLE:
		{
			const auto* p = dynamic_cast<const CListRealUnitParameter*>(param);
			if (p->Size() == 1)
			{
				ui.tableUnitParams->SetItemEditable(iRow, 2, p->GetValue(0));
				if (!p->IsInBounds())
					ui.tableUnitParams->SetItemBackgroundColor(iRow, 2, Qt::red);
			}
			else
			{
				ui.tableUnitParams->SetItemNotEditable(iRow, 2, QString(StrConst::FE_TDParamMessage));
				ui.tableUnitParams->SetItemBackgroundColor(iRow, 2, Qt::gray);
				ui.tableUnitParams->SetItemFontItalic(iRow, 2);
			}
			ui.tableUnitParams->SetItemNotEditable(iRow, 1, !p->GetUnits().empty() ? "[" + p->GetUnits() + "]" : std::string{});
			break;
		}
		case EUnitParameter::LIST_UINT64:
		{
			const auto* p = dynamic_cast<const CListUIntUnitParameter*>(param);
			if (p->Size() == 1)
			{
				ui.tableUnitParams->SetItemEditable(iRow, 2, std::to_string(p->GetValue(0)));
				if (!p->IsInBounds())
					ui.tableUnitParams->SetItemBackgroundColor(iRow, 2, Qt::red);
			}
			else
			{
				ui.tableUnitParams->SetItemNotEditable(iRow, 2, QString(StrConst::FE_TDParamMessage));
				ui.tableUnitParams->SetItemBackgroundColor(iRow, 2, Qt::gray);
				ui.tableUnitParams->SetItemFontItalic(iRow, 2);
			}
			ui.tableUnitParams->SetItemNotEditable(iRow, 1, !p->GetUnits().empty() ? "[" + p->GetUnits() + "]" : std::string{});
			break;
		}
		case EUnitParameter::LIST_INT64:
		{
			const auto* p = dynamic_cast<const CListIntUnitParameter*>(param);
			if (p->Size() == 1)
			{
				ui.tableUnitParams->SetItemEditable(iRow, 2, std::to_string(p->GetValue(0)));
				if (!p->IsInBounds())
					ui.tableUnitParams->SetItemBackgroundColor(iRow, 2, Qt::red);
			}
			else
			{
				ui.tableUnitParams->SetItemNotEditable(iRow, 2, QString(StrConst::FE_TDParamMessage));
				ui.tableUnitParams->SetItemBackgroundColor(iRow, 2, Qt::gray);
				ui.tableUnitParams->SetItemFontItalic(iRow, 2);
			}
			ui.tableUnitParams->SetItemNotEditable(iRow, 1, !p->GetUnits().empty() ? "[" + p->GetUnits() + "]" : std::string{});
			break;
		}
		case EUnitParameter::UNKNOWN:
			break;
		}

		++iRow;
	}

	ui.tableUnitParams->RestoreSelectedCell(oldPos);
}

void CFlowsheetEditor::UpdateListValuesTable() const
{
	QSignalBlocker blocker(ui.tableListValues);

	// switch off and clear some GUI elements
	ui.tableListValues->setRowCount(0);
	ui.frameListValues->setEnabled(false);

	if (!m_pModelParams) return;
	if (ui.tableUnitParams->currentRow() == -1) return;

	// get selected parameter
	const auto* param = m_pModelParams->GetParameter(ui.tableUnitParams->GetItemUserData(ui.tableUnitParams->currentRow(), 0).toInt());
	if (!param) return;

	if (param->GetType() == EUnitParameter::TIME_DEPENDENT || param->GetType() == EUnitParameter::PARAM_DEPENDENT)
	{
		const auto* paramD = dynamic_cast<const CDependentUnitParameter*>(param);

		// enable GUI elements
		ui.frameListValues->setEnabled(true);

		ui.tableListValues->ShowCol(0, true); // show params

		// create table with parameter values
		ui.tableListValues->setRowCount(static_cast<int>(paramD->Size()));
		ui.tableListValues->SetItemsColEditable(0, 0, paramD->GetParams());
		ui.tableListValues->SetItemsColEditable(0, 1, paramD->GetValues());

		ui.tableListValues->SetColHeaderItem(0, paramD->GetParamName() + " [" + paramD->GetParamUnits() + "]");
		ui.tableListValues->SetColHeaderItem(1, paramD->GetName() + " [" + paramD->GetUnits() + "]");

		// check if values are in boundaries
		if (!paramD->IsInBounds())
		{
			for (int i = 0; i < static_cast<int>(paramD->Size()); ++i)
				if (paramD->GetParams()[i] < paramD->GetParamMin() || paramD->GetParams()[i] > paramD->GetParamMax())
					ui.tableListValues->item(i, 0)->setBackground(Qt::red);
			for (int i = 0; i < static_cast<int>(paramD->Size()); ++i)
				if (paramD->GetValues()[i] < paramD->GetValueMin() || paramD->GetValues()[i] > paramD->GetValueMax())
					ui.tableListValues->item(i, 1)->setBackground(Qt::red);
		}
	}
	else if (param->GetType() == EUnitParameter::LIST_DOUBLE)
	{
		const auto* paramL = dynamic_cast<const CListRealUnitParameter*>(param);

		// enable GUI elements
		ui.frameListValues->setEnabled(true);

		ui.tableListValues->ShowCol(0, false); // hide times

		// create table with parameter values
		ui.tableListValues->setRowCount(static_cast<int>(paramL->Size()));
		ui.tableListValues->SetItemsColEditable(0, 1, paramL->GetValues());

		ui.tableListValues->SetColHeaderItem(1, paramL->GetName() + " [" + paramL->GetUnits() + "]");

		// check if values are in boundaries
		if (!paramL->IsInBounds())
			for (int i = 0; i < static_cast<int>(paramL->Size()); ++i)
				if (paramL->GetValue(i) < paramL->GetMin() || paramL->GetValue(i) > paramL->GetMax())
					ui.tableListValues->item(i, 1)->setBackground(Qt::red);
	}
	else if (param->GetType() == EUnitParameter::LIST_UINT64)
	{
		const auto* paramL = dynamic_cast<const CListUIntUnitParameter*>(param);

		// enable GUI elements
		ui.frameListValues->setEnabled(true);

		ui.tableListValues->ShowCol(0, false); // hide times

		// create table with parameter values
		ui.tableListValues->setRowCount(static_cast<int>(paramL->Size()));
		ui.tableListValues->SetItemsColEditable(0, 1, paramL->GetValues());

		ui.tableListValues->SetColHeaderItem(1, paramL->GetName() + " [" + paramL->GetUnits() + "]");

		// check if values are in boundaries
		if (!paramL->IsInBounds())
			for (int i = 0; i < static_cast<int>(paramL->Size()); ++i)
				if (paramL->GetValue(i) < paramL->GetMin() || paramL->GetValue(i) > paramL->GetMax())
					ui.tableListValues->item(i, 1)->setBackground(Qt::red);
	}
	else if (param->GetType() == EUnitParameter::LIST_INT64)
	{
		const auto* paramL = dynamic_cast<const CListIntUnitParameter*>(param);

		// enable GUI elements
		ui.frameListValues->setEnabled(true);

		ui.tableListValues->ShowCol(0, false); // hide times

		// create table with parameter values
		ui.tableListValues->setRowCount(static_cast<int>(paramL->Size()));
		ui.tableListValues->SetItemsColEditable(0, 1, paramL->GetValues());

		ui.tableListValues->SetColHeaderItem(1, paramL->GetName() + " [" + paramL->GetUnits() + "]");

		// check if values are in boundaries
		if (!paramL->IsInBounds())
			for (int i = 0; i < static_cast<int>(paramL->Size()); ++i)
				if (paramL->GetValue(i) < paramL->GetMin() || paramL->GetValue(i) > paramL->GetMax())
					ui.tableListValues->item(i, 1)->setBackground(Qt::red);
	}
}

void CFlowsheetEditor::UpdateUnitParamDescr() const
{
	ui.textParamDescr->clear();

	if (!m_pModelParams) return;
	if (ui.tableUnitParams->currentRow() == -1) return;

	auto* param = m_pModelParams->GetParameter(ui.tableUnitParams->GetItemUserData(ui.tableUnitParams->currentRow(), 0).toInt());
	if (!param) return;

	const EUnitParameter type = param->GetType();

	// compose description text
	QString text = QString::fromStdString(param->GetDescription());
	int extraLines = 0;
	if (type == EUnitParameter::CONSTANT || type == EUnitParameter::TIME_DEPENDENT)
	{
		text += "\n" + QString(StrConst::FE_UnitParamMinVal) + QString::number(type == EUnitParameter::CONSTANT
			? dynamic_cast<CConstRealUnitParameter*>(param)->GetMin()
			: dynamic_cast<CTDUnitParameter*>(param)->GetValueMin());
		text += "\n" + QString(StrConst::FE_UnitParamMaxVal) + QString::number(type == EUnitParameter::CONSTANT
			? dynamic_cast<CConstRealUnitParameter*>(param)->GetMax()
			: dynamic_cast<CTDUnitParameter*>(param)->GetValueMax());
		extraLines = 2;
	}

	// resize widget according to text length
	const QString tempText = QString::fromStdString(param->GetDescription());
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
	extraLines += tempText.split("\n", QString::SkipEmptyParts).count();
#else
	extraLines += tempText.split("\n", Qt::SkipEmptyParts).count();
#endif
	const QFontMetrics fontMetrics(ui.textParamDescr->font());
	const int fontHeight = fontMetrics.height();
	const int textWidth = fontMetrics.horizontalAdvance(tempText);
	const int linesNumber = static_cast<int>(std::ceil(static_cast<double>(textWidth) / ui.textParamDescr->width())) + extraLines;
	const int textHeight = std::max(50, std::min(fontHeight * linesNumber, 400));
	ui.textParamDescr->setMinimumHeight(textHeight);
	ui.textParamDescr->setMaximumHeight(textHeight);

	// set text
	ui.textParamDescr->setText(text);
}

void CFlowsheetEditor::EnableGUIElements() const
{
	const bool bNoUnit = ui.listModels->currentRow() == -1;
	ui.frameUnitParams->setEnabled(!bNoUnit);
	ui.framePorts->setEnabled(!bNoUnit);
	if (bNoUnit)
	{
		ui.tablePorts->clearContents();
		ui.comboUnits->setCurrentIndex(-1);
		ui.textParamDescr->clear();
		ui.tableUnitParams->clearContents();
		ui.tableListValues->clearContents();
	}
}

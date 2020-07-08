/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "CorrelationEditor.h"
#include "DescriptionEditor.h"

CCorrelationEditor::CCorrelationEditor(CCorrelation* _pCorrelation, QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.comboType->installEventFilter(this);
	m_pCorrelation = _pCorrelation;
	SetupComboType();
	InitializeConnections();
	UpdateWholeView();
}

QSize CCorrelationEditor::sizeHint() const
{
	return minimumSizeHint();
}

bool CCorrelationEditor::eventFilter(QObject* _obj, QEvent* _event)
{
	if (_event->type() == QEvent::Wheel && qobject_cast<QComboBox*>(_obj))
	{
		_event->ignore();
		return true;
	}
	return QObject::eventFilter(_obj, _event);
}

void CCorrelationEditor::InitializeConnections()
{
	connect(ui.lineTMin,		&QLineEdit::textEdited,									this, &CCorrelationEditor::TemperatureChanged);
	connect(ui.lineTMax,		&QLineEdit::textEdited,									this, &CCorrelationEditor::TemperatureChanged);
	connect(ui.linePMin,		&QLineEdit::textEdited,									this, &CCorrelationEditor::PressureChanged);
	connect(ui.linePMax,		&QLineEdit::textEdited,									this, &CCorrelationEditor::PressureChanged);
	connect(ui.comboType,		QOverload<int>::of(&QComboBox::currentIndexChanged),	this, &CCorrelationEditor::TypeChanged);
	connect(ui.tableParams,		&QTableWidget::cellChanged,								this, &CCorrelationEditor::ParameterChanged);
	connect(ui.toolButtonInfo,	&QToolButton::clicked,									this, &CCorrelationEditor::InfoButtonClicked);
}

void CCorrelationEditor::UpdateWholeView() const
{
	UpdateTemperature();
	UpdatePressure();
	UpdateType();
	UpdateTypeEquation();
	UpdateParameters();
	UpdateInfoButton();
}

void CCorrelationEditor::UpdateTemperature() const
{
	QSignalBlocker blocker1(ui.lineTMin);
	QSignalBlocker blocker2(ui.lineTMax);
	ui.lineTMin->setText(QString::number(m_pCorrelation->GetTInterval().min));
	ui.lineTMax->setText(QString::number(m_pCorrelation->GetTInterval().max));
}

void CCorrelationEditor::UpdatePressure() const
{
	QSignalBlocker blocker1(ui.linePMin);
	QSignalBlocker blocker2(ui.linePMax);
	ui.linePMin->setText(QString::number(m_pCorrelation->GetPInterval().min));
	ui.linePMax->setText(QString::number(m_pCorrelation->GetPInterval().max));
}

void CCorrelationEditor::UpdateType() const
{
	QSignalBlocker blocker(ui.comboType);
	for (int i = 0; i < ui.comboType->count(); ++i)
		if (m_pCorrelation->GetType() == static_cast<ECorrelationTypes>(ui.comboType->itemData(i).toUInt()))
		{
			ui.comboType->setCurrentIndex(i);
			break;
		}
}

void CCorrelationEditor::UpdateTypeEquation() const
{
	ui.labelEquation->setText(QString::fromStdWString(MDBDescriptors::correlations[m_pCorrelation->GetType()].formula));
}

void CCorrelationEditor::UpdateParameters() const
{
	QSignalBlocker blocker(ui.tableParams);
	ui.tableParams->clear();
	ui.tableParams->setRowCount(2);
	std::vector<double> vParams = m_pCorrelation->GetParameters();

	if (m_pCorrelation->GetType() == ECorrelationTypes::LIST_OF_T_VALUES || m_pCorrelation->GetType() == ECorrelationTypes::LIST_OF_P_VALUES)
	{
		ui.tableParams->setColumnCount(static_cast<int>(m_pCorrelation->GetParameters().size()) / 2 + 1);
		ui.tableParams->setVerticalHeaderItem(0, new QTableWidgetItem(m_pCorrelation->GetType() == ECorrelationTypes::LIST_OF_T_VALUES ? "T [K]" : "P [Pa]"));
		ui.tableParams->setVerticalHeaderItem(1, new QTableWidgetItem("Value"));
		for (int i = 0; i < static_cast<int>(vParams.size()); i += 2)
		{
			ui.tableParams->SetItemEditable(0, i / 2, vParams[i]);
			ui.tableParams->SetItemEditable(1, i / 2, vParams[i + 1]);
		}
		ui.tableParams->SetItemEditable(0, static_cast<int>(vParams.size()) / 2);
		ui.tableParams->SetItemEditable(1, static_cast<int>(vParams.size()) / 2);
	}
	else
	{
		ui.tableParams->setColumnCount(static_cast<int>(vParams.size()));
		ui.tableParams->setVerticalHeaderItem(0, new QTableWidgetItem(""));
		ui.tableParams->setVerticalHeaderItem(1, new QTableWidgetItem("Value"));
		for (int i = 0; i < static_cast<int>(vParams.size()); ++i)
		{
			ui.tableParams->SetItemNotEditable(0, i, QString('a' + i));
			ui.tableParams->SetItemEditable(1, i, vParams[i]);
		}
	}
	ui.tableParams->resizeColumnsToContents();
	ui.tableParams->resizeRowsToContents();
}

void CCorrelationEditor::UpdateInfoButton() const
{
	ui.toolButtonInfo->setToolTip(CDescriptionEditor::TextToDisplay(m_pCorrelation->GetDescription()));
}

void CCorrelationEditor::TemperatureChanged()
{
	m_pCorrelation->SetTInterval({ui.lineTMin->text().toDouble() , ui.lineTMax->text().toDouble()});
	emit CorrelationChanged();
}

void CCorrelationEditor::PressureChanged()
{
	m_pCorrelation->SetPInterval({ui.linePMin->text().toDouble() , ui.linePMax->text().toDouble()});
	emit CorrelationChanged();
}

void CCorrelationEditor::TypeChanged()
{
	m_pCorrelation->SetType(static_cast<ECorrelationTypes>(ui.comboType->currentData().toUInt()));
	UpdateTypeEquation();
	UpdateParameters();
	emit CorrelationChanged();
}

void CCorrelationEditor::ParameterChanged(int _iRow, int _iCol)
{
	if (m_pCorrelation->GetType() == ECorrelationTypes::LIST_OF_T_VALUES || m_pCorrelation->GetType() == ECorrelationTypes::LIST_OF_P_VALUES)
	{
		std::vector<double> vParams = ui.tableParams->GetRowValues(0);
		std::vector<double> vValues = ui.tableParams->GetRowValues(1);
		if (ui.tableParams->item(0, ui.tableParams->columnCount() - 1)->text().isEmpty())
		{
			vParams.pop_back();
			vValues.pop_back();
		}
		std::vector<double> vToSet;
		for (size_t i = 0; i < vParams.size(); ++i)
		{
			vToSet.push_back(vParams[i]);
			vToSet.push_back(vValues[i]);
		}
		m_pCorrelation->SetParameters(vToSet);
	}
	else
		m_pCorrelation->SetParameters(ui.tableParams->GetRowValues(1));

	UpdateParameters();
	ui.tableParams->RestoreSelectedCell(_iRow, _iCol);
	emit CorrelationChanged();
}

void CCorrelationEditor::InfoButtonClicked()
{
	auto* pEditor = new CDescriptionEditor(m_pCorrelation, this);
	pEditor->exec();
	if (pEditor->IsDescriptionChanged())
	{
		UpdateInfoButton();
		emit CorrelationChanged();
	}
	delete pEditor;
}

void CCorrelationEditor::SetupComboType() const
{
	for (auto c : MDBDescriptors::correlations)
	{
		ui.comboType->addItem(QString::fromStdString(c.second.name), static_cast<unsigned>(c.first));
		if (m_pCorrelation->GetType() == c.first)
			ui.comboType->setCurrentIndex(ui.comboType->count() - 1);
	}
}

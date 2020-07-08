/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "PropertyEditor.h"
#include "CorrelationEditor.h"
#include <QMessageBox>

CPropertyEditor::CPropertyEditor(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);
	ui.plot->SetLegendPosition(POSITION_TOP_RIGHT);
	m_pProperty = nullptr;
	m_bAvoidSignal = false;
	InitializeConnections();
}

void CPropertyEditor::InitializeConnections() const
{
	// signals from buttons
	connect(ui.buttonAdd,		&QPushButton::clicked,		this, &CPropertyEditor::AddCorrelation);
	connect(ui.buttonRemove,	&QPushButton::clicked,		this, &CPropertyEditor::RemoveCorrelation);
	connect(ui.buttonUp,		&QPushButton::clicked,		this, &CPropertyEditor::UpCorrelation);
	connect(ui.buttonDown,		&QPushButton::clicked,		this, &CPropertyEditor::DownCorrelation);

	// signals from plot
	connect(ui.radioButtonT,	&QRadioButton::clicked,		this, &CPropertyEditor::PlotTypeChanged);
	connect(ui.radioButtonP,	&QRadioButton::clicked,		this, &CPropertyEditor::PlotTypeChanged);
	connect(ui.linePlotValue,	&QLineEdit::textChanged,	this, &CPropertyEditor::UpdatePlot);

	// signals from calculator
	connect(ui.lineCalcT,		&QLineEdit::textChanged,	this, &CPropertyEditor::UpdateCalculator);
	connect(ui.lineCalcP,		&QLineEdit::textChanged,	this, &CPropertyEditor::UpdateCalculator);
}

void CPropertyEditor::setVisible(const bool _bVisible)
{
	if (_bVisible && !this->isVisible())
		UpdateWholeView();
	QWidget::setVisible(_bVisible);
}

void CPropertyEditor::SetProperty(CTPDProperty* _pProperty)
{
	if(_pProperty)
	{
		m_pProperty = _pProperty;
		UpdateWholeView();
	}
	else
		Clear();
}

void CPropertyEditor::Clear()
{
	m_bAvoidSignal = true;
	ui.listCorrelations->clear();
	ui.radioButtonT->setChecked(true);
	ui.plot->ClearPlot();
	ui.lineCalcT->clear();
	ui.lineCalcP->clear();
	ui.lineCalcVal->clear();
	m_bAvoidSignal = false;
}

void CPropertyEditor::UpdateWholeView()
{
	UpdateCorrelations();
	UpdatePropertyName();
	UpdateCalculator();
	UpdatePlot();
	PlotTypeChanged();
}

void CPropertyEditor::UpdateCorrelations()
{
	if (!m_pProperty) return;
	m_bAvoidSignal = true;

	ui.listCorrelations->clear();
	ui.listCorrelations->setColumnCount(1);
	ui.listCorrelations->setRowCount(static_cast<int>(m_pProperty->CorrelationsNumber()));
	for (size_t i = 0; i < m_pProperty->CorrelationsNumber(); ++i)
	{
		auto* pEditor = new CCorrelationEditor(m_pProperty->GetCorrelation(i), ui.listCorrelations);
		ui.listCorrelations->setCellWidget(static_cast<int>(i), 0, pEditor);
		connect(pEditor, &CCorrelationEditor::CorrelationChanged, this, &CPropertyEditor::CorrelationChanged);
	}
	ui.listCorrelations->resizeRowsToContents();

	m_bAvoidSignal = false;
}

void CPropertyEditor::UpdatePropertyName() const
{
	if (!m_pProperty)
		ui.labelPropertyName->clear();
	else
		ui.labelPropertyName->setText(QString::fromStdString(m_pProperty->GetName()) + " [" + QString::fromStdWString(m_pProperty->GetUnits()) + "]:");
}

void CPropertyEditor::UpdateCalculator() const
{
	if (!m_pProperty) return;
	if (m_bAvoidSignal) return;
	if (ui.lineCalcT->text().isEmpty() || ui.lineCalcP->text().isEmpty())
	{
		ui.lineCalcVal->clear();
		return;
	}
	const double dT = ui.lineCalcT->text().toDouble();
	const double dP = ui.lineCalcP->text().toDouble();
	const double dV = m_pProperty->GetValue(dT, dP);
	ui.lineCalcVal->setText(QString::number(dV));
}

void CPropertyEditor::UpdatePlot() const
{
	static unsigned pointsNumber = 1000;

	if (!m_pProperty) return;
	if (m_bAvoidSignal) return;
	ui.plot->ClearPlot();

	const bool bTemperature = ui.radioButtonT->isChecked();
	for (size_t i = 0; i < m_pProperty->CorrelationsNumber(); ++i)
	{
		const CCorrelation *pCorrelation = m_pProperty->GetCorrelation(i);
		const double dV1 = bTemperature ? pCorrelation->GetTInterval().min : pCorrelation->GetPInterval().min;
		const double dV2 = bTemperature ? pCorrelation->GetTInterval().max : pCorrelation->GetPInterval().max;
		const double dStep = (dV2 - dV1) / pointsNumber;
		const SInterval interv = bTemperature ? pCorrelation->GetPInterval() : pCorrelation->GetTInterval();
		const double dCurrSecond = ui.linePlotValue->text().toDouble();
		if ((bTemperature && !pCorrelation->IsPInInterval(dCurrSecond)) || (!bTemperature && !pCorrelation->IsTInInterval(dCurrSecond)))
			continue;
		const QString sName = "#" + QString::number(i + 1) + (bTemperature ? ": P=" : ": T=") + QString::number(dCurrSecond) + ", [" + QString::number(interv.min) + ".." + QString::number(interv.max) + (bTemperature ? "] Pa" : "] K");
		const QColor color = m_pProperty->CorrelationsNumber() == 1 ? Qt::blue : Qt::GlobalColor(Qt::red + i % (Qt::transparent - Qt::red));
		QtPlot::SCurve* pCurve = new QtPlot::SCurve(sName, color, 3, true, true, bTemperature ? QtPlot::LABEL_TEMPERATURE : QtPlot::LABEL_PRESSURE, QtPlot::LABEL_MANUAL);
		const unsigned nCurve = ui.plot->AddCurve(pCurve);

		if(pCorrelation->GetType() != ECorrelationTypes::LIST_OF_T_VALUES && pCorrelation->GetType() != ECorrelationTypes::LIST_OF_P_VALUES)
		{
			for (unsigned j = 0; j <= pointsNumber; ++j)
			{
				const double dCurrT = bTemperature ? dV1 + dStep*j : dCurrSecond;
				const double dCurrP = !bTemperature ? dV1 + dStep*j : dCurrSecond;
				ui.plot->AddPoint(nCurve, QPointF(dV1 + dStep*j, pCorrelation->GetValue(dCurrT, dCurrP)));
			}
		}
		else
		{
			std::vector<double> vParams = pCorrelation->GetParameters();
			for (size_t j = 0; j < vParams.size(); j+=2)
			{
				const double dCurrT = bTemperature ? vParams[j] : dCurrSecond;
				const double dCurrP = !bTemperature ? vParams[j] : dCurrSecond;
				ui.plot->AddPoint(nCurve, QPointF(vParams[j], pCorrelation->GetValue(dCurrT, dCurrP)));
			}
		}
	}

	ui.plot->SetManualLabelsNames("", QString::fromStdString(m_pProperty->GetName()) + " [" + QString::fromStdWString(m_pProperty->GetUnits()) + "]");
}

void CPropertyEditor::UpdatePlotType() const
{
	if (!m_pProperty) return;
	if (m_bAvoidSignal) return;
	QSignalBlocker blocker(ui.linePlotValue);
	ui.labelPlotValue->setText(ui.radioButtonT->isChecked() ? "P [Pa]:" : "T [K]:");
	ui.linePlotValue->setText(QString::number(ui.radioButtonT->isChecked() ? STANDARD_CONDITION_P : STANDARD_CONDITION_T));
}

void CPropertyEditor::AddCorrelation()
{
	if (!m_pProperty) return;
	m_pProperty->AddCorrelation(ECorrelationTypes::CONSTANT, { 0 });
	UpdateCorrelations();
	UpdateCalculator();
	UpdatePlot();
	emit MDBPropertyChanged();
}

void CPropertyEditor::RemoveCorrelation()
{
	if (!m_pProperty) return;

	if (QMessageBox::question(this, tr("Remove correlation"), tr("Remove selected correlation?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel) == QMessageBox::Yes)
	{
		m_pProperty->RemoveCorrelation(ui.listCorrelations->currentRow());
		UpdateCorrelations();
		UpdateCalculator();
		UpdatePlot();
		emit MDBPropertyChanged();
	}
}

void CPropertyEditor::UpCorrelation()
{
	const int iRow = ui.listCorrelations->currentRow();
	if (m_pProperty->ShiftCorrelationUp(iRow))
	{
		UpdateCorrelations();
		ui.listCorrelations->RestoreSelectedCell(iRow - 1, 0);
		emit MDBPropertyChanged();
	}
}

void CPropertyEditor::DownCorrelation()
{
	const int iRow = ui.listCorrelations->currentRow();
	if (m_pProperty->ShiftCorrelationDown(iRow))
	{
		UpdateCorrelations();
		ui.listCorrelations->RestoreSelectedCell(iRow + 1, 0);
		emit MDBPropertyChanged();
	}
}

void CPropertyEditor::CorrelationChanged()
{
	UpdateCalculator();
	UpdatePlot();
	emit MDBPropertyChanged();
}

void CPropertyEditor::PlotTypeChanged() const
{
	UpdatePlotType();
	UpdatePlot();
}

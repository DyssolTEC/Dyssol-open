/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "BasicStreamsViewer.h"
#include "DistributionsFunctions.h"
#include "DyssolStringConstants.h"
#include "DyssolUtilities.h"
#include "ContainerFunctions.h"
#include "BaseStream.h"
#include "Phase.h"
#include "DistributionsGrid.h"
#include <QMenu>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QTextStream>

CBasicStreamsViewer::CBasicStreamsViewer(CFlowsheet* _pFlowsheet, QWidget* parent)
	: QWidget(parent),
	m_pFlowsheet(_pFlowsheet),
	m_dCurrentTime(0.)
{
	ui.setupUi(this);
	ui.tabTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
	QSizePolicy spl = ui.labelDim2->sizePolicy();
	spl.setRetainSizeWhenHidden(true);
	ui.labelDim2->setSizePolicy(spl);
	ui.labelCompounds->setSizePolicy(spl);
	QSizePolicy spc = ui.comboBoxDim2->sizePolicy();
	spc.setRetainSizeWhenHidden(true);
	ui.comboBoxDim2->setSizePolicy(spc);
	ui.comboBoxCompounds->setSizePolicy(spc);

	SetupComboBoxPSDType();
	SetupComboBoxPSDGrid();
	UpdateWholeView();
}

void CBasicStreamsViewer::InitializeConnections() const
{
	connect(ui.comboBoxProperties,	QOverload<int>::of(&QComboBox::currentIndexChanged),	this, &CBasicStreamsViewer::PropertyChanged);
	connect(ui.comboBoxDim1,		QOverload<int>::of(&QComboBox::currentIndexChanged),	this, &CBasicStreamsViewer::ComboRowsChanged);
	connect(ui.comboBoxDim2,		QOverload<int>::of(&QComboBox::currentIndexChanged),	this, &CBasicStreamsViewer::ComboColsChanged);
	connect(ui.comboBoxCompounds,	QOverload<int>::of(&QComboBox::currentIndexChanged),	this, &CBasicStreamsViewer::ComboCompoundsChanged);
	connect(ui.comboBoxPSDType,		QOverload<int>::of(&QComboBox::currentIndexChanged),	this, &CBasicStreamsViewer::ComboPSDTypeChanged);
	connect(ui.comboBoxPSDGridType,	QOverload<int>::of(&QComboBox::currentIndexChanged),	this, &CBasicStreamsViewer::ComboPSDGridTypeChanged);
	connect(ui.horizontalSlider,	&QSlider::valueChanged,			this, &CBasicStreamsViewer::SliderMoved);
	connect(ui.tabWidget,			&QTabWidget::currentChanged,	this, &CBasicStreamsViewer::TabChanged);
	connect(ui.toolButtonPrevTime,	&QToolButton::clicked,			this, &CBasicStreamsViewer::PrevTimeClicked);
	connect(ui.toolButtonNextTime,	&QToolButton::clicked,			this, &CBasicStreamsViewer::NextTimeClicked);
	connect(ui.lineEditTime,		&QLineEdit::editingFinished,	this, &CBasicStreamsViewer::TimeEdited);
}

void CBasicStreamsViewer::SetStreams(const std::vector<const CBaseStream*>& _vStreams)
{
	m_vSelectedStreams = _vStreams;

	SetupComboBoxProperties();
	GetSelectedTimePoints();
	GetSelectedDistributions();
	SetupTimeSlider();
	UpdateWidgetsVisible();
	UpdateWidgetsEnable();
	UpdateTPNumberLabel();
	UpdateTimeLabel();
	UpdateSliderPosition();
	UpdateTabView();
}

void CBasicStreamsViewer::UpdateWholeView()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	SetupComboBoxes();
	GetSelectedTimePoints();
	GetSelectedDistributions();
	SetupTimeSlider();
	UpdateWidgetsVisible();
	UpdateWidgetsEnable();
	UpdateTPNumberLabel();
	UpdateTimeLabel();
	UpdateSliderPosition();
	UpdateTabView();

	QApplication::restoreOverrideCursor();
}

void CBasicStreamsViewer::setVisible(bool _bVisible)
{
	if (_bVisible && !isVisible())
		UpdateWholeView();
	QWidget::setVisible(_bVisible);
}

void CBasicStreamsViewer::SetupComboBoxes() const
{
	// setup properties
	SetupComboBoxProperties();

	// setup dimensions
	SetupComboBoxDims(ui.comboBoxDim1, 1);
	SetupComboBoxDims(ui.comboBoxDim2, 0);

	// setup compounds
	SetupComboBoxCompounds();
}

void CBasicStreamsViewer::SetupComboBoxProperties() const
{
	QSignalBlocker block(ui.comboBoxProperties);

	const bool isStream = !m_vSelectedStreams.empty() && dynamic_cast<const CStream*>(m_vSelectedStreams.front()) != nullptr;

	const int iOld = ui.comboBoxProperties->currentIndex();
	ui.comboBoxProperties->clear();

	ui.comboBoxProperties->addItem(isStream ? StrConst::BSV_PropMassFlow : StrConst::BSV_PropMass, E2I(EPropertyType::Mass));
	ui.comboBoxProperties->addItem(StrConst::BSV_PropTemperature, E2I(EPropertyType::Temperatue));
	ui.comboBoxProperties->addItem(StrConst::BSV_PropPressure, E2I(EPropertyType::Pressure));
	ui.comboBoxProperties->addItem(StrConst::BSV_PropPhaseFractions, E2I(EPropertyType::PhaseFraction));
	for (unsigned i = 0; i < m_pFlowsheet->GetPhasesNumber(); ++i)
		ui.comboBoxProperties->addItem(QString::fromStdString(m_pFlowsheet->GetPhaseName(i)), E2I(EPropertyType::Phase1) + i);
	ui.comboBoxProperties->addItem(StrConst::BSV_PropSauterDiameter, E2I(EPropertyType::SauterDiameter));
	if (m_pFlowsheet->IsPhaseDefined(SOA_SOLID))
		ui.comboBoxProperties->addItem(StrConst::BSV_PropSolidDistribution, E2I(EPropertyType::SolidDistr));

	block.unblock();
	RestorePosition(ui.comboBoxProperties, iOld);
}

void CBasicStreamsViewer::SetupComboBoxDims(QComboBox* _combo, int _defaultPos) const
{
	QSignalBlocker block(_combo);
	const int iOld = _combo->currentIndex();

	_combo->clear();
	_combo->addItem(StrConst::BSV_ComboBoxNoDimension, DISTR_UNDEFINED);

	for (size_t i = 0; i < m_pFlowsheet->GetDistributionsGrid()->GetDistributionsNumber(); ++i)
	{
		const EDistrTypes type = m_pFlowsheet->GetDistributionsGrid()->GetDistrType(i);
		const int iType = GetDistributionTypeIndex(type);
		if (iType < DISTRIBUTIONS_NUMBER && m_pFlowsheet->GetDistributionsGrid()->GetGridEntryByIndex(i) == EGridEntry::GRID_NUMERIC)
			_combo->addItem(std::vector<QString>{ DISTR_NAMES }[iType], E2I(std::vector<EDistrTypes>{ DISTR_TYPES }[iType]));
	}

	block.unblock();
	if(iOld == -1 && _combo->count() < 2)
		_combo->setCurrentIndex(-1);
	else
		RestorePosition(_combo, iOld, _defaultPos);
}

void CBasicStreamsViewer::SetupComboBoxCompounds() const
{
	QSignalBlocker block(ui.comboBoxCompounds);
	const int iOld = ui.comboBoxCompounds->currentIndex();

	ui.comboBoxCompounds->clear();
	ui.comboBoxCompounds->addItem(StrConst::BSV_ComboBoxAllCompounds, "");
	for (size_t i=0; i<m_pFlowsheet->GetCompoundsNumber(); ++i)
		ui.comboBoxCompounds->addItem(QString::fromStdString(m_pFlowsheet->GetCompoundsNames()[i]), QString::fromStdString(m_pFlowsheet->GetCompounds()[i]));

	block.unblock();
	RestorePosition(ui.comboBoxCompounds, iOld);
}

void CBasicStreamsViewer::SetupComboBoxPSDType() const
{
	QSignalBlocker block(ui.comboBoxPSDType);
	ui.comboBoxPSDType->addItem(StrConst::BSV_TableHeaderq3,		PSD_q3);
	ui.comboBoxPSDType->addItem(StrConst::BSV_TableHeaderQ3,		PSD_Q3);
	ui.comboBoxPSDType->addItem(StrConst::BSV_TableHeaderq2,		PSD_q2);
	ui.comboBoxPSDType->addItem(StrConst::BSV_TableHeaderQ2,		PSD_Q2);
	ui.comboBoxPSDType->addItem(StrConst::BSV_TableHeaderq0,		PSD_q0);
	ui.comboBoxPSDType->addItem(StrConst::BSV_TableHeaderQ0,		PSD_Q0);
	ui.comboBoxPSDType->addItem(StrConst::BSV_TableHeaderMassFrac,	PSD_MassFrac);
	ui.comboBoxPSDType->addItem(StrConst::BSV_TableHeaderNumber,	PSD_Number);
}

void CBasicStreamsViewer::SetupComboBoxPSDGrid() const
{
	QSignalBlocker block(ui.comboBoxPSDGridType);
	ui.comboBoxPSDGridType->addItem(StrConst::BSV_HeaderDiameter,	E2I(EPSDGridType::DIAMETER));
	ui.comboBoxPSDGridType->addItem(StrConst::BSV_HeaderVolume,		E2I(EPSDGridType::VOLUME));
}

void CBasicStreamsViewer::SetupTimeSlider() const
{
	if (m_vSelectedStreams.empty() || ChosenProperty() != EPropertyType::SolidDistr) return;

	QSignalBlocker block(ui.horizontalSlider);
	const int iOld = ui.horizontalSlider->value();

	ui.horizontalSlider->setMinimum(0);
	ui.horizontalSlider->setMaximum(100);
	ui.horizontalSlider->setTickInterval(1);
	if (m_vSelectedTP.size() < 3)
		ui.horizontalSlider->setMaximum(static_cast<int>(m_vSelectedTP.size()) - 1);

	if (iOld < ui.horizontalSlider->maximum())	ui.horizontalSlider->setValue(iOld);
	else if (ui.horizontalSlider->maximum())	ui.horizontalSlider->setValue(0);
	else										ui.horizontalSlider->setValue(-1);
}

void CBasicStreamsViewer::GetSelectedTimePoints()
{
	m_vSelectedTP.clear();
	for (const auto& s : m_vSelectedStreams)
		m_vSelectedTP = VectorsUnionSorted(m_vSelectedTP, s->GetAllTimePoints());
}

void CBasicStreamsViewer::GetSelectedDistributions()
{
	m_vSelected2D.clear();
	m_vSelectedMD.clear();

	const EPropertyType type = ChosenProperty();
	for (const auto& s : m_vSelectedStreams)
	{
		switch (type)
		{
		case EPropertyType::Mass:
			m_vSelected2D.push_back(s->GetOverallProperty(EOverall::OVERALL_MASS));
			break;
		case EPropertyType::Temperatue:
			m_vSelected2D.push_back(s->GetOverallProperty(EOverall::OVERALL_TEMPERATURE));
			break;
		case EPropertyType::Pressure:
			m_vSelected2D.push_back(s->GetOverallProperty(EOverall::OVERALL_PRESSURE));
			break;
		case EPropertyType::PhaseFraction:
			if (const auto* phase = s->GetPhase(EPhase::SOLID))
				m_vSelected2D.push_back(phase->Fractions());
			if (const auto* phase = s->GetPhase(EPhase::LIQUID))
				m_vSelected2D.push_back(phase->Fractions());
			if (const auto* phase = s->GetPhase(EPhase::VAPOR))
				m_vSelected2D.push_back(phase->Fractions());
			break;
		case EPropertyType::Phase1:
		case EPropertyType::Phase2:
		case EPropertyType::Phase3:
		case EPropertyType::Phase4:
			m_vSelectedMD.push_back(s->GetPhase(EPhase(E2I(type) - E2I(EPropertyType::Phase1) + 1))->MDDistr());
			break;
		case EPropertyType::SolidDistr:
		case EPropertyType::SauterDiameter:
		{
			const int iSolid = m_pFlowsheet->GetPhaseIndex(SOA_SOLID);
			if (iSolid != -1)
				m_vSelectedMD.push_back(s->GetPhase(EPhase::SOLID)->MDDistr());
			break;
		}
		}
	}
}

void CBasicStreamsViewer::UpdateSliderPosition()
{
	QSignalBlocker block(ui.horizontalSlider);

	if (m_vSelectedTP.empty())
	{
		ui.horizontalSlider->setValue(ui.horizontalSlider->minimum());
		return;
	}

	const double dIntervalLen = (m_vSelectedTP.back() - m_vSelectedTP.front()) / ui.horizontalSlider->maximum();
	ui.horizontalSlider->setValue((m_dCurrentTime - m_vSelectedTP.front()) / dIntervalLen);
}

void CBasicStreamsViewer::UpdateTimeLabel() const
{
	QSignalBlocker block(ui.lineEditTime);
	ui.lineEditTime->setText(QString::number(m_dCurrentTime));
}

void CBasicStreamsViewer::UpdateTPNumberLabel() const
{
	ui.labelPoints->setText(QString::number(m_vSelectedTP.size()));
}

void CBasicStreamsViewer::UpdateTabView()
{
	switch (ChosenTab())
	{
	case ETabType::Table:
		ui.labelDim1->setText(StrConst::BSV_LabelRows);
		UpdateTableTab();
		break;
	case ETabType::Plot:
		ui.labelDim1->setText(StrConst::BSV_LabelDistribution);
		UpdatePlotTab();
		break;
	default: break;
	}
}

void CBasicStreamsViewer::UpdateTableTab()
{
	ui.tabTable->SetGeometry(0, 0);
	if (m_vSelectedStreams.empty()) return;

	switch (ChosenProperty())
	{
	case EPropertyType::Mass:			SetMTPToTable(MTP_MASS);		break;
	case EPropertyType::Temperatue:		SetMTPToTable(MTP_TEMPERATURE);	break;
	case EPropertyType::Pressure:		SetMTPToTable(MTP_PRESSURE);	break;
	case EPropertyType::PhaseFraction:	SetPhaseFractionsToTable();		break;
	case EPropertyType::Phase1:			SetPhaseCompoundsToTable();		break;
	case EPropertyType::Phase2:			SetPhaseCompoundsToTable();		break;
	case EPropertyType::Phase3:			SetPhaseCompoundsToTable();		break;
	case EPropertyType::Phase4:			SetPhaseCompoundsToTable();		break;
	case EPropertyType::SolidDistr:		SetSolidDistrsToTable();		break;
	case EPropertyType::SauterDiameter:	SetSauterDiameterToTable();		break;
	}

	ui.tabTable->resizeColumnsToContents();
}

void CBasicStreamsViewer::UpdatePlotTab()
{
	ui.tabPlot->ClearPlot();
	if (m_vSelectedStreams.empty()) return;

	switch (ChosenProperty())
	{
	case EPropertyType::Mass:			SetMTPToPlot(MTP_MASS);			break;
	case EPropertyType::Temperatue:		SetMTPToPlot(MTP_TEMPERATURE);	break;
	case EPropertyType::Pressure:		SetMTPToPlot(MTP_PRESSURE);		break;
	case EPropertyType::PhaseFraction:	SetPhaseFractionsToPlot();		break;
	case EPropertyType::Phase1:			SetPhaseCompoundsToPlot();		break;
	case EPropertyType::Phase2:			SetPhaseCompoundsToPlot();		break;
	case EPropertyType::Phase3:			SetPhaseCompoundsToPlot();		break;
	case EPropertyType::Phase4:			SetPhaseCompoundsToPlot();		break;
	case EPropertyType::SauterDiameter:	SetSauterDiameterToPlot();		break;
	case EPropertyType::SolidDistr:		SetSolidDistrsToPlot();			break;
	}
}

void CBasicStreamsViewer::UpdateWidgetsEnable() const
{
	const bool flag = !m_vSelectedStreams.empty();
	ui.comboBoxDim1->setEnabled(flag);
	ui.comboBoxDim2->setEnabled(flag);
	ui.comboBoxCompounds->setEnabled(flag);
	ui.comboBoxPSDType->setEnabled(flag);
	ui.comboBoxPSDGridType->setEnabled(flag);
	ui.frameTime->setEnabled(flag);
	ui.comboBoxProperties->setEnabled(flag);
	ui.tabWidget->setEnabled(flag);
}

void CBasicStreamsViewer::UpdateWidgetsVisible() const
{
	const EPropertyType prop = ChosenProperty();
	const bool flag1 = prop == EPropertyType::SolidDistr;
	ui.labelDim1->setVisible(flag1);
	ui.comboBoxDim1->setVisible(flag1);
	ui.labelCompounds->setVisible(flag1);
	ui.comboBoxCompounds->setVisible(flag1);
	ui.frameTime->setVisible(flag1);

	const ETabType tab = ChosenTab();
	const bool flag2 = prop == EPropertyType::SolidDistr && tab == ETabType::Table;
	ui.labelDim2->setVisible(flag2);
	ui.comboBoxDim2->setVisible(flag2);

	const EDistrTypes dim1 = ChosenDim(EDimType::Row);
	const EDistrTypes dim2 = ChosenDim(EDimType::Col);
	bool flag3 = false;
	if(prop == EPropertyType::SolidDistr)
		switch (tab)
		{
		case ETabType::Table:	flag3 = dim1 == DISTR_SIZE && dim2 == DISTR_UNDEFINED || dim1 == DISTR_UNDEFINED && dim2 == DISTR_SIZE || dim1 == DISTR_SIZE && dim2 == DISTR_SIZE; break;
		case ETabType::Plot:	flag3 = dim1 == DISTR_SIZE;	break;
		}

	ui.labelPSDType->setVisible(flag3);
	ui.comboBoxPSDType->setVisible(flag3);
	ui.labelPSDGridType->setVisible(flag3);
	ui.comboBoxPSDGridType->setVisible(flag3);
}

CBasicStreamsViewer::EPropertyType CBasicStreamsViewer::ChosenProperty() const
{
	if (ui.comboBoxProperties->currentIndex() == -1) return static_cast<EPropertyType>(-1);
	return static_cast<EPropertyType>(ui.comboBoxProperties->currentData().toUInt());
}

EDistrTypes CBasicStreamsViewer::ChosenDim(EDimType _dim) const
{
	QComboBox* box = _dim == EDimType::Row ? ui.comboBoxDim1 : ui.comboBoxDim2;
	if (box->currentIndex() == -1) return DISTR_UNDEFINED;
	return static_cast<EDistrTypes>(box->currentData().toUInt());
}

EPSDTypes CBasicStreamsViewer::ChosenPSDType() const
{
	if (ui.comboBoxPSDType->currentIndex() == -1) return static_cast<EPSDTypes>(-1);
	return static_cast<EPSDTypes>(ui.comboBoxPSDType->currentData().toUInt());
}

EPSDGridType CBasicStreamsViewer::ChosenPSDGridType() const
{
	if (ui.comboBoxPSDGridType->currentIndex() == -1) return static_cast<EPSDGridType>(-1);
	return static_cast<EPSDGridType>(ui.comboBoxPSDGridType->currentData().toUInt());
}

std::string CBasicStreamsViewer::ChosenCompound() const
{
	if (ui.comboBoxCompounds->currentIndex() == -1) return {};
	return ui.comboBoxCompounds->currentData().toString().toStdString();
}

CBasicStreamsViewer::ETabType CBasicStreamsViewer::ChosenTab() const
{
	switch (ui.tabWidget->currentIndex())
	{
	case 0:		return ETabType::Table;
	case 1:		return ETabType::Plot;
	default:	return {};
	}
}

std::vector<std::string> CBasicStreamsViewer::TableHeaders(EDistrTypes _distr) const
{
	std::vector<std::string> vHeaders;
	std::vector<std::pair<double, double>> vNumGrid;

	if (_distr != DISTR_UNDEFINED)
	{
		const EGridEntry gridType = m_pFlowsheet->GetDistributionsGrid()->GetGridEntryByDistr(_distr);

		switch (gridType)
		{
		case EGridEntry::GRID_NUMERIC:
		{
			std::vector<double> grid = m_pFlowsheet->GetDistributionsGrid()->GetNumericGridByDistr(_distr);
			vNumGrid.resize(grid.size() - 1);
			for (int i = 0; i < static_cast<int>(grid.size()) - 1; ++i)
				vNumGrid[i] = { grid[i] , grid[i + 1] };
			break;
		}
		case EGridEntry::GRID_SYMBOLIC:
		{
			std::vector<std::string> grid = m_pFlowsheet->GetDistributionsGrid()->GetSymbolicGridByDistr(_distr);
			for (const auto& cell : grid)
				vHeaders.push_back(cell);
			break;
		}
		default:
			break;
		}

		// convert values to text
		if (gridType == EGridEntry::GRID_NUMERIC)
		{
			switch (_distr)
			{
			case DISTR_SIZE:
				switch (ChosenPSDGridType())
				{
				case EPSDGridType::DIAMETER:
					for (const auto& pair : vNumGrid)
						vHeaders.push_back(StringFunctions::Double2String(pair.first) + " : " + StringFunctions::Double2String(pair.second) + " " + StrConst::FUN_DiameterUnits);
					break;
				case EPSDGridType::VOLUME:
					for (const auto& pair : vNumGrid)
						vHeaders.push_back(StringFunctions::Double2String(DiameterToVolume(pair.first)) + " : " + StringFunctions::Double2String(DiameterToVolume(pair.second)) + " " + StrConst::FUN_VolumeUnits);
					break;
				}
				break;
			default:
				for (const auto& pair : vNumGrid)
					vHeaders.push_back(StringFunctions::Double2String(pair.first) + " : " + StringFunctions::Double2String(pair.second) + " " + StrConst::FUN_EmptyUnits);
				break;
			}
		}
	}

	return vHeaders;
}

std::string CBasicStreamsViewer::PSDSymbolicName(EDistrTypes _distr) const
{
	std::string res;
	if (_distr != DISTR_SIZE)
		res = StrConst::BSV_TableHeaderMassFrac + std::string{" "} + StrConst::FUN_EmptyUnits;
	else
	{
		switch (ChosenPSDType())
		{
		case PSD_q3:		res = StrConst::BSV_TableHeaderq3;			break;
		case PSD_Q3:		res = StrConst::BSV_TableHeaderQ3;			break;
		case PSD_q0:		res = StrConst::BSV_TableHeaderq0;			break;
		case PSD_Q0:		res = StrConst::BSV_TableHeaderQ0;			break;
		case PSD_MassFrac:	res = StrConst::BSV_TableHeaderMassFrac;	break;
		case PSD_Number:	res = StrConst::BSV_TableHeaderNumber;		break;
		case PSD_q2:		res = StrConst::BSV_TableHeaderq2;			break;
		case PSD_Q2:		res = StrConst::BSV_TableHeaderQ2;			break;
		}
		switch (ChosenPSDType())
		{
		case PSD_MassFrac: case PSD_Number: case PSD_Q0: case PSD_Q2: case PSD_Q3:
			res += std::string{ " " } + StrConst::FUN_EmptyUnits;
			break;
		case PSD_q3: case PSD_q0: case PSD_q2:
			switch (ChosenPSDGridType())
			{
			case EPSDGridType::DIAMETER:	res += std::string{ " " } + StrConst::FUN_DiameterUnitsInv;	break;
			case EPSDGridType::VOLUME:		res += std::string{ " " } + StrConst::FUN_VolumeUnitsInv;	break;
			}
			break;
		}
	}
	return res;
}

QString CBasicStreamsViewer::DistrSymbolicName(EDistrTypes _distr) const
{
	if (_distr == DISTR_SIZE)
		switch (ChosenPSDGridType())
		{
		case EPSDGridType::DIAMETER:	return StrConst::BSV_HeaderDiameter + QString{ " " } + StrConst::FUN_DiameterUnits;
		case EPSDGridType::VOLUME:		return StrConst::BSV_HeaderVolume + QString{ " " } + StrConst::FUN_VolumeUnits;
		}
	return QStringList{ DISTR_NAMES }[GetDistributionTypeIndex(_distr)] + QString{ " " } + StrConst::FUN_EmptyUnits;
}

void CBasicStreamsViewer::SetMTPToTable(int _type)
{
	if (m_vSelected2D.empty() || _type < MTP_MASS || _type > MTP_PRESSURE) return;

	ui.tabTable->SetGeometry(static_cast<int>(m_vSelectedTP.size()), static_cast<int>(m_vSelected2D.size()) + 1);
	ui.tabTable->SetColHeaderItem(0, StrConst::BSV_TableHeaderTime);
	ui.tabTable->SetItemsColNotEditable(0, 0, m_vSelectedTP);

	for (int i = 0; i < static_cast<int>(m_vSelected2D.size()); ++i)
	{
		ui.tabTable->SetColHeaderItem(i + 1, m_vSelectedStreams[i]->GetName() + "\n" + m_vSelected2D[i]->GetName() + " [" + m_vSelected2D[i]->GetUnits() + "]");
		std::vector<double> values;
		values.reserve(m_vSelectedTP.size());
		for (double t : m_vSelectedTP)
			values.push_back(m_vSelected2D[i]->GetValue(t));
		ui.tabTable->SetItemsColNotEditable(0, i + 1, values);
	}
}

void CBasicStreamsViewer::SetPhaseFractionsToTable()
{
	if (m_vSelected2D.empty()) return;

	const int colNum = static_cast<int>(m_vSelected2D.size());
	ui.tabTable->SetGeometry(static_cast<int>(m_vSelectedTP.size()), colNum + 1);
	ui.tabTable->SetColHeaderItem(0, StrConst::BSV_TableHeaderTime);
	ui.tabTable->SetItemsColNotEditable(0, 0, m_vSelectedTP);

	for (int i = 0; i < colNum; ++i)
	{
		ui.tabTable->SetColHeaderItem(i + 1, m_vSelectedStreams[i]->GetName() + "\n" + m_vSelected2D[i]->GetName() + " [" + m_vSelected2D[i]->GetUnits() + "]");
		for (int iTP = 0; iTP < static_cast<int>(m_vSelectedTP.size()); ++iTP)
			ui.tabTable->SetItemNotEditable(iTP, i + 1, m_vSelected2D[i]->GetValue(m_vSelectedTP[iTP]));
	}
}

void CBasicStreamsViewer::SetPhaseCompoundsToTable()
{
	if (m_vSelectedMD.empty()) return;

	const int colNum = static_cast<int>(m_pFlowsheet->GetCompoundsNumber());
	ui.tabTable->SetGeometry((int)m_vSelectedTP.size(), (int)m_vSelectedMD.size() * colNum + 1);
	ui.tabTable->SetColHeaderItem(0, StrConst::BSV_TableHeaderTime);
	ui.tabTable->SetItemsColNotEditable(0, 0, m_vSelectedTP);

	for (int i = 0; i < static_cast<int>(m_vSelectedMD.size()); ++i)
	{
		for (int j = 0; j < colNum; ++j)
			ui.tabTable->SetColHeaderItem(i * colNum + j + 1, m_vSelectedStreams[i]->GetName() + "\n" + m_pFlowsheet->GetCompoundName(j));
		for (int iTP = 0; iTP < static_cast<int>(m_vSelectedTP.size()); ++iTP)
			ui.tabTable->SetItemsRowNotEditable(iTP, i * colNum + 1, m_vSelectedMD[i]->GetVectorValue(m_vSelectedTP[iTP], DISTR_COMPOUNDS));
	}
}

void CBasicStreamsViewer::SetSauterDiameterToTable()
{
	if (m_vSelectedMD.empty()) return;
	std::vector<double> vSizes = m_pFlowsheet->GetDistributionsGrid()->GetNumericGridByDistr(DISTR_SIZE);
	if (vSizes.empty()) return;

	ui.tabTable->SetGeometry(static_cast<int>(m_vSelectedTP.size()), static_cast<int>(m_vSelectedMD.size()) + 1);
	ui.tabTable->SetColHeaderItem(0, StrConst::BSV_TableHeaderTime);
	ui.tabTable->SetItemsColNotEditable(0, 0, m_vSelectedTP);

	for (int i = 0; i < static_cast<int>(m_vSelectedMD.size()); ++i)
	{
		ui.tabTable->SetColHeaderItem(i + 1, m_vSelectedStreams[i]->GetName() + "\n" + StrConst::BSV_TableHeaderSauter);
		for (int iTP = 0; iTP < (int)m_vSelectedTP.size(); ++iTP)
		{
			const std::vector<double> q3 = m_vSelectedStreams[i]->GetPSD(m_vSelectedTP[iTP], PSD_q3);
			ui.tabTable->SetItemNotEditable(iTP, i + 1, GetSauterDiameter(vSizes, q3));
		}
	}
}

void CBasicStreamsViewer::SetSolidDistrsToTable()
{
	if (m_vSelectedMD.empty()) return;

	const EDistrTypes dim1 = ChosenDim(EDimType::Row);
	const EDistrTypes dim2 = ChosenDim(EDimType::Col);
	const std::string comp = ChosenCompound();

	EDistrCombination combi = EDistrCombination::Empty;
	if (dim1 == DISTR_UNDEFINED && dim2 == DISTR_UNDEFINED && !comp.empty())
		combi = EDistrCombination::Empty;
	else if (dim1 == DISTR_UNDEFINED && dim2 == DISTR_UNDEFINED && comp.empty())
		combi = EDistrCombination::Compounds;
	else if (dim1 != DISTR_UNDEFINED && dim2 != DISTR_UNDEFINED && dim1 != dim2)
		combi = EDistrCombination::TwoDimensional;
	else if (dim1 != DISTR_UNDEFINED)
		combi = EDistrCombination::OneDimensionalVertical;
	else if (dim2 != DISTR_UNDEFINED)
		combi = EDistrCombination::OneDimensionalHorizontal;

	SetSolidDistrsToTableHeaders(combi);
	SetSolidDistrsToTableData(combi);
}

void CBasicStreamsViewer::SetSolidDistrsToTableHeaders(EDistrCombination _type)
{
	switch (_type)
	{
	case EDistrCombination::Empty:
		ui.tabTable->SetGeometry(0, 0);
		break;
	case EDistrCombination::Compounds:
		ui.tabTable->SetGeometry(static_cast<int>(m_pFlowsheet->GetCompoundsNumber()), static_cast<int>(m_vSelectedMD.size()));
		for (int i = 0; i < static_cast<int>(m_vSelectedMD.size()); ++i)
			ui.tabTable->SetColHeaderItem(i, m_vSelectedStreams[i]->GetName() + "\n" + StrConst::BSV_TableHeaderMassFrac);
		for (int i = 0; i < static_cast<int>(m_pFlowsheet->GetCompoundsNumber()); ++i)
			ui.tabTable->SetRowHeaderItem(i, m_pFlowsheet->GetCompoundName(i));
		break;
	case EDistrCombination::TwoDimensional:
	{
		const EDistrTypes dimRow = ChosenDim(EDimType::Row);
		const EDistrTypes dimCol = ChosenDim(EDimType::Col);
		const int classesRow = static_cast<int>(m_pFlowsheet->GetDistributionsGrid()->GetClassesByDistr(dimRow));
		const int classesCol = static_cast<int>(m_pFlowsheet->GetDistributionsGrid()->GetClassesByDistr(dimCol));
		ui.tabTable->SetGeometry(classesRow, (classesCol + 1) * (int)m_vSelectedMD.size() - 1);
		ui.tabTable->SetRowHeaderItems(0, TableHeaders(dimRow));
		for (int i = 0; i < static_cast<int>(m_vSelectedMD.size()); ++i)
		{
			ui.tabTable->SetColHeaderItem(i * (classesCol + 1) - 1, "");
			ui.tabTable->SetColHeaderItems(i * (classesCol + 1), TableHeaders(dimCol));
		}
		break;
	}
	case EDistrCombination::OneDimensionalVertical:
	{
		const EDistrTypes dim = ChosenDim(EDimType::Row);
		const int classes = static_cast<int>(m_pFlowsheet->GetDistributionsGrid()->GetClassesByDistr(dim));
		ui.tabTable->SetGeometry(classes, static_cast<int>(m_vSelectedMD.size()));
		ui.tabTable->SetRowHeaderItems(0, TableHeaders(dim));
		for (int i = 0; i < static_cast<int>(m_vSelectedMD.size()); ++i)
			ui.tabTable->SetColHeaderItem(i, m_vSelectedStreams[i]->GetName() + "\n" + PSDSymbolicName(dim));
		break;
	}
	case EDistrCombination::OneDimensionalHorizontal:
	{
		const EDistrTypes dim = ChosenDim(EDimType::Col);
		const int classes = static_cast<int>(m_pFlowsheet->GetDistributionsGrid()->GetClassesByDistr(dim));
		ui.tabTable->SetGeometry(static_cast<int>(m_vSelectedMD.size()), classes);
		ui.tabTable->SetColHeaderItems(0, TableHeaders(dim));
		for (int i = 0; i < static_cast<int>(m_vSelectedMD.size()); ++i)
			ui.tabTable->SetRowHeaderItem(i, m_vSelectedStreams[i]->GetName() + " - " + PSDSymbolicName(dim));
		break;
	}
	}
}

void CBasicStreamsViewer::SetSolidDistrsToTableData(EDistrCombination _type)
{
	if (_type == EDistrCombination::Empty)
		return;
	if (_type == EDistrCombination::Compounds)
		for (int i = 0; i < static_cast<int>(m_vSelectedMD.size()); ++i)
			ui.tabTable->SetItemsColNotEditable(0, i, m_vSelectedMD[i]->GetVectorValue(m_dCurrentTime, DISTR_COMPOUNDS));
	else
	{
		const std::string comp = ChosenCompound();
		std::vector<unsigned> distrs = comp.empty() ? std::vector<unsigned>{} : std::vector<unsigned>(1, DISTR_COMPOUNDS);
		std::vector<unsigned> coords = comp.empty() ? std::vector<unsigned>{} : std::vector<unsigned>(1, (unsigned)m_pFlowsheet->GetCompoundIndex(comp));

		if (_type == EDistrCombination::TwoDimensional)
		{
			distrs.push_back(ChosenDim(EDimType::Row));
			distrs.push_back(ChosenDim(EDimType::Col));
			coords.push_back(0);
			const int classesRow = static_cast<int>(m_pFlowsheet->GetDistributionsGrid()->GetClassesByDistr(ChosenDim(EDimType::Row)));
			const int classesCol = static_cast<int>(m_pFlowsheet->GetDistributionsGrid()->GetClassesByDistr(ChosenDim(EDimType::Col)));
			for (int i = 0; i < static_cast<int>(m_vSelectedMD.size()); ++i)
			{
				coords.back() = 0;
				for (int j = 0; j < classesRow; ++j)
				{
					const std::vector<double> vals = m_vSelectedMD[i]->GetVectorValue(m_dCurrentTime, distrs, coords);
					ui.tabTable->SetItemsRowNotEditable(j, i * (classesCol + 1), vals);
					coords.back()++;
				}
			}
		}
		else
		{
			distrs.push_back(ChosenDim(_type == EDistrCombination::OneDimensionalVertical ? EDimType::Row : EDimType::Col));
			const std::vector<std::string> compounds = comp.empty() ? std::vector<std::string>{} : std::vector<std::string>{ comp };
			const EPSDGridType meanType = ChosenPSDGridType();
			std::vector<double> vals;
			for (int i = 0; i < static_cast<int>(m_vSelectedMD.size()); ++i)
			{
				if (distrs.back() == DISTR_SIZE)
					vals = m_vSelectedStreams[i]->GetPSD(m_dCurrentTime, ChosenPSDType(), compounds, meanType);
				else
					vals = m_vSelectedMD[i]->GetVectorValue(m_dCurrentTime, distrs, coords);

				if (_type == EDistrCombination::OneDimensionalVertical)
					ui.tabTable->SetItemsColNotEditable(0, i, vals);
				else
					ui.tabTable->SetItemsRowNotEditable(i, 0, vals);
			}
		}
	}
}

void CBasicStreamsViewer::SetMTPToPlot(int _type)
{
	if (m_vSelected2D.empty() || _type < MTP_MASS || _type > MTP_PRESSURE) return;

	QtPlot::LabelTypes labelType;
	switch (_type)
	{
	case MTP_MASS:			labelType = dynamic_cast<const CStream*>(m_vSelectedStreams.front()) ? QtPlot::LABEL_MASS_FLOW : QtPlot::LABEL_MASS;	break;
	case MTP_TEMPERATURE:	labelType = QtPlot::LABEL_TEMPERATURE;									break;
	case MTP_PRESSURE:		labelType = QtPlot::LABEL_PRESSURE;										break;
	default:				labelType = QtPlot::LABEL_NONE;											break;
	}

	for (int i = 0; i < static_cast<int>(m_vSelected2D.size()); ++i)
	{
		const QColor color = m_vSelected2D.size() == 1 ? Qt::blue : Qt::GlobalColor(Qt::red + i % (Qt::transparent - Qt::red));
		auto* curve = new QtPlot::SCurve(m_vSelectedStreams[i]->GetName(), color, QtPlot::LABEL_TIME, labelType);
		ui.tabPlot->AddCurve(curve);
		std::vector<double> times = m_vSelectedStreams[i]->GetAllTimePoints();
		std::vector<double> values;
		values.reserve(times.size());
		for (double t : times)
			values.push_back(m_vSelected2D[i]->GetValue(t));
		ui.tabPlot->AddPoints(i, times, values);
	}

	if (m_vSelected2D.size() == 1 && m_vSelectedStreams.front()->GetAllTimePoints().size() == 1)
		ui.tabPlot->SetCurveLinesVisibility(0, false);
}

void CBasicStreamsViewer::SetPhaseFractionsToPlot()
{
	if (m_vSelected2D.empty())	return;

	const unsigned dimsNum = 1;

	for (size_t i = 0; i < m_vSelectedStreams.size(); ++i)
		for (unsigned j = 0; j < dimsNum; ++j)
		{
			const std::string name = m_vSelectedStreams[i]->GetName() + " - " + m_vSelected2D[i]->GetName() + " [" + m_vSelected2D[i]->GetUnits() + "]";
			const QColor color = Qt::GlobalColor(Qt::red + (i * dimsNum + j) % (Qt::transparent - Qt::red));
			auto* curve = new QtPlot::SCurve(name, color, QtPlot::LABEL_TIME, QtPlot::LABEL_MASS_FRACTION);
			const unsigned iCurve = ui.tabPlot->AddCurve(curve);
			std::vector<double> times = m_vSelectedStreams[i]->GetAllTimePoints();
			std::vector<double> values;
			values.reserve(times.size());
			for (double t : times)
				values.push_back(m_vSelected2D[i]->GetValue(t));
			ui.tabPlot->AddPoints(iCurve, times, values);
		}

	if (m_vSelected2D.size() == 1 && m_vSelectedStreams.front()->GetAllTimePoints().size() == 1)
		for (unsigned i = 0; i < dimsNum; ++i)
			ui.tabPlot->SetCurveLinesVisibility(i, false);
}

void CBasicStreamsViewer::SetPhaseCompoundsToPlot()
{
	if (m_vSelectedMD.empty()) return;

	const size_t cmpNum = m_pFlowsheet->GetCompoundsNumber();

	for (size_t i = 0; i < m_vSelectedMD.size(); ++i)
		for (unsigned j = 0; j < cmpNum; ++j)
		{
			const std::string name = m_vSelectedStreams[i]->GetName() + " - " + m_pFlowsheet->GetCompoundName(j);
			const QColor color = Qt::GlobalColor(Qt::red + (i * cmpNum + j) % (Qt::transparent - Qt::red));
			auto* curve = new QtPlot::SCurve(name, color, QtPlot::LABEL_TIME, QtPlot::LABEL_MASS_FRACTION);
			const unsigned iCurve = ui.tabPlot->AddCurve(curve);
			ui.tabPlot->AddPoints(iCurve, m_vSelectedMD[i]->GetAllTimePoints(), m_vSelectedMD[i]->GetValues(DISTR_COMPOUNDS, j));
		}

	if (m_vSelectedMD.size() == 1 && m_vSelectedMD.front()->GetTimePointsNumber() == 1)
		for (unsigned i = 0; i < cmpNum; ++i)
			ui.tabPlot->SetCurveLinesVisibility(i, false);
}

void CBasicStreamsViewer::SetSauterDiameterToPlot()
{
	if (m_vSelectedMD.empty()) return;

	std::vector<double> vSizes = m_pFlowsheet->GetDistributionsGrid()->GetNumericGridByDistr(DISTR_SIZE);
	if (vSizes.empty()) return;

	for (unsigned i = 0; i < m_vSelectedMD.size(); ++i)
	{
		const QColor color = Qt::GlobalColor(Qt::red + i % (Qt::transparent - Qt::red));
		auto* pCurve = new QtPlot::SCurve(m_vSelectedStreams[i]->GetName(), color, QtPlot::LABEL_TIME, QtPlot::LABEL_SAUTER);
		ui.tabPlot->AddCurve(pCurve);

		std::vector<double> tp = m_vSelectedMD[i]->GetAllTimePoints();
		std::vector<double> vals(tp.size());
		for (size_t iTP = 0; iTP < tp.size(); ++iTP)
			vals[iTP] = GetSauterDiameter(vSizes, m_vSelectedStreams[i]->GetPSD(tp[iTP], PSD_q3));
		ui.tabPlot->AddPoints(i, tp, vals);
	}

	if (m_vSelectedMD.size() == 1 && m_vSelectedMD.front()->GetTimePointsNumber() == 1)
		ui.tabPlot->SetCurveLinesVisibility(0, false);
}

void CBasicStreamsViewer::SetSolidDistrsToPlot()
{
	if (m_vSelectedMD.empty()) return;

	const std::string comp = ChosenCompound();
	const std::vector<std::string> compounds = comp.empty() ? std::vector<std::string>{} : std::vector<std::string>{ comp };
	const EDistrTypes dim = ChosenDim(EDimType::Row);
	const EPSDGridType meanType = ChosenPSDGridType();
	if (dim == DISTR_UNDEFINED) return;

	const std::vector<double> vMedians = dim != DISTR_SIZE ? m_pFlowsheet->GetDistributionsGrid()->GetClassMeansByDistr(dim) : m_pFlowsheet->GetDistributionsGrid()->GetPSDMeans(meanType);

	ui.tabPlot->SetManualLabelsNames(DistrSymbolicName(dim), QString::fromStdString(PSDSymbolicName(dim)));
	for (unsigned i = 0; i < m_vSelectedMD.size(); ++i)
	{
		const QColor color = Qt::GlobalColor(Qt::red + i % (Qt::transparent - Qt::red));
		auto* pCurve = new QtPlot::SCurve(m_vSelectedStreams[i]->GetName(), color, QtPlot::LABEL_MANUAL, QtPlot::LABEL_MANUAL);
		ui.tabPlot->AddCurve(pCurve);

		std::vector<double> vals;
		if(dim == DISTR_SIZE)
			vals = m_vSelectedStreams[i]->GetPSD(m_dCurrentTime, ChosenPSDType(), compounds, meanType);
		else
			if (compounds.empty())	// for all compounds
				vals = m_vSelectedMD[i]->GetVectorValue(m_dCurrentTime, dim);
			else					// for only one compound
				vals = m_vSelectedMD[i]->GetVectorValue(m_dCurrentTime, DISTR_COMPOUNDS, static_cast<unsigned>(m_pFlowsheet->GetCompoundIndex(comp)), dim);

		ui.tabPlot->AddPoints(i, vMedians, vals);
	}
}

void CBasicStreamsViewer::RestorePosition(QComboBox* _combo, int _position, int _defaultPosition /*= 0*/)
{
	if (_position != -1 && _position < _combo->count())	_combo->setCurrentIndex(_position);
	else if (_defaultPosition < _combo->count())		_combo->setCurrentIndex(_defaultPosition);
	else if (_combo->count())							_combo->setCurrentIndex(0);
	else												_combo->setCurrentIndex(-1);
}

void CBasicStreamsViewer::ExportToFile()
{
	const QString fileName = QFileDialog::getSaveFileName(this, "Save file", "", "Text Files (*.txt)");
	QFile file(fileName);
	if (!file.open(QFile::WriteOnly | QFile::Truncate)) return;
	QTextStream fileStream(&file);
	if(ChosenProperty() != EPropertyType::SolidDistr)
	{
		for (auto& s : ui.tabTable->GetColHeaderItems(0))
			fileStream << s.replace("\n", " ") << "; ";
		const std::vector<QString> rowHeaders = ui.tabTable->GetRowHeaderItems(0);
		for (int i = 0; i < ui.tabTable->rowCount(); ++i)
		{
			fileStream << Qt::endl << rowHeaders[i];
			for (const auto& s : ui.tabTable->GetItemsRow(i, 0))
				fileStream << s << "; ";
		}
	}
	else
	{
		const std::vector<std::string> compounds = ChosenCompound().empty() ? std::vector<std::string>{} : std::vector<std::string>{ ChosenCompound() };
		const EPSDGridType meanType = ChosenPSDGridType();
		const EDistrTypes distr = ChosenDim(EDimType::Row);
		const EPSDTypes psdType = ChosenPSDType();

		fileStream << ui.tabTable->GetColHeaderItem(0).replace("\n", " ");
		for (auto& s : ui.tabTable->GetRowHeaderItems(0))
			fileStream << "; " << s.replace("\n", " ");
		for (double t : m_vSelectedMD.front()->GetAllTimePoints())
		{
			fileStream << Qt::endl << t << "; ";
			if (distr == DISTR_SIZE)
				for (double v : m_vSelectedStreams.front()->GetPSD(t, psdType, compounds, meanType))
					fileStream << v << "; ";
			else
				for (double v : m_vSelectedMD.front()->GetVectorValue(t, distr))
					fileStream << v << "; ";
		}
	}

	file.close();
}

void CBasicStreamsViewer::contextMenuEvent(QContextMenuEvent* _event)
{
	const bool bAllowExport = ChosenProperty() != EPropertyType::SolidDistr ||
		                ChosenProperty() == EPropertyType::SolidDistr && m_vSelectedMD.size() == 1 && ChosenDim(EDimType::Row) != DISTR_UNDEFINED && ChosenDim(EDimType::Col) == DISTR_UNDEFINED;
	if (!bAllowExport) return;

	QMenu menu(this);
	QAction* exportToFile  = menu.addAction("Export to file");
	connect(exportToFile, &QAction::triggered, this, &CBasicStreamsViewer::ExportToFile);
	menu.exec(_event->globalPos());
}

void CBasicStreamsViewer::PropertyChanged()
{
	GetSelectedDistributions();
	SetupTimeSlider();
	UpdateWidgetsVisible();
	UpdateTimeLabel();
	UpdateTabView();
}

void CBasicStreamsViewer::SliderMoved()
{
	if (m_vSelectedStreams.empty() || m_vSelectedTP.empty()) return;

	const double dIntervalLen = (m_vSelectedTP.back() - m_vSelectedTP.front()) / ui.horizontalSlider->maximum();
	m_dCurrentTime = m_vSelectedTP.front() + dIntervalLen * ui.horizontalSlider->value();
	UpdateTimeLabel();
	UpdateTabView();
}

void CBasicStreamsViewer::TimeEdited()
{
	if (m_vSelectedStreams.empty()) return;

	m_dCurrentTime = ui.lineEditTime->text().toDouble();
	UpdateSliderPosition();
	UpdateTabView();
}

void CBasicStreamsViewer::PrevTimeClicked()
{
	if (m_vSelectedStreams.empty()) return;

	// find smaller
	const auto it = std::find_if(m_vSelectedTP.crbegin(), m_vSelectedTP.crend(), [&](double t) { return t < m_dCurrentTime; });
	if (it == m_vSelectedTP.rend()) return;
	// set it
	m_dCurrentTime = *it;
	// update all
	UpdateTimeLabel();
	UpdateSliderPosition();
	UpdateTabView();
}

void CBasicStreamsViewer::NextTimeClicked()
{
	if (m_vSelectedStreams.empty()) return;

	// find greater
	const auto it = std::find_if(m_vSelectedTP.cbegin(), m_vSelectedTP.cend(), [&](double t) { return t > m_dCurrentTime; });
	if (it == m_vSelectedTP.end()) return;
	// set it
	m_dCurrentTime = *it;
	// update all
	UpdateTimeLabel();
	UpdateSliderPosition();
	UpdateTabView();
}

void CBasicStreamsViewer::ComboRowsChanged()
{
	UpdateWidgetsVisible();
	UpdateTabView();
}

void CBasicStreamsViewer::ComboColsChanged()
{
	UpdateWidgetsVisible();
	UpdateTabView();
}

void CBasicStreamsViewer::ComboCompoundsChanged()
{
	UpdateTabView();
}

void CBasicStreamsViewer::ComboPSDTypeChanged()
{
	UpdateTabView();
}

void CBasicStreamsViewer::ComboPSDGridTypeChanged()
{
	UpdateTabView();
}

void CBasicStreamsViewer::TabChanged()
{
	UpdateWidgetsVisible();
	UpdateTabView();
}

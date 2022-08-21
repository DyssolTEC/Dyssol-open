/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "SolidDistributionsEditor.h"
#include "ParamsItem.h"
#include "MaterialsDatabase.h"
#include "Flowsheet.h"
#include "BaseStream.h"
#include "MDMatrix.h"
#include "DyssolUtilities.h"
#include "ContainerFunctions.h"
#include "DyssolStringConstants.h"

CSolidDistributionsEditor::CSolidDistributionsEditor(QWidget* parent) : QWidget(parent),
	m_pFlowsheet(nullptr),
	m_pGrid(nullptr),
	m_pDistribution(nullptr),
	m_pStream(nullptr),
	m_pDistributor(new CDistrFunctionDialog(this)),
	m_iCurrTime(-1),
	m_bExternalViewState(false)
{
	ui.setupUi(this);

	connect(ui.comboBoxCompound,	QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CSolidDistributionsEditor::CompoundChanged);
	connect(ui.comboBoxPSDType,		QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CSolidDistributionsEditor::ComboPSDTypeChanged);
	connect(ui.comboBoxPSDGridType,	QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CSolidDistributionsEditor::ComboPSDGridTypeChanged);

	connect(ui.pushButtonApply,					&QPushButton::clicked,	this, &CSolidDistributionsEditor::ApplyPressed);
	connect(ui.pushButtonFunctional,			&QPushButton::clicked,	this, &CSolidDistributionsEditor::FunctionalCalled);
	connect(ui.tableWidgetCompoundsFractions,	&CQtTable::itemChanged, this, &CSolidDistributionsEditor::CompoundFractionChanged);
	connect(ui.tableWidgetCompoundsFractions,	&CQtTable::DataPasted,  this, &CSolidDistributionsEditor::CompoundFractionsPasted);
	connect(ui.tableWidget,						&CQtTable::itemChanged, this, &CSolidDistributionsEditor::TableItemChanged);
	connect(ui.tableWidget,						&CQtTable::DataPasted,  this, &CSolidDistributionsEditor::TableItemChanged);

	ui.listWidget->setVisible(false);
	ui.frameAllElements->setEnabled(false);
	ui.pushButtonApply->setEnabled(false);
	ui.tableWidgetCompoundsFractions->verticalHeader()->setStretchLastSection(true);

	SetupComboBoxPSDType();
	SetupComboBoxPSDGridType();
	UpdatePSDComboVisible();
}

CSolidDistributionsEditor::~CSolidDistributionsEditor()
{
	m_pFlowsheet = nullptr;
	m_pDistribution = nullptr;
	m_pStream = nullptr;
}

void CSolidDistributionsEditor::SetFlowsheet(CFlowsheet* _pFlowsheet, CMaterialsDatabase* _materialsDB)
{
	m_materialsDB = _materialsDB;
	m_pFlowsheet = _pFlowsheet;
}

void CSolidDistributionsEditor::SetDistribution(CMDMatrix* _pDistribution, CBaseStream* _pStream)
{
	m_pDistribution = _pDistribution;
	m_pStream = _pStream;
	m_pGrid = &m_pStream->GetGrid();
	UpdateWholeView();
}

void CSolidDistributionsEditor::SetTimePoint(size_t _index)
{
	if (!m_pDistribution) return;
	m_iCurrTime = _index;
	UpdateWholeView();
}

void CSolidDistributionsEditor::GetViewState(std::vector<int>& _vLastCombos, std::vector<int>& _vLastSliders) const
{
	if(m_bExternalViewState)
	{
		_vLastCombos = m_vLastCombos;
		_vLastSliders = m_vLastSliders;
	}
	else
	{
		_vLastCombos.clear();
		_vLastCombos.push_back(ui.comboBoxCompound->currentIndex() == -1 ? 0 : ui.comboBoxCompound->currentIndex());
		for (auto combo : m_vCombos)
			_vLastCombos.push_back(combo->currentIndex() == -1 ? 0 : combo->currentIndex());
		_vLastCombos.push_back(ui.comboBoxPSDType->currentIndex() == -1 ? 0 : ui.comboBoxPSDType->currentIndex());
		_vLastSliders.clear();
		for (auto slider : m_vSliders)
			_vLastSliders.push_back(slider->sliderPosition() == -1 ? 0 : slider->sliderPosition());
	}
}

void CSolidDistributionsEditor::SetViewState(const std::vector<int>& _vLastCombos, const std::vector<int>& _vLastSliders)
{
	m_vLastCombos = _vLastCombos;
	m_vLastSliders = _vLastSliders;
	m_bExternalViewState = true;
}

void CSolidDistributionsEditor::SetEditable(bool _bEditable) const
{
	ui.tableWidgetCompoundsFractions->SetEditable(_bEditable);
	ui.tableWidget->SetEditable(_bEditable);
	ui.pushButtonFunctional->setVisible(_bEditable);
	ui.pushButtonApply->setVisible(_bEditable);
}

void CSolidDistributionsEditor::setVisible(bool _bVisible)
{
	if (_bVisible && !isVisible())
		UpdateWholeView();
	QWidget::setVisible(_bVisible);
}

void CSolidDistributionsEditor::UpdateWholeView()
{
	if (!m_pDistribution) return;
	if (m_iCurrTime >= m_pDistribution->GetAllTimePoints().size())
		m_iCurrTime = -1;
	ui.frameAllElements->setEnabled(m_iCurrTime != static_cast<size_t>(-1));

	UpdateDistributionsList();
	UpdateTimeLabel();
	UpdateFractionsTable();
	SaveViewState();
	UpdateCombos();
	LoadViewState();
	UpdateDistributionTable();
	UpdatePSDComboVisible();
}

void CSolidDistributionsEditor::UpdateCombos()
{
	if (!m_pGrid) return;

	// hide and clear additional dimensions
	ui.listWidget->setVisible(false);
	ui.listWidget->clear();

	// clear combos
	for (int i = 0; i < static_cast<int>(m_vCombos.size()) - 2; ++i)
		delete m_vCombos[i];
	m_vCombos.clear();

	// clear sliders of additional dimensions
	for (auto& slider : m_vSliders)
		delete slider;
	m_vSliders.clear();

	// check if DISTR_COMPOUNDS exists
	if (!m_pGrid->HasDimension(DISTR_COMPOUNDS))
		return;

	// add additional dimensions. magic 3 is for 'Rows', 'Columns' and 'Compounds' combos
	for (int i = 0; i < static_cast<int>(m_pGrid->GetDimensionsNumber()) - 3; ++i)
		AddAdditionalDim();

	// setup 'Columns' combo
	QSignalBlocker blocker1(ui.comboBoxColumns);
	SetupComboBoxDimensions(ui.comboBoxColumns);

	// setup 'Rows' combo
	QSignalBlocker blocker2(ui.comboBoxRows);
	SetupComboBoxDimensions(ui.comboBoxRows);

	// setup 'Compounds' combo
	QSignalBlocker blocker3(ui.comboBoxRows);
	ui.comboBoxCompound->clear();
	ui.comboBoxCompound->addItem(StrConst::SDE_CompoundsMixture, "");
	for (const auto& key : m_pFlowsheet->GetCompounds())
		ui.comboBoxCompound->addItem(QString::fromStdString(m_materialsDB->GetCompound(key) ? m_materialsDB->GetCompound(key)->GetName() : ""), QString::fromStdString(key));
}

void CSolidDistributionsEditor::UpdateTimeLabel() const
{
	ui.labelTime->clear();
	if (m_pDistribution && m_iCurrTime < m_pDistribution->GetAllTimePoints().size())
		ui.labelTime->setText(QString::number(m_pDistribution->GetAllTimePoints()[m_iCurrTime]));
}

void CSolidDistributionsEditor::UpdateFractionsTable() const
{
	if (!m_pFlowsheet || !m_pDistribution || m_iCurrTime >= m_pDistribution->GetAllTimePoints().size()) return;

	QSignalBlocker blocker(ui.tableWidgetCompoundsFractions);

	// get compounds number
	const size_t compoundsNum = m_pFlowsheet->GetCompoundsNumber();

	// set table columns and rows number
	ui.tableWidgetCompoundsFractions->SetGeometry(1, static_cast<int>(compoundsNum));

	// set headers
	ui.tableWidgetCompoundsFractions->SetColHeaderItems(0, m_materialsDB->GetCompoundsNames(m_pFlowsheet->GetCompounds()));
	ui.tableWidgetCompoundsFractions->SetRowHeaderItem(0, StrConst::BSV_TableHeaderMassFrac);

	// set data
	ui.tableWidgetCompoundsFractions->SetItemsRowEditable(0, 0, m_pDistribution->GetVectorValue(static_cast<unsigned>(m_iCurrTime), DISTR_COMPOUNDS));

	EnableMainTable();

	// check normalization
	CheckCompoundNormalization();
}

void CSolidDistributionsEditor::UpdateDistributionsList() const
{
	if (!m_pDistribution) return;

	ui.tableWidgetDistributions->setColumnCount(1);
	ui.tableWidgetDistributions->setRowCount(static_cast<int>(m_pDistribution->GetDimensionsNumber()));
	for (unsigned i = 0; i < (unsigned)m_pDistribution->GetDimensionsNumber(); ++i)
	{
		const int iType = GetDistributionTypeIndex(EDistrTypes(m_pDistribution->GetDimensionTypeByIndex(i)));
		const auto name = iType != -1 ? std::vector<QString>(DISTR_NAMES)[iType] : "";
		ui.tableWidgetDistributions->setItem(static_cast<int>(i), 0, new QTableWidgetItem(name));
	}
}

void CSolidDistributionsEditor::UpdateDistributionTable() const
{
	// clear table
	ui.tableWidget->setRowCount(0);
	ui.tableWidget->setColumnCount(0);
	if (!m_pDistribution || m_iCurrTime >= m_pDistribution->GetAllTimePoints().size())
		return;

	const double dTime = m_pDistribution->GetAllTimePoints()[m_iCurrTime];	// get time point
	std::vector<EDistrTypes> vDistrTypes = ChosenTypes();					// get distributions types
	const std::vector<unsigned> vCoord = ChosenCoords();					// get distributions coords

	// get data
	std::vector<std::vector<double>> vvData;
	if (vDistrTypes.size() == 1 && vDistrTypes.front() == DISTR_SIZE)		// PSD for total mixture = all compounds
		vvData.push_back(m_pStream->GetPSD(dTime, ChosenPSDType(), ChosenPSDGridType()));
	else if (vDistrTypes.size() == 2 && vDistrTypes.front() == DISTR_COMPOUNDS && vDistrTypes.back() == DISTR_SIZE)	// PSD for one specific compound
		vvData.push_back(m_pStream->GetPSD(dTime, ChosenPSDType(), ChosenCompound(), ChosenPSDGridType()));
	else // not a PSD
		vvData = Normalize(m_pDistribution->GetMatrixValue(dTime, E2I(vDistrTypes), vCoord));

	// switch button for functional distribution on/off
	ui.pushButtonFunctional->setEnabled(vvData.size() == 1);

	if (vvData.empty())	// nothing to populate with
		return;

	SetDistributionTableHeaders();

	// fill the table with data
	const EDistrTypes rowType = GetTypeFromCombo(m_vCombos.size() - 1);
		for (int i = 0; i < static_cast<int>(vvData.size()); ++i)
			if (rowType != DISTR_UNDEFINED)
				ui.tableWidget->SetItemsColEditable(0, i, vvData[i]);
			else
				ui.tableWidget->SetItemsRowEditable(i, 0, vvData[i]);

	CheckDistrNormalization();
}

void CSolidDistributionsEditor::AddAdditionalDim()
{
	ui.listWidget->setVisible(true);
	auto* pItem = new QListWidgetItem();
	auto* pSlider = new QSlider(Qt::Horizontal);
	connect(pSlider, &QSlider::valueChanged, this, [&] { DimsSliderChanged(); });
	auto* pCombo = new QComboBox();
	SetupComboBoxDimensions(pCombo);
	CParamsItem* pWidget = new CParamsItem(m_pGrid, pCombo, pSlider);
	ui.listWidget->addItem(pItem);
	pItem->setSizeHint(pWidget->sizeHint());
	ui.listWidget->setItemWidget(pItem, pWidget);
	pWidget->UpdateWholeView();
	m_vSliders.push_back(pSlider);

	const int nRowH = ui.listWidget->sizeHintForRow(0);
	const QMargins m = ui.listWidget->contentsMargins();
	if (ui.listWidget->count() == 1)
	{
		ui.listWidget->setMinimumHeight(nRowH + m.top() + m.bottom());
		ui.listWidget->setMaximumHeight(nRowH + m.top() + m.bottom());
	}
	else
	{
		ui.listWidget->setMinimumHeight(nRowH * 2 + m.top() + m.bottom());
		ui.listWidget->setMaximumHeight(QWIDGETSIZE_MAX);
	}
}

void CSolidDistributionsEditor::SetupComboBoxDimensions(QComboBox* _pCombo)
{
	_pCombo->clear();
	// add all distributions except DISTR_COMPOUNDS to combo
	_pCombo->addItem(StrConst::SDE_TotalMixture, DISTR_UNDEFINED);
	for (auto& t : m_pGrid->GetDimensionsTypes())
		if (t != DISTR_COMPOUNDS)
			_pCombo->addItem(std::vector<QString>(DISTR_NAMES)[GetDistributionTypeIndex(t)], t);
	// connect combobox to slot
	connect(_pCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&] { DimsComboChanged(); });
	// add to list
	m_vCombos.push_back(_pCombo);
}

void CSolidDistributionsEditor::SetupComboBoxPSDType() const
{
	QSignalBlocker blocker(ui.comboBoxPSDType);
	ui.comboBoxPSDType->addItem(StrConst::BSV_TableHeaderq3, PSD_q3);
	ui.comboBoxPSDType->addItem(StrConst::BSV_TableHeaderQ3, PSD_Q3);
	ui.comboBoxPSDType->addItem(StrConst::BSV_TableHeaderq2, PSD_q2);
	ui.comboBoxPSDType->addItem(StrConst::BSV_TableHeaderQ2, PSD_Q2);
	ui.comboBoxPSDType->addItem(StrConst::BSV_TableHeaderq0, PSD_q0);
	ui.comboBoxPSDType->addItem(StrConst::BSV_TableHeaderQ0, PSD_Q0);
	ui.comboBoxPSDType->addItem(StrConst::BSV_TableHeaderMassFrac, PSD_MassFrac);
	ui.comboBoxPSDType->addItem(StrConst::BSV_TableHeaderNumber, PSD_Number);
}

void CSolidDistributionsEditor::SetupComboBoxPSDGridType() const
{
	QSignalBlocker blocker(ui.comboBoxPSDGridType);
	ui.comboBoxPSDGridType->addItem(StrConst::BSV_HeaderDiameter, E2I(EPSDGridType::DIAMETER));
	ui.comboBoxPSDGridType->addItem(StrConst::BSV_HeaderVolume, E2I(EPSDGridType::VOLUME));
}

void CSolidDistributionsEditor::UpdatePSDComboVisible() const
{
	std::vector<EDistrTypes> vTypes = ChosenTypes();
	const bool bVisible = vTypes.size() == 1 && vTypes.front() == DISTR_SIZE || vTypes.size() == 2 && vTypes.front() == DISTR_COMPOUNDS && vTypes.back() == DISTR_SIZE;
	ui.comboBoxPSDType->setVisible(bVisible);
	ui.labelPSDType->setVisible(bVisible);
	ui.comboBoxPSDGridType->setVisible(bVisible);
	ui.labelPSDGridType->setVisible(bVisible);
}

std::vector<EDistrTypes> CSolidDistributionsEditor::ChosenTypes() const
{
	if (m_vCombos.empty()) return {};

	std::vector<EDistrTypes> res;
	if (!ChosenCompound().empty())
		res.push_back(DISTR_COMPOUNDS);   // add compounds distribution
	const EDistrTypes distr1 = GetTypeFromCombo(m_vCombos.size() - 1);
	const EDistrTypes distr2 = GetTypeFromCombo(m_vCombos.size() - 2);
	for (size_t i = 0; i < m_vCombos.size(); ++i)
	{
		// get type for current combo
		const EDistrTypes distr = GetTypeFromCombo(i);
		// skip total mixture               distr1 and distr2 should be at the end, so skip them in the beginning        wasn't added yet
		if (distr != DISTR_UNDEFINED && (i >= m_vCombos.size() - 2 || distr != distr1 && distr != distr2) && !VectorContains(res, distr))
			res.push_back(distr);
	}
	return res;
}

std::vector<unsigned> CSolidDistributionsEditor::ChosenCoords() const
{
	std::vector<unsigned> res;
	if (!ChosenCompound().empty())
		res.push_back(ui.comboBoxCompound->currentIndex() - 1);   // for compounds distribution
	const EDistrTypes distr1 = GetTypeFromCombo(m_vCombos.size() - 1);
	const EDistrTypes distr2 = GetTypeFromCombo(m_vCombos.size() - 2);
	for (size_t i = 0; i < m_vSliders.size(); ++i)
	{
		const EDistrTypes distr = GetTypeFromCombo(i);
		if (distr != DISTR_UNDEFINED && distr != distr1 && distr != distr2)
			res.push_back(m_vSliders[i]->sliderPosition());
	}
	return res;
}

EPSDTypes CSolidDistributionsEditor::ChosenPSDType() const
{
	return static_cast<EPSDTypes>(ui.comboBoxPSDType->currentData().toUInt());
}

EPSDGridType CSolidDistributionsEditor::ChosenPSDGridType() const
{
	return static_cast<EPSDGridType>(ui.comboBoxPSDGridType->currentData().toUInt());
}

std::string CSolidDistributionsEditor::ChosenCompound() const
{
	return ui.comboBoxCompound->currentData().toString().toStdString();
}

void CSolidDistributionsEditor::EnableMainTable() const
{
	if (!m_pDistribution || m_iCurrTime >= m_pDistribution->GetAllTimePoints().size())
		ui.frameDistributions->setEnabled(false);
	const double sum = VectorSum(m_pDistribution->GetVectorValue(static_cast<unsigned>(m_iCurrTime), DISTR_COMPOUNDS));
	ui.frameDistributions->setEnabled(sum > 0);
}

void CSolidDistributionsEditor::CheckCompoundNormalization() const
{
	if (!m_pDistribution || m_iCurrTime >= m_pDistribution->GetAllTimePoints().size()) return;
	const double sum = VectorSum(m_pDistribution->GetVectorValue(static_cast<unsigned>(m_iCurrTime), DISTR_COMPOUNDS));
	ui.tableWidgetCompoundsFractions->SetRowBackgroundColor(0, sum == 1 ? Qt::white : Qt::lightGray);
}

void CSolidDistributionsEditor::SetDistributionTableHeaders() const
{
	// get distribution names
	const EDistrTypes distr1 = GetTypeFromCombo(m_vCombos.size() - 2);
	const EDistrTypes distr2 = GetTypeFromCombo(m_vCombos.size() - 1);
	const std::vector<EDistrTypes> distrs = ChosenTypes();
	std::vector<std::string> vHeaders = GetHeaders(distr2);
	std::vector<std::string> hHeaders = distr1 != distr2 ? GetHeaders(distr1) : std::vector<std::string>{};

	// get distribution name for PSD distribution
	std::string header = StrConst::BSV_TableHeaderMassFrac;
	if ((hHeaders.empty() || vHeaders.empty()) &&												 // 1D distribution
	    (distrs.size() == 1 && distrs.front() == DISTR_SIZE ||                                   // PSD for total mixture
		distrs.size() == 2 && distrs.front() == DISTR_COMPOUNDS && distrs.back() == DISTR_SIZE)) // PSD for specific compound
	{
		switch (ChosenPSDType())
		{
		case PSD_q3:		header = StrConst::BSV_TableHeaderq3;		break;
		case PSD_Q3:		header = StrConst::BSV_TableHeaderQ3;		break;
		case PSD_q2:		header = StrConst::BSV_TableHeaderq2;		break;
		case PSD_Q2:		header = StrConst::BSV_TableHeaderQ2;		break;
		case PSD_q0:		header = StrConst::BSV_TableHeaderq0;		break;
		case PSD_Q0:		header = StrConst::BSV_TableHeaderQ0;		break;
		case PSD_MassFrac:	header = StrConst::BSV_TableHeaderMassFrac;	break;
		case PSD_Number:	header = StrConst::BSV_TableHeaderNumber;	break;
		}
		switch (ChosenPSDType())
		{
		case PSD_MassFrac: case PSD_Number: case PSD_Q0: case PSD_Q2: case PSD_Q3:
			header += std::string{ " " } + StrConst::FUN_EmptyUnits;
			break;
		case PSD_q3: case PSD_q0: case PSD_q2:
			switch (ChosenPSDGridType())
			{
			case EPSDGridType::DIAMETER:	header += std::string{ " " } +StrConst::FUN_DiameterUnitsInv;	break;
			case EPSDGridType::VOLUME:		header += std::string{ " " } +StrConst::FUN_VolumeUnitsInv;		break;
			}
			break;
		}
	}

	if (hHeaders.empty()) hHeaders.push_back(header);
	if (vHeaders.empty()) vHeaders.push_back(header);

	QSignalBlocker blocker(ui.tableWidget);

	// setup table dimensions
	ui.tableWidget->setColumnCount(static_cast<int>(hHeaders.size()));
	ui.tableWidget->setRowCount(static_cast<int>(vHeaders.size()));

	// set header
	ui.tableWidget->SetColHeaderItems(0, hHeaders);
	ui.tableWidget->SetRowHeaderItems(0, vHeaders);
}

std::vector<std::string> CSolidDistributionsEditor::GetHeaders(EDistrTypes _distr) const
{
	if (!m_pGrid->HasDimension(_distr)) return {};
	const EPSDGridType psdGridType = ChosenPSDGridType();
	const std::string sUnits = _distr != DISTR_SIZE ? "" : psdGridType == EPSDGridType::DIAMETER ? StrConst::FUN_DiameterUnits : StrConst::FUN_VolumeUnits;

	switch (m_pGrid->GetGridDimension(_distr)->GridType())
	{
	case EGridEntry::GRID_NUMERIC:
	{
		std::vector<std::string> headers;
		const std::vector<double> grid = _distr != DISTR_SIZE ? m_pGrid->GetNumericGrid(_distr) : m_pGrid->GetPSDGrid(psdGridType);
		for (int i = 0; i < static_cast<int>(grid.size()) - 1; ++i)
			headers.push_back(StringFunctions::Double2String(grid[i]) + " : " + StringFunctions::Double2String(grid[i + 1]) + " " + sUnits);
		return headers;
	}
	case EGridEntry::GRID_SYMBOLIC:
		return m_pGrid->GetSymbolicGrid(_distr);
	}
	return {};
}

void CSolidDistributionsEditor::CheckDistrNormalization() const
{
	if (!m_pDistribution || m_iCurrTime >= m_pDistribution->GetAllTimePoints().size()) return;
	const bool bNorm = m_pDistribution->IsNormalized(m_pDistribution->GetAllTimePoints()[m_iCurrTime]);
	ui.tableWidget->SetBackgroundColor(bNorm ? Qt::white : Qt::lightGray);
}

EDistrTypes CSolidDistributionsEditor::GetSelectedDim() const
{
	const EDistrTypes distr1 = GetTypeFromCombo(m_vCombos.size() - 1);
	const EDistrTypes distr2 = GetTypeFromCombo(m_vCombos.size() - 2);
	if (distr1 == distr2 && distr1 == DISTR_UNDEFINED)
		return DISTR_UNDEFINED;
	if (distr1 == DISTR_UNDEFINED)
		return distr2;
	else
		return distr1;
}

EDistrTypes CSolidDistributionsEditor::GetTypeFromCombo(size_t _index) const
{
	if (_index >= m_vCombos.size()) return DISTR_UNDEFINED;
	return static_cast<EDistrTypes>(m_vCombos[_index]->currentData().toUInt());
}

void CSolidDistributionsEditor::SaveViewState()
{
	if (m_bExternalViewState) return;
	GetViewState(m_vLastCombos, m_vLastSliders);
}

void CSolidDistributionsEditor::LoadViewState()
{
	m_bExternalViewState = false;
	if (m_vLastCombos.empty())	return;

	if (m_vLastCombos.front() < ui.comboBoxCompound->count())
	{
		QSignalBlocker blocker(ui.comboBoxCompound);
		ui.comboBoxCompound->setCurrentIndex(m_vLastCombos.front());
	}
	if (m_vLastCombos.size() == m_vCombos.size() + 2)
		for (size_t i = 0; i < m_vCombos.size(); ++i)
			if (m_vLastCombos[i + 1] < m_vCombos[i]->count())
			{
				QSignalBlocker blocker(m_vCombos[i]);
				m_vCombos[i]->setCurrentIndex(m_vLastCombos[i + 1]);
			}
	if (m_vLastCombos.back() < ui.comboBoxPSDType->count())
	{
		QSignalBlocker blocker(ui.comboBoxPSDType);
		ui.comboBoxPSDType->setCurrentIndex(m_vLastCombos.back());
	}
	if (m_vLastSliders.size() == m_vSliders.size())
		for (unsigned i = 0; i < m_vLastSliders.size(); ++i)
			if (m_vLastSliders[i] < m_vSliders[i]->maximum())
			{
				QSignalBlocker blocker(m_vSliders[i]);
				m_vSliders[i]->setSliderPosition(m_vLastSliders[i]);
			}
}

void CSolidDistributionsEditor::TableItemChanged() const
{
	ui.pushButtonApply->setEnabled(true);
}

void CSolidDistributionsEditor::DimsComboChanged() const
{
	UpdatePSDComboVisible();
	UpdateDistributionTable();
}

void CSolidDistributionsEditor::DimsSliderChanged() const
{
	UpdatePSDComboVisible();
	UpdateDistributionTable();
}

void CSolidDistributionsEditor::CompoundChanged() const
{
	UpdateDistributionTable();
}

void CSolidDistributionsEditor::CompoundFractionChanged(QTableWidgetItem* _pItem)
{
	const unsigned coord = _pItem->column();
	double dVal = _pItem->text().toDouble();
	if (dVal < 0) dVal = 0;
	m_pDistribution->SetValue(static_cast<unsigned>(m_iCurrTime), DISTR_COMPOUNDS, coord, dVal);

	UpdateWholeView();

	emit DataChanged();
}

void CSolidDistributionsEditor::CompoundFractionsPasted()
{
	for (int i = 0; i < ui.tableWidgetCompoundsFractions->columnCount(); ++i)
	{
		double dVal = ui.tableWidgetCompoundsFractions->item(0, i)->text().toDouble();
		if (dVal < 0) dVal = 0;
		m_pDistribution->SetValue(static_cast<unsigned>(m_iCurrTime), DISTR_COMPOUNDS, static_cast<unsigned>(i), dVal);
	}

	UpdateWholeView();
	emit DataChanged();
}

void CSolidDistributionsEditor::ComboPSDTypeChanged() const
{
	UpdateDistributionTable();
	SetDistributionTableHeaders();
}

void CSolidDistributionsEditor::ComboPSDGridTypeChanged() const
{
	UpdateDistributionTable();
	SetDistributionTableHeaders();
}

void CSolidDistributionsEditor::FunctionalCalled() const
{
	const bool flag1 = ui.tableWidget->rowCount() == 1 && ui.tableWidget->columnCount() > 1;
	const bool flag2 = ui.tableWidget->rowCount() > 1 && ui.tableWidget->columnCount() == 1;
	if (!flag1 && !flag2)
		return;

	const EDistrTypes dim = GetSelectedDim();
	if (dim == DISTR_UNDEFINED) return;

	m_pDistributor->SetDistributionsGrid(m_pGrid, dim, ChosenPSDGridType());
	if (m_pDistributor->exec() == QDialog::Rejected)
		return;

	const std::vector<double> vRes = m_pDistributor->GetDistribution();

	QSignalBlocker blocker(ui.tableWidget);
	if (flag1)
		ui.tableWidget->SetItemsRowEditable(0, 0, vRes);
	else
		ui.tableWidget->SetItemsColEditable(0, 0, vRes);

	TableItemChanged(); // to enable Apply button
}

void CSolidDistributionsEditor::ApplyPressed()
{
	if (!m_pDistribution || m_iCurrTime >= m_pDistribution->GetAllTimePoints().size()) return;

	// check for empty combos 'Columns' and 'Rows'
	const EDistrTypes row = GetTypeFromCombo(m_vCombos.size() - 1);
	const EDistrTypes col = GetTypeFromCombo(m_vCombos.size() - 2);
	if (row == DISTR_UNDEFINED && col == DISTR_UNDEFINED)
		return;

	QApplication::setOverrideCursor(Qt::WaitCursor);

	// get data
	std::vector<std::vector<double>> vvData;
	if (row != DISTR_UNDEFINED)
	{
		vvData.resize(ui.tableWidget->columnCount());
		for (int i = 0; i < ui.tableWidget->columnCount(); ++i)
			for (int j = 0; j < ui.tableWidget->rowCount(); ++j)
				vvData[i].push_back(ui.tableWidget->item(j, i)->text().toDouble());
	}
	else
	{
		vvData.resize(ui.tableWidget->rowCount());
		for (int i = 0; i < ui.tableWidget->columnCount(); ++i)
			for (int j = 0; j < ui.tableWidget->rowCount(); ++j)
				vvData[j].push_back(ui.tableWidget->item(j, i)->text().toDouble());
	}

	const double dTime = m_pDistribution->GetAllTimePoints()[m_iCurrTime];	// get time point
	std::vector<EDistrTypes> vTypes = ChosenTypes();						// get distributions types
	const std::vector<unsigned> vCoord = ChosenCoords();					// get distributions coords

	if (vTypes.size() == 1 && vTypes.front() == DISTR_SIZE)												// PSD for total mixture = all compounds
		m_pStream->SetPSD(dTime, ChosenPSDType(), vvData.front(), ChosenPSDGridType());
	else if (vTypes.size() == 2 && vTypes.front() == DISTR_COMPOUNDS && vTypes.back() == DISTR_SIZE)	// PSD for specific compound
		m_pStream->SetPSD(dTime, ChosenPSDType(), ChosenCompound(), vvData.front(), ChosenPSDGridType());
	else																								// another distribution, not a PSD
		m_pDistribution->SetMatrixValue(m_pDistribution->GetAllTimePoints().at(m_iCurrTime), E2I(vTypes), vCoord, Normalize(static_cast<const std::vector<std::vector<double>>>(vvData)));

	emit DataChanged();

	UpdateDistributionTable();
	UpdateFractionsTable();

	ui.pushButtonApply->setEnabled(false);

	QApplication::restoreOverrideCursor();
}

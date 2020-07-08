/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "MaterialsDatabaseTab.h"
#include "DyssolStringConstants.h"
#include "DescriptionEditor.h"
#include "PropertyAdder.h"
#include "FileSystem.h"
#include "DyssolUtilities.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QLockFile>

CMaterialsDatabaseTab::CMaterialsDatabaseTab(CMaterialsDatabase *_pMaterialsDatabase, QSettings* _pSettings, QWidget *parent)
	: QDialog(parent),
	m_pMaterialsDB(_pMaterialsDatabase),
	m_pSettings(_pSettings)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

	ui.tableProperties->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.tableInterProperties->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

	UpdateWholeView();
}

void CMaterialsDatabaseTab::InitializeConnections()
{
	// actions
	connect(ui.actionNewDatabase,		&QAction::triggered,						this, &CMaterialsDatabaseTab::NewDatabase);
	connect(ui.actionLoadDatabase,		&QAction::triggered,						this, &CMaterialsDatabaseTab::LoadDatabase);
	connect(ui.actionSaveDatabase,		&QAction::triggered,						this, &CMaterialsDatabaseTab::SaveDatabase);
	connect(ui.actionSaveDatabaseAs,	&QAction::triggered,						this, &CMaterialsDatabaseTab::SaveDatabaseAs);

	// signals from file buttons
	connect(ui.buttonNew,				&QPushButton::clicked,						this, &CMaterialsDatabaseTab::NewDatabase);
	connect(ui.buttonLoad,				&QPushButton::clicked,						this, &CMaterialsDatabaseTab::LoadDatabase);
	connect(ui.buttonSave,				&QPushButton::clicked,						this, &CMaterialsDatabaseTab::SaveDatabase);
	connect(ui.buttonSaveAs,			&QPushButton::clicked,						this, &CMaterialsDatabaseTab::SaveDatabaseAs);

	// signals from compounds buttons
	connect(ui.buttonAddCompound,		&QPushButton::clicked,						this, &CMaterialsDatabaseTab::AddCompound);
	connect(ui.buttonDuplicateCompound,	&QPushButton::clicked,						this, &CMaterialsDatabaseTab::DuplicateCompound);
	connect(ui.buttonRemoveCompound,	&QPushButton::clicked,						this, &CMaterialsDatabaseTab::RemoveCompound);
	connect(ui.buttonUpCompound,		&QPushButton::clicked,						this, &CMaterialsDatabaseTab::ShiftCompoundUp);
	connect(ui.buttonDownCompound,		&QPushButton::clicked,						this, &CMaterialsDatabaseTab::ShiftCompoundDown);

	// signals from compounds
	connect(ui.tableCompounds,			&QTableWidget::itemSelectionChanged,		this, &CMaterialsDatabaseTab::NewCompoundSelected);
	connect(ui.tableCompounds,			&QTableWidget::itemChanged,					this, &CMaterialsDatabaseTab::CompoundChanged);

	// signals from properties buttons
	connect(ui.buttonAddProperty,       &QPushButton::clicked, this, &CMaterialsDatabaseTab::AddProperty);
	connect(ui.buttonDuplicateProperty, &QPushButton::clicked, this, &CMaterialsDatabaseTab::DuplicateProperty);
	connect(ui.buttonRemoveProperty,    &QPushButton::clicked, this, &CMaterialsDatabaseTab::RemoveProperty);
	connect(ui.buttonEditProperty,      &QPushButton::clicked, this, &CMaterialsDatabaseTab::EditProperty);

	// signals from properties table
	connect(ui.tableProperties,			&QTableWidget::itemSelectionChanged,		this, &CMaterialsDatabaseTab::NewPropertySelected);
	connect(ui.tableProperties,			&QTableWidget::cellChanged,					this, &CMaterialsDatabaseTab::PropertyValueChanged);
	connect(ui.propertyEditorMain,		&CPropertyEditor::MDBPropertyChanged,		this, &CMaterialsDatabaseTab::MaterialDatabaseWasChanged);
	connect(ui.propertyEditorMain,      &CPropertyEditor::MDBPropertyChanged,       this, [=] { SetMaterialsDatabaseModified(true); });

	// signals from interaction buttons
	connect(ui.buttonAddInterProperty,       &QPushButton::clicked, this, &CMaterialsDatabaseTab::AddInterProperty);
	connect(ui.buttonDuplicateInterProperty, &QPushButton::clicked, this, &CMaterialsDatabaseTab::DuplicateInterProperty);
	connect(ui.buttonRemoveInterProperty,    &QPushButton::clicked, this, &CMaterialsDatabaseTab::RemoveInterProperty);
	connect(ui.buttonEditInterProperty,      &QPushButton::clicked, this, &CMaterialsDatabaseTab::EditInterProperty);

	// signals from interaction tab
	connect(ui.listCompounds1,			&QListWidget::itemSelectionChanged,			this, &CMaterialsDatabaseTab::NewCompound1Selected);
	connect(ui.listCompounds2,			&QListWidget::itemSelectionChanged,			this, &CMaterialsDatabaseTab::NewCompound2Selected);
	connect(ui.tableInterProperties,	&QTableWidget::itemSelectionChanged,		this, &CMaterialsDatabaseTab::NewInteractionSelected);
	connect(ui.tableInterProperties,	&QTableWidget::cellChanged,					this, &CMaterialsDatabaseTab::InteractionValueChanged);
	connect(ui.propertyEditorInter,		&CPropertyEditor::MDBPropertyChanged,		this, &CMaterialsDatabaseTab::MaterialDatabaseWasChanged);
	connect(ui.propertyEditorInter,     &CPropertyEditor::MDBPropertyChanged,       this, [=] { SetMaterialsDatabaseModified(true); });
}

void CMaterialsDatabaseTab::setVisible(bool _bVisible)
{
	if (_bVisible && !isVisible())
		UpdateWholeView();
	QDialog::setVisible(_bVisible);
}

void CMaterialsDatabaseTab::UpdateWholeView()
{
	CreateCompoundPropertiesTable();
	CreateInteractionPropertiesTable();

	UpdateWindowTitle();
	UpdateFileButtonsActivity();
	UpdateCompoundsList();
	UpdateCompoundProperties();
	UpdateInteractionsCompoundsLists();
	UpdateInteractionProperties();
}

void CMaterialsDatabaseTab::NewDatabase()
{
	if (!IsUserConfirm()) return;
	m_pMaterialsDB->CreateNewDatabase();
	UpdateWholeView();
	SetMaterialsDatabaseModified(false);
	emit MaterialDatabaseWasChanged();
}

void CMaterialsDatabaseTab::LoadDatabase()
{
	if (!IsUserConfirm()) return;
	const QString sFileName = QFileDialog::getOpenFileName(this, StrConst::MDT_DialogLoadName, QString::fromStdWString(m_pMaterialsDB->GetFileName()), StrConst::MDT_DialogDMDBFilter);
	if (!QFile::exists(sFileName.simplified())) return;
	m_pMaterialsDB->LoadFromFile(sFileName.toStdWString());
	m_pSettings->setValue(StrConst::Dyssol_ConfigDMDBPath, sFileName);
	UpdateWholeView();
	SetMaterialsDatabaseModified(false);
	emit MaterialDatabaseWasChanged();
}

void CMaterialsDatabaseTab::SaveDatabase()
{
	if (!QString::fromStdWString(m_pMaterialsDB->GetFileName()).simplified().isEmpty())
		SaveToFile(QString::fromStdWString(m_pMaterialsDB->GetFileName()));
	else
		SaveDatabaseAs();
}

void CMaterialsDatabaseTab::SaveDatabaseAs()
{
	const QString sFileName = QFileDialog::getSaveFileName(this, StrConst::MDT_DialogSaveName, QString::fromStdWString(m_pMaterialsDB->GetFileName()), StrConst::MDT_DialogDMDBFilter);
	if (sFileName.simplified().isEmpty()) return;
	if (!SaveToFile(sFileName)) return;
	m_pSettings->setValue(StrConst::Dyssol_ConfigDMDBPath, sFileName);
	UpdateWindowTitle();
	UpdateFileButtonsActivity();
}

bool CMaterialsDatabaseTab::SaveToFile(const QString& _fileName)
{
	const bool bSuccess = m_pMaterialsDB->SaveToFile(_fileName.toStdWString());
	if (bSuccess)
		SetMaterialsDatabaseModified(false);
	else
	{
		QString message = "Unable to save the database to the file:\n'" + _fileName;
		if (FileSystem::IsWriteProtected(FileSystem::FilePath(_fileName.toStdWString())))
			message += "'\nThe selected path may be write-protected.";
		QMessageBox::warning(this, StrConst::Dyssol_MDBWindowName, message);
	}
	return bSuccess;
}

void CMaterialsDatabaseTab::AddCompound()
{
	CCompound* pCompound = m_pMaterialsDB->AddCompound();
	// pick name for new compound
	std::string sNewName;
	bool bAlreadyExist;
	size_t index = 0;
	do
	{
		bAlreadyExist = false;
		sNewName = "Compound" + std::to_string(index++);
		for (size_t i = 0; i < m_pMaterialsDB->CompoundsNumber(); ++i)
			if (m_pMaterialsDB->GetCompound(i)->GetName() == sNewName)
				bAlreadyExist = true;
	} while (bAlreadyExist);

	// set new name
	pCompound->SetName(sNewName);

	UpdateCompoundsList();
	UpdateInteractionsCompoundsLists();
	SelectCompound(pCompound);
	SetMaterialsDatabaseModified(true);
	emit MaterialDatabaseWasChanged();
}

void CMaterialsDatabaseTab::DuplicateCompound()
{
	const CCompound* pBaseCompound = GetSelectedCompound();
	if (!pBaseCompound) return;

	const std::string sBaseKey1 = pBaseCompound->GetKey();
	CCompound* pCompound = m_pMaterialsDB->AddCompound(*pBaseCompound);
	SetMaterialsDatabaseModified(true);

	// copy interactions
	const std::string sKey1 = pCompound->GetKey();
	for (size_t i = 0; i < m_pMaterialsDB->CompoundsNumber(); ++i)
	{
		const std::string sKey2 = m_pMaterialsDB->GetCompound(i)->GetKey();
		std::string sBaseKey2 = sKey2;
		if (sKey2 == sKey1 || sKey2 == sBaseKey1)	// interaction with itself
			sBaseKey2 = sBaseKey1;
		CInteraction* pInteraction = m_pMaterialsDB->GetInteraction(sKey1, sKey2);
		const CInteraction* pBaseInteraction = m_pMaterialsDB->GetInteraction(sBaseKey1, sBaseKey2);
		if (pInteraction && pBaseInteraction)
		{
			*pInteraction = *pBaseInteraction;
			pInteraction->SetKeys(sKey1, sKey2);
		}
	}

	// set new name
	pCompound->SetName(pCompound->GetName() + "_copy");

	UpdateCompoundsList();
	UpdateInteractionsCompoundsLists();
	SelectCompound(pCompound);

	emit MaterialDatabaseWasChanged();
}

void CMaterialsDatabaseTab::RemoveCompound()
{
	if(const CCompound* pCompound = GetSelectedCompound())
		if (QMessageBox::question(this, StrConst::MDT_RemoveCompoundTitle, QString::fromStdString(StrConst::MDT_RemoveCompoundConfirm(pCompound->GetName())), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel) == QMessageBox::Yes)
		{
			SetMaterialsDatabaseModified(true);
			m_pMaterialsDB->RemoveCompound(pCompound->GetKey());
			UpdateCompoundsList();
			UpdateCompoundProperties();
			NewPropertySelected();
			UpdateInteractionsCompoundsLists();
			emit MaterialDatabaseWasChanged();
		}
}

void CMaterialsDatabaseTab::ShiftCompoundUp()
{
	if (const CCompound* pCompound = GetSelectedCompound())
	{
		SetMaterialsDatabaseModified(true);
		const std::string sKey = pCompound->GetKey();
		m_pMaterialsDB->ShiftCompoundUp(sKey);
		UpdateCompoundsList();
		UpdateInteractionsCompoundsLists();
		SelectCompound(m_pMaterialsDB->GetCompound(sKey));
		emit MaterialDatabaseWasChanged();
	}
}

void CMaterialsDatabaseTab::ShiftCompoundDown()
{
	if (const CCompound* pCompound = GetSelectedCompound())
	{
		SetMaterialsDatabaseModified(true);
		const std::string sKey = pCompound->GetKey();
		m_pMaterialsDB->ShiftCompoundDown(sKey);
		UpdateCompoundsList();
		UpdateInteractionsCompoundsLists();
		SelectCompound(m_pMaterialsDB->GetCompound(sKey));
		emit MaterialDatabaseWasChanged();
	}
}

void CMaterialsDatabaseTab::NewCompoundSelected() const
{
	UpdateCompoundProperties();
	NewPropertySelected();
}

void CMaterialsDatabaseTab::CompoundChanged(QTableWidgetItem* _pItem)
{
	CCompound* pCompound = GetSelectedCompound();
	if (!pCompound) return;
	const std::string sText = _pItem->text().simplified().toStdString();
	if (sText.empty()) return;
	switch (static_cast<ECompTable>(_pItem->column()))
	{
	case CMaterialsDatabaseTab::CT_NAME_COL:	pCompound->SetName(sText);	break;
	case CMaterialsDatabaseTab::CT_KEY_COL:		pCompound->SetKey(sText);	break;
	}
	SetMaterialsDatabaseModified(true);
	UpdateCompoundsList();
	UpdateInteractionsCompoundsLists();
}

void CMaterialsDatabaseTab::AddProperty()
{
	using namespace MDBDescriptors;
	SPropertyDescriptor prop;
	prop.type = EPropertyType::CONSTANT;
	prop.key = FirstAvailableUDProp(prop.type);
	if (prop.key == CONST_PROP_NO_PROERTY)
	{
		prop.type = EPropertyType::TP_DEPENDENT;
		prop.key = FirstAvailableUDProp(prop.type);
		if (prop.key == CONST_PROP_NO_PROERTY)
		{
			QMessageBox::warning(this, "Custom properties", "Can not add more constant or TP-dependent properties.");
			return;
		}
	}

	auto* adder = new CPropertyAdder(&prop, ActiveUDProps(), this);
	if (adder->exec() == Accepted)
	{
		m_pMaterialsDB->AddProperty(prop);
		SetMaterialsDatabaseModified(true);
		CreateCompoundPropertiesTable();
		UpdateCompoundProperties();
		NewPropertySelected();
		emit MaterialDatabaseWasChanged();
	}
	delete adder;
}

void CMaterialsDatabaseTab::DuplicateProperty()
{
	using namespace MDBDescriptors;
	const int iRow = ui.tableProperties->currentRow();
	if (iRow == -1) return;
	SPropertyDescriptor newProperty;
	newProperty.type = IsConstProperty(iRow) ? EPropertyType::CONSTANT : EPropertyType::TP_DEPENDENT;
	newProperty.key = FirstAvailableUDProp(newProperty.type);
	if (newProperty.key == CONST_PROP_NO_PROERTY)
	{
		QMessageBox::warning(this, "Custom properties", "Can not add more properties of this type.");
		return;
	}

	const unsigned oldKey = GetPropertyKey(ui.tableProperties, iRow);
	if (newProperty.type == EPropertyType::CONSTANT)
	{
		const auto oldProperty = m_pMaterialsDB->ActiveConstProperties()[static_cast<ECompoundConstProperties>(oldKey)];
		newProperty.value       = oldProperty.defaultValue;
		newProperty.name        = oldProperty.name + "_copy";
		newProperty.units       = oldProperty.units;
		newProperty.description = oldProperty.description;
	}
	else
	{
		const auto oldProperty = m_pMaterialsDB->ActiveTPDepProperties()[static_cast<ECompoundTPProperties>(oldKey)];
		newProperty.value       = oldProperty.defaultParameters.front();
		newProperty.name        = oldProperty.name + "_copy";
		newProperty.units       = oldProperty.units;
		newProperty.description = oldProperty.description;
	}

	m_pMaterialsDB->AddProperty(newProperty);
	SetMaterialsDatabaseModified(true);
	CreateCompoundPropertiesTable();
	UpdateCompoundProperties();
	NewPropertySelected();
	emit MaterialDatabaseWasChanged();
}

void CMaterialsDatabaseTab::RemoveProperty()
{
	using namespace MDBDescriptors;
	const int iRow = ui.tableProperties->currentRow();
	if (iRow == -1) return;
	const EPropertyType type = IsConstProperty(iRow) ? EPropertyType::CONSTANT : EPropertyType::TP_DEPENDENT;
	const unsigned key = GetPropertyKey(ui.tableProperties, iRow);
	if (key < FIRST_CONST_USER_PROP || key >= FIRST_CONST_USER_PROP + MAX_USER_DEFINED_PROP_NUMBER &&
		key < FIRST_TPDEP_USER_PROP || key >= FIRST_TPDEP_USER_PROP + MAX_USER_DEFINED_PROP_NUMBER)
	{
		QMessageBox::warning(this, "Custom properties", "This property can not be removed.");
		return;
	}
	if (QMessageBox::question(this, "Custom properties", "Do you really want to remove this property from materials database?") != QMessageBox::Yes) return;
	m_pMaterialsDB->RemoveProperty(type, key);
	SetMaterialsDatabaseModified(true);
	CreateCompoundPropertiesTable();
	UpdateCompoundProperties();
	NewPropertySelected();
	emit MaterialDatabaseWasChanged();
}

void CMaterialsDatabaseTab::EditProperty()
{
	using namespace MDBDescriptors;
	const int iRow = ui.tableProperties->currentRow();
	if (iRow == -1) return;
	SPropertyDescriptor newProperty;
	newProperty.type = IsConstProperty(iRow) ? EPropertyType::CONSTANT : EPropertyType::TP_DEPENDENT;
	newProperty.key = GetPropertyKey(ui.tableProperties, iRow);
	if (newProperty.key < FIRST_CONST_USER_PROP || newProperty.key >= FIRST_CONST_USER_PROP + MAX_USER_DEFINED_PROP_NUMBER &&
		newProperty.key < FIRST_TPDEP_USER_PROP || newProperty.key >= FIRST_TPDEP_USER_PROP + MAX_USER_DEFINED_PROP_NUMBER)
	{
		QMessageBox::warning(this, "Custom properties", "This property can not be edited.");
		return;
	}

	if (newProperty.type == EPropertyType::CONSTANT)
	{
		const auto oldProperty = m_pMaterialsDB->ActiveConstProperties()[static_cast<ECompoundConstProperties>(newProperty.key)];
		newProperty.value       = oldProperty.defaultValue;
		newProperty.name        = oldProperty.name;
		newProperty.units       = oldProperty.units;
		newProperty.description = oldProperty.description;
	}
	else
	{
		const auto oldProperty = m_pMaterialsDB->ActiveTPDepProperties()[static_cast<ECompoundTPProperties>(newProperty.key)];
		newProperty.value       = oldProperty.defaultParameters.front();
		newProperty.name        = oldProperty.name;
		newProperty.units       = oldProperty.units;
		newProperty.description = oldProperty.description;
	}

	const EPropertyType currType = newProperty.type;
	const unsigned currKey = newProperty.key;

	auto activeProps = ActiveUDProps();
	activeProps.erase(std::remove(activeProps.begin(), activeProps.end(), currKey), activeProps.end());

	auto* adder = new CPropertyAdder(&newProperty, activeProps, this);
	if (adder->exec() == Accepted)
	{
		m_pMaterialsDB->RemoveProperty(currType, currKey);
		m_pMaterialsDB->AddProperty(newProperty);
		SetMaterialsDatabaseModified(true);
		CreateCompoundPropertiesTable();
		UpdateCompoundProperties();
		NewPropertySelected();
		emit MaterialDatabaseWasChanged();
	}
	delete adder;
}

void CMaterialsDatabaseTab::NewPropertySelected() const
{
	CCompound* pCompound = GetSelectedCompound();
	if (!pCompound) return;
	const int iRow = ui.tableProperties->currentRow();
	if (iRow == -1) return;

	const unsigned type = GetPropertyKey(ui.tableProperties, iRow);
	const bool bConst = IsConstProperty(iRow) || ui.tableProperties->GetCheckBoxChecked(iRow, EPropTable::PT_CONST_COL); // if constant property or TP property with constant value selected

	ui.propertyEditorMain->setEnabled(!bConst);
	ui.propertyEditorMain->SetProperty(bConst ? nullptr : pCompound->GetTPProperty(static_cast<ECompoundTPProperties>(type)));
	if(IsConstProperty(iRow))	// const property selected
		ui.textDescription->setText(QString::fromStdString(m_pMaterialsDB->ActiveConstProperties()[static_cast<ECompoundConstProperties>(type)].description));
	else						// TP  property selected
		ui.textDescription->setText(QString::fromStdString(m_pMaterialsDB->ActiveTPDepProperties()[static_cast<ECompoundTPProperties>(type)].description));
}

void CMaterialsDatabaseTab::PropertyValueChanged(int _iRow, int _iCol)
{
	if (_iRow < 0) return;
	if (_iCol != EPropTable::PT_VALUE_COL) return;
	CCompound* pCompound = GetSelectedCompound();
	if (!pCompound) return;
	SetMaterialsDatabaseModified(true);
	const unsigned type = GetPropertyKey(ui.tableProperties, _iRow);
	const double dValue = ui.tableProperties->item(_iRow, _iCol)->text().toDouble();

	if (IsConstProperty(_iRow)) // const property
		pCompound->GetConstProperty(static_cast<ECompoundConstProperties>(type))->SetValue(dValue);
	else						// TP property
		pCompound->GetTPProperty(static_cast<ECompoundTPProperties>(type))->SetCorrelation(0, ECorrelationTypes::CONSTANT, { dValue });

	emit MaterialDatabaseWasChanged();
}

void CMaterialsDatabaseTab::PropertyConstFlagChanged(const int _iRow)
{
	if (_iRow < 0) return;
	CCompound* pCompound = GetSelectedCompound();
	if (!pCompound) return;

	QSignalBlocker blocker(ui.tableProperties);
	const unsigned type = GetPropertyKey(ui.tableProperties, _iRow);
	TogglePropertyConstancy(ui.tableProperties, _iRow, pCompound->GetTPProperty(static_cast<ECompoundTPProperties>(type)));

	NewPropertySelected();
}

void CMaterialsDatabaseTab::PropertyInfoClicked(int _iRow)
{
	CCompound* pCompound = GetSelectedCompound();
	if (!pCompound) return;

	CDescriptable* pObject;
	const unsigned type = GetPropertyKey(ui.tableProperties, _iRow);
	if (IsConstProperty(_iRow)) // const property
		pObject = pCompound->GetConstProperty(static_cast<ECompoundConstProperties>(type));
	else						// TP property
		pObject = pCompound->GetTPProperty(static_cast<ECompoundTPProperties>(type));

	auto* pEditor = new CDescriptionEditor(pObject, this);
	pEditor->exec();
	if (pEditor->IsDescriptionChanged())
	{
		ui.tableProperties->GetToolButton(_iRow, EPropTable::PT_INFO_COL)->setToolTip(CDescriptionEditor::TextToDisplay(pObject->GetDescription()));
		SetMaterialsDatabaseModified(true);
		emit MaterialDatabaseWasChanged();
	}
	delete pEditor;
}

void CMaterialsDatabaseTab::AddInterProperty()
{
	using namespace MDBDescriptors;
	SPropertyDescriptor prop;
	prop.type = EPropertyType::INTERACTION;
	prop.key = FirstAvailableUDProp(prop.type);
	if (prop.key == CONST_PROP_NO_PROERTY)
	{
		QMessageBox::warning(this, "Custom properties", "Can not add more interaction properties.");
		return;
	}

	auto* adder = new CPropertyAdder(&prop, ActiveUDProps(), this);
	if (adder->exec() == Accepted)
	{
		m_pMaterialsDB->AddProperty(prop);
		SetMaterialsDatabaseModified(true);
		CreateInteractionPropertiesTable();
		UpdateInteractionProperties();
		NewInteractionSelected();
		emit MaterialDatabaseWasChanged();
	}
	delete adder;
}

void CMaterialsDatabaseTab::DuplicateInterProperty()
{
	using namespace MDBDescriptors;
	const int iRow = ui.tableInterProperties->currentRow();
	if (iRow == -1) return;
	SPropertyDescriptor newProperty;
	newProperty.type = EPropertyType::INTERACTION;
	newProperty.key = FirstAvailableUDProp(newProperty.type);
	if (newProperty.key == CONST_PROP_NO_PROERTY)
	{
		QMessageBox::warning(this, "Custom properties", "Can not add more interaction properties.");
		return;
	}

	const unsigned oldKey = GetPropertyKey(ui.tableInterProperties, iRow);
	SCompoundTPDPropertyDescriptor oldProperty = m_pMaterialsDB->ActiveInterProperties()[static_cast<EInteractionProperties>(oldKey)];
	newProperty.value = oldProperty.defaultParameters.front();
	newProperty.name = oldProperty.name + "_copy";
	newProperty.units = oldProperty.units;
	newProperty.description = oldProperty.description;

	m_pMaterialsDB->AddProperty(newProperty);
	SetMaterialsDatabaseModified(true);
	CreateInteractionPropertiesTable();
	UpdateInteractionProperties();
	NewInteractionSelected();
	emit MaterialDatabaseWasChanged();
}

void CMaterialsDatabaseTab::RemoveInterProperty()
{
	using namespace MDBDescriptors;
	const int iRow = ui.tableInterProperties->currentRow();
	if (iRow == -1) return;
	const EPropertyType type = EPropertyType::INTERACTION;
	const unsigned key = GetPropertyKey(ui.tableInterProperties, iRow);
	if (key < FIRST_INTER_USER_PROP || key >= FIRST_INTER_USER_PROP + MAX_USER_DEFINED_PROP_NUMBER)
	{
		QMessageBox::warning(this, "Custom properties", "This property can not be removed.");
		return;
	}
	if (QMessageBox::question(this, "Custom properties", "Do you really want to remove this property from materials database?") != QMessageBox::Yes) return;
	m_pMaterialsDB->RemoveProperty(type, key);
	SetMaterialsDatabaseModified(true);
	CreateInteractionPropertiesTable();
	UpdateInteractionProperties();
	NewInteractionSelected();
	emit MaterialDatabaseWasChanged();
}

void CMaterialsDatabaseTab::EditInterProperty()
{
	using namespace MDBDescriptors;
	const int iRow = ui.tableInterProperties->currentRow();
	if (iRow == -1) return;
	SPropertyDescriptor newProperty;
	newProperty.type = EPropertyType::INTERACTION;
	newProperty.key = GetPropertyKey(ui.tableInterProperties, iRow);
	if (newProperty.key < FIRST_INTER_USER_PROP || newProperty.key >= FIRST_INTER_USER_PROP + MAX_USER_DEFINED_PROP_NUMBER)
	{
		QMessageBox::warning(this, "Custom properties", "This property can not be edited.");
		return;
	}
	SCompoundTPDPropertyDescriptor oldProperty = m_pMaterialsDB->ActiveInterProperties()[static_cast<EInteractionProperties>(newProperty.key)];
	newProperty.value = oldProperty.defaultParameters.front();
	newProperty.name = oldProperty.name;
	newProperty.units = oldProperty.units;
	newProperty.description = oldProperty.description;

	const EPropertyType currType = newProperty.type;
	const unsigned currKey = newProperty.key;

	auto activeProps = ActiveUDProps();
	activeProps.erase(std::remove(activeProps.begin(), activeProps.end(), currKey), activeProps.end());

	auto* adder = new CPropertyAdder(&newProperty, activeProps, this);
	if (adder->exec() == Accepted)
	{
		m_pMaterialsDB->RemoveProperty(currType, currKey);
		m_pMaterialsDB->AddProperty(newProperty);
		SetMaterialsDatabaseModified(true);
		CreateInteractionPropertiesTable();
		UpdateInteractionProperties();
		NewInteractionSelected();
		emit MaterialDatabaseWasChanged();
	}
	else
		m_pMaterialsDB->AddProperty(newProperty);
	delete adder;
}

void CMaterialsDatabaseTab::NewCompound1Selected() const
{
	if(const CCompound* pCompound = GetSelectedInterCompounds().first)
		ui.labelCompound1->setText(QString::fromStdString(pCompound->GetName()));
	else
		ui.labelCompound1->clear();
	UpdateInteractionProperties();
	NewInteractionSelected();
}

void CMaterialsDatabaseTab::NewCompound2Selected() const
{
	if (const CCompound* pCompound = GetSelectedInterCompounds().second)
		ui.labelCompound2->setText(QString::fromStdString(pCompound->GetName()));
	else
		ui.labelCompound2->clear();
	UpdateInteractionProperties();
	NewInteractionSelected();
}

void CMaterialsDatabaseTab::NewInteractionSelected() const
{
	CInteraction *pInteraction = GetSelectedInteraction();
	if (!pInteraction) return;
	const int iRow = ui.tableInterProperties->currentRow();
	if (iRow == -1) return;

	const bool bConst = ui.tableInterProperties->GetCheckBoxChecked(iRow, EPropTable::PT_CONST_COL);  // if constant property selected

	ui.propertyEditorInter->setEnabled(!bConst);
	const unsigned type = GetPropertyKey(ui.tableInterProperties, iRow);
	ui.propertyEditorInter->SetProperty(bConst ? nullptr : pInteraction->GetProperty(static_cast<EInteractionProperties>(type)));
}

void CMaterialsDatabaseTab::InteractionValueChanged(int _iRow, int _iCol)
{
	if (_iRow < 0) return;
	if (_iCol != EPropTable::PT_VALUE_COL) return;
	CInteraction *pInteraction = GetSelectedInteraction();
	if (!pInteraction) return;

	const unsigned type = GetPropertyKey(ui.tableInterProperties, _iRow);
	const double dValue = ui.tableInterProperties->item(_iRow, _iCol)->text().toDouble();
	pInteraction->GetProperty(static_cast<EInteractionProperties>(type))->SetCorrelation(0, ECorrelationTypes::CONSTANT, { dValue });

	SetMaterialsDatabaseModified(true);
	emit MaterialDatabaseWasChanged();
}

void CMaterialsDatabaseTab::InteractionConstFlagChanged(int _iRow)
{
	CInteraction *pInteraction = GetSelectedInteraction();
	if (!pInteraction) return;

	QSignalBlocker blocker(ui.tableInterProperties);
	const unsigned type = GetPropertyKey(ui.tableInterProperties, _iRow);
	TogglePropertyConstancy(ui.tableInterProperties, _iRow, pInteraction->GetProperty(static_cast<EInteractionProperties>(type)));

	NewInteractionSelected();
}

void CMaterialsDatabaseTab::InteractionInfoClicked(int _iRow)
{
	CInteraction *pInteraction = GetSelectedInteraction();
	if (!pInteraction) return;

	const unsigned type = GetPropertyKey(ui.tableInterProperties, _iRow);
	CDescriptable* pObject = pInteraction->GetProperty(static_cast<EInteractionProperties>(type));

	auto* pEditor = new CDescriptionEditor(pObject, this);
	pEditor->exec();
	if (pEditor->IsDescriptionChanged())
	{
		ui.tableInterProperties->GetToolButton(_iRow, EPropTable::PT_INFO_COL)->setToolTip(CDescriptionEditor::TextToDisplay(pObject->GetDescription()));
		SetMaterialsDatabaseModified(true);
		emit MaterialDatabaseWasChanged();
	}
	delete pEditor;
}

void CMaterialsDatabaseTab::UpdateWindowTitle()
{
	if (!m_pMaterialsDB) return;
	setWindowTitle(StrConst::MDT_WindowTitle + QString{ ": " } + QString::fromStdWString(m_pMaterialsDB->GetFileName() + L"[*]"));
}

void CMaterialsDatabaseTab::UpdateFileButtonsActivity() const
{
	const QString sLockerFileName = QString::fromStdWString(m_pMaterialsDB->GetFileName()) + ".lock";
	QLockFile locker(sLockerFileName);
	locker.setStaleLockTime(0);
	const bool bSuccessfullyLocked = locker.tryLock(10);
	locker.unlock();
	ui.buttonSave->setEnabled(bSuccessfullyLocked);
}

void CMaterialsDatabaseTab::UpdateCompoundsList() const
{
	if (!m_pMaterialsDB) return;
	QSignalBlocker blocker(ui.tableCompounds);

	const int iOldRow = ui.tableCompounds->currentRow();
	ui.tableCompounds->setRowCount(static_cast<int>(m_pMaterialsDB->CompoundsNumber()));
	for (int i = 0; i < static_cast<int>(m_pMaterialsDB->CompoundsNumber()); ++i)
	{
		ui.tableCompounds->SetItemEditable(i, ECompTable::CT_NAME_COL, m_pMaterialsDB->GetCompound(i)->GetName(), QString::fromStdString(m_pMaterialsDB->GetCompound(i)->GetKey()));
		ui.tableCompounds->SetItemEditable(i, ECompTable::CT_KEY_COL, m_pMaterialsDB->GetCompound(i)->GetKey(), QString::fromStdString(m_pMaterialsDB->GetCompound(i)->GetKey()));
	}
	ui.tableCompounds->RestoreSelectedCell(iOldRow, ECompTable::CT_NAME_COL);
}

void CMaterialsDatabaseTab::UpdateCompoundProperties() const
{
	const CCompound* pCompound = GetSelectedCompound();
	ui.groupBoxProperties->setEnabled(pCompound != nullptr);
	if (!pCompound)	return;

	QSignalBlocker blocker(ui.tableProperties);
	int iRow = 0;

	// const properties
	for (const auto& propDescr : m_pMaterialsDB->ActiveConstProperties())
	{
		const CConstProperty* prop = pCompound->GetConstProperty(propDescr.first);
		if (!prop) continue;
		ui.tableProperties->SetItemEditable(iRow, EPropTable::PT_VALUE_COL, prop->GetValue(), prop->GetType());
		ui.tableProperties->GetToolButton(iRow, EPropTable::PT_INFO_COL)->setToolTip(CDescriptionEditor::TextToDisplay(prop->GetDescription()));
		iRow++;
	}

	// PT properties
	for (const auto& propDescr : m_pMaterialsDB->ActiveTPDepProperties())
	{
		const CTPDProperty* prop = pCompound->GetTPProperty(propDescr.first);
		if (!prop) continue;
		const bool isConst = prop->CorrelationsNumber() == 1 && prop->GetCorrelation(0)->GetType() == ECorrelationTypes::CONSTANT;
		ui.tableProperties->SetCheckBoxChecked(iRow, EPropTable::PT_CONST_COL, isConst);
		if (isConst)
			ui.tableProperties->SetItemEditable(iRow, EPropTable::PT_VALUE_COL, QString::number(prop->GetValue(STANDARD_CONDITION_T, STANDARD_CONDITION_P)), prop->GetType());
		else
			ui.tableProperties->SetItemNotEditable(iRow, EPropTable::PT_VALUE_COL, QString{}, prop->GetType());
		ui.tableProperties->GetToolButton(iRow, EPropTable::PT_INFO_COL)->setToolTip(CDescriptionEditor::TextToDisplay(prop->GetDescription()));
		ui.tableProperties->SetItemBackgroundColor(iRow, EPropTable::PT_VALUE_COL, isConst ? Qt::white : Qt::lightGray);
		iRow++;
	}
}

void CMaterialsDatabaseTab::UpdateInteractionsCompoundsLists() const
{
	QSignalBlocker blocker1(ui.listCompounds1);
	QSignalBlocker blocker2(ui.listCompounds2);

	const int iOldRow1 = ui.listCompounds1->currentRow();
	const int iOldRow2 = ui.listCompounds2->currentRow();
	ui.listCompounds1->clear();
	ui.listCompounds2->clear();
	for (int i = 0; i < static_cast<int>(m_pMaterialsDB->CompoundsNumber()); ++i)
	{
		QListWidgetItem *pItem1 = new QListWidgetItem(QString::fromStdString(m_pMaterialsDB->GetCompound(i)->GetName()));
		pItem1->setData(Qt::UserRole, QString::fromStdString(m_pMaterialsDB->GetCompound(i)->GetKey()));
		ui.listCompounds1->insertItem(i, pItem1);

		QListWidgetItem *pItem2 = new QListWidgetItem(QString::fromStdString(m_pMaterialsDB->GetCompound(i)->GetName()));
		pItem2->setData(Qt::UserRole, QString::fromStdString(m_pMaterialsDB->GetCompound(i)->GetKey()));
		ui.listCompounds2->insertItem(i, pItem2);
	}
	if (iOldRow1 >= 0 && iOldRow1 < static_cast<int>(m_pMaterialsDB->CompoundsNumber()))
		ui.listCompounds1->setCurrentRow(iOldRow1);
	if (iOldRow2 >= 0 && iOldRow2 < static_cast<int>(m_pMaterialsDB->CompoundsNumber()))
		ui.listCompounds2->setCurrentRow(iOldRow2);
}

void CMaterialsDatabaseTab::UpdateInteractionProperties() const
{
	const CInteraction *pInteraction = GetSelectedInteraction();
	ui.tableInterProperties->setEnabled(pInteraction != nullptr);
	if (!pInteraction) return;

	QSignalBlocker blocker(ui.tableInterProperties);
	int iRow = 0;

	for (const auto& propDescr : m_pMaterialsDB->ActiveInterProperties())
	{
		const CTPDProperty* prop = pInteraction->GetProperty(propDescr.first);
		const bool isConst = prop->CorrelationsNumber() == 1 && prop->GetCorrelation(0)->GetType() == ECorrelationTypes::CONSTANT;
		ui.tableInterProperties->SetCheckBoxChecked(iRow, EPropTable::PT_CONST_COL, isConst);
		ui.tableInterProperties->SetItemEditable(iRow, EPropTable::PT_VALUE_COL, isConst ? QString::number(prop->GetValue(STANDARD_CONDITION_T, STANDARD_CONDITION_P)) : QString{}, prop->GetType());
		ui.tableInterProperties->GetToolButton(iRow, EPropTable::PT_INFO_COL)->setToolTip(CDescriptionEditor::TextToDisplay(prop->GetDescription()));
		ui.tableInterProperties->SetItemBackgroundColor(iRow, EPropTable::PT_VALUE_COL, isConst ? Qt::white : Qt::lightGray);
		iRow++;
	}
}

void CMaterialsDatabaseTab::CreateCompoundPropertiesTable()
{
	QSignalBlocker blocker(ui.tableProperties);
	ui.tableProperties->clearContents();
	ui.tableProperties->setRowCount(static_cast<int>(m_pMaterialsDB->ActiveConstProperties().size() + m_pMaterialsDB->ActiveTPDepProperties().size()));
	int iRow = 0;

	// const properties
	for (const auto& prop : m_pMaterialsDB->ActiveConstProperties())
	{
		AddCheckBoxOnTable(ui.tableProperties, iRow, EPropTable::PT_CONST_COL, true, false);
		ui.tableProperties->SetItemNotEditable(iRow, EPropTable::PT_NAME_COL, prop.second.name, prop.first);
		ui.tableProperties->SetLabel(iRow, EPropTable::PT_UNITS_COL, QString::fromStdWString(prop.second.units));
		ui.tableProperties->SetItemEditable(iRow, EPropTable::PT_VALUE_COL, QString{}, prop.first);
		AddToolButtonOnTable(ui.tableProperties, iRow, EPropTable::PT_INFO_COL);
		iRow++;
	}

	// PT properties
	for (const auto& prop : m_pMaterialsDB->ActiveTPDepProperties())
	{
		AddCheckBoxOnTable(ui.tableProperties, iRow, EPropTable::PT_CONST_COL, false, true);
		ui.tableProperties->SetItemNotEditable(iRow, EPropTable::PT_NAME_COL, prop.second.name, prop.first);
		ui.tableProperties->SetLabel(iRow, EPropTable::PT_UNITS_COL, QString::fromStdWString(prop.second.units));
		ui.tableProperties->SetItemEditable(iRow, EPropTable::PT_VALUE_COL, QString{}, prop.first);
		AddToolButtonOnTable(ui.tableProperties, iRow, EPropTable::PT_INFO_COL);
		iRow++;
	}

	ui.tableProperties->horizontalHeader()->setVisible(true);
}

void CMaterialsDatabaseTab::CreateInteractionPropertiesTable()
{
	QSignalBlocker blocker(ui.tableInterProperties);
	ui.tableInterProperties->clearContents();
	ui.tableInterProperties->setRowCount(static_cast<int>(m_pMaterialsDB->ActiveInterProperties().size()));
	int iRow = 0;

	for (const auto& prop : m_pMaterialsDB->ActiveInterProperties())
	{
		AddCheckBoxOnTable(ui.tableInterProperties, iRow, EPropTable::PT_CONST_COL, false, true);
		ui.tableInterProperties->SetItemNotEditable(iRow, EPropTable::PT_NAME_COL, prop.second.name, prop.first);
		ui.tableInterProperties->SetLabel(iRow, EPropTable::PT_UNITS_COL, QString::fromStdWString(prop.second.units));
		ui.tableInterProperties->SetItemEditable(iRow, EPropTable::PT_VALUE_COL, QString{}, prop.first);
		AddToolButtonOnTable(ui.tableInterProperties, iRow, EPropTable::PT_INFO_COL);
		iRow++;
	}

	ui.tableInterProperties->horizontalHeader()->setVisible(true);
}

void CMaterialsDatabaseTab::AddCheckBoxOnTable(CQtTable* _pTable, const int _iRow, const int _iCol, const bool _bChecked, const bool _bEnabled /*= true*/)
{
	QCheckBox* pCheckBox = _pTable->SetCheckBox(_iRow, _iCol, _bChecked);
	pCheckBox->setEnabled(_bEnabled);
	if(_pTable == ui.tableProperties)
		connect(pCheckBox, &QCheckBox::stateChanged, this, [=] { PropertyConstFlagChanged(_iRow); });
	else if(_pTable == ui.tableInterProperties)
		connect(pCheckBox, &QCheckBox::stateChanged, this, [=] { InteractionConstFlagChanged(_iRow); });
}

void CMaterialsDatabaseTab::AddToolButtonOnTable(CQtTable* _pTable, int _iRow, int _iCol)
{
	QToolButton* pToolButton = _pTable->SetToolButton(_iRow, _iCol, "i");
	if (_pTable == ui.tableProperties)
		connect(pToolButton, &QToolButton::clicked, this, [=] { PropertyInfoClicked(_iRow); });
	else if (_pTable == ui.tableInterProperties)
		connect(pToolButton, &QToolButton::clicked, this, [=] { InteractionInfoClicked(_iRow); });
}

CCompound* CMaterialsDatabaseTab::GetSelectedCompound(const int _row /*= -1*/) const
{
	return m_pMaterialsDB->GetCompound(ui.tableCompounds->GetItemUserData(_row, ECompTable::CT_NAME_COL).toStdString());
}

std::pair<CCompound*, CCompound*> CMaterialsDatabaseTab::GetSelectedInterCompounds() const
{
	std::pair<CCompound*, CCompound*> res{ nullptr, nullptr };
	if (const QListWidgetItem* pItem = ui.listCompounds1->currentItem())
		res.first = m_pMaterialsDB->GetCompound(pItem->data(Qt::UserRole).toString().toStdString());
	if (const QListWidgetItem* pItem = ui.listCompounds2->currentItem())
		res.second = m_pMaterialsDB->GetCompound(pItem->data(Qt::UserRole).toString().toStdString());
	return  res;
}

CInteraction* CMaterialsDatabaseTab::GetSelectedInteraction() const
{
	const auto compounds = GetSelectedInterCompounds();
	if (!compounds.first || !compounds.second) return nullptr;
	return  m_pMaterialsDB->GetInteraction(compounds.first->GetKey(), compounds.second->GetKey());
}

unsigned CMaterialsDatabaseTab::GetPropertyKey(const CQtTable* _pTable, int _iRow)
{
	return _pTable->GetItemUserData(_iRow, EPropTable::PT_VALUE_COL).toUInt();
}

bool CMaterialsDatabaseTab::IsConstProperty(int _iRow) const
{
	return _iRow < static_cast<int>(m_pMaterialsDB->ActiveConstProperties().size());
}

void CMaterialsDatabaseTab::SelectCompound(const CCompound* _pCompound) const
{
	for (int i = 0; i < static_cast<int>(m_pMaterialsDB->CompoundsNumber()); ++i)
		if (GetSelectedCompound(i)->GetKey() == _pCompound->GetKey())
		{
			ui.tableCompounds->setCurrentCell(i, ECompTable::CT_NAME_COL);
			break;
		}
}

void CMaterialsDatabaseTab::TogglePropertyConstancy(CQtTable* _pTable, const int _iRow, CTPDProperty* _pProp)
{
	if (_pTable->GetCheckBoxChecked(_iRow, EPropTable::PT_CONST_COL))
		if (QMessageBox::question(this, StrConst::MDT_RemoveCorrelationsTitle, QString::fromStdString(StrConst::MDT_RemoveCorrelationsConfirm(_pTable->item(_iRow, EPropTable::PT_NAME_COL)->text().toStdString())),
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel) == QMessageBox::Yes)
		{
			_pProp->RemoveAllCorrelations();
			_pProp->AddCorrelation(ECorrelationTypes::CONSTANT, { 0 });

			QTableWidgetItem *pItem = _pTable->item(_iRow, EPropTable::PT_VALUE_COL);
			pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
			pItem->setBackgroundColor(Qt::white);
			pItem->setText("0");

			SetMaterialsDatabaseModified(true);
			emit MaterialDatabaseWasChanged();
		}
		else
			_pTable->SetCheckBoxChecked(_iRow, EPropTable::PT_CONST_COL, false);
	else
	{
		QTableWidgetItem *pItem = _pTable->item(_iRow, EPropTable::PT_VALUE_COL);
		pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);
		pItem->setBackgroundColor(Qt::lightGray);
		pItem->setText("");
	}
}

void CMaterialsDatabaseTab::SetMaterialsDatabaseModified(bool _bModified)
{
	m_bMaterialsDatabaseChanged = _bModified;
	setWindowModified(_bModified);
}

bool CMaterialsDatabaseTab::IsUserConfirm()
{
	if (m_bMaterialsDatabaseChanged)
	{
		const QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::Cancel | QMessageBox::No;
		const QMessageBox::StandardButton reply = QMessageBox::question(this, StrConst::Dyssol_MDBWindowName, StrConst::Dyssol_SaveMDBMessageBoxText, buttons);
		if (reply == QMessageBox::Yes)
			SaveDatabase();
		return reply != QMessageBox::Cancel;
	}
	return true;
}

std::vector<unsigned> CMaterialsDatabaseTab::ActiveUDProps() const
{
	using namespace MDBDescriptors;
	std::vector<unsigned> res;
	for (const auto& d : m_pMaterialsDB->ActiveConstProperties())
		if (FIRST_CONST_USER_PROP <= d.first && d.first < FIRST_CONST_USER_PROP + MAX_USER_DEFINED_PROP_NUMBER)
			res.push_back(d.first);
	for (const auto& d : m_pMaterialsDB->ActiveTPDepProperties())
		if (FIRST_TPDEP_USER_PROP <= d.first && d.first < FIRST_TPDEP_USER_PROP + MAX_USER_DEFINED_PROP_NUMBER)
			res.push_back(d.first);
	for (const auto& d : m_pMaterialsDB->ActiveInterProperties())
		if (FIRST_INTER_USER_PROP <= d.first && d.first < FIRST_INTER_USER_PROP + MAX_USER_DEFINED_PROP_NUMBER)
			res.push_back(d.first);
	return res;
}

unsigned CMaterialsDatabaseTab::FirstAvailableUDProp(MDBDescriptors::EPropertyType _type) const
{
	using namespace MDBDescriptors;
	unsigned iStart{ FIRST_CONST_USER_PROP };
	switch (_type)
	{
	case EPropertyType::CONSTANT:		iStart = FIRST_CONST_USER_PROP;	break;
	case EPropertyType::TP_DEPENDENT:	iStart = FIRST_TPDEP_USER_PROP;	break;
	case EPropertyType::INTERACTION:	iStart = FIRST_INTER_USER_PROP;	break;
	}

	const auto active = ActiveUDProps();
	for (unsigned i = iStart; i < iStart + MAX_USER_DEFINED_PROP_NUMBER; ++i)
		if (!VectorContains(active, i))
			return i;

	return CONST_PROP_NO_PROERTY;
}

void CMaterialsDatabaseTab::closeEvent(QCloseEvent* _event)
{
	if (!IsUserConfirm())
		_event->ignore();
	else
		close();
}

void CMaterialsDatabaseTab::keyPressEvent(QKeyEvent* _event)
{
	if (_event == QKeySequence::Save)
		SaveDatabase();
	else if (_event->matches(QKeySequence::New))
		NewDatabase();
	else if (_event->matches(QKeySequence::Open))
		LoadDatabase();
	else if (_event->matches(QKeySequence::SaveAs) || _event->key() == Qt::Key_S && _event->modifiers() & Qt::ControlModifier && _event->modifiers() & Qt::ShiftModifier)
		SaveDatabaseAs();
	else
		QDialog::keyPressEvent(_event);
}

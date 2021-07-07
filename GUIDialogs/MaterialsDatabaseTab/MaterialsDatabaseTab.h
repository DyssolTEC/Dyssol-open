/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_MaterialsDatabaseTab.h"
#include "MaterialsDatabase.h"
#include <QSettings>

class CMaterialsDatabaseTab : public QDialog
{
	Q_OBJECT

private:
	enum ECompTable { CT_NAME_COL = 0, CT_KEY_COL = 1 };
	enum EPropTable { PT_CONST_COL = 0, PT_NAME_COL = 1, PT_UNITS_COL = 2, PT_VALUE_COL = 3, PT_INFO_COL = 4 };

	Ui::CMaterialsDatabaseTab ui;

	CMaterialsDatabase* m_materialsDB;
	QSettings* m_pSettings;

	bool m_bMaterialsDatabaseChanged=false;

public:
	CMaterialsDatabaseTab(CMaterialsDatabase *_pMaterialsDatabase, QSettings* _pSettings, QWidget *parent = nullptr);

	void InitializeConnections();

public slots:
	void setVisible(bool _bVisible) override;
	void UpdateWholeView();

private slots:
	void NewDatabase();
	void LoadDatabase();
	void SaveDatabase();
	void SaveDatabaseAs();
	bool SaveToFile(const QString& _fileName);

	void AddCompound();
	void DuplicateCompound();
	void RemoveCompound();
	void ShiftCompoundUp();		// Moves selected compound upwards in the list of compounds.
	void ShiftCompoundDown();	// Moves selected compound downwards in the list of compounds.

	void NewCompoundSelected() const;
	void CompoundChanged(QTableWidgetItem* _pItem);

	void AddProperty();
	void DuplicateProperty();
	void RemoveProperty();
	void EditProperty();

	void NewPropertySelected() const;
	void PropertyValueChanged(int _iRow, int _iCol);
	void PropertyConstFlagChanged(int _iRow);
	void PropertyInfoClicked(int _iRow);

	void AddInterProperty();
	void DuplicateInterProperty();
	void RemoveInterProperty();
	void EditInterProperty();

	void NewCompound1Selected() const;
	void NewCompound2Selected() const;
	void NewInteractionSelected() const;
	void InteractionValueChanged(int _iRow, int _iCol);
	void InteractionConstFlagChanged(int _iRow);
	void InteractionInfoClicked(int _iRow);

protected:
	void closeEvent(QCloseEvent* _event) override;	// closing of the MDB window event
	void keyPressEvent(QKeyEvent* _event) override; // override of method keyPressEvent

private:
	void UpdateWindowTitle();
	void UpdateFileButtonsActivity() const;	// Update activity of new/save/save as/load buttons

	void UpdateCompoundsList() const;
	void UpdateCompoundProperties() const;

	void UpdateInteractionsCompoundsLists() const;
	void UpdateInteractionProperties() const;

	void CreateCompoundPropertiesTable();		// Create entries of the table that do not change.
	void CreateInteractionPropertiesTable();	// Create entries of the table that do not change.
	void AddCheckBoxOnTable(CQtTable* _pTable, int _iRow, int _iCol, bool _bChecked, bool _bEnabled = true);
	void AddToolButtonOnTable(CQtTable* _pTable, int _iRow, int _iCol);

	CCompound* GetSelectedCompound(int _row = -1) const;					// Returns pointer to a currently selected compound (if _row == -1) or to a compound from specified row.
	std::pair<CCompound*, CCompound*> GetSelectedInterCompounds() const;	// Returns pointers to currently selected compounds on Interactions tab.
	CInteraction* GetSelectedInteraction() const;							// Returns pointers to currently selected interaction.
	static unsigned GetPropertyKey(const CQtTable* _pTable, int _iRow);
	bool IsConstProperty(int _iRow) const;										// Returns true if the property in the specified row is constant.

	void SelectCompound(const CCompound* _pCompound) const;	// Makes specified compound currently selected.
	void TogglePropertyConstancy(CQtTable* _pTable, int _iRow, CTPDProperty* _pProp);

	void SetMaterialsDatabaseModified(bool _bModified);//set value for state MDB flag
	bool IsUserConfirm(); // Asks user about saving of current database state and saves it if needed. Returns false if user pressed Cancel.

	// Returns a list of user defined properties of all types.
	std::vector<unsigned> ActiveUDProps() const;
	// Retunrs the first available user defined property of a specified type.
	unsigned FirstAvailableUDProp(MDBDescriptors::EPropertyType _type) const;

signals:
	void MaterialDatabaseWasChanged();	// Called when material database has been changed.
};

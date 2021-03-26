/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_FlowsheetEditor.h"

class CModelsManager;
class CMaterialsDatabase;
class CFlowsheet;
class CStream;
class CUnitContainer;
class CUnitParametersManager;

class CFlowsheetEditor : public QWidget
{
	Q_OBJECT

	Ui::CFlowsheetEditorClass ui;
	CFlowsheet* m_pFlowsheet;					// Pointer to a current flowsheet.
	const CMaterialsDatabase* m_materialsDB;	// Pointer to a materials database.
	CModelsManager* m_modelsManager;			// Pointer to a models manager.
	CUnitContainer* m_pSelectedModel;		    // Currently selected model.
	CStream *m_pSelectedStream;					// Currently selected stream.
	CUnitParametersManager *m_pModelParams;		// Unit parameters of currently selected model.

public:
	CFlowsheetEditor(CFlowsheet* _pFlowsheet, const CMaterialsDatabase* _matrialsDB, CModelsManager* _modelsManager, QWidget *parent = nullptr);

	void InitializeConnections();

public slots:
	void setVisible(bool _bVisible) override;

	void UpdateWholeView();
	void UpdateAvailableUnits() const;   // Update information about available units.
	void UpdateAvailableSolvers() const; // Update information about available solvers.

private slots:
	void AddModel();	 // Add new model to the flowsheet.
	void DeleteModel();	 // Delete selected unit from the flowsheet.
	void UpModel();		 // Change the model's position in the list for one row above.
	void DownModel();	 // Change the model's position in the list for one row below.
	void AddStream();	 // Add new stream to the flowsheet.
	void DeleteStream(); // Delete selected stream from the flowsheet.
	void UpStream();	 // Change the stream's position in the list for one row above.
	void DownStream();	 // Change the stream's position in the list for one row below.

	void ChangeSelectedStream();		// User selected another stream in the list.
	void ChangeSelectedModel();			// User selected another model in the list.
	void ChangeUnitInModel(int _index); // User selected new unit for current model.

	void ChangeModelName(int _iRow, int _iCol);	 // User changed name of current unit.
	void ChangeStreamName(int _iRow, int _iCol); // User changed name of current stream.

	void NewPortStreamSelected(int _iRow, int _iCol, QComboBox* _comboBox); // User selected new stream for some port.

	void NewUnitParameterSelected() const;					       // User selected new unit parameter.

	void AddUnitParamListItem();									// Add new time point to selected unit parameter.
	void DeleteUnitParamListItem();								// Remove time point from selected unit parameter.
	void ListValueChanged();											// User changed value or time of selected unit parameter.
	void UnitParamValueChanged(int _row, int _col);				// Value of unit parameter has been changed.

private:
	void UpdateModelsView();		   // Update list of models in the flowsheet.
	void UpdateStreamsView();		   // Update list of material streams in the flowsheet.
	void UpdateUnitCombo() const;	   // Update selected unit according to a selected model.
	void UpdatePortsView() const;	   // Update ports according to selected model and unit.
	void UpdateUnitParamTable() const; // Update main view of unit parameters.
	void UpdateListValuesTable() const;  // Update time-dependent unit parameters table according to selected model and parameter.
	void UpdateUnitParamDescr() const; // Update unit parameter description according to selected model and parameter.

	void EnableGUIElements() const; // Disables block of GUI elements if model is not selected.

signals:
	void ModelsChanged();  // A model has been added, removed or renamed.
	void StreamsChanged(); // A stream has been added, removed or renamed.
	void UnitChanged();	   // New unit has been selected for model.
	void DataChanged();	   // User has made some changes to the flowsheet.
};

/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_GridEditor.h"
#include "DimensionParameters.h"
#include "MultidimensionalGrid.h"
#include "QtDialog.h"

class CFlowsheet;

class CGridEditor
	: public CQtDialog
{
	Q_OBJECT
	Ui::CGridEditorClass ui{};

	const CMaterialsDatabase& m_materialsDB;	// Reference to a global database of materials.
	CFlowsheet* m_flowsheet;					// Pointer to the flowsheet.
	CMultidimensionalGrid m_grid;				// Working copy of the grid.

public:
	CGridEditor(CFlowsheet* _flowsheet, const CMaterialsDatabase& _materialsDB, QWidget* _parent = nullptr, Qt::WindowFlags _flags = Qt::WindowFlags());

	// Connects all signals and slots.
	void InitializeConnections() const;

	// Change visibility of the dialog.
	void setVisible(bool _visible) override;
	// Tries to apply all changes and closes the dialog on success.
	void accept() override;

	// Updates all widgets.
	void UpdateWholeView();

private:
	// Updates the list of grids sources.
	void UpdateGridsList() const;
	// Updates the list of grid dimensions.
	void UpdateDimensionsList();

	// Called when new grid is selected.
	void GridSelected();
	// Called when activity of unit-specific changes.
	void GridActivityChanged(const QCheckBox* _checkbox, const QTreeWidgetItem* _item);
	// Called when add distribution button clicked.
	void AddDistributionClicked();
	// Called when remove distribution button clicked.
	void RemDistributionClicked();

	// Adds new grid dimension widget to the view.
	CDimensionParameters* AddGridDimension(const CGridDimension& _dim);

	// Tries to apply all changes if they are valid.
	bool ApplyChanges();
	// Checks validity of all settings.
	bool IsValid();

signals:
	void DataChanged();
};

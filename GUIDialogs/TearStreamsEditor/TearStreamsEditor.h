/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_TearStreamsEditor.h"
#include "QtDialog.h"

class CCalculationSequence;
class CParametersHolder;

class CTearStreamsEditor
	: public CQtDialog
{
	Q_OBJECT
private:
	Ui::CTearStreamsEditorClass ui;

	CFlowsheet* m_pFlowsheet;	             // Pointer to the flowsheet.
	const CCalculationSequence* m_pSequence; // Pointer to the calculation sequence.

public:
	CTearStreamsEditor(CFlowsheet* _pFlowsheet, CMaterialsDatabase* _materialsDB, QWidget* _parent = nullptr);

	void InitializeConnections();
	void UpdateFromFlowsheet();
	void UpdateWholeView();

private slots:
	void setVisible(bool _bVisible) override;

private:
	void UpdatePartitionsList();	// Update list of partitions.
	void UpdateStreamsList();		// Update list of tear streams within selected partition.
	void UpdateMode() const;				// Update selected mode (Auto/User).

	void NewStreamSelected() const;		// User selected new tear stream.
	void ClearAllStreams();			// Remove all time points from all recycle streams.
	void ModeChanged() const;				// Called when a new mode (Auto/User) selected.
	void SetEditable() const;				// Turn on/off edit possibility according to selected mode (Auto/User).

signals:
	void DataChanged();				// Some information in stream have been changed.
};

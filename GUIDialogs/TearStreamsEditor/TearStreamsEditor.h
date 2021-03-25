/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_TearStreamsEditor.h"

class CCalculationSequence;
class CParametersHolder;

class CTearStreamsEditor : public QDialog
{
	Q_OBJECT
private:
	Ui::CTearStreamsEditorClass ui;

	CFlowsheet* m_pFlowsheet;	             // Pointer to the flowsheet.
	const CCalculationSequence* m_pSequence; // Pointer to the calculation sequence.

public:
	CTearStreamsEditor(CFlowsheet* _pFlowsheet, CMaterialsDatabase* _materialsDB, QWidget *parent = Q_NULLPTR);

	void InitializeConnections();
	void UpdateWholeView();

private slots:
	void setVisible(bool _bVisible) override;

private:
	void UpdatePartitionsList();	// Update list of partitions.
	void UpdateStreamsList();		// Update list of tear streams within selected partition.
	void UpdateMode();				// Update selected mode (Auto/User).

	void NewStreamSelected();		// User selected new tear stream.
	void ClearAllStreams();			// Remove all time points from all recycle streams.
	void SetEditable();				// Turn on/off edit possibility according to selected mode (Auto/User).

signals:
	void DataChanged();				// Some information in stream have been changed.
};

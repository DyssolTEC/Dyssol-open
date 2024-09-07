/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_StreamsViewer.h"
#include "QtDialog.h"

class CMaterialsDatabase;
class CFlowsheet;
class CBasicStreamsViewer;

class CStreamsViewer
	: public CQtDialog
{
	Q_OBJECT
	Ui::CStreamsViewerClass ui;

	CFlowsheet* m_pFlowsheet;		// pointer to the flowsheet
	CBasicStreamsViewer *m_pViewer;	// streams viewer

public:
	CStreamsViewer(CFlowsheet* _pFlowsheet, CMaterialsDatabase* _materialsDB, QWidget* _parent = nullptr, Qt::WindowFlags _flags = Qt::WindowFlags());

	void SetPointers(CFlowsheet* _flowsheet, CModelsManager* _modelsManager, QSettings* _settings) override;
	void InitializeConnections() const;

public slots:
	void UpdateWholeView() const;
	void setVisible(bool _bVisible) override;

	void UpdateStreamsView() const;

private slots:
	void StreamChanged() const;
};

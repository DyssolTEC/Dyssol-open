/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_StreamsViewer.h"
#include "QtDialog.h"

class CMaterialsDatabase;
class CFlowsheet;
class CBasicStreamsViewer;

class CStreamsViewer : public CQtDialog
{
	Q_OBJECT
	Ui::CStreamsViewerClass ui;

	CFlowsheet* m_pFlowsheet;		// pointer to the flowsheet
	CBasicStreamsViewer *m_pViewer;	// streams viewer

public:
	CStreamsViewer(CFlowsheet* _pFlowsheet, CMaterialsDatabase* _materialsDB, CModelsManager* _modelsManager, QWidget* _parent = nullptr, Qt::WindowFlags _flags = Qt::WindowFlags());

	void InitializeConnections() const;

public slots:
	void UpdateWholeView() const;
	void setVisible(bool _bVisible) override;

	void UpdateStreamsView() const;

private slots:
	void StreamChanged() const;
};

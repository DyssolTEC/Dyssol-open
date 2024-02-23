/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_SimulatorTab.h"
#include "SimulationThread.h"
#include "Simulator.h"
#include "QtDialog.h"
#include <QTimer>
#include <QElapsedTimer>

class CSimulator;
class CFlowsheet;

class CSimulatorTab
	: public CQtDialog
{
	Q_OBJECT

	enum EStatTable : int
	{
		TIME_WIN_START = 0,
		TIME_WIN_END,
		TIME_WIN_LENGTH,
		ITERATION_NUMBER,
		WINDOW_NUMBER,
		UNIT_NAME,
		STARTED_TIME,
		ELAPSED_TIME
	};

	Ui::CSimulatorTabClass ui;

	CFlowsheet* m_pFlowsheet;			// Pointer to a current flowsheet.
	CSimulator* m_pSimulator;			// Pointer to a current simulator.

	CSimulationThread* m_simulationThread = new CSimulationThread([&]() {m_pSimulator->Simulate(); }, [&]() {m_pSimulator->Stop(); });	// Separate thread for simulator.

	QElapsedTimer m_simulationTimer;	// Timer to determine simulation time.
	QTimer m_logTimer;				    // Interrupt timer to update simulation log.

public:
	CSimulatorTab(CFlowsheet* _pFlowsheet, CSimulator* _pSimulator, QWidget* _parent = nullptr);
	CSimulatorTab(const CSimulatorTab&)            = delete;
	CSimulatorTab(CSimulatorTab&&)                 = delete;
	CSimulatorTab& operator=(const CSimulatorTab&) = delete;
	CSimulatorTab& operator=(CSimulatorTab&&)      = delete;
	~CSimulatorTab() override;

	void InitializeConnections() const;

public slots:
	void setVisible(bool _visible) override;

	void UpdateWholeView() const;
	void OnNewFlowsheet() const;

private slots:
	void SetSimulationTime();
	void StartSimulation();
	void SimulationFinished();

	void ClearSimulationResults();
	void ClearInitialRecycleStreams();
	void ClearAll();

	void UpdateLog() const;

private:
	void AbortSimulation();

	void ClearLog() const;
	void UpdateSimulationTime() const;
	void BlockUI(bool _block) const;

signals:
	void DataChanged();	// User has made some changes
	void SimulatorStateToggled(bool _running); // Emitted when the simulation is started or finished.
};

/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ui_Dyssol.h"
#include "Simulator.h"
#include "Flowsheet.h"
#include "CalculationSequenceEditor.h"
#include "MaterialsDatabaseTab.h"
#include "CompoundsManager.h"
#include "FlowsheetEditor.h"
#include "GridEditor.h"
#include "HoldupsEditor.h"
#include "ModulesManagerTab.h"
#include "OptionsEditor.h"
#include "PhasesEditor.h"
#include "StreamsViewer.h"
#include "UnitsViewer.h"
#include "TearStreamsEditor.h"
#include "DustFormationTesterTab.h"
#include "SettingsEditor.h"
#include "SaveLoadThread.h"
#include "SimulatorTab.h"
#include <QSharedMemory>

#define MAX_RECENT_FILES 10

#define FLOWSHEET_TAB	0
#define SIMULATOR_TAB	1
#define STREAMS_TAB		2
#define UNITS_TAB		3

class CStatusWindow;

//////////////////////////////////////////////////////////////////////////
/// Main Dyssol GUI class
//////////////////////////////////////////////////////////////////////////
class Dyssol : public QMainWindow
{
	Q_OBJECT

public:
	static const int EXIT_CODE_REBOOT = -0x9EB007;

private:
	CMaterialsDatabase m_MaterialsDatabase;		// database of materials
	CModelsManager m_ModelsManager;				// units and solvers manager
	CFlowsheet m_Flowsheet{ m_ModelsManager, m_MaterialsDatabase };	// flowsheet
	CSimulator m_Simulator;						// simulator

	QSettings* m_pSettings;		// Config file.
	QString m_sSettingsPath;	// Path to store settings and temporary data, where config file and caches are stored.

	CCalculationSequenceEditor* m_pCalcSequenceEditor;
	CMaterialsDatabaseTab *m_pMaterialsDatabaseTab;
	CCompoundsManager *m_pCompoundsManager;
	CFlowsheetEditor *m_pFlowsheetEditor;
	CGridEditor *m_pGridEditor;
	CHoldupsEditor *m_pHoldupsEditor;
	CModulesManagerTab *m_pModelsManagerTab;
	COptionsEditor *m_pOptionsEditor;
	CPhasesEditor *m_pPhasesEditor;
	CSimulatorTab *m_pSimulatorTab;
	CStreamsViewer *m_pStreamsViewer;
	CUnitsViewer *m_pUnitsViewer;
	CTearStreamsEditor* m_pTearStreamsEditor;
	CDustFormationTesterTab* m_pDustTesterTab;
	CSettingsEditor* m_pSettingsEditor;

	QString m_sCurrFlowsheetFile;			// file corresponding to a current flowsheet
	QList<QAction*> m_vRecentFilesActions;	// list of actions for recent files
	bool m_bFlowsheetChanged;				// if the flowsheet has been changed after loading

	CSaveLoadThread *m_pSavingThread;	// thread for saving of flowsheets
	CSaveLoadThread *m_pLoadingThread;	// thread for loading of flowsheets

	CStatusWindow *m_pSavingWindow;		// status window for saving
	CStatusWindow *m_pLoadingWindow;	// status window for loading

public:
	Dyssol(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
	~Dyssol() override;

	void InitializeConnections() const;	// initialize all qt connections
	void UpdateWholeView() const;			// update all window

	void RestoreLastState() const;					// open last flowsheet
	void OpenFromCommandLine(const QString& _sPath);	// open user defined flowsheet
	void OpenDyssol() const;

protected:
	void closeEvent( QCloseEvent* ) override;	// closing of the main window event

private:
	Ui::DyssolClass ui;

	void SetupCache(); // Setup cache path.
	void ClearCache();

	void CreateMenu(); // create window menu
	void UpdateMenu(); // update recent files options in the menu

	void SaveToFile( const QString &_sFileName ) const;	// save flowsheet to specified file
	void LoadFromFile( const QString &_sFileName ) const;	// load flowsheet from specified file

	void SetCurrFlowsheetFile(const QString &_fileName);		// set new flowsheet file
	void AddFileToRecentList(const QString& _fileToAdd);	// Adds a flowsheet file _fileName to the list of recent files.
	bool CheckAndAskUnsaved();	// Asks user about saving of current flowsheet and saves it if needed. Returns false if the calling process should be cancelled.
	bool SaveAndWait(); // Starts saving thread and waits until current flowsheet is saved. Returns true on successful save.
	static void CloseDyssol(int _errCode = 0); // terminates the application with selected error code
	void SetFlowsheetModified(bool _bModified);
	bool IsFlowsheetModified() const;

	void LoadMaterialsDatabase();
	static size_t OtherRunningDyssolCount(); // Returns the amount of running instances of Dyssol except this one.

public slots:
	void setVisible(bool _visible) override;

private slots:
	void LoadRecentFile();	// load one of the recent flowsheets after choosing from menu list

	void NewFlowsheet();	// create new flowsheet
	void OpenFlowsheet();	// open new flowsheet
	void SaveFlowsheet();	// save flowsheet to current file
	void SaveFlowsheetAs();	// save flowsheet to user specified file
	void SaveScriptFile();	// Saves the flowsheet as a script file for command line mode.
	void SaveGraphFile();	// Saves the flowsheet as a dot graph file.

	void SavingFinished();	// saving process is finished
	void LoadingFinished();	// loading process is finished

	void OpenHelp(const QString& _sFile) const;	// open help file
	void ShowAboutDialog();				// open about dialog

	void SlotSaveAndReopen();
	void SlotRestart();
	void SlotClearCache();

	void FlowsheetStateChanged();
	void BlockUI(bool _block) const;

signals:
	void NewFlowsheetLoaded();
};

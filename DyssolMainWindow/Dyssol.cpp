/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "Dyssol.h"
#include "StatusWindow.h"
#include "AboutWindow.h"
#include "ScriptExporter.h"
#include "FileSystem.h"
#include "DyssolStringConstants.h"
#include "DyssolSystemFunctions.h"
#include "ThreadPool.h"
#include <QDesktopWidget>
#include <QCloseEvent>
#include <QFileDialog>
#include <QDesktopServices>
#include <filesystem>
#include <fstream>


//////////////////////////////////////////////////////////////////////////
/// Dyssol
//////////////////////////////////////////////////////////////////////////
Dyssol::Dyssol(QWidget *parent /*= 0*/, Qt::WindowFlags flags /*= {}*/)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	resize(QDesktopWidget().availableGeometry(this).size() * 0.7);
	SetFlowsheetModified(false);

	InitializeThreadPool();

	// setup flowsheet and simulator
	m_Simulator.SetFlowsheet(&m_Flowsheet);

	// setup config file
	m_sSettingsPath = QFileInfo(QSettings(QSettings::IniFormat, QSettings::UserScope, StrConst::Dyssol_ApplicationName, StrConst::Dyssol_ConfigApp).fileName()).absolutePath();
	// create directory for temporary data if it doesn't exist
	if (!std::filesystem::exists(m_sSettingsPath.toStdString()))
		std::filesystem::create_directories(m_sSettingsPath.toStdString());

	const QString globalConfigFile = m_sSettingsPath + "/" + StrConst::Dyssol_ConfigFileName;
	const QString localConfigFile  = QString{ "./" } + StrConst::Dyssol_ConfigFileName;

#ifdef _MSC_VER
	const QString currConfigFile = QFile::exists(globalConfigFile) ? globalConfigFile : localConfigFile;
#else
	const QString currConfigFile = QFile::exists(localConfigFile) ? localConfigFile : globalConfigFile;

	std::cout << "Use config file: " << currConfigFile.toStdString()<< std::endl;

	// If the config file not exist at all, we will copy the template of config file into the currConfigFile
	// It is needed mostly for packages
	if (currConfigFile == globalConfigFile && !QFile::exists(currConfigFile))
	{
		std::cout << "Config file: " << currConfigFile.toStdString()<< "does not exist. It will be copied from "<< INSTALL_CONFIG_PATH << StrConst::Dyssol_ConfigFileName << std::endl;
		if (!std::filesystem::exists(INSTALL_CONFIG_PATH))
			std::filesystem::create_directories(INSTALL_CONFIG_PATH);
		std::filesystem::copy_file(std::filesystem::path{ INSTALL_CONFIG_PATH } / StrConst::Dyssol_ConfigFileName, currConfigFile.toStdString());
	}
#endif

	// create config file
	m_pSettings = new QSettings(currConfigFile, QSettings::IniFormat, this);

	// configure cache parameters
	SetupCache();

	// create dialogs and windows
	m_pModelsManagerTab     = new CModulesManagerTab(&m_ModelsManager, m_pSettings, this);
	m_pCalcSequenceEditor   = new CCalculationSequenceEditor(&m_Flowsheet, this);
	m_pMaterialsDatabaseTab = new CMaterialsDatabaseTab(&m_MaterialsDatabase, m_pSettings, this);
	m_pCompoundsManager     = new CCompoundsManager(&m_Flowsheet, &m_MaterialsDatabase, this);
	m_pFlowsheetEditor      = new CFlowsheetEditor(&m_Flowsheet, &m_MaterialsDatabase, &m_ModelsManager, m_pSettings, this);
	m_pGridEditor           = new CGridEditor(&m_Flowsheet, m_MaterialsDatabase, this);
	m_pHoldupsEditor        = new CHoldupsEditor(&m_Flowsheet, &m_MaterialsDatabase, this);
	m_pOptionsEditor        = new COptionsEditor(&m_Flowsheet, &m_MaterialsDatabase, this);
	m_pPhasesEditor         = new CPhasesEditor(&m_Flowsheet, this);
	m_pSimulatorTab         = new CSimulatorTab(&m_Flowsheet, &m_Simulator, this);
	m_pStreamsViewer        = new CStreamsViewer(&m_Flowsheet, &m_MaterialsDatabase, this);
	m_pUnitsViewer          = new CUnitsViewer(&m_Flowsheet, &m_MaterialsDatabase, this);
	m_pTearStreamsEditor    = new CTearStreamsEditor(&m_Flowsheet, &m_MaterialsDatabase, this);
	m_pDustTesterTab		= new CDustFormationTesterTab(&m_Flowsheet, &m_MaterialsDatabase, this);
	m_pSettingsEditor		= new CSettingsEditor(m_pSettings, this);

	// setup main window: add tabs to mainTabWidget
	ui.mainTabWidget->addTab(m_pFlowsheetEditor, StrConst::Dyssol_FlowsheetTabName);
	ui.mainTabWidget->addTab(m_pSimulatorTab, StrConst::Dyssol_SimulatorTabName);
	ui.mainTabWidget->addTab(m_pStreamsViewer, StrConst::Dyssol_StreamsTabName);
	ui.mainTabWidget->addTab(m_pUnitsViewer, StrConst::Dyssol_UnitsTabName);
	ui.mainTabWidget->setStyleSheet("QTabBar::tab { min-width: 100px; }");

	// status modal windows
	m_pLoadingWindow = new CStatusWindow(StrConst::Dyssol_StatusLoadingTitle, StrConst::Dyssol_StatusLoadingText, StrConst::Dyssol_StatusLoadingQuestion, false, this);
	m_pSavingWindow  = new CStatusWindow(StrConst::Dyssol_StatusSavingTitle,  StrConst::Dyssol_StatusSavingText,  StrConst::Dyssol_StatusSavingQuestion,  false);

	// threads for saving and loading
	m_pLoadingThread = new CSaveLoadThread(&m_Flowsheet, false);
	m_pSavingThread  = new CSaveLoadThread(&m_Flowsheet, true);

	// create and setup menu
	CreateMenu();
	UpdateMenu();

	// load materials database
	LoadMaterialsDatabase();
}

Dyssol::~Dyssol()
{
	delete m_pSavingThread;
	delete m_pLoadingThread;
	delete m_pLoadingWindow;
	delete m_pSavingWindow;
}

void Dyssol::InitializeConnections() const
{
	// initialize connections of widgets
	m_pCalcSequenceEditor->InitializeConnections();
	m_pMaterialsDatabaseTab->InitializeConnections();
	m_pCompoundsManager->InitializeConnections();
	m_pFlowsheetEditor->InitializeConnections();
	m_pGridEditor->InitializeConnections();
	m_pHoldupsEditor->InitializeConnections();
	m_pModelsManagerTab->InitializeConnections();
	m_pOptionsEditor->InitializeConnections();
	m_pPhasesEditor->InitializeConnections();
	m_pSimulatorTab->InitializeConnections();
	m_pStreamsViewer->InitializeConnections();
	m_pUnitsViewer->InitializeConnections();
	m_pTearStreamsEditor->InitializeConnections();
	m_pDustTesterTab->InitializeConnections();
	m_pSettingsEditor->InitializeConnections();

	// signals to dialogs from menu entries
	connect(ui.actionCalcSequencerEditor, &QAction::triggered, m_pCalcSequenceEditor,   &QDialog::raise);
	connect(ui.actionCalcSequencerEditor, &QAction::triggered, m_pCalcSequenceEditor,   &QDialog::show);
	connect(ui.actionCompoundsEditor,     &QAction::triggered, m_pMaterialsDatabaseTab, &QDialog::raise);
	connect(ui.actionCompoundsEditor,     &QAction::triggered, m_pMaterialsDatabaseTab, &QDialog::show);
	connect(ui.actionCompoundsManager,    &QAction::triggered, m_pCompoundsManager,     &QDialog::raise);
	connect(ui.actionCompoundsManager,    &QAction::triggered, m_pCompoundsManager,     &QDialog::show);
	connect(ui.actionGridEditor,          &QAction::triggered, m_pGridEditor,           &QDialog::raise);
	connect(ui.actionGridEditor,          &QAction::triggered, m_pGridEditor,           &QDialog::show);
	connect(ui.actionHoldupsEditor,       &QAction::triggered, m_pHoldupsEditor,        &QDialog::raise);
	connect(ui.actionHoldupsEditor,       &QAction::triggered, m_pHoldupsEditor,        &QDialog::show);
	connect(ui.actionModelsManager,       &QAction::triggered, m_pModelsManagerTab,     &QDialog::raise);
	connect(ui.actionModelsManager,       &QAction::triggered, m_pModelsManagerTab,     &QDialog::show);
	connect(ui.actionOptionsEditor,       &QAction::triggered, m_pOptionsEditor,        &QDialog::raise);
	connect(ui.actionOptionsEditor,       &QAction::triggered, m_pOptionsEditor,        &QDialog::show);
	connect(ui.actionPhasesEditor,        &QAction::triggered, m_pPhasesEditor,         &QDialog::raise);
	connect(ui.actionPhasesEditor,        &QAction::triggered, m_pPhasesEditor,         &QDialog::show);
	connect(ui.actionRecycleStreams,      &QAction::triggered, m_pTearStreamsEditor,    &QDialog::raise);
	connect(ui.actionRecycleStreams,      &QAction::triggered, m_pTearStreamsEditor,    &QDialog::show);
	connect(ui.actionDustFormationTester, &QAction::triggered, m_pDustTesterTab,        &QDialog::raise);
	connect(ui.actionDustFormationTester, &QAction::triggered, m_pDustTesterTab,        &QDialog::show);
	connect(ui.actionSettingsEditor,      &QAction::triggered, m_pSettingsEditor,       &QDialog::raise);
	connect(ui.actionSettingsEditor,      &QAction::triggered, m_pSettingsEditor,       &QDialog::show);

	// connections from file menu entries
	connect(ui.actionNewFlowsheet,    &QAction::triggered, this, &Dyssol::NewFlowsheet);
	connect(ui.actionOpenFlowsheet,   &QAction::triggered, this, &Dyssol::OpenFlowsheet);
	connect(ui.actionSaveFlowsheet,   &QAction::triggered, this, &Dyssol::SaveFlowsheet);
	connect(ui.actionSaveFlowsheetAs, &QAction::triggered, this, &Dyssol::SaveFlowsheetAs);
	connect(ui.actionSaveScript,      &QAction::triggered, this, &Dyssol::SaveScriptFile);
	connect(ui.actionAbout,           &QAction::triggered, this, &Dyssol::ShowAboutDialog);

	// signals from threads
	connect(m_pLoadingThread, &CSaveLoadThread::Finished, this, &Dyssol::LoadingFinished);
	connect(m_pSavingThread,  &CSaveLoadThread::Finished, this, &Dyssol::SavingFinished);

	// signals between widgets
	connect(m_pFlowsheetEditor,      &CFlowsheetEditor::ModelsChanged,                   m_pHoldupsEditor,      &CHoldupsEditor::UpdateWholeView);
	connect(m_pFlowsheetEditor,      &CFlowsheetEditor::ModelsChanged,                   m_pCalcSequenceEditor, &CCalculationSequenceEditor::UpdateWholeView);
	connect(m_pFlowsheetEditor,      &CFlowsheetEditor::ModelsChanged,                   m_pTearStreamsEditor,  &CTearStreamsEditor::UpdateWholeView);
	connect(m_pFlowsheetEditor,      &CFlowsheetEditor::StreamsChanged,                  m_pCalcSequenceEditor, &CCalculationSequenceEditor::UpdateWholeView);
	connect(m_pFlowsheetEditor,      &CFlowsheetEditor::StreamsChanged,                  m_pTearStreamsEditor,  &CTearStreamsEditor::UpdateWholeView);
	connect(m_pFlowsheetEditor,      &CFlowsheetEditor::StreamsChanged,                  m_pDustTesterTab,      &CDustFormationTesterTab::UpdateWholeView);
	connect(m_pFlowsheetEditor,      &CFlowsheetEditor::UnitChanged,                     m_pHoldupsEditor,      &CHoldupsEditor::UpdateWholeView);
	connect(m_pFlowsheetEditor,      &CFlowsheetEditor::UnitChanged,                     m_pDustTesterTab,      &CDustFormationTesterTab::UpdateWholeView);

	connect(m_pHoldupsEditor,        &CHoldupsEditor::DataChanged,                       m_pUnitsViewer,        &CUnitsViewer::UpdateWholeView);
	connect(m_pHoldupsEditor,        &CHoldupsEditor::DataChanged,                       m_pDustTesterTab,      &CDustFormationTesterTab::UpdateWholeView);

	connect(m_pGridEditor,           &CGridEditor::DataChanged,                          m_pHoldupsEditor,      &CHoldupsEditor::UpdateWholeView);
	connect(m_pGridEditor,           &CGridEditor::DataChanged,                          m_pUnitsViewer,        &CUnitsViewer::UpdateWholeView);
	connect(m_pGridEditor,           &CGridEditor::DataChanged,                          m_pTearStreamsEditor,  &CTearStreamsEditor::UpdateWholeView);
	connect(m_pGridEditor,           &CGridEditor::DataChanged,                          m_pDustTesterTab,      &CDustFormationTesterTab::UpdateWholeView);

	connect(m_pMaterialsDatabaseTab, &CMaterialsDatabaseTab::MaterialDatabaseWasChanged, m_pHoldupsEditor,      &CHoldupsEditor::UpdateWholeView);
	connect(m_pMaterialsDatabaseTab, &CMaterialsDatabaseTab::MaterialDatabaseWasChanged, m_pGridEditor,         &CGridEditor::UpdateWholeView);
	connect(m_pMaterialsDatabaseTab, &CMaterialsDatabaseTab::MaterialDatabaseWasChanged, m_pCompoundsManager,   &CCompoundsManager::UpdateWholeView);
	connect(m_pMaterialsDatabaseTab, &CMaterialsDatabaseTab::MaterialDatabaseWasChanged, m_pTearStreamsEditor,  &CTearStreamsEditor::UpdateWholeView);

	connect(m_pCompoundsManager,     &CCompoundsManager::DataChanged,                    m_pHoldupsEditor,      &CHoldupsEditor::UpdateWholeView);
	connect(m_pCompoundsManager,     &CCompoundsManager::DataChanged,                    m_pUnitsViewer,        &CUnitsViewer::UpdateWholeView);
	connect(m_pCompoundsManager,     &CCompoundsManager::DataChanged,                    m_pTearStreamsEditor,  &CTearStreamsEditor::UpdateWholeView);
	connect(m_pCompoundsManager,     &CCompoundsManager::DataChanged,                    m_pDustTesterTab,      &CDustFormationTesterTab::UpdateWholeView);

	connect(m_pPhasesEditor,         &CPhasesEditor::DataChanged,                        m_pHoldupsEditor,      &CHoldupsEditor::UpdateWholeView);
	connect(m_pPhasesEditor,         &CPhasesEditor::DataChanged,                        m_pUnitsViewer,        &CUnitsViewer::UpdateWholeView);
	connect(m_pPhasesEditor,         &CPhasesEditor::DataChanged,                        m_pTearStreamsEditor,  &CTearStreamsEditor::UpdateWholeView);
	connect(m_pPhasesEditor,         &CPhasesEditor::DataChanged,                        m_pDustTesterTab,      &CDustFormationTesterTab::UpdateWholeView);

	connect(m_pModelsManagerTab,     &CModulesManagerTab::ModelsListWasChanged,          m_pFlowsheetEditor,    &CFlowsheetEditor::UpdateAvailableUnits);
	connect(m_pModelsManagerTab,     &CModulesManagerTab::ModelsListWasChanged,          m_pFlowsheetEditor,    &CFlowsheetEditor::UpdateAvailableSolvers);

	connect(m_pSimulatorTab,         &CSimulatorTab::SimulatorStateToggled,              this,                  &Dyssol::BlockUI);
	connect(m_pOptionsEditor,        &COptionsEditor::NeedSaveAndReopen,                 this,                  &Dyssol::SlotSaveAndReopen);
	connect(m_pSettingsEditor,       &CSettingsEditor::NeedRestart,                      this,                  &Dyssol::SlotRestart);
	connect(m_pSettingsEditor,       &CSettingsEditor::NeedCacheClear,                   this,                  &Dyssol::SlotClearCache);

	connect(this, &Dyssol::NewFlowsheetLoaded, m_pCalcSequenceEditor, &CCalculationSequenceEditor::UpdateWholeView);
	connect(this, &Dyssol::NewFlowsheetLoaded, m_pCompoundsManager,   &CCompoundsManager::UpdateWholeView);
	connect(this, &Dyssol::NewFlowsheetLoaded, m_pGridEditor,         &CGridEditor::UpdateWholeView);
	connect(this, &Dyssol::NewFlowsheetLoaded, m_pHoldupsEditor,      &CHoldupsEditor::UpdateWholeView);
	connect(this, &Dyssol::NewFlowsheetLoaded, m_pOptionsEditor,      &COptionsEditor::UpdateWholeView);
	connect(this, &Dyssol::NewFlowsheetLoaded, m_pPhasesEditor,       &CPhasesEditor::UpdateWholeView);
	connect(this, &Dyssol::NewFlowsheetLoaded, m_pTearStreamsEditor,  &CTearStreamsEditor::UpdateWholeView);
	connect(this, &Dyssol::NewFlowsheetLoaded, m_pDustTesterTab,      &CDustFormationTesterTab::UpdateWholeView);
	connect(this, &Dyssol::NewFlowsheetLoaded, m_pSimulatorTab,       &CSimulatorTab::OnNewFlowsheet);

	// modification signals
	connect(m_pCalcSequenceEditor, &CCalculationSequenceEditor::DataChanged, this, &Dyssol::FlowsheetStateChanged);
	connect(m_pCompoundsManager,   &CCompoundsManager::DataChanged,          this, &Dyssol::FlowsheetStateChanged);
	connect(m_pFlowsheetEditor,	   &CFlowsheetEditor::DataChanged,           this, &Dyssol::FlowsheetStateChanged);
	connect(m_pGridEditor,		   &CGridEditor::DataChanged,                this, &Dyssol::FlowsheetStateChanged);
	connect(m_pHoldupsEditor,	   &CHoldupsEditor::DataChanged,             this, &Dyssol::FlowsheetStateChanged);
	connect(m_pOptionsEditor,	   &COptionsEditor::DataChanged,             this, &Dyssol::FlowsheetStateChanged);
	connect(m_pPhasesEditor,	   &CPhasesEditor::DataChanged,              this, &Dyssol::FlowsheetStateChanged);
	connect(m_pSimulatorTab,	   &CSimulatorTab::DataChanged,              this, &Dyssol::FlowsheetStateChanged);
	connect(m_pTearStreamsEditor,  &CTearStreamsEditor::DataChanged,         this, &Dyssol::FlowsheetStateChanged);
}

void Dyssol::UpdateWholeView() const
{
	switch (ui.mainTabWidget->currentIndex())
	{
	case FLOWSHEET_TAB:	m_pFlowsheetEditor->UpdateWholeView(); break;
	case SIMULATOR_TAB:	m_pSimulatorTab->UpdateWholeView();	   break;
	case STREAMS_TAB:	m_pStreamsViewer->UpdateWholeView();   break;
	case UNITS_TAB:		m_pUnitsViewer->UpdateWholeView();	   break;
	default:												   break;
	}
}

void Dyssol::RestoreLastState() const
{
	const QVariant path = m_pSettings->value(StrConst::Dyssol_ConfigLastParamName);
	if (path.isValid() && !path.toString().isEmpty())
		LoadFromFile(path.toString());
}

void Dyssol::OpenFromCommandLine(const QString& _sPath)
{
	if(_sPath.simplified().isEmpty()) return;
	if (!CheckAndAskUnsaved()) return;
	LoadFromFile(_sPath);
}

void Dyssol::OpenDyssol() const
{
	if (m_pSettings->value(StrConst::Dyssol_ConfigLoadLastFlag).toBool())
		RestoreLastState();
}

void Dyssol::closeEvent(QCloseEvent* event)
{
	if (m_Simulator.GetCurrentStatus() != ESimulatorStatus::SIMULATOR_IDLE)
	{
		const QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::Cancel | QMessageBox::No;
		const QMessageBox::StandardButton reply = QMessageBox::question(this, StrConst::Dyssol_MainWindowName, StrConst::Dyssol_AbortMessage, buttons);
		if (reply == QMessageBox::Yes)
			CloseDyssol();
		else
		{
			event->ignore();
			return;
		}
	}

	if (!CheckAndAskUnsaved())
		event->ignore();
	else
		CloseDyssol();
}

void Dyssol::SetupCache()
{
	// remove old cache data
	ClearCache();

	// set the default cache path to config.ini if it has not been set or does not exist
	const QVariant cachePathVar = m_pSettings->value(StrConst::Dyssol_ConfigCachePath);
	if (!cachePathVar.isValid() || cachePathVar.toString().isEmpty() || !std::filesystem::exists(cachePathVar.toString().toStdString()))
		m_pSettings->setValue(StrConst::Dyssol_ConfigCachePath, m_sSettingsPath);

	// setup cache path
	const QString cachePath = m_pSettings->value(StrConst::Dyssol_ConfigCachePath).toString();
	m_Flowsheet.GetParameters()->CachePath((cachePath + StrConst::Dyssol_CacheDir).toStdWString());

	// check whether the cache path is accessible
	if (FileSystem::IsWriteProtected(cachePath.toStdWString()))
	{
		m_pSavingThread->Block();
		m_pLoadingThread->Block();
		QMessageBox::critical(this, StrConst::Dyssol_MainWindowName, "Unable to access the selected cache path because it is write-protected:\n'" + cachePath + "'\nPlease choose another path using Tools -> Settings -> Change path...\nSaving/loading of flowsheets is blocked until that.");
	}

	m_Flowsheet.UpdateCacheSettings();
}

void Dyssol::ClearCache()
{
	// check whether other instances are running
	if (OtherRunningDyssolCount()) return;
	// close flowsheet
	m_Flowsheet.Clear();
	// clear cache
	std::filesystem::remove_all((m_pSettings->value(StrConst::Dyssol_ConfigCachePath).toString() + StrConst::Dyssol_CacheDirDebug).toStdString());
	std::filesystem::remove_all((m_pSettings->value(StrConst::Dyssol_ConfigCachePath).toString() + StrConst::Dyssol_CacheDirRelease).toStdString());
}

void Dyssol::CreateMenu()
{
	/// recent files
	for (size_t i = 0; i < MAX_RECENT_FILES; ++i)
	{
		auto* pAction = new QAction(this);
		pAction->setVisible(false);
		pAction->setToolTip("");
		connect(pAction, &QAction::triggered, this, &Dyssol::LoadRecentFile);
		ui.menuFile->insertAction(ui.actionExit, pAction);
		m_vRecentFilesActions.push_back(pAction);
	}
	ui.menuFile->insertSeparator(ui.actionExit);

	/// help files
	// Introduction
	QMenu* menuIntroduction = ui.menuDocumentation->addMenu("Introduction");
	menuIntroduction->addAction("Get Started"       , this, [&] { OpenHelp("000_get_started/get_started.html"); });
	menuIntroduction->addAction("Architecture"      , this, [&] { OpenHelp("002_theory/brief.html"           ); });
	menuIntroduction->addAction("Algorithms"        , this, [&] { OpenHelp("002_theory/theory.html"          ); });
	menuIntroduction->addAction("User Interface"    , this, [&] { OpenHelp("001_ui/gui.html"                 ); });

	// Units
	QMenu* menuUnits = ui.menuDocumentation->addMenu("Units");
	menuUnits->addAction("Agglomerator"  , this, [&] { OpenHelp("003_models/unit_agglomerator.html"); });
	menuUnits->addAction("Bunker"        , this, [&] { OpenHelp("003_models/unit_bunker.html"      ); });
	menuUnits->addAction("Crusher"       , this, [&] { OpenHelp("003_models/unit_crusher.html"     ); });
	menuUnits->addAction("Granulator"    , this, [&] { OpenHelp("003_models/unit_granulator.html"  ); });
	menuUnits->addAction("Inlet Flow"    , this, [&] { OpenHelp("003_models/unit_inletflow.html"   ); });
	menuUnits->addAction("Mixer"         , this, [&] { OpenHelp("003_models/unit_mixer.html"       ); });
	menuUnits->addAction("Outlet Flow"   , this, [&] { OpenHelp("003_models/unit_outletflow.html"  ); });
	menuUnits->addAction("Screen"        , this, [&] { OpenHelp("003_models/unit_screen.html"      ); });
	menuUnits->addAction("Splitter"      , this, [&] { OpenHelp("003_models/unit_splitter.html"    ); });
	menuUnits->addAction("Time Delay"    , this, [&] { OpenHelp("003_models/unit_timedelay.html"   ); });

	// Solvers
	QMenu* menuSolvers = ui.menuDocumentation->addMenu("Solvers");
	menuSolvers->addAction("Agglomeration Cell Average", this, [&] { OpenHelp("003_models/solver_cellaverage.html"); });
	menuSolvers->addAction("Agglomeration Fixed Pivot" , this, [&] { OpenHelp("003_models/solver_fixedpivot.html" ); });
	menuSolvers->addAction("Agglomeration FFT"         , this, [&] { OpenHelp("003_models/solver_fft.html"        ); });

	// Development
	QMenu* menuDevelopment = ui.menuDocumentation->addMenu("Development");
	menuDevelopment->addAction("Configuration of VCProject", this, [&] { OpenHelp("004_development/models_development.html#configuration-of-visual-studio-project-template"); });
	menuDevelopment->addAction("Units Development"         , this, [&] { OpenHelp("004_development/models_development.html#unit-development"                               ); });
	menuDevelopment->addAction("Solvers Development"       , this, [&] { OpenHelp("004_development/models_development.html#solver-development"                             ); });

	// Development - Program Interfaces
	QMenu* menuInterfaces = menuDevelopment->addMenu("Program Interfaces");
	menuInterfaces->addAction("BaseUnit"            , this, [&] { OpenHelp("004_development/class_baseunit.html"       ); });
	menuInterfaces->addAction("Stream"              , this, [&] { OpenHelp("004_development/class_stream.html"         ); });
	menuInterfaces->addAction("DAESolver"           , this, [&] { OpenHelp("004_development/solver_dae.html"           ); });
	menuInterfaces->addAction("ExternalSolver"      , this, [&] { OpenHelp("004_development/solver_external.html"      ); });
	menuInterfaces->addAction("TransformMatrix"     , this, [&] { OpenHelp("004_development/class_transformmatrix.html"); });
	menuInterfaces->addAction("MDMatrix"            , this, [&] { OpenHelp("004_development/class_densemdmatrix.html"  ); });
	menuInterfaces->addAction("Matrix2D"            , this, [&] { OpenHelp("004_development/class_matrix2d.html"       ); });
	menuInterfaces->addAction("PSD Functions"       , this, [&] { OpenHelp("004_development/distr_functions.html"      ); });
	menuInterfaces->addAction("Predefined Constants", this, [&] { OpenHelp("004_development/constants.html"            ); });

	// Main
	ui.menuDocumentation->addAction("Command Line Interface", this, [&] { OpenHelp("001_ui/cli.html"                           ); });
	ui.menuDocumentation->addAction("Convergence"           , this, [&] { OpenHelp("002_theory/theory.html#convergence-methods"); });
}

void Dyssol::UpdateMenu()
{
	const QStringList filesList = m_pSettings->value(StrConst::Dyssol_ConfigRecentParamName).toStringList();
	for (int i = 0; i < filesList.size(); ++i)
	{
		const std::wstring cleanFileName = CH5Handler::DisplayFileName(std::filesystem::path{ filesList[i].toStdWString() }).wstring();
		const QString displayFileName = QFileInfo(QString::fromStdWString(cleanFileName)).fileName();
		QString displayText = tr("&%1 %2").arg(i + 1).arg(displayFileName);
		m_vRecentFilesActions[i]->setText(displayText);
		m_vRecentFilesActions[i]->setData(filesList[i]);
		m_vRecentFilesActions[i]->setVisible(true);
	}
	// hide empty
	for (int i = filesList.size(); i < MAX_RECENT_FILES; ++i)
		m_vRecentFilesActions[i]->setVisible(false);
}

void Dyssol::SaveToFile(const QString& _sFileName) const
{
	if (_sFileName.isEmpty()) return;

	QApplication::setOverrideCursor(Qt::WaitCursor);

	m_pSavingWindow->SetFileName(_sFileName);
	m_pSavingWindow->show();
	m_pSavingWindow->raise();

	m_pSavingThread->SetFileName(_sFileName);
	m_pSavingThread->Run();
}

void Dyssol::LoadFromFile(const QString& _sFileName) const
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	m_pLoadingWindow->SetFileName(QString::fromStdWString(CH5Handler::DisplayFileName(std::filesystem::path{ _sFileName.toStdWString() }).wstring()));
	m_pLoadingWindow->show();
	m_pLoadingWindow->raise();

	m_pLoadingThread->SetFileName(_sFileName);
	m_pLoadingThread->Run();
}

void Dyssol::SetCurrFlowsheetFile(const QString& _fileName)
{
	const QString newFile = !_fileName.isEmpty() ? QFileInfo(_fileName).absoluteFilePath() : "";

	AddFileToRecentList(newFile);
	const QString sWinNamePrefix = QString(StrConst::Dyssol_MainWindowName);
	if (!newFile.isEmpty())
		setWindowTitle(sWinNamePrefix + " - " + QString::fromStdWString(CH5Handler::DisplayFileName(newFile.toStdWString()).wstring()) + "[*]");
	else
		setWindowTitle(sWinNamePrefix);
	if (!newFile.isEmpty())
		m_pSettings->setValue(StrConst::Dyssol_ConfigLastParamName, newFile);
}

void Dyssol::AddFileToRecentList(const QString& _fileToAdd)
{
	if (_fileToAdd.isEmpty()) return;
	QStringList filesList = m_pSettings->value(StrConst::Dyssol_ConfigRecentParamName).toStringList(); // get the list of recent files
	filesList.removeAll(_fileToAdd);                                                                   // remove all occurrences of the file itself
	if (filesList.size() == MAX_RECENT_FILES)                                                          // max amount of recent files reached
		filesList.pop_back();                                                                          // remove the oldest file
	filesList.push_front(_fileToAdd);                                                                  // put the file to the list as the first one
	m_pSettings->setValue(StrConst::Dyssol_ConfigRecentParamName, filesList);                          // write down updated list to the file
	UpdateMenu();
}

bool Dyssol::CheckAndAskUnsaved()
{
	if (m_Flowsheet.IsEmpty() || !IsFlowsheetModified()) return true;

	const QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::Cancel | QMessageBox::No;
	const QMessageBox::StandardButton reply = QMessageBox::question(this, StrConst::Dyssol_MainWindowName, StrConst::Dyssol_SaveMessageBoxText, buttons);
	if (reply == QMessageBox::Yes)
		if(!SaveAndWait())
			return false;

	return reply != QMessageBox::Cancel;
}

bool Dyssol::SaveAndWait()
{
	SaveFlowsheet();

	// wait until the end of saving
	QEventLoop loop;
	QTimer timer;
	connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
	while (m_pSavingThread->IsRunning())
	{
		timer.start(200);
		loop.exec();
	}
	return m_pSavingThread->IsSuccess();
}

void Dyssol::CloseDyssol(int _errCode /*= 0*/)
{
	QCoreApplication::exit(_errCode);
}

void Dyssol::SetFlowsheetModified(bool _bModified)
{
	m_bFlowsheetChanged = _bModified;
	setWindowModified(_bModified);
}

bool Dyssol::IsFlowsheetModified() const
{
	return m_bFlowsheetChanged;
}

void Dyssol::LoadMaterialsDatabase()
{
	const QVariant mdbPath = m_pSettings->value(StrConst::Dyssol_ConfigDMDBPath);
	if (!mdbPath.isValid()) return;
	m_MaterialsDatabase.LoadFromFile(mdbPath.toString().toStdWString());                         // try to load as from absolute path
	if (m_MaterialsDatabase.CompoundsNumber() == 0)                                              // loaded MDB is empty
	{
		const QString sPath = QCoreApplication::applicationDirPath() + "/" + mdbPath.toString(); // get relative path
		m_MaterialsDatabase.LoadFromFile(sPath.toStdWString());                                  // try to load as from relative path
		if (m_MaterialsDatabase.CompoundsNumber() != 0)
			m_pSettings->setValue(StrConst::Dyssol_ConfigDMDBPath, sPath);                       // save full path to config file
	}
	else
		m_pSettings->setValue(StrConst::Dyssol_ConfigDMDBPath, mdbPath.toString());              // save used path to config file
}

size_t Dyssol::OtherRunningDyssolCount()
{
#ifdef _MSC_VER
	return SystemFunctions::ActiveInstancesCount(StringFunctions::String2WString(std::string(StrConst::Dyssol_ApplicationName) + ".exe")) - 1;
#else
	// HACK
	// TODO: implement for Linux to allow proper cache cleaning
	return 1;
#endif
}

void Dyssol::setVisible(bool _visible)
{
	QMainWindow::setVisible(_visible);
	if (_visible)
		UpdateWholeView();
}

void Dyssol::LoadRecentFile()
{
	auto* pAction = qobject_cast<QAction*>(sender());
	if (!pAction) return;
	if (!CheckAndAskUnsaved()) return;
	const QString newFile = pAction->data().toString();
	if (newFile == QString::fromStdString(m_Flowsheet.GetFileName().string())) return;
	LoadFromFile(newFile);
	SetFlowsheetModified(false);
}

void Dyssol::NewFlowsheet()
{
	if (!CheckAndAskUnsaved()) return;
	SetFlowsheetModified(true);
	m_Flowsheet.Clear();
	SetCurrFlowsheetFile("");
	UpdateWholeView();
	emit NewFlowsheetLoaded();
}

void Dyssol::OpenFlowsheet()
{
	if (!CheckAndAskUnsaved()) return;
	const QString sFileName = QFileDialog::getOpenFileName(this, StrConst::Dyssol_DialogOpenName, QString::fromStdString(m_Flowsheet.GetFileName().string()), StrConst::Dyssol_DialogDflwFilter);
	if (sFileName.isEmpty()) return;
	LoadFromFile(sFileName);
	SetFlowsheetModified(false);
}

void Dyssol::SaveFlowsheet()
{
	if (!QString::fromStdString(m_Flowsheet.GetFileName().string()).isEmpty())
		SaveToFile(QString::fromStdString(m_Flowsheet.GetFileName().string()));
	else
		SaveFlowsheetAs();
}

void Dyssol::SaveFlowsheetAs()
{
	const QString sFileName = QFileDialog::getSaveFileName(this, StrConst::Dyssol_DialogSaveName, QString::fromStdString(m_Flowsheet.GetFileName().string()), StrConst::Dyssol_DialogDflwFilter);
	SaveToFile(sFileName);
}

void Dyssol::SaveScriptFile()
{
	const QString filePath = QString::fromStdWString(CH5Handler::DisplayFileName(m_Flowsheet.GetFileName()).wstring());
	const QString txtFileName = QFileInfo(filePath).absolutePath() + "/" + QFileInfo(filePath).baseName() + ".txt";
	const QString sFileName = QFileDialog::getSaveFileName(this, StrConst::Dyssol_DialogSaveConfigName, txtFileName, StrConst::Dyssol_DialogTxtFilter);
	QApplication::setOverrideCursor(Qt::WaitCursor);
	ScriptInterface::ExportScript(sFileName.toStdWString(), m_Flowsheet, m_ModelsManager, m_MaterialsDatabase);
	QApplication::restoreOverrideCursor();
}

void Dyssol::SavingFinished()
{
	if(m_pSavingThread->IsSuccess())
	{
		SetCurrFlowsheetFile(m_pSavingThread->GetFinalFileName());
		SetFlowsheetModified(false);
	}
	else
	{
		QString message = "Unable to save the flowsheet to the file:\n'" + m_pSavingThread->GetFileName();
		if (FileSystem::IsWriteProtected(FileSystem::FilePath(m_pSavingThread->GetFileName().toStdWString())))
			message += "'\nThe selected path may be write-protected.";
		QMessageBox::warning(this, StrConst::Dyssol_MainWindowName, message);
	}

	m_pSavingThread->Stop();
	m_pSavingWindow->accept();

	QApplication::restoreOverrideCursor();
}

void Dyssol::LoadingFinished()
{
	if (m_pLoadingThread->IsSuccess())
		SetCurrFlowsheetFile(m_pLoadingThread->GetFinalFileName());

	m_pLoadingThread->Stop();
	UpdateWholeView();
	emit NewFlowsheetLoaded();
	m_pLoadingWindow->accept();

	QApplication::restoreOverrideCursor();

	if (!m_pLoadingThread->IsSuccess())
		QMessageBox::warning(this, StrConst::Dyssol_MainWindowName, "Unable to load the selected file\n" + m_pLoadingThread->GetFileName());
}

void Dyssol::OpenHelp(const QString& _link) const
{
	QDesktopServices::openUrl(QUrl(StrConst::Dyssol_HelpURL + _link));
}

void Dyssol::ShowAboutDialog()
{
	CAboutWindow about(this);
	about.exec();
}

void Dyssol::SlotSaveAndReopen()
{
	SaveAndWait();
	const auto oldFile = m_Flowsheet.GetFileName();
	m_Flowsheet.Clear();
	LoadFromFile(QString::fromStdString(oldFile.string()));
}

void Dyssol::SlotRestart()
{
	if (CheckAndAskUnsaved())
		CloseDyssol(Dyssol::EXIT_CODE_REBOOT);
}

void Dyssol::SlotClearCache()
{
	if (!CheckAndAskUnsaved()) return;

	while (OtherRunningDyssolCount())
	{
		const auto reply = QMessageBox::warning(this, "Clear cache",
			tr("%1 other instances of Dyssol are still running. Close them to proceed.").arg(OtherRunningDyssolCount()),
			QMessageBox::Retry | QMessageBox::Cancel);
		if (reply == QMessageBox::Cancel) return;
	}

	ClearCache();
	CloseDyssol(EXIT_CODE_REBOOT);
}

void Dyssol::FlowsheetStateChanged()
{
	SetFlowsheetModified(true);
}

void Dyssol::BlockUI(bool _block) const
{
	for (int i = 0; i < ui.mainTabWidget->count(); ++i)
		if (i != SIMULATOR_TAB)
			ui.mainTabWidget->setTabEnabled(i, !_block);

	ui.menuFile->setEnabled(!_block);
	ui.menuFlowsheet->setEnabled(!_block);
	ui.menuModules->setEnabled(!_block);
	ui.menuTools->setEnabled(!_block);
}

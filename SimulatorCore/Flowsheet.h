/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "BaseModel.h"
#include "CalculationSequence.h"

class CFlowsheetParameters;

/* Stores the whole information about the flowsheet. */
class CFlowsheet
{
public:
	CFlowsheetParameters* m_pParams;
	std::vector<std::vector<CMaterialStream>> m_vvInitTearStreams;	// List of streams used as initial values for tear streams for each partition.

private:
	static const unsigned m_cnSaveVersion;

	CMaterialsDatabase* m_pMaterialsDatabase;
	CModelsManager* m_pModelsManager;
	std::vector<CBaseModel*> m_vpModels;
	std::vector<CMaterialStream*> m_vpStreams;
	CCalculationSequence m_calculationSequence{ &m_vpModels , &m_vpStreams };
	bool m_topologyModified; // Determines whether the flowsheet structure has been changed since the last topology analysis.

	std::vector<std::string> m_vPhasesNames;
	std::vector<unsigned> m_vPhasesSOA;

	std::vector<std::string> m_vCompoundsKeys; // keys of the chemical compounds which are used in this flowsheet
	CDistributionsGrid* m_pDistributionsGrid;
	double m_dSimulationTime;

public:
	CFlowsheet();
	~CFlowsheet();

	void InitializeFlowsheet();
	void Clear(); // removes all existing models
	bool AnalyzeTopology();
	void CreateInitTearStreams();
	void SetTopologyModified(bool _modified);

	void SetMaterialsDatabase( CMaterialsDatabase* _pNewDatabase );
	const CMaterialsDatabase* GetMaterialsDatabase() const;
	CDistributionsGrid* GetDistributionsGrid() const;
	void SetDistributionsGrid( /*CDistributionsGrid* _pNewGrid*/ );
	CModelsManager* GetModelsManager() const;
	void SetModelsManager(CModelsManager* _pModelsManager);
	const CCalculationSequence* GetCalculationSequence() const;
	CCalculationSequence* GetCalculationSequence();

	void SetSimulationTime( double _dSimulationTime );
	double GetSimulationTime();

	// Initializes flowsheet before simulation and checks for errors. On error returns error description, or empty string otherwise.
	std::string Initialize();
	// Checks connections of ports. On error returns error description, or empty string otherwise.
	std::string CheckConnections();
	// Sets pointers to all input and output streams directly to the units. Is called before simulation.
	void SetStreamsToPorts();

	void ClearSimulationResults();
	bool Empty() const;

	// ========== Functions to work with PHASES

	void AddPhase( const std::string& _sName, unsigned _nAggregationState );
	void RemovePhase( unsigned _nIndex );
	void ChangePhase( unsigned _nIndex, const std::string& _sName, unsigned _nAggregationState );
	unsigned GetPhasesNumber();
	const std::vector<std::string>& GetPhasesNames() const;
	std::string GetPhaseName( unsigned _nIndex );
	std::vector<unsigned>* GetPhasesAggregationStates();
	int GetPhaseAggregationState( unsigned _nIndex );
	void ClearPhases();
	bool IsPhaseDefined( unsigned _nSOA );
	int GetPhaseIndex( unsigned _nSOA ); // returns index of the specified phase, -1 if no such phase has been defined

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// ========== Functions to work with COMPOUNDS

	size_t GetCompoundsNumber() const;									// Returns the number of defined compounds.
	void AddCompound(const std::string& _sCompoundKey);					// Adds new compound with the specified unique key to the flowsheet.
	void RemoveCompound(const std::string& _sCompoundKey);				// Remove compound with the specified unique key from the flowsheet.
	std::vector<std::string> GetCompounds() const;						// Returns unique keys of all defined compounds.
	std::vector<std::string> GetCompoundsNames() const;					// Returns names of all defined compounds.
	std::string GetCompoundName(size_t _iCompound) const;				// Returns name of the specified compound.
	std::string GetCompoundKey(size_t _iCompound) const;				// Returns unique key of the compound with the specified index.
	size_t GetCompoundIndex(const std::string& _sCompoundKey) const;	// Returns index of the specified compound. If no such compound was defined, returns -1.

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// ========== Interface functions to work with MODELS

	size_t GetModelsCount() const;										// Returns the number of defined models.
	CBaseModel* AddModel(const std::string& _modelKey = "");			// Adds new model to the flowsheet and returns pointer to it.
	void DeleteModel(const std::string& _sModelKey);					// Removes model with the specified unique key.
	const CBaseModel* GetModel(size_t _index) const;					// Returns model with the specified index. If no such model was defined, returns nullptr.
	CBaseModel* GetModel(size_t _index);								// Returns model with the specified index. If no such model was defined, returns nullptr.
	const CBaseModel* GetModel(const std::string& _sModelKey) const;	// Returns model with specified unique key. If no such model was defined, returns nullptr.
	CBaseModel* GetModel(const std::string& _sModelKey);				// Returns model with specified unique key. If no such model was defined, returns nullptr.
	void ShiftModelUp(const std::string& _sModelKey);					// Moves upwards model with the specified unique key.
	void ShiftModelDown(const std::string& _sModelKey);					// Moves downwards model with the specified unique key.
	size_t GetModelIndex(const std::string& _sModelKey) const;			// Returns index of the model with the specified unique key. If no such model was defined, returns -1.
	void InitializeModel(const std::string& _sModelKey);				// Initializes model with the specified unique key.

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// ========== Interface functions to work with MATERIAL STREAMS

	size_t GetStreamsCount() const;											// Returns the number of defined material streams.
	CMaterialStream* AddStream(const std::string& _streamKey = "");		// Adds new stream to the flowsheet and returns pointer to it.
	void DeleteStream(const std::string& _sStreamKey);						// Removes material stream with the specified unique key.
	const CMaterialStream* GetStream(size_t _index) const;					// Returns material stream with the specified index. If no such stream was defined, returns nullptr.
	CMaterialStream* GetStream(size_t _index);								// Returns material stream with the specified index. If no such stream was defined, returns nullptr.
	const CMaterialStream* GetStream(const std::string& _sStreamKey) const;	// Returns stream with the specified unique key. If no such stream was defined, returns nullptr.
	CMaterialStream* GetStream(const std::string& _sStreamKey);				// Returns stream with the specified unique key. If no such stream was defined, returns nullptr.
	void ShiftStreamUp(const std::string& _sStreamKey);						// Moves upwards material stream with the specified unique key.
	void ShiftStreamDown(const std::string& _sStreamKey);					// Moves downwards material stream with the specified unique key.
	size_t GetStreamIndex(const std::string& _sStreamKey) const;			// Returns index of the material stream with the specified unique key. If no such stream was defined, returns -1.

	// ========== SAVE/LOAD flowsheet from the files

	bool SaveToFile(CH5Handler& _h5Saver, const std::wstring& _sFileName);
	bool LoadFromFile(CH5Handler& _h5Loader, const std::wstring& _sFileName);
	void LoadInitTearStreamsOld(CH5Handler& _h5Loader);

	void SaveConfigFile(const std::wstring& _fileName, const std::wstring& _flowsheetFile) const;

private:
	std::string GenerateUniqueModelKey(const std::string& _key = "") const;		// Generates a unique key for a model.
	std::string GenerateUniqueStreamKey(const std::string& _key = "") const;	// Generates a unique key for a stream.
	void EnsureUniqueModelsKeys();	// Checks keys of models and replaces them if they have duplicates.
	void EnsureUniqueStreamsKeys();	// Checks keys of streams and replaces them if they have duplicates.
};

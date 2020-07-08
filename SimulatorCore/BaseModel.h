/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ModelsManager.h"

/* The main class which is used to describe any dynamic model */
class CBaseModel
{
private:
	static const unsigned m_cnSaveVersion;

	std::string m_sModelKey; // the unique identifier of this model
	std::string m_sModelName;
	std::wstring m_sCachePath;
	bool m_bCacheEnabled;
	unsigned m_nCacheWindow;
	CModelsManager* m_pModelsManager;
	CBaseUnit* m_pUnit;
	std::vector<CExternalSolver*> m_vExternalSolvers;

public:
	CBaseModel(CModelsManager* _pModelsManager, const std::string& _sModelKey = ""); // the key can be already specified for this unit
	~CBaseModel();

	std::string GetModelKey() const{ return m_sModelKey; } // returns the key of the unit
	void SetModelKey( const std::string &_sNewModelKey );
	std::string GetModelName() const{ return m_sModelName; }
	void SetModelName(std::string _sModelName);

	std::string GetUnitKey() const; // get the unique key of the unit which is accorded to this model
	std::string GetUnitName() const; // returns the name of the unit

	// Sets new unit with specified key. if the unit has been previously created, it is removed and new instance is created.
	void SetUnit(const std::string& _sUnitKey);
	// Returns pointer to set unit.
	const CBaseUnit* GetUnit() const;

	std::vector<sPortStruct> GetUnitPorts();
	void SetPortStreamKey( unsigned _nPortIndex, std::string _sStreamKey );
	void SetPortStream( unsigned _nPortIndex, CMaterialStream* _pMaterialStream );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Functions to work with HOLDUPS

	size_t GetHoldupsCount() const;
	const CHoldup* GetHoldup(size_t _index) const;
	CHoldup* GetHoldup(size_t _index);
	const CHoldup* GetHoldup(const std::string& _sKey) const;
	CHoldup* GetHoldup(const std::string& _sKey);
	const CHoldup* GetHoldupInit(size_t _index) const;
	CHoldup* GetHoldupInit(size_t _index);

	// Removes time points, which are closer as _dStep, from internal holdups and streams within the specified interval [_dStart; _dEnd).
	void ReduceTimePoints(double _dStart, double _dEnd, double _dStep);

	/** Sets pointer to a compounds vector.*/
	void SetCompounds( const std::vector<std::string>* _pvCompoundsKeys );
	void SetCompoundsPtr( const std::vector<std::string>* _pvCompoundsKeys );
	/** Sets pointer to a compounds vector.*/
	void SetPhases( const std::vector<std::string>* _pvPhasesNames, const std::vector<unsigned>* _pvPhasesSOAs );
	void SetPhasesPtr( const std::vector<std::string>* _pvPhasesNames, const std::vector<unsigned>* _pvPhasesSOAs );
	/**	Sets absolute tolerance.*/
	void SetAbsTolerance( double _dATol );
	/**	Set relative tolerance.*/
	void SetRelTolerance( double _dRTol );
	/** Sets pointer to a distributions grid of solids.*/
	void SetDistributionsGrid( const CDistributionsGrid* _pGrid );
	/** Set database of materials.*/
	void SetMaterialsDatabase( const CMaterialsDatabase* _pDatabase );

	void AddPhase( std::string _sName, unsigned _nAggrState );
	void RemovePhase( unsigned _nIndex );
	void ChangePhase( unsigned _nIndex, std::string _sName, unsigned _nAggrState );

	void AddCompound( std::string _sCompoundKey );
	void RemoveCompound( std::string _sCompoundKey );

	// function to calculate this model on specified time interval
	void Simulate( double _dStartTime, double _dEndTime );
	void Simulate( double _dTime );
	void SaveInternalState(double _dT1, double _dT2);
	void LoadInternalState();
	void Initialize( double _dTime );
	void Finalize();

	/** Save model to file.*/
	void SaveToFile( CH5Handler& _h5Saver, const std::string& _sPath );
	/** Load model from file*/
	void LoadFromFile( CH5Handler& _h5Loader, const std::string& _sPath );


	//////////////////////////////////////////////////////////////////////////
	/// Functions to work with UNIT PARAMETERS

	/** Returns a pointer to unit parameters manager. */
	CUnitParametersManager* GetUnitParametersManager() const;

	// functions to work with state variables
	unsigned GetStoredStateVariablesNumber();
	std::string GetStoredStateVariableName( unsigned _nIndex );
	bool GetStoredStateVariableData( unsigned _nIndex, std::vector<double>* _pvValues, std::vector<double>* _pvTimes );

	bool IsDynamic();

	std::vector<double> GetAllInletTimePoints( double _dStartTime, double _dEndTime, bool _bForceStartBoundary = false, bool _bForceEndBoundary = false );

	void ClearSimulationResults();

	bool CheckError();
	std::string GetErrorDescription();
	void ClearError();
	bool CheckWarning();
	std::string GetWarningDescription();
	void ClearWarning();
	bool CheckInfo();
	std::string GetInfoDescription();
	void ClearInfo();


	void SetCachePath(const std::wstring& _sPath);
	void SetCacheParams( bool _bEnabled, unsigned _nWindow );

	void SetMinimalFraction( double _dFraction );

	//////////////////////////////////////////////////////////////////////////
	/// Functions to work with plots
	//////////////////////////////////////////////////////////////////////////

	unsigned GetPlotsNumber() const;
	unsigned GetCurvesNumber(unsigned _nPlot) const;
	std::string GetPlotName(unsigned _nPlot) const;
	std::string GetPlotXAxisName(unsigned _nPlot) const;
	std::string GetPlotYAxisName(unsigned _nPlot) const;
	std::string GetPlotZAxisName(unsigned _nPlot) const;
	std::string GetCurveName(unsigned _nPlot, unsigned _nCurve) const;
	std::vector<double> GetCurveX(unsigned _nPlot, unsigned _nCurve) const;
	std::vector<double> GetCurveY(unsigned _nPlot, unsigned _nCurve) const;
	double GetCurveZ(unsigned _nPlot, unsigned _nCurve) const;
	bool IsPlot2D(unsigned _nPlot);

	//////////////////////////////////////////////////////////////////////////
	/// Functions to work with solvers
	//////////////////////////////////////////////////////////////////////////

	std::string InitializeExternalSolvers();
	void ClearExternalSolvers();
};

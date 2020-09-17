/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitParameters.h"
#include "AgglomerationSolver.h"
#include "PBMSolver.h"

#ifdef _DEBUG
#define DYSSOL_CREATE_MODEL_FUN CreateDYSSOLUnitV2_DEBUG
#define DYSSOL_CREATE_MODEL_FUN_NAME "CreateDYSSOLUnitV2_DEBUG"
#else
#define DYSSOL_CREATE_MODEL_FUN CreateDYSSOLUnitV2
#define DYSSOL_CREATE_MODEL_FUN_NAME "CreateDYSSOLUnitV2"
#endif

enum EPortType
{
	INPUT_PORT		= 0,
	OUTPUT_PORT		= 1,
	UNDEFINED_PORT	= 2
};

/** Description of the unit's port.*/
struct sPortStruct
{
	std::string sName;			///< The name of the port, should be unique for this port
	std::string sStreamKey;		///< The key of the stream which connected to this port
	CStream* pStream;	///< Pointer to the material stream - updated during flowsheet initialization
	EPortType nType;			///< INPUT_PORT or OUTPUT_PORT
};

/** Description of the state variable.*/
struct sStateVariable
{
	double dValue;						///< Current value of internal time dependent variable
	std::string sName;					///< Name of the variable
	bool bIsSaved;						///< True - history of value changes should be saved
	std::vector<double> vHistoryTimes;	///< Memory for storing history of time points
	std::vector<double> vHistoryValues;	///< Memory for storing history of values
	double dSavedValue;					///< Memory for temporary storing variable
};

/**	Basic class for dynamic and steady-state units*/
class CBaseUnit
{
private:
	static const unsigned m_cnSaveVersion;

	// ========== Internal data and structures
	std::vector<sPortStruct> m_vPorts;						///< All ports of the unit
	CUnitParametersManager m_unitParameters;				///< Handler of unit parameters.
	std::vector<CHoldup*> m_vHoldupsInit;					///< Holdups in this unit, will be displayed in UI if holdups editor. Will not be changed during the simulation.
	std::vector<CHoldup*> m_vHoldupsWork;					///< Working copy of holdups, take part in simulation, will be displayed in results.
	int m_nPermanentHoldups;								///< Is used to distinguish between holdups which have been added in constructor (permanent) and elsewhere (temp)
	std::vector<CStream*> m_vStreams;				///< Internal material streams.
	int m_nPermanentStreams;								///< Is used to distinguish between material streams which have been added in constructor (permanent) and elsewhere (temp)
	std::vector<sStateVariable*> m_vStateVariables;			///< State variables of the unit
	bool m_bIsDynamic;										///< Contains true - if this unit is defined as dynamic, false - as steady-state
	std::vector<CHoldup*> m_vStoreHoldupsWork;				///< Used to store data from m_vHoldupsWork for cyclic calculations
	std::vector<CStream*> m_vStoreStreams;			///< Used to store data from m_vStreams for cyclic calculations
	double m_dStoreT1, m_dStoreT2;							///< Stored time window in m_vStoreHoldupsWork and m_vStoreStreams

	// ========== Variables to work with errors and warnings
	bool m_bError;						///< Contains true if an error was detected
	bool m_bWarning;					///< Contains true if an warning was detected
	bool m_bInfo;						///< Contains true if an info tag was detected
	std::string m_sErrorDescription;	///< Description of the last detected error
	std::string m_sWarningDescription;	///< Description of the last detected warning
	std::string m_sInfoDescription;		///< Description of the last info

	// ========== Cache parameters
	std::wstring m_sCachePath;
	bool m_bCacheEnabled;
	unsigned m_nCacheWindow;

	double m_dMinFraction;

protected:
	// ========== Basic info of the unit
	std::string m_sUnitName;	///< Name of the unit
	std::string m_sUniqueID;	///< Unique identifier of the unit
	std::string m_sAuthorName;	///< Name of the unit's author
	double m_dUnitVersion;		///< Version of the unit

	// ========== Pointers to constants - same for all units
	const std::vector<std::string>* m_pvCompoundsKeys;	///< Keys of the chemical compounds which contains this unit
	const std::vector<std::string>* m_pvPhasesNames;	///< Names of phases
	const std::vector<unsigned>* m_pvPhasesSOA;			///< Aggregation states of phases
	const CDistributionsGrid* m_pDistributionsGrid;		///< Pointer to a grid with size classes of MD parameters
	const CMaterialsDatabase* m_pMaterialsDB;			///< Pointer to a database of materials
	double m_dATol;										///< Value of an absolute tolerance
	double m_dRTol;										///< Value of a relative tolerance

	// ========== Variable info of the unit
	std::map<ECompoundTPProperties, CLookupTable> m_vLookupTables;	///< Map with all lookup tables for fast use

public:
	unsigned m_nCompilerVer;

	/** Basic constructor.*/
public:		CBaseUnit(void);
	/**	Basic destructor.*/
public:		virtual ~CBaseUnit(void) = 0;

	// ========== Functions to work with the BASIC INFO of the unit

	/** Returns name of the unit.*/
public:		std::string GetUnitName() const;
	/**	Returns name of the unit's author.*/
public:		std::string GetAuthorName() const;
	/**	Returns version of this unit.*/
public:		double GetUnitVersion() const;
	/**	Returns unique identifier of the unit.*/
public:		std::string GetUniqueID() const;

	// ========== Functions to work with PORTS

	/**	Returns all ports of the unit.*/
public:		std::vector<sPortStruct> GetPorts() const;
	/**	Returns number of ports of the unit.*/
public:
	unsigned GetPortsNumber() const;
	/** Returns type of the port (INPUT_PORT or OUTPUT_PORT). If port with such index hasn't been defined, UNDEFINED_PORT will be returned.
	 *	\param _nPortIndex Index of the port*/
public:		unsigned GetPortType( unsigned _nPortIndex ) const;
	/** Returns type of the port (INPUT_PORT or OUTPUT_PORT). If port with such index hasn't been defined, UNDEFINED_PORT will be returned.
	 *	\param _sPortName Name of the port*/
public:		unsigned GetPortType( const std::string &_sPortName ) const;
	/** Returns the key of the stream which is connected to the port. If port with such index hasn't been defined, empty string will be returned.
	 *	\param _nPortIndex Index of the port*/
public:		std::string GetPortStreamKey( unsigned _nPortIndex ) const;
	/** Returns pointer to a stream which is connected to the port. If port with such index hasn't been defined, NULL will be returned.
	 *	\param _nPortIndex Index of the port*/
public:		CStream* GetPortStream( unsigned _nPortIndex ) const;
	/** Returns pointer to a stream which is connected to the port. If port with such index hasn't been defined, NULL will be returned.
	 *	\param _sPortName Name of the port*/
public:		CStream* GetPortStream( const std::string &_sPortName ) const;
	/**	Sets key of the stream for specified port.
	 *	\param _nPortIndex Index of the port
	 *	\param _sMaterialKey Unique key of the stream*/
public:		void SetPortStreamKey( unsigned _nPortIndex, std::string _sStreamKey );
	/**	Sets stream to the port.
	 *	\param _nPortIndex Index of the port
	 *	\param _pStream Pointer to a stream*/
public:		void SetPortStream( unsigned _nPortIndex, CStream* _pStream );

	/** Adds port to the unit. This function is called from the constructors of the child units.
	 *  If there is already a port with the same name within the unit, logic_error exception will be thrown, and index of this existing port returned.
	 *	\param _sPortName Name of the new port
	 *	\param _nPortType Port type: INPUT_PORT or OUTPUT_PORT
	 *	\return Index of the port*/
protected:	unsigned AddPort( const std::string &_sPortName, EPortType _nPortType );


	// ========== Functions to work with HOLDUPS, FEEDS and MATERIALSTREAMS ====================

				/**	Get all initial holdups of the unit.*/
public:		std::vector<CHoldup*> GetHoldupsInit() const;
				/**	Get all holdups of the unit.*/
public:		std::vector<CHoldup*> GetHoldups() const;

	/**	Adds new holdup to the unit. User can add holdups into the unit during creation of the unit (in the constructor) or inside of it.
	 *	The structure of the holdup will be the same as the global structure (phases, grids, compounds).
	 *  If there is already a holdup with the same name within the unit, it will be returned.
	 *	If the key is equal to one of the existing holdups within the unit, it will be replaced with a unique one.
	 *	\param _sHoldupName Name of the new holdup
	 *	\param _sStreamKey Unique key of the holdup
	 *	\return Pointer to a holdup object*/
protected:	CHoldup* AddHoldup( const std::string &_sHoldupName, const std::string &_sStreamKey = "" );
public:		CHoldup* GetHoldup( const std::string &_sHoldupName );
	/**	Adds new feed to the unit. User can add feeds into the unit (in the constructor) during creation of the unit.
	 *	The structure of the feed will be the same as the global structure (phases, grids, compounds).
	 *  If there is already a feed with the same name within the unit, logic_error exception will be thrown, and pointer to this existing feed will be returned.
	 *	If the key is equal to one of the existing feeds within the unit, it will be replaced with a unique one.
	 *	\param _sFeedName Name of the new feed
	 *	\param _sStreamKey Unique key of the feed
	 *	\return Pointer to a feed object*/
protected:	CStream* AddFeed( const std::string &_sFeedName, const std::string &_sStreamKey = "" );
public:		CStream* GetFeed( const std::string &_sFeedName );

	/** Removes holdup from the unit.*/
public:	void RemoveHoldup( CHoldup* _pHoldup );
	/** Removes holdup from the unit.*/
public:	void RemoveHoldup( const std::string &_sName );
private:	void InitializeHoldups();
private:	void RemoveTempHoldups();

	/**	Adds new material stream to the unit. User can add material streams into the unit during creation of the unit (in the constructor) or inside of it.
	 *	The structure of the material stream will be the same as the global structure (phases, grids, compounds).
	 *  If there is already a material stream with the same name within the unit, it will be returned.
	 *	If the key is equal to one of the existing material streams within the unit, it will be replaced with a unique one.
	 *	\param _sStreamName Name of the new material stream
	 *	\param _sStreamKey Unique key of the material stream
	 *	\return Pointer to a material stream object*/
public:		CStream* AddMaterialStream( const std::string &_sStreamName, const std::string &_sStreamKey = "" );
public:		CStream* GetMaterialStream( const std::string &_sStreamName );
public:		void RemoveMaterialStream( CStream* _pStream );
public:		void RemoveMaterialStream( const std::string &_sStreamName );
private:	void InitializeMaterialStreams();
private:	void RemoveTempMaterialStreams();

private:	void ClearHoldups();
private:	void ClearMaterialStreams();

			/** Set grid, phases, compounds, etc.*/
private:	void SetupStream( CBaseStream* _pStream );
			/*Returns vector of keys of all defined holdups in this unit.*/
private:	std::vector<std::string> GetHoldupsKeys() const;
private:	std::vector<std::string> GetMaterialStreamsKeys() const;

public:
	/** Performs a heat exchange of the material streams _pStream1 and _pStream2 at specified time point with a specified efficiency (0 <= _dEfficiency <= 1).
	*	\param _pStream1 Pointer to stream1
	*	\param _pStream2 Pointer to stream2
	*	\param _dTime Time point that should be copied
	*	\param _dEfficiency Efficiency of heat exchange, i.e. 0 - stream temperatures remain unchanged and 1 - both streams leave with the same temperature*/
	void HeatExchange(CStream* _pStream1, CStream* _pStream2, double _dTime, double _dEfficiency);

	// ========== Functions for STREAMS MANIPULATIONS ====================

	/** Copies data from one stream to another on a specified time point.
	 *	\param _srcStream Pointer to a source stream
	 *	\param _dstStream Pointer to a destination stream
	 *	\param _dTime Time point*/
public:		void CopyStreamToStream( const CStream* _pSrcStream, CStream* _pDstStream, double _dTime, bool _bDeleteDataAfter = true );
	/** Copies data from one stream to another on a specified time interval.
	 *	\param _srcStream Pointer to a source stream
	 *	\param _dstStream Pointer to a destination stream
	 *	\param _dStartTime Start of the time interval
	 *	\param _dEndTime End of the time interval*/
public:		void CopyStreamToStream( const CStream* _pSrcStream, CStream* _pDstStream, double _dStartTime, double _dEndTime, bool _bDeleteDataAfter = true );
	/** Copies stream data to output port.
	 *	\param _pStream Pointer to a stream
	 *	\param _nPortIndex Index of the port
	 *	\param _dTime Time point*/
public:		void CopyStreamToPort( const CStream* _pStream, unsigned _nPortIndex, double _dTime, bool _bDeleteDataAfter = true );
public:		void CopyStreamToPort( const CStream* _pStream, const std::string &_sPortName, double _dTime, bool _bDeleteDataAfter = true );
	/** Copies stream data to output port.
	 *	\param _pStream Pointer to a stream
	 *	\param _nPortIndex Index of the port
	 *	\param _dStartTime Start of the time interval
	 *	\param _dEndTime End of the time interval*/
public:		void CopyStreamToPort( const CStream* _pStream, unsigned _nPortIndex, double _dStartTime, double _dEndTime, bool _bDeleteDataAfter = true );
public:		void CopyStreamToPort( const CStream* _pStream, const std::string &_sPortName, double _dStartTime, double _dEndTime, bool _bDeleteDataAfter = true );
	/** Copies stream data of the input port to another stream.
	 *	\param _pStream Pointer to a stream
	 *	\param _nPortIndex Index of the port
	 *	\param _dTime Time point*/
public:		void CopyPortToStream( unsigned _nPortIndex, CStream* _pStream, double _dTime, bool _bDeleteDataAfter = true );
public:		void CopyPortToStream( const std::string &_sPortName, CStream* _pStream, double _dTime, bool _bDeleteDataAfter = true );
	/** Copies stream data of the input port to another stream.
	 *	\param _pStream Pointer to a stream
	 *	\param _nPortIndex Index of the port
	 *	\param _dStartTime Start of the time interval
	 *	\param _dEndTime End of the time interval*/
public:		void CopyPortToStream( unsigned _nPortIndex, CStream* _pStream, double _dStartTime, double _dEndTime, bool _bDeleteDataAfter = true );
public:		void CopyPortToStream( const std::string &_sPortName, CStream* _pStream, double _dStartTime, double _dEndTime, bool _bDeleteDataAfter = true );

	// ========== VIRTUAL FUNCTIONS which should be overridden in child classes

	/** Calculates unit on specified time interval (for dynamic units).
	 *	\param _dStartTime Start of the time interval
	 *	\param _dEndTime End of the time interval*/
public:		virtual void Simulate( double _dStartTime, double _dEndTime );
	/**	Calculates unit on a time point (for steady-state units).
	 *	\param _dTime Time point to calculate*/
public:		virtual void Simulate( double _dTime );
	/**	Initializes unit for a specified time point before starting a simulation.*/
public:		virtual void Initialize( double _dTime );
	/** Saves time dependent parameters. Here must be saved all time dependent variables, which are not added with help of AddStateVariable() function.*/
public:		virtual void SaveState();
	/** Loads time dependent parameters. Here must be loaded all time dependent variables, which are not added with help of AddStateVariable() function.*/
public:		virtual void LoadState();
			/// Is called at the end of each simulation.
public:		virtual void Finalize();

	// ========== Functions to work with TIME POINT

	/** Returns all time points for specified time interval on which input streams and input time dependent parameters are defined.*/
public:		std::vector<double> GetAllDefinedTimePoints( double _dStartTime, double _dEndTime, bool _bForceStartBoundary = false, bool _bForceEndBoundary = false ) const;
	/** Returns all time points for time interval on which inlet streams are defined.*/
public:		std::vector<double> GetAllInputTimePoints( double _dStartTime, double _dEndTime, bool _bForceStartBoundary = false, bool _bForceEndBoundary = false ) const;
	/** Returns all time points for specified time interval on which _vSrteams are defined.*/
public:		std::vector<double> GetAllStreamsTimePoints( const std::vector<CStream*>& _vSrteams, double _dStartTime, double _dEndTime ) const;

			// Removes time points, which are closer as _dStep, from internal holdups and streams within the specified interval [_dStart; _dEnd).
public:		void ReduceTimePoints(double _dStart, double _dEnd, double _dStep);

//////////////////////////////////////////////////////////////////////////
/// Functions to work with UNIT PARAMETERS

	/** Returns a const reference to unit parameters manager. */
public:		const CUnitParametersManager& GetUnitParametersManager() const;
	/** Returns a reference to unit parameters manager. */
public:		CUnitParametersManager& GetUnitParametersManager();

	/** Returns value of the constant unit parameter. Throws logic_error exception if constant unit parameter with given name does not exist.
	*	\param _name Name of the constant unit parameter.
	*	\return Value of the constant unit parameter.*/
public:		double GetConstParameterValue(const std::string& _name) const;
	/** Returns value of the time-dependent unit parameter at the specified time point. Throws logic_error exception if time-dependent unit parameter with given name does not exist.
	*	\param _name Name of the time-dependent unit parameter.
	*	\param _time Time point.
	*	\return Value of the time-dependent unit parameter at the specified time point.*/
public:		double GetTDParameterValue(const std::string& _name, double _time) const;
	/** Returns value of the string unit parameter. Throws logic_error exception if string unit parameter with given name does not exist.
	*	\param _name Name of the string unit parameter.
	*	\return Value of the string unit parameter.*/
public:		std::string GetStringParameterValue(const std::string& _name) const;
	/** Returns value of the check box unit parameter. Throws logic_error exception if check box unit parameter with given name does not exist.
	*	\param _name Name of the check box unit parameter.
	*	\return Value of the check box unit parameter.*/
public:		bool GetCheckboxParameterValue(const std::string& _name) const;
	/** Returns value of the combo unit parameter. Throws logic_error exception if combo unit parameter with given name does not exist.
	*	\param _name Name of the combo unit parameter.
	*	\return Selected value of the combo unit parameter.*/
public:		size_t GetComboParameterValue(const std::string& _name) const;
	/** Returns value of the group unit parameter. Throws logic_error exception if group unit parameter with given name does not exist.
	*	\param _name Name of the group unit parameter.
	*	\return Selected value of the group unit parameter.*/
public:		size_t GetGroupParameterValue(const std::string& _name) const;
	/** Returns value of the compound unit parameter. Throws logic_error exception if compound unit parameter with given name does not exist.
	*	\param _name Name of the compound unit parameter.
	*	\return Compound's key selected in compound unit parameter.*/
public:		std::string GetCompoundParameterValue(const std::string& _name) const;
	/** Returns pointer to a 'typeless' solver with specified name. Is used internally in all GetSolver*() functions. Throws logic_error exception if solver unit parameter with given name does not exist.
	*	\param _name Name of the solver unit parameter.
	*	\return Pointer to a solver.*/
private:	CBaseSolver* GetSolverParameterValue(const std::string& _name) const;
	/** Returns pointer to a agglomeration solver of a type SOLVER_AGGLOMERATION_1 with the specified name. Throws logic_error exception if solver unit parameter with given name does not exist.
	*	\param _name Name of the agglomeration solver unit parameter.
	*	\return Pointer to an agglomeration solver.*/
public:		CAgglomerationSolver* GetSolverAgglomeration(const std::string& _name) const;
	/** Returns pointer to a PBM solver of a type SOLVER_PBM_1 with the specified name. Throws logic_error exception if solver unit parameter with given name does not exist.
	*	\param _name Name of the PBM solver unit parameter.
	*	\return Pointer to an PBM solver.*/
public:		CPBMSolver* GetSolverPBM(const std::string& _name) const;

			/** Adds new constant unit parameter. Should be used in unit's constructor only.
			*	If there is already a unit parameter with the same name within the unit, logic_error exception is thrown.
			*	\param _name Parameter name.
			*	\param _minValue Minimum allowed value.
			*	\param _maxValue Maximum allowed value.
			*	\param _initValue Initial value.
			*	\param _units Measurement units.
			*	\param _description Text description of the parameter.*/
public:		void AddConstParameter(const std::string& _name, double _minValue, double _maxValue, double _initValue, const std::string& _units, const std::string& _description);
			/** Adds new time dependent unit parameter. Should be used in unit's constructor only.
			*	If there is already a unit parameter with the same name within the unit, logic_error exception is thrown.
			*	\param _name Parameter name.
			*	\param _minValue Minimum allowed value.
			*	\param _maxValue Maximum allowed value.
			*	\param _initValue Initial value for time point 0s.
			*	\param _units Measurement units.
			*	\param _description Text description of the parameter.*/
public:		void AddTDParameter(const std::string& _name, double _minValue, double _maxValue, double _initValue, const std::string& _units, const std::string& _description);
			/** Adds new string unit parameter. Should be used in unit's constructor only.
			*	If there is already a unit parameter with the same name within the unit, logic_error exception is thrown.
			*	\param _name Parameter name.
			*	\param _initValue Initial value.
			*	\param _description Text description of the parameter.*/
public:		void AddStringParameter(const std::string& _name, const std::string& _initValue, const std::string& _description);
			/** Adds new check box unit parameter. Should be used in unit's constructor only.
			*	If there is already a unit parameter with the same name within the unit, logic_error exception is thrown.
			*	\param _name Parameter name.
			*	\param _initValue Initial value.
			*	\param _description Text description of the parameter.*/
public:		void AddCheckBoxParameter(const std::string& _name, bool _initValue, const std::string& _description);
			/** Adds new combo unit parameter. Should be used in unit's constructor only.
			*	If there is already a unit parameter with the same name within the unit, logic_error exception is thrown.
			*	\param _name Parameter name.
			*	\param _initValue Initial value.
			*	\param _values A set of values to choose from, or an empty vector for default values.
			*	\param _valuesNames Names of values to choose from.
			*	\param _description Text description of the parameter.*/
public:		void AddComboParameter(const std::string& _name, size_t _initValue, const std::vector<size_t>& _values, const std::vector<std::string>& _valuesNames, const std::string& _description);
			/** Adds new group unit parameter. Similar to combo parameter, but additionally allows showing/hiding other parameters. Should be used in unit's constructor only.
			*	If there is already a unit parameter with the same name within the unit, logic_error exception is thrown.
			*	\param _name Parameter name.
			*	\param _initValue Initial value.
			*	\param _values A set of values to choose from, or an empty vector for default values.
			*	\param _valuesNames Names of values to choose from.
			*	\param _description Text description of the parameter.*/
public:		void AddGroupParameter(const std::string& _name, size_t _initValue, const std::vector<size_t>& _values, const std::vector<std::string>& _valuesNames, const std::string& _description);
			/** Adds new compound unit parameter. Should be used in unit's constructor only.
			*	If there is already a unit parameter with the same name within the unit, logic_error exception is thrown.
			*	\param _name Parameter name.
			*	\param _description Text description of the parameter.*/
public:		void AddCompoundParameter(const std::string& _name, const std::string& _description);
			/** Adds new solver unit parameter of the specified type. Allows to choose one of available solvers of the specified type. Should be used in unit's constructor only.
			*	If there is already a unit parameter with the same name within the unit, logic_error exception is thrown.
			*	\param _name Parameter name.
			*	\param _solverType Type of the solver (SOLVER_AGGLOMERATION_1).
			*	\param _description Text description of the parameter.*/
private:	void AddSolverParameter(const std::string& _name, ESolverTypes _solverType, const std::string& _description);
			/** Adds new solver unit parameter with type SOLVER_AGGLOMERATION_1. Allows to choose one of available solvers of the specified type. Should be used in unit's constructor only.
			*	If there is already a unit parameter with the same name within the unit, logic_error exception is thrown.
			*	\param _name Parameter name.
			*	\param _description Text description of the parameter.*/
public:		void AddSolverAgglomeration(const std::string& _name, const std::string& _description);
			/** Adds new solver unit parameter with type SOLVER_PBM_1. Allows to choose one of available solvers of the specified type. Should be used in unit's constructor only.
			*	If there is already a unit parameter with the same name within the unit, logic_error exception is thrown.
			*	\param _name Parameter name.
			*	\param _description Text description of the parameter.*/
public:		void AddSolverPBM(const std::string& _name, const std::string& _description);

			/** Adds unit parameters with names _paramNames to the group _groupName of group parameter _groupParamName.
			 * Group parameter, the group within and all adding parameters must already exist. If something not exist, logic_error exception is thrown.
			 *	\param _groupParamName Name of the group parameter.
			 *	\param _groupName Name of the group within the group parameter.
			 *	\param _paramNames Names of parameters to add.*/
public:		void AddParametersToGroup(const std::string& _groupParamName, const std::string& _groupName, const std::vector<std::string>& _paramNames);


	// ========== Functions to work with STATE VARIABLES

	/** Get number of time dependent internal variables. Used for UI.*/
public:		unsigned GetStoredSVNumber() const;
	/** Get name of internal variable. Used for UI.
	 *	\param _nIndex Index among stored internal variables.*/
public:		std::string GetStoredSVName( unsigned _nIndex ) const;
	/** Get all data of internal variable. Used for UI.
	 *	\param _nIndex Index among stored internal variables
	 *	\param _pvValues Pointer to a vector of values of internal variable
	 *	\param _pvTimes Pointer to vector of time points
	 *	\retval true Success*/
public:		bool GetStoredSVData( unsigned _nIndex, std::vector<double>& _vTimes, std::vector<double>& _vValues ) const;
	/** Returns value of internal variable.
	 *	\param _nIndex Index of the variable
	 *	\param _dTime Time point. '-1' - return last value*/
public:		double GetStateVariable( unsigned _nIndex ) const;
public:		double GetStateVariable( const std::string &_sName ) const;
	/** Set value of internal variable.
	 *	\param _nIndex Index of the variable
	 *	\param _dValue New value*/
public:		void SetStateVariable( unsigned _nIndex, double _dValue );
public:		void SetStateVariable( const std::string &_sName, double _dValue );
		/** Save last values of those internal variables, which were determined as having history. _dTime is a current time point.
	 *	\param _dTime Current time*/
public:		void SaveStateVariables( double _dTime );

	/** Adds new time dependent state variable. Returns unique index of the variable. Parameter _bSaveHistory specifies
	 *	if the history of all changes of variable could be saved during calculation. To save history function SaveStateVariables() should be called.
	 *	All variables which are added with this function will be automatically saved and restored during simulation.
	 *	Should be used in Initialize() function.
	 *	If there is already a state variable with the same name within the unit, index of this existing state variable will be returned.
	 *	\param _sName Name of variable
	 *	\param _dInitValue Initial value
	 *	\param _bSaveHistory Whether to record the history of variable
	 *	\return Index of the variable*/
protected:	unsigned AddStateVariable( const std::string &_sName, double _dInitValue, bool _bSaveHistory = false );
	/** Remove all state variables.*/
protected:	void ClearStateVariables( /*double _dTime = -1*/ );
	/** Get name of internal variable.
	 *	\param _nIndex Index of the variable*/
protected:	std::string GetStateVariableName( unsigned _nIndex ) const;
	/** Get number of internal variables.*/
protected:	unsigned GetStateVariablesNumber() const;

	// ========== Functions to work with COMPOUNDS

	/** Set database of materials.
	 *	\param _pDatabase Pointer to a materials database*/
public:		void SetMaterialsDatabase( const CMaterialsDatabase* _pDatabase );
	/** Sets pointer to a compounds vector.
	 *	\param _vCompoundsKeys Vector of compounds keys*/
public:		void SetCompounds( const std::vector<std::string>* _pvCompoundsKeys );
public:		void SetCompoundsPtr( const std::vector<std::string>* _pvCompoundsKeys );
public:		void AddCompound( std::string _sCompoundKey );
public:		void RemoveCompound( std::string _sCompoundKey );
	/** Returns unique keys of all using compounds.*/
public:		std::vector<std::string> GetCompoundsList() const;
	/** Returns names of all using compounds.*/
public:		std::vector<std::string> GetCompoundsNames() const;
	/** Get compounds number.*/
public:		unsigned GetCompoundsNumber() const;
	/** Get the value of constant physical property (CRITICAL_TEMPERATURE, MOLECULAR_WEIGHT, etc) for the specified compound.*/
public:		double GetCompoundConstant( const std::string &_sCompoundKey, unsigned _nConstant ) const;
	/** Returns the value of temperature/pressure-dependent physical property (HEAT_CAPACITY, THERMAL_CONDUCTIVITY, BOILING_POINT_TEMPERATURE, etc)
		for specified compound with specified temperature [K] and pressure [Pa].*/
public:		double GetCompoundTPDProp( const std::string &_sCompoundKey, unsigned _nProperty, double _dTemperature, double _dPressure ) const;
			/** Returns the value of the interaction property (INTERFACE_TENSION, INT_PROP_USER_DEFINED_01, INT_PROP_USER_DEFINED_02, etc) between specified compounds	under specified temperature [K] and pressure [Pa].*/
public:		double GetCompoundsInteractionProp(const std::string &_sCompoundKey1, const std::string &_sCompoundKey2, unsigned _nProperty, double _dTemperature = STANDARD_CONDITION_T, double _dPressure = STANDARD_CONDITION_P) const;
			// Returns true if compound with specified name has been defined, otherwise returns false.
public:		bool IsCompoundNameDefined(const std::string& _sCompoundName) const;
			// Returns true if compound with specified unique key has been defined, otherwise returns false.
public:		bool IsCompoundKeyDefined(const std::string& _sCompoundKey) const;
			// Determines, whether a property with the specified key is present in materials database.
public:		bool IsPropertyDefined(unsigned _propertyKey) const;

	// ========== Functions to work with PHASES

	/** Sets pointer to a compounds vector.
	 *	\param _vCompoundsKeys Vector of compounds keys*/
public:		void SetPhases( const std::vector<std::string>* _pvPhasesNames, const std::vector<unsigned>* _pvPhasesSOAs );
public:		void SetPhasesPtr( const std::vector<std::string>* _pvPhasesNames, const std::vector<unsigned>* _pvPhasesSOAs );
public:		void AddPhase( std::string _sName, unsigned _nAggrState );
public:		void RemovePhase( unsigned _nIndex );
public:		void ChangePhase( unsigned _nIndex, std::string _sName, unsigned _nAggrState );
	/** Get index of the solid phase. Returns -1 if solid phase has not been defined.*/
	//int GetSolidPhaseIndex() const;
	//int GetLiquidPhaseIndex() const;
	//int GetVaporPhaseIndex() const;
public:		bool IsPhaseDefined( EPhaseTypes _nPhaseType ) const;
public:		unsigned GetLiquidPhasesNumber() const;

	/** Returns number of phases which are defined in the unit.*/
public:		unsigned GetPhasesNumber() const;
	/** Returns name of the specified phase. Indexes are starting from 0. If _nIndex is larger than number of phases, empty string is returned.*/
public:		std::string GetPhaseName( EPhaseTypes _nPhaseType ) const;
	/** Returns state of aggregation of the specified phase. Returning values: SOA_SOLID, SOA_LIQUID, SOA_VAPOR.
		If _nIndex is larger than number of phases, SOA_UNKNOWN is returned.*/
public:		unsigned GetPhaseSOA( unsigned _nPhaseIndex ) const;
	/** Returns index of the specified phase. Returns -1 if such phase has not been defined.*/
public:
	unsigned GetPhaseIndex(unsigned _nPhaseType) const;

	// ========== Functions to work with LOOKUP TABLES

private:
	/**	Checks if lookup table for a certain property was already defined in map m_vLookupTables
	*	\param _nProperty property of lookup table
	*	\param _nDependenceType dependence of lookup table (temperature/pressure)
	*	\return bool if property is already defined in map */
	bool IsDefined(ECompoundTPProperties _nProperty, EDependencyTypes _nDependenceType);

	/**	Adds a new lookup table object for a certain property to the map m_vLookupTables
	*	\param _nProperty property of lookup table
	*	\param _nDependenceType dependence of lookup table (temperature/pressure) */
	void AddPropertyTable(ECompoundTPProperties _nProperty, EDependencyTypes _nDependenceType);

	/**	Reads the parameter of a lookup table of respective property for a specified value
	*	\param _nProperty property of lookup table
	*	\param _nDependenceType dependence of lookup table (temperature/pressure)
	*	\param _dTime time point
	*	\param _dValue read out value of lookup table
	*	\return double parameter read from lookup table*/
	double GetParamFromLookup(ECompoundTPProperties _nProperty, EDependencyTypes _nDependenceType, const std::vector<double>& _vCompoundFractions, double _dValue);

public:
	/**	Reads the temperature of a lookup table of respective property for a specified value
	*	\param _nProperty property of lookup table
	*	\param _dTime time point of evaluation
	*	\param _dValue read out value of lookup table
	*	\return double temperature*/
	double CalcTemperatureFromProperty(ECompoundTPProperties _nProperty, const std::vector<double>& _vCompoundFractions, double _dValue);

	/**	Reads the pressure of a lookup table of respective property for a specified value
	*	\param _nProperty property of lookup table
	*	\param _dTime time point of evaluation
	*	\param _dValue read out value of lookup table
	*	\return double pressure*/
	double CalcPressureFromProperty(ECompoundTPProperties _nProperty, const std::vector<double>& _vCompoundFractions, double _dValue);

	// ========== Functions to work with SOLID DISTRIBUTED PROPERTIES

	/** Sets pointer to a distributions grid of solids.
	 *	\param _pGrid pointer to a new distributions grid*/
public:		void SetDistributionsGrid( const CDistributionsGrid* _pGrid );

public:		static void CalculateTM( EDistrTypes _nDistrType, std::vector<double> _vInDistr, std::vector<double> _vOutDistr, CTransformMatrix &_outTM );

	/** Returns list of defined dimensions of solid distribution.*/
public:		std::vector<EDistrTypes> GetDistributionsTypes() const;
	/** Returns list of classes numbers for defined dimensions of solid distribution.*/
public:		std::vector<unsigned> GetDistributionsClasses() const;
	/** Returns number of specified dimensions of solid distribution.*/
public:		unsigned GetDistributionsNumber() const;
	/** Returns state of distributed property: CONTINUOUS, DISCRETE_NUMERIC or DISCRETE_SYMBOLIC.
		If specified dimension or solid phase have not been defined, UNKNOWN will be returned.*/
public:		EGridEntry GetDistributionGridType( EDistrTypes _nDistrType ) const;
	/** Returns grid of classes for specified dimension for continuous or discrete numeric distribution.
	 *	If solid phase has not been defined or specified dimension has not been defined or has symbolic grid, than empty vector will be returned.
	 *	\param _nDimension Dimension type*/
public:		std::vector<double> GetNumericGrid( EDistrTypes _nDistrType ) const;
	/** Returns grid of classes for specified dimension for symbolic discrete distribution.
	 *	If solid phase has not been defined or specified dimension has not been defined or has numeric grid, than empty vector will be returned.
	 *	\param _nDimension Dimension type*/
public:		std::vector<std::string> GetSymbolicGrid( EDistrTypes _nDistrType ) const;
	/** Returns mean values of the particle size classes for the chosen distribution.*/
public:		std::vector<double> GetClassesMeans( EDistrTypes _nDistrType ) const;
	/** Returns size grid for particle diameters.*/
public:		std::vector<double> GetPSDGridDiameters() const;
	/** Returns size grid for particle volumes.*/
public:		std::vector<double> GetPSDGridVolumes() const;
	/** Returns mean particle diameters/volumes for PSD distribution.*/
public:		std::vector<double> GetPSDMeanDiameters() const;
	/** Returns mean particle surfaces for PSD distribution.*/
public:		std::vector<double> GetPSDMeanSurfaces() const;
	/** Returns mean particle volumes for PSD distribution.*/
public:		std::vector<double> GetPSDMeanVolumes() const;

public:		std::vector<double> GetClassesSizes( EDistrTypes _nDistrType ) const;

public:		bool IsDistributionDefined( EDistrTypes _nDistrType ) const;
	/** Returns number of classes for specified dimension. If such dimension has not been defined, 0 will be returned.*/
public:		unsigned GetClassesNumber( EDistrTypes _nDistrType ) const;


	// ========== Functions to work with TOLERANCES

	/**	Sets absolute tolerance.
	 *	\param _dATol New value of an absolute tolerance*/
public:		void SetAbsTolerance( double _dATol );
	/**	Set relative tolerance.
	 *	\param _dRTol New value of a relative tolerance*/
public:		void SetRelTolerance( double _dRTol );

	/**	Returns absolute tolerance.*/
public:		double GetAbsTolerance() const;
	/**	Returns relative tolerance.*/
public:		double GetRelTolerance() const;

public:		void SetMinimalFraction( double _dFraction );


	// ========== Functions to SAVE/LOAD unit

	/** Save unit to file.*/
public:		void SaveToFile( CH5Handler& _h5Saver, const std::string& _sPath );
	/** Load unit from file*/
public:		void LoadFromFile( CH5Handler& _h5Loader, const std::string& _sPath );
private:	void LoadFromFileOldVer( CH5Handler& _h5Loader, const std::string& _sPath );


	// ========== Functions to work with ERRORS and WARNINGS

	/**	Checks unit's error.
	 *	\retval true An error has occurred*/
public:		bool CheckError() const;
	/**	Checks unit's warning.
	 *	\retval true A warning has occurred*/
public:		bool CheckWarning() const;
	/**	Checks unit's info.
		*	\retval true An info should be written*/
public:		bool CheckInfo() const;
	/**	Get text description of the last error.*/
public:		std::string GetErrorDescription() const;
	/**	Get text description of the last warning.*/
public:		std::string GetWarningDescription() const;
	/**	Get text description of the last info.*/
public:		std::string GetInfoDescription() const;

//protected:
	/** Sets error. Is called to indicate that an error occurred. Simulation will be stopped.
	 *	\param _sDescription Error description*/
public:		void RaiseError( const std::string &_sDescription = "" );
	/** Set warning. Is called to indicate warning. Simulation will not be stopped.
	 *	\param _sDescription Warning description*/
public:		void RaiseWarning( const std::string &_sDescription = "" );
	/** Prints string to simulation log. For giving further information during simulation.
			*	\param _sDescription Info description*/
public:		void ShowInfo(const std::string &_sDescription = "");
public:		void ClearError();
public:		void ClearWarning();
public:		void ClearInfo();

	// ========== Functions to work with DYNAMIC FLAG

	/** Specify type of the unit.
	 *	\param _bIsDynamic Contains \a true for dynamic unit, \a false for steady-state unit*/
protected:	void SetDynamicUnit( bool _bIsDynamic );

	// ========== Functions to call from SIMULATOR

	/**	Initialize unit before starting a simulation. Makes internal initialization and calls Initialize()*/
public:		void InitializeUnit( double _dTime );
			/// Finalize unit after each simulation. Makes internal finalization and calls Finalize()*/
public:		void FinalizeUnit();
	/** Save current state of the unit on specified time interval. If T2 == -1, time window is [T1; LastTimePoint]. Performs internal saving procedure and calls SaveState()*/
public:		void SaveStateUnit(double _dT1, double _dT2 = -1);
	/** Load previously saved state of the unit. Performs internal loading procedure and calls LoadState()*/
public:		void LoadStateUnit();
	/** Get unit type.
	 *	\retval true Dynamic unit
	 *	\retval false Steady state unit*/
public:		bool IsDynamicUnit() const;

public:		void SetCachePath(const std::wstring& _sPath);
public:		void SetCacheParams( bool _bEnabled, unsigned _nWindow );

public:		void ClearSimulationResults();
private:
	static EPhase PhaseSOA2EPhase(unsigned _soa);

	//////////////////////////////////////////////////////////////////////////
	/// Functions to work with plots
	//////////////////////////////////////////////////////////////////////////

private:
	struct SCurve
	{
		std::string sName;
		std::vector<double> vX;
		std::vector<double> vY;
		double dZ;
	};
	struct SPlot
	{
		std::string sName;
		std::string sXAxis;
		std::string sYAxis;
		std::string sZAxis;
		std::vector<SCurve> vCurves;
		bool bIs2D;
	};
	std::vector<SPlot*> m_vPlots;
	std::vector<SPlot*> m_vStorePlots;

	int p_AddPlot(const std::string& _sPlotName, const std::string& _sXAxisName, const std::string& _sYAxisName, const std::string& _sZAxisName, bool _bIs2D);
public:
	/// Adds new 2-dimensional plot with specified name and axis, returns index of the plot. Returns -1 on error.
	/// If there is already a plot with the same name and parameters within the unit, index of this existing plot will be returned.
	/// If there is already a plot with the same name and other parameters within the unit, logic_error exception will be thrown.
	int AddPlot(const std::string& _sPlotName, const std::string& _sXAxisName, const std::string& _sYAxisName);
	/// Adds new 3-dimensional plot with specified name and axis, returns index of the plot. Returns -1 on error.
	/// If there is already a plot with the same name and parameters within the unit, index of this existing plot will be returned.
	/// If there is already a plot with the same name and other parameters within the unit, logic_error exception will be thrown.
	int AddPlot(const std::string& _sPlotName, const std::string& _sXAxisName, const std::string& _sYAxisName, const std::string& _sZAxisName);
	/// Adds new curve with specified name on the 2-dimensional plot with index _nPlotIndex. Returns index of the curve within specified plot. Returns -1 on error.
	int AddCurveOnPlot(unsigned _nPlotIndex, const std::string& _sCurveName);
	/// Adds new curve with specified name on the 2-dimensional plot with name _sPlotName. Returns index of the curve within specified plot. Returns -1 on error.
	int AddCurveOnPlot(const std::string& _sPlotName, const std::string& _sCurveName);
	/// Adds new curve with specified z-value on the 2- or 3-dimensional plot with index _nPlotIndex. Returns index of the curve within specified plot. Returns -1 on error.
	int AddCurveOnPlot(unsigned _nPlotIndex, double _dZValue);
	/// Adds new curve with specified z-value on the 2- or 3-dimensional plot with name _sPlotName. Returns index of the curve within specified plot. Returns -1 on error.
	int AddCurveOnPlot(const std::string& _sPlotName, double _dZValue);
	/// Adds new point on specified curve for 2- or 3-dimensional plot.
	void AddPointOnCurve(unsigned _nPlotIndex, unsigned _nCurveIndex, double _dX, double _dY);
	/// Adds new point on specified curve for 2-dimensional plot.
	void AddPointOnCurve(const std::string& _sPlotName, const std::string& _sCurveName, double _dX, double _dY);
	/// Adds new point on specified curve for 3-dimensional plot.
	void AddPointOnCurve(const std::string& _sPlotName, double _dZValue, double _dX, double _dY);
	/// Adds new points on specified curve for 2- or 3-dimensional plot.
	void AddPointOnCurve(unsigned _nPlotIndex, unsigned _nCurveIndex, const std::vector<double>& _vX, const std::vector<double>& _vY);
	/// Adds new points on specified curve for 2-dimensional plot.
	void AddPointOnCurve(const std::string& _sPlotName, const std::string& _sCurveName, const std::vector<double>& _vX, const std::vector<double>& _vY);
	/// Adds new points on specified curve for 3-dimensional plot.
	void AddPointOnCurve(const std::string& _sPlotName, double _dZValue, const std::vector<double>& _vX, const std::vector<double>& _vY);

	/// Returns number of specified plots.
	unsigned GetPlotsNumber() const;
	/// Returns number of curves on plot _nPlot.
	unsigned GetCurvesNumber(size_t _nPlot) const;
	/// Returns name of the plot by its index.
	std::string GetPlotName(size_t _nPlot) const;
	/// Returns name of the x-axis of the plot by its index.
	std::string GetPlotXAxisName(unsigned _nPlot) const;
	/// Returns name of the y-axis of the plot by its index.
	std::string GetPlotYAxisName(unsigned _nPlot) const;
	/// Returns name of the z-axis of the plot by its index.
	std::string GetPlotZAxisName(unsigned _nPlot) const;
	/// Returns name of the curve on the plot by their indexes.
	std::string GetCurveName(unsigned _nPlot, unsigned _nCurve) const;
	/// Returns vector of X of the curve on the plot by their indexes.
	std::vector<double> GetCurveX(unsigned _nPlot, unsigned _nCurve) const;
	/// Returns vector of Y of the curve on the plot by their indexes.
	std::vector<double> GetCurveY(unsigned _nPlot, unsigned _nCurve) const;
	/// Returns vector of Z of the curve on the plot by their indexes.
	double GetCurveZ(size_t _nPlot, size_t _nCurve) const;
	/// Returns true if the plot is 2D.
	bool IsPlot2D(unsigned _nPlot);

	/// Clears all plots
	void ClearPlots();
	/// Saves all plots data into internal variables.
	void SavePlots();
	/// Saves plots data from internal variables.
	void LoadPlots();


	//////////////////////////////////////////////////////////////////////////
	/// Functions to work with external solvers
	//////////////////////////////////////////////////////////////////////////

private:
	std::vector<CBaseSolver*> m_vExternalSolvers;

public:
	void InitializeExternalSolvers() const;
	void FinalizeExternalSolvers() const;
	void SetSolversPointers(const std::vector<CBaseSolver*>& _pvPointers);
};

typedef DECLDIR CBaseUnit* (*CreateUnit)();

/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#define NOMINMAX

#include "MDMatrix.h"
#include "DistributionsGrid.h"
#include "MaterialsDatabase.h"
#include "DenseDistr2D.h"
#include "LookupTable.h"

class CStream;
class CMaterialStream;
class CHoldup;

/** Phase description.*/
struct SPhase
{
	std::string sName;				///< Phase name
	unsigned nAggregationState;	///< Aggregation state of phase
	CMDMatrix distribution;			///< Distributed parameters
};

/** Base not instantiated class for material stream description.*/
class CStream
{
private:
	static const unsigned m_cnSaveVersion;

	std::string m_sStreamKey;						///< Unique key which specify this stream
	std::string m_sStreamName;						///< Name of the stream

	std::wstring m_sCachePath;
	bool m_bCacheEnabled;
	unsigned m_nCacheWindow;

protected:
	std::vector<double> m_vTimePoints;				///< Vector of time points where this stream has been defined
	std::vector<std::string> m_vCompoundsKeys;		///< Keys of the chemical compounds which contains this stream
	std::vector<SPhase*> m_vpPhases;				///< Vector of defined phases

	CDenseDistr2D m_StreamMTP;						///< Overall properties of the stream: mass/temperature/pressure
	CDenseDistr2D m_PhaseFractions;					///< Fractions of phases: solid/liquid/vapor and their combinations

	const CDistributionsGrid* m_pDistributionsGrid;	///< Pointer to a grid with size classes
	const CMaterialsDatabase* m_pMaterialsDB;		///< Pointer to a database of materials

	std::vector<CDenseDistr2D*> m_DistrArrays;		///< Pointers to all dense distributed properties, to simplify massive make operations

	mutable std::map<ECompoundTPProperties, CLookupTable> m_vTLookupTables;	///< Map with all lookup tables for Temperature (for fast use)
	mutable std::map<ECompoundTPProperties, CLookupTable> m_vPLookupTables;	///< Map with all lookup tables for Pressure (for fast use)
	CLookupTable m_TLookup1, m_TLookup2;	/// Lookup tabel to calculate mixtures (for speed-up).

public:
	/** Basic constructor.
	 *	\param _sKey Unique key of this stream*/
	CStream(const std::string& _sKey = "");
	/// Copy constructor.
	CStream(const CStream& _stream);
	/** Basic destructor.*/
	virtual ~CStream();

private:
	void initStream(const std::string& _sKey = "");

public:
	/** Copy stream structure (phases, compounds, etc.)*/
	void SetupStream(const CStream *_pStream);

	void Clear();

	// ============= Functions to work with BASIC STREAM PROPERTIES

	/** Get unique key of the stream.*/
	std::string GetStreamKey() const;
	/** Sets unique key to the stream.*/
	void SetStreamKey(const std::string& _key);
	/**	Get name of the stream.*/
	std::string GetStreamName() const;
	/**	Set name of the stream.
	 *	\param _sNewName New name of the stream*/
	void SetStreamName( std::string _sNewName );

	/** Returns pointer to mass/temperature/pressure distribution.*/
	CDenseDistr2D* GetDistrStreamMTP();
	const CDenseDistr2D* GetDistrStreamMTP() const;
	/** Returns pointer to phase fractions distribution.*/
	CDenseDistr2D* GetDistrPhaseFractions();
	const CDenseDistr2D* GetDistrPhaseFractions() const;


	// ============ Functions to work with TIME POINTS

	/** Adds new time point. If such time point already exists, nothing will be done.
	 *	\param _dTime New time point
	 *	\param _dSourceTime Copy data from this time point. '-1' - data will be copied from the last specified time point*/
	void AddTimePoint( double _dTime, double _dSourceTime = -1 );
	/** Removes specified time point if such point exists.
	 *	\param _dTime Time point to remove*/
	void RemoveTimePoint( double _dTime );
	/** Removes time points from specified interval. Returns index of first remaining time point after interval.*/
	void RemoveTimePoints( double _dStart, double _dEnd );
	/** Removes all data after specified time point.
	*	\param _dStart All data after this time will be removed
	*	\param _bIncludeStart Specifies if _dStart will be included into a time interval*/
	void RemoveTimePointsAfter( double _dStart, bool _bIncludeStart = false );
	/** Gets all time points from the stream.*/
	std::vector<double> GetAllTimePoints() const;
	/** Returns the list of time points in specified time interval. Boundaries will be included if _bForceInclBoudaries == true.
	 *	\param _dStart Start of the time interval
	 *	\param _dEnd End of the time interval
	 *	\param _bForceInclBoudaries Always add boundaries to interval*/
	std::vector<double> GetTimePointsForInterval( double _dStart, double _dEnd, bool _bForceInclBoudaries = false ) const;
	/** Return last time point defined in the stream.
	 *	\retval -1 No time points have been defined*/
	double GetLastTimePoint() const;
	/** Returns the nearest time point before _dTime.
	 *	\param _dTime Current time point
	 *	\retval -1 No time points have been defined or \a _dTime is the first time point*/
	double GetPreviousTimePoint( double _dTime ) const;
	/** Changes time point with a specified index. For UI purposes
		It is not allowed to makes this time point larger than next time point and smaller than the previous one.
	 *	\param _nTimeIndex Index of the time point
	 *	\param _dNewTime New value of the time point
	 *	\retval false Changing was not successful*/
	void ChangeTimePoint( unsigned _nTimeIndex, double _dNewTime );


	// ============= Functions to work with OVERALL PROPERTIES

	/** Gets temperature of the stream in specified time point in [K]. If there is no specified time point, the value will be interpolated.
	 *	\param _dTime Time point*/
	double GetTemperature( double _dTime ) const;
	/** Sets temperature of the stream in specified time point in [K]. Negative values before setting will be converted to zero.
	 *	If time point was not defined in the stream, then the value will not be set.
	 *	\param _dTime Time point
	 *	\param _dValue New temperature value*/
	void SetTemperature( double _dTime, double _dValue );

	double GetPressure( double _dTime ) const;
	void SetPressure( double _dTime, double _dValue );

	/** Returns non-constant physical property value (FLOW, PRESSURE, etc) for overall mixture for specified time point.
		_nBasis is a basis of results (BASIS_MASS or BASIS_MOLL).*/
	double GetOverallProperty( double _dTime, unsigned _nProperty, unsigned _nBasis = BASIS_MASS ) const;
	/** Sets non-constant physical property value (FLOW, PRESSURE, etc) for overall mixture for specified time point.
		_nBasis is a basis of value (BASIS_MASS or BASIS_MOLL).*/
	void SetOverallProperty( double _dTime, unsigned _nProperty, double _dValue, unsigned _nBasis = BASIS_MASS );


	// ============= Functions to work with COMPOUNDS
	/** Adds compound to the stream. If this compound already defined in the stream, than nothing will be done.
	 *	\param _sCompoundKey Unique key of the compound*/
	void AddCompound( std::string _sCompoundKey );
	/** Removes compound from the stream.
	 *	\param _sCompoundKey Unique key of the compound*/
	void RemoveCompound( std::string _sCompoundKey );
	/** Sets new compounds with removing of previous set.
	 *	\param _vCompoundsKeys Vector of compounds keys*/
	void SetCompounds( const std::vector<std::string>& _vCompoundsKeys );
	/** Returns number of using compounds.*/
	size_t GetCompoundsNumber() const;
	/** Returns unique keys of all using compounds.*/
	std::vector<std::string> GetCompoundsList() const;
	/** Returns names of all using compounds.*/
	std::vector<std::string> GetCompoundsNames() const;
protected:
	/** Returns the index of the compound.
	 *	\param _sCompoundKey Unique key of the compound
	 *	\return Index of the specified compound
	 *	\retval -1 Such compound does not exist*/
	int GetCompoundIndex(const std::string& _sCompoundKey) const;

public:
	/** Returns total fraction of the compound with key _sCompoundKey at the time point _dTime. If there is no such compound, than 0 will be returned.*/
	double GetCompoundFraction( double _dTime, std::string _sCompoundKey, unsigned _nBasis = BASIS_MASS ) const;
	/** Returns total fraction of the compound with index _nCompoundIndex at the time point _dTime.
		If there is no such compound, than 0 will be returned.*/
	double GetCompoundFraction(double _dTime, unsigned _nCompoundIndex, unsigned _nBasis = BASIS_MASS) const;
	/** Returns total fraction of all compounds at the time point _dTime.
	*	\param _dTime Time point.
	*	\retval Vector with compound mass fractions.*/
	std::vector<double> GetCompoundsFractions(double _dTime, unsigned _nBasis = BASIS_MASS) const;
	/** Returns value of compound in phase for specified time point. If there is no such phase or compound, than 0 will be returned.
	 *	\param _sCompoundKey Key of compound
	 *	\param _nPhase Phase
	 *	\param _dTime Time point*/
	double GetCompoundPhaseFraction(double _dTime, const std::string& _sCompoundKey, unsigned _nPhase, unsigned _nBasis = BASIS_MASS) const;
	/** Returns value of compound in phase for specified time point. If there is no such phase or compound, than 0 will be returned.
	 *	\param _nCompoundIndex Index of compound
	 *	\param _nPhase Phase
	 *	\param _dTime Time point*/
	double GetCompoundPhaseFraction(double _dTime, unsigned _nCompoundIndex, unsigned _nPhase, unsigned _nBasis = BASIS_MASS) const;
	/** Sets fraction of the compound with key _sCompoundKey in phase _nPhaseIndex at the time point _dTime.*/
	void SetCompoundPhaseFraction( double _dTime, std::string _sCompoundKey, unsigned _nPhase, double _dFraction, unsigned _nBasis = BASIS_MASS );

private:
	/** Returns true if all compounds from the list are present in stream. */
	bool IsCompoundsCorrect(const std::vector<std::string>& _vCompounds) const;

	// ============= Functions to work with PHASES

public:
	/** Adds new phase.
	 *	\param _sName Phase name
	 *	\param _nAggregationState Aggregation state*/
	void AddPhase( std::string _sName, unsigned _nAggrState );
	/** Removes phase.
	 *	\param _nIndex Phase index*/
	void RemovePhase( unsigned _nIndex );
	/** Changes phase parameters.
	 *	\param _nIndex Phase index
	 *	\param _sName Phase name
	 *	\param _nAggregationState Aggregation state*/
	void ChangePhase( unsigned _nIndex, std::string _sName, unsigned _nAggrState );
	/** Sets new phases with removing of previous phases.
	 *	\param _vPhasesNames Vector of phases names
	 *	\param _vPhasesAggregationStates Vector of aggregation states*/
	void SetPhases( const std::vector<std::string>& _vNames, const std::vector<unsigned>& _vAggrStates );

	/** Returns number of phases which are defined in the stream.*/
	size_t GetPhasesNumber() const;
	/** Returns name of the specified phase. Indexes are starting from 0. If _nIndex is larger than number of phases, empty string is returned.*/
	std::string GetPhaseName( unsigned _nIndex ) const;
	/** Returns state of aggregation of the specified phase. Returning values: SOA_SOLID, SOA_LIQUID, SOA_VAPOR.
		If _nIndex is larger than number of phases, SOA_UNKNOWN is returned.*/
	unsigned GetPhaseSOA( unsigned _nIndex ) const;
	/** Returns index of the specified phase. Returns -1 if such phase has not been defined.*/
	size_t GetPhaseIndex(unsigned _phase) const;
	/** Returns pointer to a phase distribution. If _nIndex is larger than number of phases, NULL is returned.
	 * \param _nIndex Phase index*/
	CMDMatrix* GetPhaseDistribution( unsigned _nIndex ) const;

	/** Returns non-constant physical property value (MOLECULAR_WEIGHT, PHASE_FRACTION, TEMPERATURE, etc) for phase mixture for specified time point.
		_nBasis is a basis of results (BASIS_MASS or BASIS_MOLL).*/
	double GetSinglePhaseProp( double _dTime, unsigned _nProperty, unsigned _nPhase, unsigned _nBasis = BASIS_MASS ) const;
	/** Sets non-constant physical property value (PHASE_FRACTION, TEMPERATURE, etc) for phase mixture for specified time point.
		_nBasis is a basis of results (BASIS_MASS or BASIS_MOLL).*/
	void SetSinglePhaseProp( double _dTime, unsigned _nProperty, unsigned _nPhase, double _dValue, unsigned _nBasis = BASIS_MASS );
	///** Returns two-phase non-constant physical property value (K_VALUE, LOG_K_VALUE, SURFACE_TENSION) for a mixture of phases for specified time point.*/
	//double GetTwoPhaseProp( double _dTime, unsigned _nProperty, unsigned _nPhaseIndex1, unsigned _nPhaseIndex2 ) const;
	/** Returns the value of temperature/pressure-dependent physical property (HEAT_CAPACITY, THERMAL_CONDUCTIVITY, BOILING_POINT_TEMPERATURE, etc)
		for specified phase in time point _dTime.*/
	double GetPhaseTPDProp( double _dTime, unsigned _nProperty, unsigned _nPhase ) const;

	/** Returns pointer to a vector of phases.*/
	std::vector<SPhase*>* GetPhases();
	const std::vector<SPhase*>* GetPhases() const;

	//void CalcEquilibrium( double _dTime );


	// ============= Functions to work with	GRID of SOLID DISTRIBUTED PROPERTIES

	/** Set distributions grid of solid.
	 *	\param _pGrid pointer to a new distributions grid*/
	void SetDistributionsGrid( const CDistributionsGrid* _pGrid );


	// ============= Functions to work with	SOLID DISTRIBUTED PROPERTIES

	/** Returns solid mass fraction by specified coordinates according to all dimensions.*/
	double GetFraction( double _dTime, const std::vector<unsigned>& _vCoords ) const;
	/** Sets solid mass fraction by specified coordinates according to all dimensions.*/
	void SetFraction( double _dTime, const std::vector<unsigned>& _vCoords, double _dValue );

	/** Get multidimensional matrix of distributed dependent properties of solid for specified time point and all defined dimensions.
	*	If such time point has not been defined in the stream, then linear interpolation will be used to obtain data.
	*	\param _dTime Time point
	*	\return Distribution*/
	CDenseMDMatrix GetDistribution(double _dTime) const;
	/** Returns vector of solid distributed property for specified time point and dimension.
	 *	If such time point has not been defined in the stream, then linear interpolation will be used to obtain data.
	 *	\param _dTime Time point
	 *	\param _nDim Property type
	 *	\param _vResult Distribution
	 *	\return Error flag*/
	bool GetDistribution( double _dTime, EDistrTypes _nDim, std::vector<double>& _vResult ) const;
	/** Returns vector of solid distributed property for specified time point and dimension.
	*	If such time point has not been defined in the stream, then linear interpolation will be used to obtain data.
	*	\param _dTime Time point
	*	\param _nDim Property type
	*	\return Distribution */
	std::vector<double> GetDistribution(double _dTime, EDistrTypes _nDim) const;
	/** Returns matrix of two distributed dependent properties of solid for specified time point.
	 *	If such time point has not been defined in the stream, then linear interpolation will be used to obtain data.
	 *	\param _dTime Time point
	 *	\param _nDim1 Property type of the first dimension
	 *	\param _nDim2 Property type of the second dimension
	 *	\param _2DResult Distribution
	 *	\return Error flag*/
	bool GetDistribution( double _dTime, EDistrTypes _nDim1, EDistrTypes _nDim2, CMatrix2D& _2DResult ) const;
	/** Returns matrix of two distributed dependent properties of solid for specified time point.
	 *	If such time point has not been defined in the stream, then linear interpolation will be used to obtain data.
	 *	\param _dTime Time point
	 *	\param _nDim1 Property type of the first dimension
	 *	\param _nDim2 Property type of the second dimension
	 *	\return Distribution*/
	CMatrix2D GetDistribution(double _dTime, EDistrTypes _nDim1, EDistrTypes _nDim2) const;
	/** Get multidimensional matrix of distributed dependent properties of solid for specified time point and dimensions.
	 *	If such time point has not been defined in the stream, then linear interpolation will be used to obtain data.
	 *	\param _dTime Time point
	 *	\param _vDims Vector of properties types
	 *	\param _MDResult Distribution
	 *	\return Error flag*/
	bool GetDistribution( double _dTime, const std::vector<EDistrTypes>& _vDims, CDenseMDMatrix& _MDResult ) const;
	/** Returns vector of solid distributed property for specified time point, dimension and compound.
	 *	If such time point has not been defined in the stream, then linear interpolation will be used to obtain data. Neither of dimensions should be equal to DISTR_COMPOUNDS.
	 *	\param _dTime Time point
	 *	\param _nDim Property type
	 *	\param _sCompound Compound key
	 *	\param _vResult Distribution
	 *	\return Error flag*/
	bool GetDistribution( double _dTime, EDistrTypes _nDim, const std::string& _sCompound, std::vector<double>& _vResult ) const;
	std::vector<double> GetDistribution(double _dTime, EDistrTypes _nDim, const std::string& _sCompound) const;
	/** Returns matrix of two distributed dependent properties of solid for specified time point and compound.
	 *	If such time point has not been defined in the stream, then linear interpolation will be used to obtain data. Neither of dimensions should be equal to DISTR_COMPOUNDS.
	 *	\param _dTime Time point
	 *	\param _nDim1 Property type of the first dimension
	 *	\param _nDim2 Property type of the second dimension
	 *	\param _sCompound Compound key
	 *	\param _2DResult Distribution
	 *	\return Error flag*/
	bool GetDistribution( double _dTime, EDistrTypes _nDim1, EDistrTypes _nDim2, const std::string& _sCompound, CMatrix2D& _2DResult ) const;
	/** Get multidimensional matrix of distributed dependent properties of solid for specified time point, dimensions and compound.
	 *	If such time point has not been defined in the stream, then linear interpolation will be used to obtain data. Neither of dimensions should be equal to DISTR_COMPOUNDS.
	 *	\param _dTime Time point
	 *	\param _vDims Vector of properties types
	 *	\param _sCompound Compound key
	 *	\param _MDResult Distribution
	 *	\return Error flag*/
	bool GetDistribution( double _dTime, const std::vector<EDistrTypes>& _vDims, const std::string& _sCompound, CDenseMDMatrix& _MDResult ) const;


	/** Sets solid distributed property of type _nDim for specified time point. If such time point or dimension doesn't exist nothing will be done.
	 *	\param _dTime Time point
	 *	\param _nDim Property type
	 *	\param _vDistr Distribution to set
	 *	\return Error flag*/
	bool SetDistribution( double _dTime, EDistrTypes _nDim, const std::vector<double>& _vDistr );
	/** Sets matrix of two dependent distributed properties of types _nDim1 and _nDim2 of solid for specified time point.
		If such time point or dimensions don't exist nothing will be done.
	 *	\param _dTime Time point
	 *	\param _nDim1 Property type of the first dimension
	 *	\param _nDim2 Property type of the second dimension
	 *	\param _2DDistr Distribution to set
	 *	\return Error flag*/
	bool SetDistribution( double _dTime, EDistrTypes _nDim1, EDistrTypes _nDim2, const CMatrix2D& _2DDistr );
	/** Sets multidimensional matrix of dependent distributed properties of solid for specified time point.
	 *	If such time point or dimensions, which are specified in _MDDistr, don't exist nothing will be done.
	 *	\param _dTime Time point
	 *	\param _MDDistr Distribution to set
	 *	\return Error flag*/
	bool SetDistribution( double _dTime, const CDenseMDMatrix& _MDDistr );
	/** Sets solid distributed property of type _nDim for specified time point and compound. If such time point, dimension or compound doesn't exist nothing will be done.
	 *  Neither of dimensions should be equal to DISTR_COMPOUNDS.
	 *	\param _dTime Time point
	 *	\param _nDim Property type
	 *	\param _sCompound Compound key
	 *	\param _vDistr Distribution to set
	 *	\return Error flag*/
	bool SetDistribution( double _dTime, EDistrTypes _nDim, const std::string& _sCompound, const std::vector<double>& _vDistr );
	/** Sets matrix of two dependent distributed properties of types _nDim1 and _nDim2 of solid for specified time point and compound.
	 *	If such time point, compound or dimensions don't exist nothing will be done. Neither of dimensions should be equal to DISTR_COMPOUNDS.
	 *	\param _dTime Time point
	 *	\param _nDim1 Property type of the first dimension
	 *	\param _nDim2 Property type of the second dimension
	 *	\param _sCompound Compound key
	 *	\param _2DDistr Distribution to set
	 *	\return Error flag*/
	bool SetDistribution( double _dTime, EDistrTypes _nDim1, EDistrTypes _nDim2, const std::string& _sCompound, const CMatrix2D& _2DDistr );
	/** Sets multidimensional matrix of dependent distributed properties of solid for specified time point and compound.
	 *	If such time point, compound or dimensions, which are specified in _MDDistr, don't exist nothing will be done. Neither of dimensions should be equal to DISTR_COMPOUNDS.
	 *	\param _dTime Time point
	 *	\param _sCompound Compound key
	 *	\param _MDDistr Distribution to set
	 *	\return Error flag*/
	bool SetDistribution( double _dTime, const std::string& _sCompound, const CDenseMDMatrix& _MDDistr );

	/** Transforms matrix of distributed parameters of solids for specified time point by applying a movement matrix.
	 *	\param _dTime Time point
	 *	\param _TMatrix Movement matrix
	 *	\return Error flag*/
	bool ApplyTM( double _dTime, const CTransformMatrix& _TMatrix );
	/** Transforms matrix of distributed parameters of solids for certain compound and specified time point by applying a movement matrix.
	 *	\param _dTime Time point
	 *	\param _sCompound Compound key
	 *	\param _TMatrix Movement matrix
	 *	\return Error flag*/
	bool ApplyTM(double _dTime, const std::string& _sCompound, const CTransformMatrix& _TMatrix);

	/** Normalizes data in solid distribution matrix for specified time point. If time point has not been defined, nothing will be done.
	 *	\param _dTime Time point*/
	void NormalizeDistribution( double _dTime );
	/** Normalizes data in solid distribution matrix for time interval.
	 *	\param _dStart Start of time interval
	 *	\param _dEnd End of time interval.*/
	void NormalizeDistribution( double _dStart, double _dEnd );
	/** Normalizes data in solid distribution matrix for all time points.*/
	void NormalizeDistribution();


	// ============= Functions to work with PARTICLE SIZE DISTRIBUTIONS

	/** Returns PSD distribution of a total mixture. Returns empty vector on error.
	*	\param _dTime Time point.
	*	\param _PSDType Type of PSD (PSD_q0, PSD_Q0, PSD_q3, PSD_Q3, PSD_MassFrac, PSD_Number).
	*	\param _PSDGridType Method for calculating the mean values (DIAMETER, VOLUME). */
	std::vector<double> GetPSD(double _dTime, EPSDTypes _PSDType, EPSDGridType _PSDGridType = EPSDGridType::DIAMETER) const;
	/** Returns PSD distribution of a specified compound. Returns empty vector on error.
	*	\param _dTime Time point.
	*	\param _PSDType Type of PSD (PSD_q0, PSD_Q0, PSD_q3, PSD_Q3, PSD_MassFrac, PSD_Number).
	*	\param _sCompound Key of the compound which should be taken into account.
	*	\param _PSDGridType Method for calculating the mean values (DIAMETER, VOLUME). */
	std::vector<double> GetPSD(double _dTime, EPSDTypes _PSDType, const std::string& _sCompound, EPSDGridType _PSDGridType = EPSDGridType::DIAMETER) const;
	/** Returns PSD distribution. Returns empty vector on error.
	 *	\param _dTime Time point.
	 *	\param _PSDType Type of PSD (PSD_q0, PSD_Q0, PSD_q3, PSD_Q3, PSD_MassFrac, PSD_Number).
	 *	\param _vCompounds List of compounds which should be taken into account. If empty, PSD will be calculated for the whole mixture.
	 *	\param _PSDGridType Method for calculating the mean values (DIAMETER, VOLUME). */
	std::vector<double> GetPSD(double _dTime, EPSDTypes _PSDType, const std::vector<std::string>& _vCompounds, EPSDGridType _PSDGridType = EPSDGridType::DIAMETER) const;
private:
	/** Calculates PSD of the stream in terms of mass fractions. Considers compounds from the list. If _vCompounds is empty, PSD for the whole mixture is calculated.
	*	Takes porosity into account if defined. All checks on parameters, solid phases, grid existence, etc. must be performed by caller.*/
	std::vector<double> p_GetPSDMassFrac(double _dTime,  const std::vector<std::string>& _vCompounds) const;
	/** Calculates number particle distribution of the stream, considering compounds from the list. If _vCompounds is empty q0 for the whole mixture is calculated.
	*	Takes porosity into account if defined. All checks on parameters, solid phases, grid existence, etc. must be performed by caller.*/
	std::vector<double> p_GetPSDNumber(double _dTime, const std::vector<std::string>& _vCompounds, EPSDGridType _PSDGridType) const;

public:
	/** Sets PSD distribution for the whole mixture. When setting number-related PSD, distribution will be normalized and total particle mass remains unchanged.
	 *	\param _dTime Time point.
	 *	\param _PSDType Type of PSD (PSD_q0, PSD_Q0, PSD_q3, PSD_Q3, PSD_MassFrac, PSD_Number).
	 *	\param _vPSD Distribution.
	 *	\param _PSDGridType Method for calculating the mean values (DIAMETER, VOLUME). */
	void SetPSD(double _dTime, EPSDTypes _PSDType, const std::vector<double>& _vPSD, EPSDGridType _PSDGridType = EPSDGridType::DIAMETER);
	/** Sets PSD distribution for specified compound. When setting number-related PSD, distribution will be normalized and total particle mass remains unchanged.
	 *	\param _dTime Time point.
	 *	\param _PSDType Type of PSD (PSD_q0, PSD_Q0, PSD_q3, PSD_Q3, PSD_MassFrac, PSD_Number).
	 *	\param _sCompound Compound key.
	 *	\param _vPSD Distribution.
	 *	\param _PSDGridType Method for calculating the mean values (DIAMETER, VOLUME). */
	void SetPSD(double _dTime, EPSDTypes _PSDType, const std::string& _sCompound, const std::vector<double>& _vPSD, EPSDGridType _PSDGridType = EPSDGridType::DIAMETER);


	// ============= Functions to work with MATERIALS DATABASE

	/** Set database of materials.
	 *	\param _pDatabase Pointer to a materials database*/
	void SetMaterialsDatabase( const CMaterialsDatabase* _pDatabase );

	// ============= Functions to work with MINIMAL FRACTION

	/** Sets minimal fraction of MDMatrix.*/
	void SetMinimalFraction( double _dMinFraction );
	double GetMinimalFraction();

	// ============= Functions to SAVE/LOAD streams

	/** Save data to file.*/
	void SaveToFile( CH5Handler& _h5Saver, const std::string& _sPath );
	/** Load data from file*/
	void LoadFromFile( CH5Handler& _h5Loader, const std::string& _sPath );

	// Removes time points within the specified interval [_dStart; _dEnd), which are closer as _dStep.
	void ReduceTimePoints(double _dStart, double _dEnd, double _dStep);

	void SetCachePath(const std::wstring& _sPath);
	void SetCacheParams( bool _bEnabled, unsigned _nWindow );

	// Nearest-neighbor extrapolation.
	void ExtrapolateToPoint(double _dT, double _dTExtra);
	// Linear extrapolation.
	void ExtrapolateToPoint(double _dT1, double _dT2, double _dTExtra);
	// Spline extrapolation.
	void ExtrapolateToPoint(double _dT0, double _dT1, double _dT2, double _dTExtra);

	//void PSDConvert( double _dTime, unsigned _nPSDTypeIn, const std::vector<double> &_vPSDin, unsigned _nPSDTypeOut, std::vector<double> &_vPSDout );

private:

	// ============= Functions to work with	GRID of SOLID DISTRIBUTED PROPERTIES

	/** Returns list of defined dimensions of solid distribution.*/
	std::vector<unsigned> GetDistributionsTypes() const;
	/** Returns list of classes numbers for defined dimensions of solid distribution.*/
	std::vector<unsigned> GetDistributionsClasses() const;
	/** Returns number of specified dimensions of solid distribution.*/
public:
	size_t GetDistributionsNumber() const;
	/** Returns state of distributed property: CONTINUOUS, DISCRETE_NUMERIC or DISCRETE_SYMBOLIC.
		If specified dimension or solid phase have not been defined, UNKNOWN will be returned.*/
private:
	EGridEntry GetDistributionGridType( EDistrTypes _nDimension ) const;
	/** Returns grid of classes for specified dimension for continuous or discrete numeric distribution.
	 *	If solid phase has not been defined or specified dimension has not been defined or has symbolic grid, than empty vector will be returned.
	 *	\param _nDimension Dimension type*/
	std::vector<double> GetNumericGrid( EDistrTypes _nDimension ) const;
	/** Returns grid of classes for specified dimension for symbolic discrete distribution.
	 *	If solid phase has not been defined or specified dimension has not been defined or has numeric grid, than empty vector will be returned.
	 *	\param _nDimension Dimension type*/
	std::vector<std::string> GetSymbolicGrid( EDistrTypes _nDimension ) const;

	//void Convert_q3_to_q0( double _dTime, const std::vector<double> &_q3, std::vector<double> &_q0 ) const;

public:

	// ============= Functions to work with OTHER STREAMS

	/** Copies all stream data for specified time point.
	 *	Before copying all data after the time point _dTime in the destination stream can be removed if flag _bDeleteDataAfter is set to true.
	 *	\param _srcStream Pointer to a stream that should be copied
	 *	\param _dTime Time point to copy
	 *	\param _bDeleteDataAfter Before copying the all data in the destination stream is removed after _dTime*/
	void CopyFromStream_Base(const CStream& _srcStream, double _dTime, bool _bDeleteDataAfter = true);
	/** Copies all stream data for specified time interval.
	 *	Before copying all data after the time point _dStart in the destination stream can be removed if flag _bDeleteDataAfter is set to true.
	 *	\param _srcStream Pointer to a stream that should be copied
	 *	\param _dStart Start of the time interval to copy
	 *	\param _dEnd End of the time interval to copy
	 *	\param _bDeleteDataAfter Before copying the all data in the destination stream is removed after _dStart*/
	void CopyFromStream_Base(const CStream& _srcStream, double _dStart, double _dEnd, bool _bDeleteDataAfter = true);
	/** Copies all stream data from one time point of _pSrcStream to another time point in this stream.
	 *	Before copying all data after the time point _dTimeDst in the destination stream can be removed if flag _bDeleteDataAfter is set to true.
	 *	\param _dTimeDst Time point in this stream where data should be copied
	 *	\param _srcStream Pointer to a stream that should be copied
	 *	\param _dTimeSrc Time point of the source stream to copy
	 *	\param _bDeleteDataAfter Before copying the all data in the destination stream is removed after _dTimeDst*/
	void CopyFromStream_Base(double _dTimeDst, const CStream& _srcStream, double _dTimeSrc, bool _bDeleteDataAfter = true);

	/** Performs a mixing of the stream with stream _Stream at specified time point.
	 *	Only streams with the same data structure (MD dimensions, phases, etc) can be mixed.
	 *	\param _Stream Pointer to a stream
	 *	\param _dTime Time point that should be copied*/
	void AddStream_Base(const CStream& _Stream, double _dTime);
	/** Performs a mixing of the stream with stream _Stream for specified time interval. Boundary points are included into this interval.
	 *	Parameter _nTPType specifies which time points will be present in the resulting stream (combining points from two streams (BOTH_TP),
	 *	only from the first stream (DST_TP), or only from the second stream (SRC_TP)). Data for non-existent points are obtained by linear approximation.
	 *	Only streams with the same data structure (MD dimensions, phases, etc) can be mixed.
	 *	\param _Stream Pointer to a stream
	 *	\param _dStart Start of the time interval
	 *	\param _dEnd End of the time interval
	 *	\param _nTPTypes Specifies which time points will be present in the resulting stream*/
	void AddStream_Base(const CStream& _Stream, double _dStart, double _dEnd, unsigned _nTPTypes = BOTH_TP);

protected:
	// Returns pressure of a mixture of two streams.
	double CalcMixPressure(const CStream& _str1, double _time1, const CStream& _str2, double _time2) const;
	// Returns temperature of a mixture of two streams.
	double CalcMixTemperature(const CStream& _str1, double _time1, double _mass1, const CStream& _str2, double _time2, double _mass2);
	// Returns phase fractions of a mixture of two streams. Returns <frac_1, frac_2, frac_tot>.
	std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> CalcMixPhaseFractions(const CStream& _str1, double _time1, double _mass1, const CStream& _str2, double _time2, double _mass2) const;
	// Returns multidimensional distributions of a mixture of two streams for each phase.
	std::vector<CDenseMDMatrix> CalcMixMDDistributions(const CStream& _str1, double _time1, double _mass1, const std::vector<double>& _phaseFracs1, const CStream& _str2, double _time2, double _mass2, const std::vector<double>& _phaseFracs2) const;

public:
	//void SubStream_Base( CStream& _Stream, double _dTime );
	//void SubStream_Base( CStream& _Stream, double _dStart, double _dEnd, unsigned _nTPTypes = BOTH_TP );

	// ============= Functions to work with OVERALL PROPERTIES

	/** Returns mass flow of the stream. If there is no specified time point, the value will be interpolated
	 *	\param _dTime Time point
	 *	\param _nBasis Basis of results (BASIS_MASS [kg/s] or BASIS_MOLL [mol/s])*/
	double GetMass_Base( double _dTime, unsigned _nBasis = BASIS_MASS ) const;
	/** Sets mass flow of the stream. If there is no specified time point, the value will not be set
	 *	\param _dTimePoint Time point
	 *	\param _dValue New mass flow value
	 *	\param _nBasis Basis of value (BASIS_MASS [kg/s] or BASIS_MOLL [mol/s])*/
	void SetMass_Base( double _dTime, double _dValue, unsigned _nBasis = BASIS_MASS );

	// ============= Functions to work with COMPOUNDS

	/** Returns mass of the compound with key _sCompoundKey in phase _nPhaseIndex at the time point _dTime.
		_nBasis is a basis of value (BASIS_MASS [kg/s] or BASIS_MOLL [mol/s]).*/
	double GetCompoundMass_Base( double _dTime, unsigned _nCompoundIndex, unsigned _nPhase, unsigned _nBasis = BASIS_MASS ) const;
//	/** Sets mass of the compound with key _sCompoundKey in phase _nPhaseIndex at the time point _dTime.
//		_nBasis is a basis of value (BASIS_MASS [kg/s] or BASIS_MOLL [mol/s]).*/
//	void SetCompoundMass_Base( unsigned _nCompoundIndex, unsigned _nPhaseIndex, double _dTime, double _dMass, unsigned _nBasis = BASIS_MASS );

	/** Get the value of the constant physical property (CRITICAL_TEMPERATURE, MOLECULAR_WEIGHT, etc) for the specified compound.*/
	double GetCompoundConstant(const std::string& _sCompoundKey, ECompoundConstProperties _nConstProperty) const;
	/** Return the value of temperature/pressure-dependent physical property (HEAT_CAPACITY, THERMAL_CONDUCTIVITY, BOILING_POINT_TEMPERATURE, etc)
		for specified compound with specified temperature [K] and pressure [Pa].*/
	double GetCompoundTPDProp(const std::string& _sCompoundKey, unsigned _nProperty, double _dTemperature, double _dPressure) const;
	double GetCompoundTPDProp( double _dTime, const std::string& _sCompoundKey, unsigned _nProperty ) const;
	/** Returns the value of interaction property (INTERFACE_TENSION, INT_PROP_USER_DEFINED_01, INT_PROP_USER_DEFINED_02, etc)
	 * between specified compounds under specified temperature [K] and pressure [Pa].*/
	double GetCompoundInteractionProp(const std::string& _sCompoundKey1, const std::string& _sCompoundKey2, unsigned _nProperty, double _dTemperature, double _dPressure) const;
	/** Returns the value of interaction property (INTERFACE_TENSION, INT_PROP_USER_DEFINED_01, INT_PROP_USER_DEFINED_02, etc)
	 * between specified compounds under current temperature and pressure at the specified time point.*/
	double GetCompoundInteractionProp(double _dTime, const std::string _sCompoundKey1, const std::string _sCompoundKey2, unsigned _nProperty) const;

	// ============= Functions to work with PHASES

	/** Returns mass flow of the specified phase in the stream. If there is no specified time point, the value will be interpolated
	 *	\param _dTime Time point
	 *	\param _nPhaseIndex Index of the phase*/
	double GetPhaseMass_Base(double _dTime, unsigned _nPhase, unsigned _nBasis = BASIS_MASS) const;

	/** Sets mass flow of the specified phase in the stream for time point. Negative values before setting will be converted to zero.
		If there is no specified time point or phase in stream, the value will not be set.
		_nBasis is a basis of value (BASIS_MASS [kg/s] or BASIS_MOLL [mol/s]).*/
	void SetPhaseMass_Base(double _dTime, unsigned _nPhase, double _dMassFlow, unsigned _nBasis = BASIS_MASS);

	// ============= Internal functions

	/**	Returns index of the specified time point. Strict search returns -1 if there is no such time, not strict search returns index to paste.
	 *	\param _dTime Time point
	 *	\return Index of time point
	 *	\retval -1 There is no such time point*/
	size_t GetTimeIndex(double _dTime, bool _bIsStrict = true);
	/** Compares structures of the streams (phases, dimensions, etc).*/
	bool CompareStreamStructure( const CStream& _stream ) const;

private:
	/** const index of the phase. Returns -1 if solid phase has not been defined.*/
	int GetSolidPhaseIndex() const;
	//int GetLiquidPhaseIndex() const;
	//int GetVaporPhaseIndex() const;
	bool IsPhaseDefined( unsigned _nPhaseType ) const;
	unsigned GetLiquidPhasesNumber() const;

	/**	Returns a value for the pressure correction of the enthalpy of the stream, i.e. liquid1 and liquid2 phase of stream.
		The pressure correction only depends on the liquid phase, as gas is assumed with ideal behavior and solids enthalpy doesn't depend on pressure.
	*	\param _dTime Time point
	*	\return double Enthalpy pressure correction value for stream */
	//double CalcEnthalpyPressureCorrection(double _dTime, unsigned _nBasis = BASIS_MASS) const;

	/**	Returns a value for the pressure correction of the enthalpy of a specific phase.
	The pressure correction only depends on the liquid phase, as gas is assumed with ideal behavior and solids enthalpy doesn't depend on pressure.
	*	\param _dTime Time point
	*	\return double Enthalpy pressure correction value for respective phase */
	//double CalcEnthalpyPressureCorrection(double _dTime, unsigned _nPhase, unsigned _nBasis = BASIS_MASS) const;	// For one phase

	// ============= Functions to work with LOOKUP TABLES

private:
	/**	Checks if lookup table for a certain property was already defined in map m_vLookupTables
	*	\param _nProperty property of lookup table
	*	\param _nDependenceType dependence of lookup table (temperature/pressure)
	*	\return bool if property is already defined in map */
	bool IsDefined(ECompoundTPProperties _nProperty, EDependencyTypes _nDependenceType) const;

	/**	Adds a new lookup table object for a certain property to the map m_vLookupTables
	*	\param _nProperty property of lookup table
	*	\param _nDependenceType dependence of lookup table (temperature/pressure) */
	void AddPropertyTable(ECompoundTPProperties _nProperty, EDependencyTypes _nDependenceType) const;

public:
	/**	Returns corresponding lookup table.
	*	\param _nProperty Property of lookup table.
	*	\param _nDependenceType Dependence of lookup table (temperature/pressure).
	*	\param _dTime Time point.
	*	\return Pointer ot lookup table. */
	CLookupTable* GetLookupTable(ECompoundTPProperties _nProperty, EDependencyTypes _nDependenceType, double _dTime) const;

	/**	Reads the temperature of a lookup table of respective property for a specified value
	*	\param _nProperty property of lookup table
	*	\param _dTime time point of evaluation
	*	\param _dValue read out value of lookup table
	*	\return double temperature*/
	double CalcTemperatureFromProperty(ECompoundTPProperties _nProperty, double _dTime, double _dValue) const;

	/**	Reads the pressure of a lookup table of respective property for a specified value
	*	\param _nProperty property of lookup table
	*	\param _dTime time point of evaluation
	*	\param _dValue read out value of lookup table
	*	\return double pressure*/
	double CalcPressureFromProperty(ECompoundTPProperties _nProperty, double _dTime, double _dValue) const;

	/**	Reads the value of the specified property for the given temperature from a lookup table.
	*	\param _nProperty Property of lookup table.
	*	\param _dTime Time point for the evaluation.
	*	\param _dT Temperature.
	*	\return Property value. */
	double CalcPropertyFromTemperature(ECompoundTPProperties _nProperty, double _dTime, double _dT) const;

	/**	Reads the value of the specified property for the given pressure from a lookup table.
	*	\param _nProperty Property of lookup table.
	*	\param _dTime Time point for the evaluation.
	*	\param _dP Pressure.
	*	\return Property value. */
	double CalcPropertyFromPressure(ECompoundTPProperties _nProperty, double _dTime, double _dP) const;
};

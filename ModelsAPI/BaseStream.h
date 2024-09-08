/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "DefinesMDB.h"
#include "MixtureEnthalpyLookup.h"
#include "MultidimensionalGrid.h"
#include <limits>

class CH5Handler;
class CDenseMDMatrix;
class CPhase;
class CTimeDependentValue;
class CMaterialsDatabase;
class CMatrix2D;
class CTransformMatrix;

class CBaseStream;
class CStream;
class CHoldup;

/**
 * \brief Basic class for material flow description.
 * \details This is a base class from which CStream and CHoldup are derived.
 */
class CBaseStream
{
	/**
	 * \private
	 * Current version of the saving procedure.
	 */
	static const unsigned m_saveVersion{ 3 };

	/**
	 * \private
	 * Name of the stream.
	 */
	std::string m_name{ "Stream" };
	/**
	 * \private
	 * Unique key of the stream.
	 */
	std::string m_key;

protected:
	// TODO: make it global and rename
	/**
	 * \private
	 * Tolerance to compare doubles.
	 */
	inline static const double m_epsilon{ 16 * std::numeric_limits<double>::epsilon() };

	/**
	 * \private
	 * Pointer to a database of materials.
	 */
	const CMaterialsDatabase* m_materialsDB{ nullptr };

	/**
	 * \private
	 * Time points on which the stream is defined.
	 */
	std::vector<double> m_timePoints;
	/**
	 * \private
	 * Defined distribution grid.
	 */
	CMultidimensionalGrid m_grid;
	/**
	 * \private
	 * Defined overall properties.
	 */
	std::map<EOverall, std::unique_ptr<CTimeDependentValue>> m_overall;
	/**
	 * \private
	 * Defined phases.
	 */
	std::map<EPhase, std::unique_ptr<CPhase>> m_phases;
	/**
	 * \private
	 * Lookup table to calculate temperature<->enthalpy.
	 */
	mutable std::unique_ptr<CMixtureEnthalpyLookup> m_enthalpyCalculator;
	/**
	 * \private
	 * Settings for caching in the stream.
	 */
	SCacheSettings m_cacheSettings;
	/**
	 * \private
	 * Settings for tolerances in the stream.
	 */
	SToleranceSettings m_toleranceSettings;
	/**
	 * \private
	 * Settings for thermodynamics in the stream.
	 */
	SThermodynamicsSettings m_thermodynamicsSettings;

public:
	// TODO: remove empty constructor, always set pointers to MDB and grid.
	/**
	 * \private
	 * \brief Basic constructor creating an empty object.
	 * \details Creates a stream object with a unique key and a single overall property - mass.
	 * If unique key is empty, it is randomly generated.
	 * \param _key Unique key of the stream.
	 */
	CBaseStream(const std::string& _key = "");
	/**
	 * \private
	 * \brief Constructor configuring the whole structure.
	 * \details Creates a stream object with a unique key and all structural settings.
	 * If unique key is empty, it is randomly generated.
	 * \param _key Unique key of the stream.
	 * \param _materialsDB Pointer to materials database.
	 * \param _grid Multidimensional grid of distributed parameters of solids.
	 * \param _overall List of overall stream properties.
	 * \param _phases List of phases.
	 * \param _cache Cache settings.
	 * \param _tolerance Tolerance settings.
	 * \param _thermodynamics Thermodynamic settings.
	 */
	CBaseStream(const std::string& _key, const CMaterialsDatabase* _materialsDB, const CMultidimensionalGrid& _grid,
		const std::vector<SOverallDescriptor>* _overall, const std::vector<SPhaseDescriptor>* _phases,
		const SCacheSettings* _cache, const SToleranceSettings* _tolerance, const SThermodynamicsSettings* _thermodynamics);
	/**
	 * \private
	 * \brief Copy constructor.
	 * \details Copies all structural parameters and settings from the source stream, including unique key and data.
	 * \param _other Source stream.
	 */
	CBaseStream(const CBaseStream& _other);
	/**
	 * \private
	 * \brief Move constructor
	 * \details Moves all structural parameters and settings from the source stream, including unique key and data.
	 * \param _other Source stream.
	 */
	CBaseStream(CBaseStream&& _other) noexcept;
	/**
	 * \private
	 * \brief Destructor
	 */
	virtual ~CBaseStream() = default;

	CBaseStream& operator=(const CBaseStream& _other) = delete;
	CBaseStream& operator=(CBaseStream&& _other) = delete;

	/**
	 * \private
	 * \brief Removes all existing data from the stream.
	 * \details Structural data and settings are not affected.
	 */
	void Clear();

	////////////////////////////////////////////////////////////////////////////////
	// Work with structural data
	//

	/**
	 * \private
	 * \brief Sets up the stream structure (MD dimensions, phases, materials, etc.) the same as an in the given stream.
	 * \details Also removes all existing data.
	 * \param _other Pointer to source stream.
	 */
	void SetupStructure(const CBaseStream* _other);

	/**
	 * \private
	 * \brief Checks whether both streams have the same overall properties.
	 * \details
	 * \param _stream1 Const reference to the first stream.
	 * \param _stream2 Const reference to the second stream.
	 * \return Whether both streams have the same overall properties.
	 */
	static bool HaveSameOverall(const CBaseStream& _stream1, const CBaseStream& _stream2);
	/**
	 * \private
	 * \brief Checks whether both streams have the same phases.
	 * \details
	 * \param _stream1 Const reference to the first stream.
	 * \param _stream2 Const reference to the second stream.
	 * \return Whether both streams have the same phases.
	 */
	static bool HaveSamePhases(const CBaseStream& _stream1, const CBaseStream& _stream2);
	/**
	 * \private
	 * \brief Checks whether both streams have the same grid of distributed parameters of solids.
	 * \details
	 * \param _stream1 Const reference to the first stream.
	 * \param _stream2 Const reference to the second stream.
	 * \return Whether both streams have the same grid of distributed parameters of solids.
	 */
	static bool HaveSameGrids(const CBaseStream& _stream1, const CBaseStream& _stream2);
	/**
	 * \private
	 * \brief Checks whether both streams have the same overall properties and phases.
	 * \details
	 * \param _stream1 Const reference to the first stream.
	 * \param _stream2 Const reference to the second stream.
	 * \return Whether both streams have the same overall properties and phases.
	 */
	static bool HaveSameOverallAndPhases(const CBaseStream& _stream1, const CBaseStream& _stream2);
	/**
	 * \private
	 * \brief Checks whether both streams have the same overall properties, phases and grids.
	 * \details
	 * \param _stream1 Const reference to the first stream.
	 * \param _stream2 Const reference to the second stream.
	 * \return Whether both streams have the same overall properties, phases and grids.
	 */
	static bool HaveSameStructure(const CBaseStream& _stream1, const CBaseStream& _stream2);

	////////////////////////////////////////////////////////////////////////////////
	// Basic stream properties
	//

	/**
	 * \brief Returns the name of the stream.
	 * \return Name of the stream.
	 */
	[[nodiscard]] std::string GetName() const;
	/**
	 * \brief Sets new name of the stream.
	 * \param _name Name of the stream.
	 */
	void SetName(const std::string& _name);
	/**
	 * \brief Returns unique key of the stream.
	 * \return Unique key of the stream.
	 */
	[[nodiscard]] std::string GetKey() const;
	/**
	 * \brief Sets new unique key of the stream.
	 * \param _key Unique key of the stream.
	 */
	void SetKey(const std::string& _key);

	////////////////////////////////////////////////////////////////////////////////
	// Time points
	//

	/**
	 * \brief Adds a new time point.
	 * \details All data to the new time point are copied from the previous existing time point.
	 * If no other time points exist, all data are set to zero. If this time point already exists, nothing is done.
	 * \param _time New time point.
	 */
	void AddTimePoint(double _time);
	/**
	 * \brief Copies all data from one time point to another one.
	 * \details If the destination time point does not yet exist, it is added.
	 * \param _timeDst Target time point.
	 * \param _timeSrc Source time point.
	 */
	void CopyTimePoint(double _timeDst, double _timeSrc);
	/**
	 * \brief Removes the specified time point if it already exists.
	 * \param _time Time point.
	 */
	void RemoveTimePoint(double _time);
	/**
	 * \brief Removes all existing time points in the specified interval.
	 * \details The boundaries of the interval can be included or excluded from the list of deleted time points.
	 * \param _timeBeg Begin of the time interval.
	 * \param _timeEnd End of the time interval.
	 * \param _inclusive Whether to include boundaries of the interval.
	 */
	void RemoveTimePoints(double _timeBeg, double _timeEnd, bool _inclusive = true);
	/**
	 * \brief Removes all existing time points after the specified one.
	 * \details The time point itself can be included or excluded from the list of deleted time points.
	 * \param _time Begin of the time interval.
	 * \param _inclusive Whether to include boundary time point.
	 */
	void RemoveTimePointsAfter(double _time, bool _inclusive = false);
	/**
	 * \brief Removes all existing time points.
	 */
	void RemoveAllTimePoints();
	/**
	 * \brief Removes time points within the specified interval that are closer together than a specified step.
	 * \param _timeBeg Begin of the time interval.
	 * \param _timeEnd End of the time interval.
	 * \param _step Step between time points to be kept.
	 */
	void ReduceTimePoints(double _timeBeg, double _timeEnd, double _step);

	/**
	 * \brief Returns all defined time points.
	 * \return All time points which are defined in the stream.
	 */
	std::vector<double> GetAllTimePoints() const;
	/**
	 * \brief Returns all defined time points in the specified time interval.
	 * \param _timeBeg Begin of the time interval.
	 * \param _timeEnd End of the time interval.
	 * \return All time points which are defined in the stream within the time interval.
	 */
	std::vector<double> GetTimePoints(double _timeBeg, double _timeEnd) const;
	/**
	 * \brief Returns all defined time points in the specified closed time interval
	 * \details Boundaries are unconditionally included into result.
	 * \param _timeBeg Begin of the time interval.
	 * \param _timeEnd End of the time interval.
	 * \return All time points which are defined in the stream within the time interval.
	 */
	std::vector<double> GetTimePointsClosed(double _timeBeg, double _timeEnd) const;
	/**
	 * \brief Returns the last (largest) defined time point
	 * \details Returns zero if no time points have been defined in the stream.
	 * \return Last defined time point in the stream.
	 */
	double GetLastTimePoint() const;
	/**
	 * \brief Returns the nearest time point before the given one.
	 * \details Returns zero if such time point does not exist.
	 * \param _time Target time point.
	 * \return Previous time point.
	 */
	double GetPreviousTimePoint(double _time) const;

	////////////////////////////////////////////////////////////////////////////////
	// Overall parameters
	//

	/**
	 * \private
	 * \brief Adds new overall property to the stream.
	 * \details If the property already exists, returns a pointer to it.
	 * \param _property Identifier of time-dependent overall property.
	 * \param _name Name of the property.
	 * \param _units Units of measurement for the values in this property.
	 * \return Pointer to the added or already existing property.
	 */
	CTimeDependentValue* AddOverallProperty(EOverall _property, const std::string& _name, const std::string& _units);
	/**
	 * \private
	 * \brief Removes an overall property from the stream.
	 * \param _property Identifier of time-dependent overall property.
	 */
	void RemoveOverallProperty(EOverall _property);
	// TODO: maybe remove
	/**
	 * \private
	 * \brief Returns a pointer to an overall property.
	 * \details Returns nullptr if such property does not exist.
	 * \return Pointer to the property.
	 */
	CTimeDependentValue* GetOverallProperty(EOverall _property);
	// TODO: maybe remove
	/**
	 * \private
	 * \brief Returns a const pointer to an overall property.
	 * \details Returns nullptr if such property does not exist.
	 * \return Const pointer to the property.
	 */
	const CTimeDependentValue* GetOverallProperty(EOverall _property) const;

	/**
	 * \brief Returns a value of the specified overall property at the given time point.
	 * \details Returns default value if such overall property has not been defined.
	 * \param _time Target time point.
	 * \param _property Identifier of time-dependent overall property.
	 * \return Value of the specified overall property at the given time point.
	 */
	double GetOverallProperty(double _time, EOverall _property) const;
	/**
	 * \brief Returns a value of the overall mass in [kg] at the given time point.
	 * \details If such time point has not been defined, interpolation of data is done.
	 * \param _time Target time point.
	 * \return Value of the overall mass at the given time point.
	 */
	double GetMass(double _time) const;
	/**
	 * \brief Returns a value of the overall temperature in [K] at the given time point.
	 * \details Returns standard condition temperature if temperature overall property has not been defined.
	 * \param _time Target time point.
	 * \return Value of the overall temperature at the given time point.
	 */
	double GetTemperature(double _time) const;
	/**
	 * \brief Returns a value of the overall pressure in [Pa] at the given time point.
	 * \details Returns standard condition pressure if pressure overall property has not been defined.
	 * \param _time Target time point.
	 * \return Value of the overall pressure at the given time point.
	 */
	double GetPressure(double _time) const;

	/**
	 * \brief Sets a value of the specified overall property at the given time point.
	 * \details If the specified time point does not exist, it is added to the stream.
	 * If this property does not exist, nothing is done.
	 * \param _time Target time point.
	 * \param _property Identifier of the time-dependent overall property.
	 * \param _value Value of the specified overall property.
	 */
	void SetOverallProperty(double _time, EOverall _property, double _value);
	/**
	 * \brief Sets a value of the overall mass in [kg] at the given time point.
	 * \details If the specified time point does not exist, it is added to the stream.
	 * If this property does not exist, nothing is done.
	 * \param _time Target time point.
	 * \param _value Value of the overall mass.
	 */
	void SetMass(double _time, double _value);
	/**
	 * \brief Sets a value of the overall temperature in [K] at the given time point.
	 * \details If the specified time point does not exist, it is added to the stream.
	 * If this property does not exist, nothing is done.
	 * \param _time Target time point.
	 * \param _value Value of the overall temperature.
	 */
	void SetTemperature(double _time, double _value);
	/**
	 * \brief Sets a value of the overall pressure in [Pa] at the given time point.
	 * \details If the specified time point does not exist, it is added to the stream.
	 * If this property does not exist, nothing is done.
	 * \param _time Target time point.
	 * \param _value Value of the overall pressure.
	 */
	void SetPressure(double _time, double _value);

	/**
	 * \brief Returns a value of the overall amount of substance in [mol] at the given time point.
	 * \details \f$m\sum_i{w_i\sum_j{\frac{f_{i,j}}{M_j}}}\f$ with
	 * \f$m\f$ overall mass of the stream,
	 * \f$w_i\f$ mass fraction of phase \f$i\f$,
	 * \f$f_{i,j}\f$ mass fraction of compound \f$j\f$ in phase \f$i\f$,
	 * \f$M_j\f$ molar mass of compound \f$j\f$.
	 * \param _time Target time point.
	 * \return Value of the overall amount of substance.
	 */
	double GetMol(double _time) const;
	/**
	 * \brief Sets a value of the overall amount of substance in [mol] at the given time point.
	 * \details If the specified time point does not exist, it is added to the stream.
	 * The overall amount of substance \p _value is converted to mass as
	 * \f$\frac{m\cdot \_value}{\nu}\f$ with
	 * \f$m\f$ overall mass of the stream,
	 * \f$\nu\f$ overall amount of substance in [mol] as returned by CBaseStream::GetMol(double) const.
	 * \param _time Target time point.
	 * \param _value Value of the overall amount of substance.
	 */
	void SetMol(double _time, double _value);

	////////////////////////////////////////////////////////////////////////////////
	// Compounds
	//

	/**
	 * \private
	 * \brief Adds a compound with the specified unique key to the stream.
	 * \details If this compound already exists in the stream, nothing is done.
	 * \param _compoundKey Unique key of the new compound.
	 */
	void AddCompound(const std::string& _compoundKey);
	/**
	 * \private
	 * \brief Removes a compound with the specified unique key from the stream.
	 * \details If this compound does not exist in the stream, nothing is done.
	 * \param _compoundKey Unique key of the compound.
	 */
	void RemoveCompound(const std::string& _compoundKey);
	/**
	 * \brief Returns unique keys of all defined compounds.
	 * \return List of all defined compounds.
	 */
	std::vector<std::string> GetAllCompounds() const;

	/**
	 * \brief Returns the mass fraction of the compound in the total mixture at the given time point.
	 * \details \f$f_j = \sum_{i} w_i f_{i,j}\f$ with
	 * \f$f_j\f$ mass fraction of compound \f$j\f$,
	 * \f$w_i\f$ mass fraction of phase \f$i\f$,
	 * \f$f_{i,j}\f$ mass fraction of compound \f$j\f$ in phase \f$i\f$.
	 * \param _time Target time point.
	 * \param _compoundKey Unique key of the compound.
	 * \return Mass fraction of the compound.
	 */
	double GetCompoundFraction(double _time, const std::string& _compoundKey) const;
	/**
	 * \brief Returns the mass fraction of the compound in the specified phase at the given time point.
	 * \param _time Target time point.
	 * \param _compoundKey Unique key of the compound.
	 * \param _phase Phase type identifier.
	 * \return Mass fraction of the compound.
	 */
	double GetCompoundFraction(double _time, const std::string& _compoundKey, EPhase _phase) const;
	/**
	 * \brief Returns the mass of the compound in the total mixture at the given time point.
	 * \details \f$m_j = m f_j\f$ with
	 * \f$m_j\f$ mass of compound \f$j\f$,
	 * \f$m\f$ overall mass of the stream,
	 * \f$f_j\f$ mass fraction of compound \f$j\f$.
	 * \param _time Target time point.
	 * \param _compoundKey Unique key of the compound.
	 * \return Mass of the compound.
	 */
	double GetCompoundMass(double _time, const std::string& _compoundKey) const;
	/**
	 * \brief Returns the mass of the compound in the specified phase at the given time point.
	 * \details \f$m_{i,j} = m w_i f_{i,j}\f$ with
	 * \f$m_{i,j}\f$ mass of compound \f$j\f$ in phase \f$i\f$,
	 * \f$m\f$ overall mass of the stream,
	 * \f$w_i\f$ mass fraction of phase \f$i\f$,
	 * \f$f_{i,j}\f$ mass fraction of compound \f$j\f$ in phase \f$i\f$.
	 * \param _time Target time point.
	 * \param _compoundKey Unique key of the compound.
	 * \param _phase Phase type identifier.
	 * \return Mass of the compound.
	 */
	double GetCompoundMass(double _time, const std::string& _compoundKey, EPhase _phase) const;
	/**
	 * \brief Returns mass fraction of all defined compounds at the given time point.
	 * \param _time Target time point.
	 * \return Mass fraction of all defined compounds.
	 */
	std::vector<double> GetCompoundsFractions(double _time) const;
	/**
	 * \brief Returns mass fraction of all defined compounds in the specified phase at the given time point.
	 * \param _time Target time point.
	 * \param _phase Phase type identifier.
	 * \return Mass fraction of all defined compounds.
	 */
	std::vector<double> GetCompoundsFractions(double _time, EPhase _phase) const;
	/**
	 * \brief Returns masses of all defined compounds at the given time point.
	 * \param _time Target time point.
	 * \return Masses of all defined compounds.
	 */
	std::vector<double> GetCompoundsMasses(double _time) const;
	/**
	 * \brief Returns masses of all defined compounds in the specified phase at the given time point.
	 * \param _time Target time point.
	 * \param _phase Phase type identifier.
	 * \return Masses of all defined compounds.
	 */
	std::vector<double> GetCompoundsMasses(double _time, EPhase _phase) const;

	/**
	 * \brief Sets mass fraction of the compound in the specified phase at the given time point.
	 * \details If the specified time point does not exist, it is added to the stream.
	 * \param _time Target time point.
	 * \param _compoundKey Unique key of the compound.
	 * \param _phase Phase type identifier.
	 * \param _value Value of the mass fraction of the compound.
	 */
	void SetCompoundFraction(double _time, const std::string& _compoundKey, EPhase _phase, double _value);
	/**
	 * \brief Sets mass fraction of all defined compounds in all defined phases at the given time point.
	 * \details If the specified time point does not exist, it is added to the stream.
	 * \param _time Target time point.
	 * \param _value List of mass fractions of all defined compounds.
	 */
	void SetCompoundsFractions(double _time, const std::vector<double>& _value);
	/**
	 * \brief Sets mass fraction of all defined compounds in the specified phase at the given time point.
	 * \details If the specified time point does not exist, it is added to the stream.
	 * \param _time Target time point.
	 * \param _phase Phase type identifier.
	 * \param _value List of mass fractions of all defined compounds.
	 */
	void SetCompoundsFractions(double _time, EPhase _phase, const std::vector<double>& _value);
	/**
	 * \brief Sets the mass of the specified compound in the specified phase at the given time point.
	 * \details Total mass of the stream is correspondingly adjusted, masses of other compounds and phases remain the same.
	 * \param _time Target time point.
	 * \param _compoundKey Unique key of the compound.
	 * \param _phase Phase type identifier.
	 * \param _value Mass of the compound.
	 */
	void SetCompoundMass(double _time, const std::string& _compoundKey, EPhase _phase, double _value);

	/**
	 * \brief Returns the molar fraction of the compound in the specified phase at the given time point.
	 * \details \f$f_{i,j}^{mol} = f_{i,j} \frac{M_i}{M_j}\f$ with
	 * \f$f_{i,j}^{mol}\f$ mole fraction of compound \f$j\f$ in phase \f$i\f$,
	 * \f$f_{i,j}\f$ mass fraction of compound \f$j\f$ in phase \f$i\f$,
	 * \f$M_i\f$ molar mass of phase \f$i\f$,
	 * \f$M_j\f$ molar mass of compound \f$j\f$.
	 * \param _time Target time point.
	 * \param _compoundKey Unique key of the compound.
	 * \param _phase Phase type identifier.
	 * \return Molar fraction of the compound.
	 */
	double GetCompoundMolFraction(double _time, const std::string& _compoundKey, EPhase _phase) const;
	/**
	 * \brief Returns the amount of substance of the compound in the specified phase at the given time point.
	 * \details \f$\nu_{i,j} = \frac{m w_i f_{i,j}}{M_j}\f$ with
	 * \f$\nu_{i,j}\f$ amount of substance of compound \f$j\f$ in phase \f$i\f$,
	 * \f$m\f$ overall mass of the stream,
	 * \f$w_i\f$ mass fraction of phase \f$i\f$,
	 * \f$f_{i,j}\f$ mass fraction of compound \f$j\f$ in phase \f$i\f$,
	 * \f$M_j\f$ molar mass of compound \f$j\f$.
	 * \param _time Target time point.
	 * \param _compoundKey Unique key of the compound.
	 * \param _phase Phase type identifier.
	 * \return Amount of substance of the compound.
	 */
	double GetCompoundMol(double _time, const std::string& _compoundKey, EPhase _phase) const;
	/**
	 * \brief Sets the molar fraction of the compound in the specified phase at the given time point.
	 * \details If the specified time point does not exist, it is added to the stream.
	 * The molar fraction of the compound \p _value is converted to a mass fraction as
	 * \f$f_{i,j} = \f$\p _value \f$\cdot M_j \sum_j \frac{f_{i,j}}{M_j}\f$ with
	 * \f$f_{i,j}\f$ mass fraction of compound \f$j\f$ in phase \f$i\f$,
	 * \f$M_j\f$ molar mass of compound \f$j\f$.
	 * \param _time Target time point.
	 * \param _compoundKey Unique key of the compound.
	 * \param _phase Phase type identifier.
	 * \param _value Molar fraction of the compound.
	 */
	void SetCompoundMolFraction(double _time, const std::string& _compoundKey, EPhase _phase, double _value);

	////////////////////////////////////////////////////////////////////////////////
	// Phases
	//

	/**
	 * \private
	 * \brief Adds new phase to the stream.
	 * \details If the phase already exists, returns a pointer to it.
	 * \param _phase Phase type identifier.
	 * \param _name Name of the new phase.
	 * \return Pointer to the added or already existing phase.
	 */
	CPhase* AddPhase(EPhase _phase, const std::string& _name);
	/**
	 * \private
	 * \brief Removes the specified phase from the stream.
	 * \details If this phase does not exist in the stream, nothing is done.
	 * \param _phase Phase type identifier.
	 */
	void RemovePhase(EPhase _phase);
	// TODO: maybe remove
	/**
	 * \private
	 * \brief Returns a pointer to the specified phase.
	 * \details Returns nullptr if such phase does not exist.
	 * \return Pointer to a phase descriptor.
	 */
	CPhase* GetPhase(EPhase _phase);
	// TODO: maybe remove
	/**
	 * \private
	 * \brief Returns a const pointer to the specified phase.
	 * \details Returns nullptr if such phase does not exist.
	 * \return Const pointer to a phase descriptor.
	 */
	const CPhase* GetPhase(EPhase _phase) const;
	// TODO: remove this.
	/**
	 * \private
	 * \brief Removes all defined phases.
	 */
	void ClearPhases();

	/**
	 * \brief Returns the mass fraction of the specified phase at the given time point.
	 * \param _time Target time point.
	 * \param _phase Phase type identifier.
	 * \return Mass fraction of the specified phase.
	 */
	double GetPhaseFraction(double _time, EPhase _phase) const;
	/**
	 * \brief Returns the mass of the specified phase at the given time point.
	 * \details \f$m_i = m w_i\f$ with
	 * \f$m_i\f$ mass of phase \f$i\f$,
	 * \f$m\f$ overall mass of the stream,
	 * \f$w_i\f$ mass fraction of phase \f$i\f$.
	 * \param _time Target time point.
	 * \param _phase Phase type identifier.
	 * \return Mass of the specified phase.
	 */
	double GetPhaseMass(double _time, EPhase _phase) const;
	/**
	 * \brief Returns the value of the overall property of the specified phase at the given time point.
	 * \param _time Target time point.
	 * \param _phase Phase type identifier.
	 * \param _property Identifier of time-dependent overall property.
	 * \return Value of the overall property.
	 */
	double GetPhaseProperty(double _time, EPhase _phase, EOverall _property) const;
	/**
	 * \brief Returns the value of the constant physical property of the specified phase at the given time point.
	 * \details
	 * - For ::MOLAR_MASS:
	 * \f$M_i = \frac{1}{\sum_j {\frac{f_{i,j}}{M_j}}}\f$ with
	 * \f$M_i\f$ molar mass of phase \f$i\f$,
	 * \f$M_j\f$ molar mass of compound \f$j\f$,
	 * \f$f_{i,j}\f$ mass fraction of compound \f$j\f$ in phase \f$i\f$.
	 * - For other const material properties:
	 * \f$v_i = \sum_j f_{i,j} v_j\f$ with
	 * \f$v_i\f$ const physical property of phase \f$i\f$,
	 * \f$v_j\f$ value of the specified const \p _property of compound \f$j\f$,
	 * \f$f_{i,j}\f$ mass fraction of compound \f$j\f$ in phase \f$i\f$.
	 *
	 * See also: \verbatim embed:rst:inline :ref:`sec.mdb.const` \endverbatim
	 * \param _time Target time point.
	 * \param _phase Phase type identifier.
	 * \param _property Identifier of a constant material property.
	 * \return Value of the constant physical property.
	 */
	double GetPhaseProperty(double _time, EPhase _phase, ECompoundConstProperties _property) const;
	/**
	 * \brief Returns the value of the temperature/pressure-dependent physical property of the specified phase at the given time point.
	 * \details Available properties are:
	 * - ::DENSITY:
	 *	- For solid phase with porosity distribution:
	 *	\f$\rho = \sum_{i,j} \rho_i (1 - \varepsilon_j) f_{i,j}\f$ with
	 *	\f$\varepsilon_j\f$ porosity in interval \f$j\f$,
	 *	\f$f_{i,j}\f$ mass fraction of compound \f$i\f$ with porosity \f$j\f$.
	 *	- For solid and liquid phase:
	 *	\f$\rho = \sum_i w_i \cdot \rho_i \f$ with
	 *	\f$w_i\f$ mass fraction of compound \f$i\f$ in \p _phase.
	 *  - For vapor phase:
	 *  \f$\rho = \sum x_i \cdot \rho_i \f$ with
	 *  \f$x_i\f$ as the mole fraction of component \f$i\f$ in \p _phase.
	 * - ::HEAT_CAPACITY_CP:
	 * \f$C_p = \sum_i w_i \cdot C_{p,i}\f$ with
	 * \f$C_{p,i}\f$ heat capacity of compound \f$i\f$,
	 * \f$w_i\f$ mass fraction of compound \f$i\f$ in \p _phase.
	 * - ::VAPOR_PRESSURE:
	 * \f$P_v = \min_{i} (P_v)_i\f$ with
	 * \f$(P_v)_i\f$ vapor pressure of compound \f$i\f$.
	 * - ::VISCOSITY:
	 *	- For solid phase:
	 *	\f$\eta = \sum\limits_i w_i \eta_i\f$ with
	 *	\f$\eta_i\f$ viscosity of compound \f$i\f$,
	 *	\f$w_i\f$ mass fraction of compound \f$i\f$.
	 *	- For liquid phase:
	 *	\f$\ln \eta = \sum_i w_i \ln \eta_i\f$ with
	 *	\f$\eta_i\f$ viscosity of compound \f$i\f$,
	 *	\f$w_i\f$ mass fraction of compound \f$i\f$ in \p _phase.
	 *	- For vapor phase:
	 *	\f$\eta = \frac{\sum_i x_i \sqrt{M_i} \eta_i}{\sum_i x_i \sqrt{M_i}}\f$ with
	 *	\f$\eta_i\f$ viscosity of compound \f$i\f$,
	 *	\f$w_i\f$ mass fraction of compound \f$i\f$ in \p _phase,
	 *	\f$x_i\f$ the mole fraction of compound \f$i\f$ in \p _phase.
	 * - ::THERMAL_CONDUCTIVITY:
	 *	- For solid phase:
	 *	\f$\lambda = \sum_i w_i \lambda_i\f$ with
	 *	\f$\lambda_i\f$ thermal conductivity of compound \f$i\f$.
	 *	- For liquid phase:
	 *	\f$\lambda = \frac{1}{\sqrt{\sum_i x_i \lambda_i^{-2}}}\f$ with
	 *	\f$\lambda_i\f$ thermal conductivity of compound \f$i\f$.
	 *	- For vapor phase:
	 *	\f$\lambda = \sum_i \frac{x_i \lambda_i}{\sum_j x_j F_{i,j}}\f$,
	 *	\f$F_{i,j} = \frac{(1 + \sqrt{\lambda_i^4 / \lambda_j} \sqrt{M_j / M_i})^2}{\sqrt{8(1 + M_i / M_j)}}\f$ with
	 *	\f$M_i\f$ the molar mass of compound \f$i\f$.
	 * - ::PERMITTIVITY:
	 * \f$\varepsilon = \sum_i w_i \varepsilon_i\f$ with
	 * \f$\varepsilon_i\f$ permittivity of compound \f$i\f$,
	 * \f$w_i\f$ mass fraction of compound \f$i\f$ in \p _phase.
	 * - ::ENTHALPY:
	 * \f$H = \sum_i w_i H_i\f$ with
	 * \f$H_i\f$ enthalpy of compound \f$i\f$,
	 * \f$w_i\f$ mass fraction of compound \f$i\f$ in \p _phase.
	 * - ::EQUILIBRIUM_MOISTURE_CONTENT:
	 * \f$M = \sum_i w_i M_i\f$ with
	 * \f$M_i\f$ equilibrium moisture content of compound \f$i\f$,
	 * \f$w_i\f$ mass fraction of compound \f$i\f$ in \p _phase.
	 * - ::MASS_DIFFUSION_COEFFICIENT:
	 * \f$D = \sum_i w_i D_i\f$ with
	 * \f$D_i\f$ mass diffusion coefficient of compound \f$i\f$,
	 * \f$w_i\f$ mass fraction of compound \f$i\f$ in \p _phase.
	 * - ::TP_PROP_USER_DEFINED_01 - ::TP_PROP_USER_DEFINED_20:
	 * \f$Y = \sum_i w_i Y_i\f$ with
	 * \f$Y_i\f$ property value of compound \f$i\f$,
	 * \f$w_i\f$ mass fraction of compound \f$i\f$ in \p _phase.
	 *
	 * See also: \verbatim embed:rst:inline :ref:`sec.mdb.tpd` \endverbatim
	 * \param _time Target time point.
	 * \param _phase Phase type identifier.
	 * \param _property Identifier of temperature/pressure-dependent property.
	 * \return Value of the temperature/pressure-dependent physical property.
	 */
	double GetPhaseProperty(double _time, EPhase _phase, ECompoundTPProperties _property) const;

	/**
	 * \brief Sets the mass fraction of the specified phase at the given time point.
	 * \details If the specified time point does not exist, it is added to the stream.
	 * If there is no specified phase in the stream, nothing is done.
	 * \param _time Target time point.
	 * \param _phase Phase type identifier.
	 * \param _value Phase mass fraction.
	 */
	void SetPhaseFraction(double _time, EPhase _phase, double _value);
	/**
	 * \brief Sets the mass of the specified phase at the given time point.
	 * \details If the specified time point does not exist, it is added to the stream.
	 * Total mass of the stream is correspondingly adjusted, masses of other phases remain the same.
	 * If there is no specified phase in the stream, nothing is done.
	 * Input parameter \p _value is the mass of the defined phase: \f$m_i =\f$ \p _value and \f$w_i = m_i / m\f$.
	 * The total mass \f$m\f$ changes due to assignment of \f$m_i\f$: \f$m = m_{old} + (\f$ \p _value \f$- m_{i,old})\f$.
	 * \f$m_i\f$ mass of phase \f$i\f$, \f$w_i\f$ mass fraction of phase \f$i\f$, \f$m\f$ overall mass of the stream.
	 * \param _time Target time point.
	 * \param _phase Phase type identifier.
	 * \param _value Phase mass.
	 */
	void SetPhaseMass(double _time, EPhase _phase, double _value);

	/**
	 * \brief Returns the molar fraction of the specified phase at the given time point.
	 * \details \f$x_i = \frac{\nu_i}{\nu}\f$,
	 * \f$x_i\f$ molar fraction of phase \f$i\f$,
	 * \f$\nu_i\f$ amount of substance of phase \f$i\f$,
	 * \f$\nu\f$ value of the overall amount of substance.
	 * \param _time Target time point.
	 * \param _phase Phase type identifier.
	 * \return Molar fraction of the specified phase.
	 */
	double GetPhaseMolFraction(double _time, EPhase _phase) const;
	/**
	 * \brief Returns the amount of substance of the specified phase at the given time point.
	 * \details \f$\nu_i = \sum_j \nu_{i,j}\f$,
	 * \f$\nu_i\f$ amount of substance of phase \f$i\f$,
	 * \f$\nu_{i,j}\f$ amount of substance of compound \f$j\f$ in phase \f$i\f$.
	 * \param _time Target time point.
	 * \param _phase Phase type identifier.
	 * \return Amount of substance of the specified phase.
	 */
	double GetPhaseMol(double _time, EPhase _phase) const;
	/**
	 * \brief Sets the molar fraction of the specified phase at the given time point.
	 * \details If the specified time point does not exist, it is added to the stream.
	 * Input parameter \p _value is the molar fraction of the defined phase:
	 * \f$f_i =\f$ \p _value \f$\cdot \frac{f_i}{x_i}\f$ with
	 * \f$f_i\f$ mass fraction of phase \f$i\f$,
	 * \f$x_i\f$ molar fraction of phase \f$i\f$.
	 * \param _time Target time point.
	 * \param _phase Phase type identifier.
	 * \param _value Molar fraction of the specified phase.
	 */
	void SetPhaseMolFraction(double _time, EPhase _phase, double _value);
	/**
	 * \brief Sets the amount of substance of the specified phase at the given time point.
	 * \details If the specified time point does not exist, it is added to the stream.
	 * Total mass of the stream is correspondingly adjusted, masses of other phases remain the same.
	 * Input parameter \p _value is the amount of substance of one defined phase:
	 * \f$m_i = \f$\p _value \f$\cdot \frac{m_i}{n_i}\f$ with
	 * \f$m_i\f$ mass of phase \f$i\f$,
	 * \f$n_i\f$ amount of substance of phase \f$i\f$.
	 * \param _time Target time point.
	 * \param _phase Phase type identifier.
	 * \param _value Amount of substance of the specified phase.
	 */
	void SetPhaseMol(double _time, EPhase _phase, double _value);

	////////////////////////////////////////////////////////////////////////////////
	// Properties of the total mixture
	//

	/**
	 * \brief Returns the value of the property of the total mixture in the stream at the given time point.
	 * \details Refer to function CBaseStream::GetOverallProperty(double, EOverall) const.
	 * \param _time Target time point.
	 * \param _property Identifier of time-dependent overall property.
	 * \return Value of the property of the total mixture.
	 */
	double GetMixtureProperty(double _time, EOverall _property) const;
	/**
	 * \brief Returns the value of the constant physical property of the total mixture in the stream at the given time point.
	 * \details \f$v = \sum_i v_i w_i\f$ with
	 * \f$v\f$ value of the const physical property of the total mixture,
	 * \f$v_i\f$ value of the const physical property of phase \f$i\f$,
	 * \f$w_i\f$ mass fraction of phase \f$i\f$.
	 * \param _time Target time point.
	 * \param _property Identifier of constant material property.
	 * \return Value of the constant physical property of the total mixture.
	 */
	double GetMixtureProperty(double _time, ECompoundConstProperties _property) const;
	/**
	 * \brief Returns the value of the temperature/pressure-dependent physical property of the total mixture in the stream at the given time point.
	 * \details \f$v = \sum_i v_i w_i\f$ with
	 * \f$v\f$ value of the temperature/pressure-dependent physical property of the total mixture,
	 * \f$v_i\f$ value of the temperature/pressure-dependent physical property of phase \f$i\f$,
	 * \f$w_i\f$ mass fraction of phase \f$i\f$.
	 * \param _time Target time point.
	 * \param _property Identifier of temperature/pressure-dependent property.
	 * \return Value of the temperature/pressure-dependent physical property of the total mixture.
	 */
	double GetMixtureProperty(double _time, ECompoundTPProperties _property) const;

	/**
	 * \brief Sets the value of the property of the total mixture in the stream at the given time point.
	 * \details If the specified time point does not exist, it is added to the stream.
	 * Refer to function CBaseStream::SetOverallProperty(double, EOverall, double).
	 * \param _time Target time point.
	 * \param _property Identifier of time-dependent overall property.
	 * \param _value Value of the property of the total mixture.
	 */
	void SetMixtureProperty(double _time, EOverall _property, double _value);

	////////////////////////////////////////////////////////////////////////////////
	// Material database
	//

	/**
	 * \brief Returns the value of the constant physical property of the specified compound.
	 * \details See also: \verbatim embed:rst:inline :ref:`sec.mdb.const` \endverbatim
	 * \param _compoundKey Unique key of the compound.
	 * \param _property Identifier of constant material property.
	 * \return Value of the constant physical property of the specified compound.
	 */
	double GetCompoundProperty(const std::string& _compoundKey, ECompoundConstProperties _property) const;
	/**
	 * \brief Returns the value of the temperature/pressure-dependent physical property of the specified compound with the given temperature [K] and pressure [Pa].
	 * \details See also: \verbatim embed:rst:inline :ref:`sec.mdb.tpd` \endverbatim
	 * \param _compoundKey Unique key of the compound.
	 * \param _property Identifier of temperature/pressure-dependent property.
	 * \param _temperature Value of temperature in [K].
	 * \param _pressure Value of pressure in [Pa].
	 * \return Value of the temperature/pressure-dependent physical property of the specified compound.
	 */
	double GetCompoundProperty(const std::string& _compoundKey, ECompoundTPProperties _property, double _temperature, double _pressure) const;
	/**
	 * \brief Returns the value of the temperature/pressure-dependent physical property of the specified compound at temperature and pressure at the given time point.
	 * \details Refer to function CBaseStream::GetCompoundProperty(const std::string&, ECompoundTPProperties, double, double) const.
	 * See also: \verbatim embed:rst:inline :ref:`sec.mdb.tpd` \endverbatim
	 * \param _time Target time point.
	 * \param _compoundKey Unique key of the compound.
	 * \param _property Identifier of temperature/pressure-dependent property.
	 * \return Value of the temperature/pressure-dependent physical property of the specified compound.
	 */
	double GetCompoundProperty(double _time, const std::string& _compoundKey, ECompoundTPProperties _property) const;
	/**
	 * \brief Returns the value of the interaction physical property between the specified compounds with the given specified temperature [K] and pressure [Pa].
	 * \details See also: \verbatim embed:rst:inline :ref:`sec.mdb.interactions` \endverbatim
	 * \param _compoundKey1 Unique key of the first compound.
	 * \param _compoundKey2 Unique key of the second compound.
	 * \param _property Identifier of property, defined for interaction of two compounds.
	 * \param _temperature Value of temperature in [K].
	 * \param _pressure Value of pressure in [Pa].
	 * \return Value of the interaction physical property between the specified compounds.
	 */
	double GetCompoundProperty(const std::string& _compoundKey1, const std::string& _compoundKey2, EInteractionProperties _property, double _temperature, double _pressure) const;
	/**
	 * \brief Returns the value of the interaction physical property between the specified compounds at temperature and pressure at the given time point.
	 * \details Refer to function CBaseStream::GetCompoundProperty(const std::string&, const std::string&, EInteractionProperties, double, double) const.
	 * See also: \verbatim embed:rst:inline :ref:`sec.mdb.interactions` \endverbatim
	 * \param _time Target time point.
	 * \param _compoundKey1 Unique key of the first compound.
	 * \param _compoundKey2 Unique key of the second compound.
	 * \param _property Identifier of property, defined for interaction of two compounds.
	 * \return Value of the interaction physical property between the specified compounds.
	 */
	double GetCompoundProperty(double _time, const std::string& _compoundKey1, const std::string& _compoundKey2, EInteractionProperties _property) const;

	////////////////////////////////////////////////////////////////////////////////
	// Distributed properties of the solid phase
	//

	/**
	 * \brief Returns mass fraction of the solid material at the specified multidimensional coordinates given for all defined dimensions.
	 * \param _time Target time point.
	 * \param _coords Multidimensional coordinates.
	 * \return Mass fraction of the solid material.
	 */
	double GetFraction(double _time, const std::vector<size_t>& _coords) const;
	/**
	 * \brief Sets mass fraction of the solid material at the specified multidimensional coordinates given for all defined dimensions.
	 * \details If the specified time point does not exist, it is added to the stream.
	 * \param _time Target time point.
	 * \param _coords Multidimensional coordinates.
	 * \param _value Mass fraction of the solid material.
	 */
	void SetFraction(double _time, const std::vector<size_t>& _coords, double _value);

	/**
	 * \brief Returns one-dimensional distribution of the solid material over the specified parameter at the given time point.
	 * \param _time Target time point.
	 * \param _distribution Type of distributed parameter of the solid phase.
	 * \return One-dimensional distribution of the solid material.
	 */
	std::vector<double> GetDistribution(double _time, EDistrTypes _distribution) const;
	/**
	 * \brief Returns two-dimensional distribution of the solid material over the specified parameters at the given time point.
	 * \param _time Target time point.
	 * \param _distribution1 First distributed parameter type of the solid phase.
	 * \param _distribution2 Second distributed parameter type of the solid phase.
	 * \return Two-dimensional distribution of the solid material.
	 */
	CMatrix2D GetDistribution(double _time, EDistrTypes _distribution1, EDistrTypes _distribution2) const;
	/**
	 * \brief Returns multi-dimensional distribution of the solid material over the specified parameters at the given time point.
	 * \param _time Target time point.
	 * \param _distributions List of distributed parameter types of the solid phase.
	 * \return Multi-dimensional distribution of the solid material.
	 */
	CDenseMDMatrix GetDistribution(double _time, const std::vector<EDistrTypes>& _distributions) const;
	/**
	 * \brief Returns one-dimensional distribution of the solid material over the specified parameter for the given compound and time point.
	 * \details Input dimensions should not include distribution by compounds (::DISTR_COMPOUNDS).
	 * If the specified compound has not been defined in the stream, empty vector is returned.
	 * \param _time Target time point.
	 * \param _distribution Type of distributed parameter of the solid phase.
	 * \param _compoundKey Unique key of the compound.
	 * \return One-dimensional distribution of the solid material.
	 */
	std::vector<double> GetDistribution(double _time, EDistrTypes _distribution, const std::string& _compoundKey) const;
	/**
	 * \brief Returns two-dimensional distribution of the solid material over the specified parameters for the given compound and time point.
	 * \details Input dimensions should not include distribution by compounds (::DISTR_COMPOUNDS).
	 * If the specified compound has not been defined in the stream, empty matrix is returned.
	 * \param _time Target time point.
	 * \param _distribution1 First distributed parameter type of the solid phase.
	 * \param _distribution2 Second distributed parameter type of the solid phase.
	 * \param _compoundKey Unique key of the compound.
	 * \return Two-dimensional distribution of the solid material.
	 */
	CMatrix2D GetDistribution(double _time, EDistrTypes _distribution1, EDistrTypes _distribution2, const std::string& _compoundKey) const;
	/**
	 * \brief Returns multi-dimensional distribution of the solid material over the specified parameters for the given compound and time point.
	 * \details Input dimensions should not include distribution by compounds (::DISTR_COMPOUNDS).
	 * If specified compound has not been defined in the stream, empty matrix is returned.
	 * \param _time Target time point.
	 * \param _distributions List of distributed parameter types of the solid phase.
	 * \param _compoundKey Unique key of the compound.
	 * \return Multi-dimensional distribution of the solid material.
	 */
	CDenseMDMatrix GetDistribution(double _time, const std::vector<EDistrTypes>& _distributions, const std::string& _compoundKey) const;

	/**
	 * \brief Sets one-dimensional distribution of the solid material over the specified parameter at the given time point.
	 * \details If such dimension does not exist, nothing is done.
	 * If the specified time point does not exist, it is added to the stream.
	 * \param _time Target time point.
	 * \param _distribution Type of distributed parameter of the solid phase.
	 * \param _value One-dimensional distribution of the solid material.
	 */
	void SetDistribution(double _time, EDistrTypes _distribution, const std::vector<double>& _value);
	/**
	 * \brief Sets two-dimensional distribution of the solid material over the specified parameter at the given time point.
	 * \details If such dimensions do not exist, nothing is done.
	 * If the specified time point does not exist, it is added to the stream.
	 * \param _time Target time point.
	 * \param _distribution1 First distributed parameter type of the solid phase.
	 * \param _distribution2 Second distributed parameter type of the solid phase.
	 * \param _value Two-dimensional distribution of the solid material.
	 */
	void SetDistribution(double _time, EDistrTypes _distribution1, EDistrTypes _distribution2, const CMatrix2D& _value);
	/**
	 * \brief Sets multi-dimensional distribution of the solid material over the specified parameter at the given time point.
	 * \details If such dimensions do not exist, nothing is done.
	 * If the specified time point does not exist, it is added to the stream.
	 * \param _time Target time point.
	 * \param _value Multi-dimensional distribution of the solid material.
	 */
	void SetDistribution(double _time, const CDenseMDMatrix& _value);
	/**
	 * \brief Sets one-dimensional distribution of the solid material over the specified parameter for the given compound and time point.
	 * \details If such compound or dimension does not exist, nothing is done.
	 * Input dimensions should not include distribution by compounds (::DISTR_COMPOUNDS).
	 * If the specified time point does not exist, it is added to the stream.
	 * \param _time Target time point.
	 * \param _distribution Type of distributed parameter of the solid phase.
	 * \param _compoundKey Unique key of the compound.
	 * \param _value One-dimensional distribution of the solid material.
	 */
	void SetDistribution(double _time, EDistrTypes _distribution, const std::string& _compoundKey, const std::vector<double>& _value);
	/**
	 * \brief Sets two-dimensional distribution of the solid material over the specified parameters for the given compound and time point.
	 * \details If such time point, compound or dimensions do not exist, nothing is done.
	 * Input dimensions should not include distribution by compounds (::DISTR_COMPOUNDS).
	 * If the specified time point does not exist, it is added to the stream.
	 * \param _time Target time point.
	 * \param _distribution1 First distributed parameter type of the solid phase.
	 * \param _distribution2 Second distributed parameter type of the solid phase.
	 * \param _compoundKey Unique key of the compound.
	 * \param _value Two-dimensional distribution of the solid material.
	 */
	void SetDistribution(double _time, EDistrTypes _distribution1, EDistrTypes _distribution2, const std::string& _compoundKey, const CMatrix2D& _value);
	/**
	 * \brief Sets multi-dimensional distribution of the solid material over the specified parameters for the given compound and time point.
	 * \details If such compound or dimensions do not exist, nothing is done.
	 * Input dimensions should not include distribution by compounds (::DISTR_COMPOUNDS).
	 * If the specified time point does not exist, it is added to the stream.
	 * \param _time Target time point.
	 * \param _compoundKey Unique key of the compound.
	 * \param _value Multi-dimensional distribution of the solid material.
	 */
	void SetDistribution(double _time, const std::string& _compoundKey, const CDenseMDMatrix& _value);

	/**
	 * \brief Applies the transformation matrix to transform the multidimensional distributed parameters of the solid material at the given time point.
	 * \param _time Target time point.
	 * \param _matrix Transformation matrix.
	 */
	void ApplyTM(double _time, const CTransformMatrix& _matrix);
	/**
	 * \brief Applies the transformation matrix to transform the multidimensional distributed parameters of the solid material for the given compound and time point.
	 * \details Dimensions of transformation matrix should not include distribution by compounds (::DISTR_COMPOUNDS).
	 * \param _time Target time point.
	 * \param _compoundKey Unique key of the compound.
	 * \param _matrix Transformation matrix.
	 */
	void ApplyTM(double _time, const std::string& _compoundKey, const CTransformMatrix& _matrix);

	/**
	 * \brief Normalizes data in the solids distribution matrix for the given time point.
	 * \details If time has not been defined, nothing is done.
	 * \param _time Target time point.
	 */
	void Normalize(double _time);
	/**
	 * \brief Normalizes data in the solids distribution matrix for the given time interval.
	 * \param _timeBeg Begin of the time interval.
	 * \param _timeEnd End of the time interval.
	 */
	void Normalize(double _timeBeg, double _timeEnd);
	/**
	 * \brief Normalizes data in the solids distribution matrix for all time points.
	 */
	void Normalize();

	////////////////////////////////////////////////////////////////////////////////
	// Particle size distributions
	//

	/**
	 * \brief Returns the specified type of the PSD of the total mixture of all solid materials at the given time point.
	 * \details Refer to function CBaseStream::GetPSD(double, EPSDTypes, const std::vector<std::string>&, EPSDGridType) const.
	 * \param _time Target time point.
	 * \param _type Identifier of the PSD type.
	 * \param _grid Identifier of grid units type.
	 * \return Particle size distribution.
	 */
	std::vector<double> GetPSD(double _time, EPSDTypes _type, EPSDGridType _grid = EPSDGridType::DIAMETER) const;
	/**
	 * \brief Returns the specified type of the PSD of the selected compound at the given time point.
	 * \details Refer to function CBaseStream::GetPSD(double, EPSDTypes, const std::vector<std::string>&, EPSDGridType) const.
	 * \param _time Target time point.
	 * \param _type Identifier of the PSD type.
	 * \param _compoundKey Unique key of the compound.
	 * \param _grid Identifier of grid units type.
	 * \return Particle size distribution.
	 */
	std::vector<double> GetPSD(double _time, EPSDTypes _type, const std::string& _compoundKey, EPSDGridType _grid = EPSDGridType::DIAMETER) const;
	/**
	* \brief Returns the specified type of the PSD of the mixture of selected compounds at the given time point.
	* \details If the list of compounds is empty, the whole mixture is considered.
	* \param _time Target time point.
	* \param _type Identifier of the PSD type.
	* \param _compoundKeys Unique keys of the compounds.
	* \param _grid Identifier of grid units type.
	* \return Particle size distribution.
	*/
	std::vector<double> GetPSD(double _time, EPSDTypes _type, const std::vector<std::string>& _compoundKeys, EPSDGridType _grid = EPSDGridType::DIAMETER) const;
	/**
	* \brief Sets the specified type of the PSD of the total mixture of all solid materials at the given time point.
	* \details For number-related PSD, the distribution is normalized and the total particle mass remains unchanged.
	* If the specified time point does not exist, it is added to the stream.
	* Refer to function CBaseStream::SetPSD(double, EPSDTypes, const std::string&, const std::vector<double>&, EPSDGridType).
	* \param _time Target time point.
	* \param _type Identifier of PSD type.
	* \param _value Particle size distribution.
	* \param _grid Identifier of grid units type.
	*/
	void SetPSD(double _time, EPSDTypes _type, const std::vector<double>& _value, EPSDGridType _grid = EPSDGridType::DIAMETER);
	 /**
	 * \brief Sets the specified type of the PSD of the selected compound at the given time point.
	 * \details For number-related PSD, the distribution is normalized and the total particle mass remains unchanged.
	 * If the compound key if empty, the whole mixture is considered.
	 * If the specified time point does not exist, it is added to the stream.
	 * As mass fractions are used to store data, PSD is converted using functions
	 * Convertq0ToMassFractions(), ConvertQ0ToMassFractions(), Convertq2ToMassFractions(), ConvertQ2ToMassFractions(), Convertq3ToMassFractions(), ConvertQ3ToMassFractions(), ConvertNumbersToMassFractions().
	 * \param _time Target time point.
	 * \param _type Identifier of PSD type.
	 * \param _compoundKey Unique key of the compound.
	 * \param _value Particle size distribution.
	 * \param _grid Identifier of grid units type.
	 */
	void SetPSD(double _time, EPSDTypes _type, const std::string& _compoundKey, const std::vector<double>& _value, EPSDGridType _grid = EPSDGridType::DIAMETER);

	////////////////////////////////////////////////////////////////////////////////
	// Interactions with other streams
	//

	/**
	 * \brief Copies all stream data at the given time point.
	 * \details All data after the time point are removed from the destination stream.
	 * \param _time Time point to copy.
	 * \param _source Source stream.
	 */
	void Copy(double _time, const CBaseStream& _source);
	/**
	 * \brief Copies all stream data at the given time interval.
	 * \details All data after the end time point are removed from the destination stream.
	 * \param _timeBeg Begin of the time interval to copy.
	 * \param _timeEnd End of the time interval to copy.
	 * \param _source Source stream.
	 */
	void Copy(double _timeBeg, double _timeEnd, const CBaseStream& _source);
	/**
	 * \brief Copies all stream data to the given time point from another time point of the source stream.
	 * \details All data after the time point are removed from the destination stream.
	 * \param _timeDst Time point of the destination stream to copy to.
	 * \param _source Source stream.
	 * \param _timeSrc Time point of the source stream to copy.
	 */
	void Copy(double _timeDst, const CBaseStream& _source, double _timeSrc);

	/**
	 * \brief Mixes the specified stream with the current stream at the given time point.
	 * \details Can be applied only for streams with the same structure (MD dimensions, phases, materials, etc.).
	 * \param _time Time point to copy.
	 * \param _source Source stream.
	 */
	void Add(double _time, const CBaseStream& _source);
	/**
	 * \brief Mixes the specified stream with the current stream at the given time interval.
	 * \details Can be applied only for streams with the same structure (MD dimensions, phases, materials, etc.).
	 * The stream will contain the union of time points from both streams.
	 * \param _timeBeg Begin of the time interval to copy.
	 * \param _timeEnd End of the time interval to copy.
	 * \param _source Source stream.
	 */
	void Add(double _timeBeg, double _timeEnd, const CBaseStream& _source);

	/**
	 * \private
	 * \brief Tests whether all values in the streams at the given time point are equal up to the global tolerances.
	 * \param _time Target time point.
	 * \param _stream1 First stream.
	 * \param _stream2 Second stream.
	 * \return Whether streams are equal.
	 */
	static bool AreEqual(double _time, const CBaseStream& _stream1, const CBaseStream& _stream2);

	/**
	 * \private
	 * \brief Tests whether all values in the stream at the given time points are equal up to the tolerances.
	 * \param _time1 First target time point.
	 * \param _time2 Second target time point.
	 * \param _stream Target stream.
	 * \param _absTol Absolute tolerance.
	 * \param _relTol Relative tolerance.
	 * \return Whether streams are equal.
	 */
	static bool AreEqual(double _time1, double _time2, const CBaseStream& _stream, double _absTol, double _relTol);
	/**
	 * \private
	 * \brief Tests whether all values in the stream at the given time points are equal up to the global tolerances.
	 * \param _time1 First target time point.
	 * \param _time2 Second target time point.
	 * \param _stream Target stream.
	 * \return Whether streams are equal.
	 */
	static bool AreEqual(double _time1, double _time2, const CBaseStream& _stream);

	////////////////////////////////////////////////////////////////////////////////
	// Thermodynamics
	//

	/**
	 * \private
	 * \brief Returns a pointer to enthalpy calculator.
	 * \return Pointer to enthalpy calculator.
	 */
	[[nodiscard]] CMixtureEnthalpyLookup* GetEnthalpyCalculator() const;

	/**
	 * \brief Calculates enthalpy of the stream mixture for the temperature at the given time point using a lookup table.
	 * \details See also: \verbatim embed:rst:inline :ref:`sec.development.api.thermodynamics` \endverbatim
	 * \param _time Target time point.
	 * \return Calculated value of enthalpy.
	 */
	[[nodiscard]] double CalculateEnthalpyFromTemperature(double _time) const;
	/**
	 * \brief Calculates temperature of the stream mixture for the enthalpy at the given time point using a lookup table.
	 * \details See also: \verbatim embed:rst:inline :ref:`sec.development.api.thermodynamics` \endverbatim
	 * \param _time Target time point.
	 * \return Calculated value of temperature.
	 */
	[[nodiscard]] double CalculateTemperatureFromEnthalpy(double _time) const;

	////////////////////////////////////////////////////////////////////////////////
	// Other
	//

	/**
	 * \private
	 * \brief Returns current grid of distributed parameters.
	 * \return Const reference to current multidimensional grid.
	 */
	const CMultidimensionalGrid& GetGrid() const;

	// TODO: remove, initialize MDB in constructor
	/**
	 * \private
	 * \brief Sets new pointer to the used materials database.
	 * \param _database Const pointer to the materials database.
	 */
	void SetMaterialsDatabase(const CMaterialsDatabase* _database);

	/**
	 * \private
	 * \brief Sets grids of distributed parameters.
	 * \param _grid Const reference to the multidimensional grid.
	 */
	void SetGrid(const CMultidimensionalGrid& _grid);

	/**
	 * \private
	 * \brief Sets new cache settings.
	 * \param _settings Const reference to the cache settings.
	 */
	void SetCacheSettings(const SCacheSettings& _settings);

	/**
	 * \private
	 * \brief Sets tolerance settings.
	 * \param _settings Const reference to the tolerance settings.
	 */
	void SetToleranceSettings(const SToleranceSettings& _settings);

	/**
	 * \private
	 * \brief Sets thermodynamics settings.
	 * \param _settings Const reference to the thermodynamics settings.
	 */
	void SetThermodynamicsSettings(const SThermodynamicsSettings& _settings);

	/**
	 * \private
	 * \brief Performs nearest-neighbor extrapolation of all stream data.
	 * \param _timeExtra Time point to extrapolate.
	 * \param _time Source time point.
	 */
	void Extrapolate(double _timeExtra, double _time);
	/**
	 * \private
	 * \brief Performs linear extrapolation of all stream data.
	 * \param _timeExtra Time point to extrapolate.
	 * \param _time1 Begin of the source time interval.
	 * \param _time2 End of the source time interval.
	 */
	void Extrapolate(double _timeExtra, double _time1, double _time2);
	/**
	 * \private
	 * \brief Performs cubic spline extrapolation of all stream data.
	 * \param _timeExtra Time point to extrapolate.
	 * \param _time1 First source time point.
	 * \param _time2 Second source time point.
	 * \param _time3 Third source time point.
	 */
	void Extrapolate(double _timeExtra, double _time1, double _time2, double _time3);

	/**
	 * \private
	 * \brief Saves data to file.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data.
	 */
	void SaveToFile(CH5Handler& _h5File, const std::string& _path);
	/**
	 * \private
	 * \brief Loads data from file.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data.
	 */
	void LoadFromFile(const CH5Handler& _h5File, const std::string& _path);
	/**
	 * \private
	 * \brief Loads data from file of an older version.
	 * \param _h5File Reference to the file handler.
	 * \param _path Path to data.
	 */
	void LoadFromFile_v1(const CH5Handler& _h5File, const std::string& _path);

protected:
	/**
	 * \private
	 */
	using mix_type = std::tuple<std::map<EOverall, double>, std::map<EPhase, double>, std::map<EPhase, CDenseMDMatrix>>;
	/**
	 * \private
	 * \brief Calculates mixture of two streams.
	 * \details Does not perform any checks.
	 * \param _time1 Target time point of the first stream.
	 * \param _stream1 Const reference to the first stream.
	 * \param _mass1 Overall mass of the first stream.
	 * \param _time2 Target time point of the second stream.
	 * \param _stream2 Const reference to the second stream.
	 * \param _mass2 Overall mass of the second stream.
	 * \return Calculated mixture.
	 */
	static mix_type CalculateMix(double _time1, const CBaseStream& _stream1, double _mass1, double _time2, const CBaseStream& _stream2, double _mass2);
	/**
	 * \private
	 * \brief Calculates pressure of the mixture of two streams.
	 * \details Does not perform any checks.
	 * \param _time1 Target time point of the first stream.
	 * \param _stream1 Const reference to the first stream.
	 * \param _time2 Target time point of the second stream.
	 * \param _stream2 Const reference to the second stream.
	 * \return Calculated pressure.
	 */
	static double CalculateMixPressure(double _time1, const CBaseStream& _stream1, double _time2, const CBaseStream& _stream2);
	/**
	 * \private
	 * \brief Calculates temperature of the mixture of two streams.
	 * \details Does not perform any checks.
	 * \param _time1 Target time point of the first stream.
	 * \param _stream1 Const reference to the first stream.
	 * \param _mass1 Overall mass of the first stream.
	 * \param _time2 Target time point of the second stream.
	 * \param _stream2 Const reference to the second stream.
	 * \param _mass2 Overall mass of the second stream.
	 * \return Calculated temperature.
	 */
	static double CalculateMixTemperature(double _time1, const CBaseStream& _stream1, double _mass1, double _time2, const CBaseStream& _stream2, double _mass2);
	/**
	 * \private
	 * \brief Calculates general overall property of the mixture of two streams.
	 * \details Does not perform any checks.
	 * \param _time1 Target time point of the first stream.
	 * \param _stream1 Const reference to the first stream.
	 * \param _mass1 Overall mass of the first stream.
	 * \param _time2 Target time point of the second stream.
	 * \param _stream2 Const reference to the second stream.
	 * \param _mass2 Overall mass of the second stream.
	 * \param _property Identifier of time-dependent overall parameter that needs to be calculated.
	 * \return Calculated overall property.
	 */
	static double CalculateMixOverall(double _time1, const CBaseStream& _stream1, double _mass1, double _time2, const CBaseStream& _stream2, double _mass2, EOverall _property);
	/**
	 * \private
	 * \brief Calculates phase fractions of the mixture of two streams for the given phase.
	 * \details Does not perform any checks.
	 * \param _time1 Target time point of the first stream.
	 * \param _stream1 Const reference to the first stream.
	 * \param _mass1 Overall mass of the first stream.
	 * \param _time2 Target time point of the second stream.
	 * \param _stream2 Const reference to the second stream.
	 * \param _mass2 Overall mass of the second stream.
	 * \param _phase Phase type identifier.
	 * \return Calculated phase fraction.
	 */
	static double CalculateMixPhaseFractions(double _time1, const CBaseStream& _stream1, double _mass1, double _time2, const CBaseStream& _stream2, double _mass2, EPhase _phase);
	/**
	 * \private
	 * \brief Calculates multidimensional distributions of the mixture of two streams for the given phase.
	 * \details Does not perform any checks.
	 * \param _time1 Target time point of the first stream.
	 * \param _stream1 Const reference to the first stream.
	 * \param _mass1 Overall mass of the first stream.
	 * \param _time2 Target time point of the second stream.
	 * \param _stream2 Const reference to the second stream.
	 * \param _mass2 Overall mass of the second stream.
	 * \param _phase Phase type identifier.
	 * \return Calculated multidimensional distribution.
	 */
	static CDenseMDMatrix CalculateMixDistribution(double _time1, const CBaseStream& _stream1, double _mass1, double _time2, const CBaseStream& _stream2, double _mass2, EPhase _phase);
	/**
	 * \private
	 * \brief Sets the result of mixing two streams into this stream at the given time point.
	 * \param _time Target time point.
	 * \param _data Mixture of two streams.
	 */
	void SetMix(double _time, const mix_type& _data);

private:
	/**
	 * \private
	 * \brief Inserts the new time into the list of time points, if it does not exist yet.
	 * \param _time New time point that needs to be inserted.
	 */
	void InsertTimePoint(double _time);
	/**
	 * \private
	 * \brief Checks whether the given time point exists.
	 * \param _time Time point.
	 * \return Whether the given time point exists.
	 */
	bool HasTime(double _time) const;

	/**
	 * \private
	 * \brief Checks whether the specified overall property is defined in the stream.
	 * \param _property Identifier of time-dependent overall parameter.
	 * \return Whether the specified overall property is defined in the stream.
	 */
	bool HasOverallProperty(EOverall _property) const;

	/**
	 * \private
	 * \brief Checks whether the specified compound is defined in the stream.
	 * \param _compoundKey Unique key of the compound.
	 * \return Whether the specified compound is defined in the stream.
	 */
	bool HasCompound(const std::string& _compoundKey) const;
	/**
	 * \private
	 * \brief Checks whether all the specified compounds are defined in the stream.
	 * \param _compoundKeys Unique keys of the compounds.
	 * \return Whether all the specified compounds are defined in the stream.
	 */
	bool HasCompounds(const std::vector<std::string>& _compoundKeys) const;
	/**
	 * \private
	 * \brief Returns index of the compound.
	 * \param _compoundKey Unique key of the compound.
	 * \return Index of the existing compound.
	 */
	size_t CompoundIndex(const std::string& _compoundKey) const;

	/**
	 * \private
	 * \brief Checks whether the specified phase is defined in the stream.
	 * \param _phase Phase type identifier.
	 * \return Whether the specified phase is defined in the stream.
	 */
	bool HasPhase(EPhase _phase) const;

	/**
	 * \private
	 * \brief Calculates the PSD of the stream in mass fractions for the selected compounds.
	 * \details If the list of components is empty, calculates the PSD for the entire mixture. Takes into account porosity, if specified.
	 * All checks of parameters, phases, grids availability, etc. must be executed by the calling code.
	 * \param _time Target time point.
	 * \param _compoundKeys Unique keys of the compounds.
	 * \return Calculated PSD in mass fractions.
	 */
	std::vector<double> GetPSDMassFraction(double _time, const std::vector<std::string>& _compoundKeys) const;
	/**
	 * \private
	 * \brief Calculates the number particle distribution of the stream for the selected compounds.
	 * \details If the list of components is empty, calculates the PSD for the entire mixture. Takes into account porosity, if specified.
	 * All checks of parameters, phases, grids availability, etc. must be executed by the calling code.
	 * \param _time Target time point.
	 * \param _compoundKeys Unique keys of the compounds.
	 * \param _grid Unique keys of the compounds.
	 * \return Calculated number particle distribution.
	 */
	std::vector<double> GetPSDNumber(double _time, const std::vector<std::string>& _compoundKeys, EPSDGridType _grid) const;

	/**
	 * \private
	 * \brief Clears enthalpy calculator.
	 */
	void ClearEnthalpyCalculator();

	// TODO: move it somewhere
	////////////////////////////////////////////////////////////////////////////////
	// Deprecated functions
public:
	/**
	 * \private
	 */
	[[deprecated("WARNING! AddTimePoint(double, double) is deprecated. Use CopyTimePoint(double, double) instead.")]]
	void AddTimePoint(double _timeDst, double _timeSrc);
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetStreamName() is deprecated. Use GetName() instead.")]]
	std::string GetStreamName() const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! SetupStream(const CBaseStream*) is deprecated. Use SetupStructure(const CBaseStream*) instead.")]]
	void SetupStream(const CBaseStream* _stream);
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetTimePointsForInterval(double, double, bool) is deprecated. Use GetTimePoints(double, double) or GetTimePointsClosed(double, double) instead.")]]
	std::vector<double> GetTimePointsForInterval(double _timeBeg, double _timeEnd, bool _inclusive) const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetOverallProperty(double, unsigned) is deprecated. Use GetOverallProperty(double, EOverall), GetMixtureProperty(double, EOverall), GetMixtureProperty(double, ECompoundConstProperties) or GetMixtureProperty(double, ECompoundTPProperties) instead.")]]
	double GetOverallProperty(double _time, unsigned _property) const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetMass_Base(double) is deprecated. Use GetMass(double) instead.")]]
	double GetMass_Base(double _time) const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! SetMass_Base(double, double) is deprecated. Use SetMass(double, double) instead.")]]
	void SetMass_Base(double _time, double _value);
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetCompoundsList() is deprecated. Use a unit-level version CBaseUnit::GetCompoundsList() instead.")]]
	std::vector<std::string> GetCompoundsList() const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetCompoundsNames() is deprecated. Use a unit-level version CBaseUnit::GetAllCompoundsNames(), CBaseUnit::GetCompoundName(const std::string&) or CBaseUnit::GetCompoundName(size_t) instead.")]]
	std::vector<std::string> GetCompoundsNames() const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetCompoundsNumber() is deprecated. Use a unit-level version CBaseUnit::GetCompoundsNumber() instead.")]]
	size_t GetCompoundsNumber() const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetCompoundPhaseFraction(double, const std::string&, EPhaseTypes) is deprecated. Use GetCompoundFraction(double, const std::string&, EPhase) or GetCompoundsFractions(_time, EPhase) instead.")]]
	double GetCompoundPhaseFraction(double _time, const std::string& _compoundKey, unsigned _soa) const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetCompoundPhaseFraction(double, unsigned, EPhaseTypes) is deprecated. Use GetCompoundFraction(double, const std::string&, EPhase) or GetCompoundsFractions(_time, EPhase) instead.")]]
	double GetCompoundPhaseFraction(double _time, unsigned _index, unsigned _soa) const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! SetCompoundPhaseFraction(double, const std::string&, EPhaseTypes, double, eValueBasises) is deprecated. Use SetCompoundFraction(double, const std::string&, EPhase, double) or SetCompoundMolFraction(double, const std::string&, EPhase, double) instead.")]]
	void SetCompoundPhaseFraction(double _time, const std::string& _compoundKey, unsigned _soa, double _value, unsigned _basis = BASIS_MASS);
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetPhaseMass_Base(double, EPhaseTypes) is deprecated. Use GetPhaseMass(double, EPhase) instead.")]]
	double GetPhaseMass_Base(double _time, unsigned _soa) const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! SetPhaseMass_Base(double, EPhaseTypes, double) is deprecated. Use SetPhaseMass(double, EPhase, double) instead.")]]
	void SetPhaseMass_Base(double _time, unsigned _soa, double _value);
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetPhaseSOA(unsigned) is deprecated. Use a unit-level version CBaseUnit::GetPhaseType(size_t) instead.")]]
	unsigned GetPhaseSOA(unsigned _index) const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetPhaseIndex(EPhaseTypes) is deprecated. Access phases by their type.")]]
	unsigned GetPhaseIndex(unsigned _soa) const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetPhasesNumber() is deprecated. Use a unit-level version CBaseUnit::GetPhasesNumber() instead.")]]
	size_t GetPhasesNumber() const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetCompoundConstant(const std::string&, unsigned) is deprecated. Use GetCompoundProperty(const std::string&, ECompoundConstProperties) instead.")]]
	double GetCompoundConstant(const std::string& _compoundKey, unsigned _property) const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetCompoundTPDProp(double, const std::string&, unsigned) is deprecated. Use GetCompoundProperty(double, const std::string&, ECompoundTPProperties) instead.")]]
	double GetCompoundTPDProp(double _time, const std::string& _compoundKey, unsigned _property) const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetCompoundTPDProp(const std::string&, unsigned, double, double) is deprecated. Use GetCompoundProperty(const std::string&, ECompoundTPProperties, double, double) instead.")]]
	double GetCompoundTPDProp(const std::string& _compoundKey, unsigned _property, double _temperature, double _pressure) const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetCompoundInteractionProp(double, const std::string&, const std::string&, unsigned) is deprecated. Use GetCompoundProperty(double, const std::string&, const std::string&, ECompoundTPProperties) instead.")]]
	double GetCompoundInteractionProp(double _time, const std::string& _compoundKey1, const std::string& _compoundKey2, unsigned _property) const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetCompoundInteractionProp(const std::string&, const std::string&, unsigned, double, double) is deprecated. Use GetCompoundProperty(const std::string&, const std::string&, ECompoundTPProperties, double, double) instead.")]]
	double GetCompoundInteractionProp(const std::string& _compoundKey1, const std::string& _compoundKey2, unsigned _property, double _temperature, double _pressure) const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetDistribution(double, EDistrTypes, std::vector<double>&) is deprecated. Use GetDistribution(double, EDistrTypes) instead.")]]
	bool GetDistribution(double _time, EDistrTypes _distribution, std::vector<double>& _result) const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! CopyFromStream_Base(const CBaseStream&, double, bool) is deprecated. Use Copy(double, const CBaseStream&) instead.")]]
	void CopyFromStream_Base(const CBaseStream& _source, double _time, bool _deleteDataAfter = true);
	/**
	 * \private
	 */
	[[deprecated("WARNING! AddStream_Base(const CBaseStream&, double) is deprecated. Use Add(double, const CBaseStream&) instead.")]]
	void AddStream_Base(const CBaseStream& _source, double _time);
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetSinglePhaseProp(double, unsigned, EPhaseTypes) is deprecated. Use GetPhaseFraction(double, EPhase), GetPhaseMass(double, EPhase), GetPhaseProperty(double, EPhase, EOverall), GetPhaseProperty(double, EPhase, ECompoundConstProperties) or GetPhaseProperty(double, EPhase, ECompoundTPProperties) instead.")]]
	double GetSinglePhaseProp(double _time, unsigned _property, unsigned _soa) const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! GetPhaseTPDProp(double, unsigned, EPhaseTypes) is deprecated. Use GetPhaseProperty(double, EPhase, ECompoundTPProperties) instead.")]]
	double GetPhaseTPDProp(double _time, unsigned _property, unsigned _soa) const;
	/**
	 * \private
	 */
	[[deprecated("WARNING! SetSinglePhaseProp(double, unsigned, EPhaseTypes, double) is deprecated. Use SetPhaseFraction(double, EPhase, double) or SetPhaseMass(double, EPhase, double) instead.")]]
	void SetSinglePhaseProp(double _time, unsigned _property, unsigned _soa, double _value);
protected:
	/**
	 * \private
	 */
	static EPhase SOA2EPhase(unsigned _soa);
	/**
	 * \private
	 */
	static unsigned EPhase2SOA(EPhase _phase);
};

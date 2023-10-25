/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

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
 * \details
 */
class CBaseStream
{
	static const unsigned m_saveVersion{ 3 }; // Current version of the saving procedure.

	std::string m_name{ "Stream" };	// Name of the stream.
	std::string m_key;				// Unique key of the stream.

protected:
	// TODO: make it global and rename
	inline static const double m_epsilon{ 16 * std::numeric_limits<double>::epsilon() };

	const CMaterialsDatabase* m_materialsDB{ nullptr };	// Pointer to a database of materials.

	std::vector<double> m_timePoints;											// Time points on which the stream is defined.
	CMultidimensionalGrid m_grid;												// Defined distribution grid.
	std::map<EOverall, std::unique_ptr<CTimeDependentValue>> m_overall;			// Defined overall properties.
	std::map<EPhase, std::unique_ptr<CPhase>> m_phases;							// Defined phases.
	mutable std::unique_ptr<CMixtureEnthalpyLookup> m_enthalpyCalculator;		// Lookup table to calculate temperature<->enthalpy.
	SCacheSettings m_cacheSettings;												// Settings for caching in the stream.
	SToleranceSettings m_toleranceSettings;										// Settings for tolerances in the stream.
	SThermodynamicsSettings m_thermodynamicsSettings;							// Settings for thermodynamics in the stream.

public:
	// TODO: remove empty constructor, always set pointers to MDB and grid.
	// Basic constructor.
	CBaseStream(const std::string& _key = "");
	// Constructor configuring the whole structure.
	CBaseStream(const std::string& _key, const CMaterialsDatabase* _materialsDB, const CMultidimensionalGrid& _grid,
		const std::vector<SOverallDescriptor>* _overall, const std::vector<SPhaseDescriptor>* _phases,
		const SCacheSettings* _cache, const SToleranceSettings* _tolerance, const SThermodynamicsSettings* _thermodynamics);
	// Copy constructor.
	CBaseStream(const CBaseStream& _other);
	CBaseStream(CBaseStream&& _other);
	virtual ~CBaseStream() = default;

	CBaseStream& operator=(const CBaseStream& _other) = delete;
	CBaseStream& operator=(CBaseStream&& _other) = delete;

	/**
	* \private
	* \brief Removes all existing data from the stream.
	* \details
	*/
	void Clear();

	/**
	* \private
	* \brief Sets up the stream structure (MD dimensions, phases, materials, etc.) the same as an in the given stream.
	* \details Removes all existing data.
	* \param _other Pointer to source stream.
	*/
	void SetupStructure(const CBaseStream* _other);

	/**
	* \private
	* \brief Checks whether both streams have the same overall properties.
	* \details
	* \param _stream1 Reference to first stream.
	* \param _stream2 Reference to second stream.
	* \return Whether both streams have the same overall properties.
	*/
	static bool HaveSameOverall(const CBaseStream& _stream1, const CBaseStream& _stream2);
	/**
	* \private
	* \brief Checks whether both streams have the same phases.
	* \details
	* \param _stream1 Reference to first stream.
	* \param _stream2 Reference to second stream.
	* \return Whether both streams have the same phases.
	*/
	static bool HaveSamePhases(const CBaseStream& _stream1, const CBaseStream& _stream2);
	/**
	* \private
	* \brief Checks whether both streams have the same solids distribution grids.
	* \details
	* \param _stream1 Reference to first stream.
	* \param _stream2 Reference to second stream.
	* \return Whether both streams have the same solids distribution grids.
	*/
	static bool HaveSameGrids(const CBaseStream& _stream1, const CBaseStream& _stream2);
	/**
	* \private
	* \brief Checks whether both streams have the same overall properties and phases.
	* \details
	* \param _stream1 Reference to first stream.
	* \param _stream2 Reference to second stream.
	* \return Whether both streams have the same overall properties and phases.
	*/
	static bool HaveSameOverallAndPhases(const CBaseStream& _stream1, const CBaseStream& _stream2);
	/**
	* \private
	* \brief Checks whether both streams have the same overall properties, phases and grids.
	* \details
	* \param _stream1 Reference to first stream.
	* \param _stream2 Reference to second stream.
	* \return Whether both streams have the same overall properties, phases and grids.
	*/
	static bool HaveSameStructure(const CBaseStream& _stream1, const CBaseStream& _stream2);

	////////////////////////////////////////////////////////////////////////////////
	// Basic stream properties
	//

	/**
	* \brief Returns the name of the stream.
	* \details
	* \return Name of the stream.
	*/
	std::string GetName() const;
	/**
	* \brief Sets new name of the stream.
	* \details
	* \param _name New name of the stream.
	*/
	void SetName(const std::string& _name);
	/**
	* \brief Returns unique key of the stream.
	* \details
	* \return Unique key of the stream.
	*/
	std::string GetKey() const;
	/**
	* \brief Sets new unique key of the stream.
	* \details
	* \param _key New unique key of the stream.
	*/
	void SetKey(const std::string& _key);

	////////////////////////////////////////////////////////////////////////////////
	// Time points
	//

	/**
	* \brief Adds a new time point _time if it doesn't already exist and fills it with the data of existing previous time point.
	* \details
	* \param _time New time point.
	*/
	void AddTimePoint(double _time);
	/**
	* \brief Adds a new temp point _timeDst if it doesn't already exist and fills it with the data of existing time point _timeSrc.
	* \details
	* \param _timeDst New time point.
	* \param _timeSrc Source time point.
	*/
	void CopyTimePoint(double _timeDst, double _timeSrc);
	/**
	* \brief Removes the specified time point if it does already exist.
	* \details
	* \param _time Time point of the stream.
	*/
	void RemoveTimePoint(double _time);
	/**
	* \brief Removes all existing time points in the specified interval, including or excluding boundaries.
	* \details
	* \param _timeBeg Start time point of the interval.
	* \param _timeEnd End time point of the interval.
	* \param _inclusive Inclusive flag. Default value is true (Specified interval includes boundaries).
	*/
	void RemoveTimePoints(double _timeBeg, double _timeEnd, bool _inclusive = true);
	/**
	* \brief Removes all existing time points after the specified one, inclusive or exclusive _time.
	* \details
	* \param _time Start time point of the interval.
	* \param _inclusive Inclusive flag. Default value is false (Specified interval includes star time point).
	*/
	void RemoveTimePointsAfter(double _time, bool _inclusive = false);
	/**
	* \brief Removes all existing time points.
	* \details
	*/
	void RemoveAllTimePoints();
	/**
	* \brief Removes time points within the specified interval [timeBeg; timeEnd) that are closer together than step.
	* \details
	* \param _timeBeg Start time point of the interval.
	* \param _timeEnd End time point of the interval.
	* \param _step Specified step.
	*/
	void ReduceTimePoints(double _timeBeg, double _timeEnd, double _step);
	/**
	* \brief Returns all defined time points.
	* \details
	* \return All time points which are defined in the stream.
	*/
	std::vector<double> GetAllTimePoints() const;
	/**
	* \brief Returns all defined time points in the specified time interval.
	* \details
	* \param _timeBeg Start time point of the interval.
	* \param _timeEnd End time point of the interval.
	* \return All time points which are defined in the stream for the time interval.
	*/
	std::vector<double> GetTimePoints(double _timeBeg, double _timeEnd) const;
	/**
	* \brief Returns all defined time points in the specified closed time interval, boundaries are unconditionally included into result.
	* \details
	* \param _timeBeg Start time point of the interval.
	* \param _timeEnd End time point of the interval.
	* \return All time points which are defined in the stream for the time interval.
	*/
	std::vector<double> GetTimePointsClosed(double _timeBeg, double _timeEnd) const;
	/**
	* \brief Returns the last (largest) defined time point, or zero if no time points have been defined.
	* \details
	* \return Last defined time point in the stream.
	*/
	double GetLastTimePoint() const;
	/**
	* \brief Returns the nearest time point before _time, or zero if such time can not be found.
	* \details
	* \param _time The specified time point.
	* \return The nearest time point before _time.
	*/
	double GetPreviousTimePoint(double _time) const;

	////////////////////////////////////////////////////////////////////////////////
	// Overall parameters
	//

	/**
	* \brief Adds new overall property to the stream, if it does not exist yet and returns a pointer to it or already existing property.
	* \details
	* \param _property Identifier of time-dependent overall property.
	* \param _name Name of the property.
	* \param _units Units of measurement.
	* \return Pointer to the added overall property or already existing property.
	*/
	CTimeDependentValue* AddOverallProperty(EOverall _property, const std::string& _name, const std::string& _units);
	/**
	* \brief Removes an overall property from the stream, if it does exist.
	* \details
	* \param _property Identifier of time-dependent overall property.
	*/
	void RemoveOverallProperty(EOverall _property);
	// TODO: maybe remove
	/**
	* \private
	* Returns a pointer to an overall property or nullptr if such property doesn't exist.
	*/
	CTimeDependentValue* GetOverallProperty(EOverall _property);
	// TODO: maybe remove
	/**
	* \brief Returns a const pointer to an overall property or nullptr if such property doesn't exist.
	* \details
	* \param _property Identifier of time-dependent overall property.
	* \return Const pointer to the overall property or nullptr if such property doesn't exist.
	*/
	const CTimeDependentValue* GetOverallProperty(EOverall _property) const;

	/**
	* \brief Returns a value of the specified overall property at the given time point. Returns default value if such overall property has not been defined.
	* \details
	* \param _time The given time point.
	* \param _property Identifier of time-dependent overall property.
	* \return Value of the specified overall property at the given time point or default value.
	*/
	double GetOverallProperty(double _time, EOverall _property) const;
	/**
	* \brief Returns a value of the overall mass at the given time point. If such time point has not been defined, interpolation of data will be done.
	* \details
	* \param _time The given time point.
	* \return Value of the overall mass at the given time point (\f$m\f$ in [kg], total mass of the holdup).
	*/
	double GetMass(double _time) const;
	/**
	* \brief Returns a value of the overall temperature in [K] at the given time point. Returns standard condition temperature if temperature overall property has not been defined.
	* \details
	* \param _time The given time point.
	* \return Value of the overall temperature at the given time point or standard condition temperature.
	*/
	double GetTemperature(double _time) const;
	/**
	* \brief Returns a value of the overall pressure in [Pa] at the given time point. Returns standard condition pressure if pressure overall property has not been defined.
	* \details
	* \param _time The given time point.
	* \return Value of the overall pressure at the given time point or standard condition pressure.
	*/
	double GetPressure(double _time) const;

	/**
	* \brief Sets a value of the specified overall property at the given time point if such property exists.
	* \details
	* \param _time The given time point.
	* \param _property Identifier of time-dependent overall property.
	* \param _value Value of the specified overall property.
	*/
	void SetOverallProperty(double _time, EOverall _property, double _value);
	/**
	* \brief Sets a value of the overall mass at the given time point.
	* \details
	* \param _time The given time point.
	* \param _value Value of the overall mass.
	*/
	void SetMass(double _time, double _value);
	/**
	* \brief Sets a value of the overall temperature in [K] at the given time point if such property exists.
	* \details
	* \param _time The given time point.
	* \param _value Value of the overall temperature.
	*/
	void SetTemperature(double _time, double _value);
	/**
	* \brief Sets a value of the overall pressure in [Pa] at the given time point if such property exists.
	* \details
	* \param _time The given time point.
	* \param _value Value of the overall pressure.
	*/
	void SetPressure(double _time, double _value);

	/**
	* \brief Returns a value of the overall amount of substance \f$\sum_{i}\frac{m \cdot w_i}{M_i}\f$ in [mol] at the given time point.
	* \details With \f$w_i\f$ mass fraction of the phase \f$i\f$, and \f$M_i\f$ molar mass of the phase \f$i\f$.
	* \param _time The given time point.
	* \return Value of the overall amount of substance.
	*/
	double GetMol(double _time) const;
	/**
	* \brief Sets a value of the overall amount of substance in [mol] at the given time point.
	* \details
	* \param _time The given time point.
	* \param _value Value of the overall amount of substance.
	*/
	void SetMol(double _time, double _value);

	////////////////////////////////////////////////////////////////////////////////
	// Compounds
	//

	/**
	* \private
	* \brief Adds a compound with the specified unique key to the stream.
	* \details
	* \param _compoundKey Unique key for new compound.
	*/
	void AddCompound(const std::string& _compoundKey);
	/**
	* \private
	* \brief Removes a compound with the specified unique key from the stream.
	* \details
	* \param _compoundKey Unique key of the compound.
	*/
	void RemoveCompound(const std::string& _compoundKey);
	/**
	* \brief Returns all defined materials.
	* \details
	* \return List of all defined materials.
	*/
	std::vector<std::string> GetAllCompounds() const;

	/**
	* \brief Returns the mass fraction of the compound in the total mixture at the given time point.
	* \details \f$f_i = \sum_{i} w_i \cdot f_{i}\f$, with \f$f_i\f$ the mass fraction of compound \f$i\f$, and \f$w_i\f$ the mass fraction of phase \f$i\f$.
	* \param _time The specified time point.
	* \param _compoundKey Unique key of the compound.
	* \return The total mass fraction of the compound at the given time point.
	*/
	double GetCompoundFraction(double _time, const std::string& _compoundKey) const;
	/**
	* \brief Returns the mass fraction of the compound in the specified phase at the given time point.
	* \details \f$f_{i,j}\f$, mass fraction of compound \f$j\f$ in phase \f$i\f$.
	* \param _time The specified time point.
	* \param _compoundKey Unique key of the compound.
	* \param _phase The specified phase type.
	* \return The mass fraction of the compound in the specified phase at the given time point.
	*/
	double GetCompoundFraction(double _time, const std::string& _compoundKey, EPhase _phase) const;
	/**
	* \brief Returns the mass of the compound in the total mixture at the given time point.
	* \details \f$m_i = f_i \cdot m\f$, with \f$m_i\f$ the mass of compound \f$i\f$, \f$f_i\f$ the mass fraction of compound \f$i\f$, and \f$m\f$ the total mass of the holdup.
	* \param _time The specified time point.
	* \param _compoundKey Unique key of the compound.
	* \return The mass of the compound in the total mixture at the given time point.
	*/
	double GetCompoundMass(double _time, const std::string& _compoundKey) const;
	/**
	* \brief Returns the mass of the compound in the specified phase at the given time point.
	* \details \f$m_{i,j} = w_i \cdot f_{i,j} \cdot m\f$, with \f$m_{i,j}\f$ the mass of compound \f$j\f$ in phase \f$i\f$, \f$w_i\f$ the mass fraction of phase \f$i\f$,\f$f_{i,j}\f$ the mass fraction of compound \f$j\f$ in phase \f$i\f$, and \f$m\f$ the total mass of the holdup.
	* \param _time The specified time point.
	* \param _compoundKey Unique key of the compound.
	* \param _phase The specified phase type.
	* \return The mass of the compound in the total mixture at the given time point.
	*/
	double GetCompoundMass(double _time, const std::string& _compoundKey, EPhase _phase) const;
	/**
	* \brief Returns mass fraction of all defined compounds at the given time point.
	* \details
	* \param _time The specified time point.
	* \return Mass fraction of all defined compounds at the given time point
	*/
	std::vector<double> GetCompoundsFractions(double _time) const;
	/**
	* \brief Returns mass fraction of all defined compounds in the specified phase at the given time point.
	* \details
	* \param _time The specified time point.
	* \param _phase The specified phase type.
	* \return Mass fraction of all defined compounds in the specified phase at the given time point.
	*/
	std::vector<double> GetCompoundsFractions(double _time, EPhase _phase) const;
	/**
	* \brief Returns masses of all defined compounds at the given time point.
	* \details
	* \param _time The specified time point.
	* \return Masses of all defined compounds at the given time point.
	*/
	std::vector<double> GetCompoundsMasses(double _time) const;
	/**
	* \brief Returns masses of all defined compounds in the specified phase at the given time point.
	* \details
	* \param _time The specified time point.
	* \param _phase The specified phase type.
	* \return Masses of all defined compounds in the specified phase at the given time point.
	*/
	std::vector<double> GetCompoundsMasses(double _time, EPhase _phase) const;

	/**
	* \brief Sets the mass fraction of the compound in the specified phase at the given time point.
	* \details
	* \param _time The specified time point.
	* \param _compoundKey Unique key of the compound.
	* \param _phase The specified phase type.
	* \param _value Value of the mass fraction of the compound in the specified phase at the given time point.
	*/
	void SetCompoundFraction(double _time, const std::string& _compoundKey, EPhase _phase, double _value);
	/**
	* \brief Sets mass fraction of all defined compounds in all defined phases at the given time point.
	* \details
	* \param _time The specified time point.
	* \param _value List of mass fractions of all defined compounds in all defined phases at the given time point.
	*/
	void SetCompoundsFractions(double _time, const std::vector<double>& _value);
	/**
	* \brief Sets mass fraction of all defined compounds in the specified phase at the given time point.
	* \details
	* \param _time The specified time point.
	* \param _phase The specified phase type.
	* \param _value List of mass fraction of all defined compounds in the specified phase at the given time point.
	*/
	void SetCompoundsFractions(double _time, EPhase _phase, const std::vector<double>& _value);

	/**
	* \brief Returns the molar fraction of the compound in the specified phase at the given time point.
	* \details \f$f_i^{mol} = f_{i,j} \cdot \frac{M_i}{M_j}\f$, with \f$f_i^{mol}\f$ mole fraction of compound \f$i\f$, \f$f_{i,j}\f$ mass fraction of compound \f$j\f$ in phase \f$i\f$, \f$M_i\f$ molar mass of phase \f$i\f$, and \f$M_j\f$ molar mass of compound \f$j\f$.
	* \param _time The specified time point.
	* \param _compoundKey Unique key of the compound.
	* \param _phase The specified phase type.
	* \return The molar fraction of the compound in the specified phase at the given time point.
	*/
	double GetCompoundMolFraction(double _time, const std::string& _compoundKey, EPhase _phase) const;
	/**
	* \brief Returns the amount of substance of the compound in the specified phase at the given time point.
	* \details \f$n_{i,j}^{mol} = \frac{m_{i,j}}{M_j}\f$, with \f$n_{i,j}^{mol}\f$ amount of substance of compound \f$j\f$ in phase \f$i\f$, \f$m_{i,j}\f$ the mass of compound \f$j\f$ in phase \f$i\f$, and \f$M_j\f$ molar mass of compound \f$j\f$.
	* \param _time The specified time point.
	* \param _compoundKey Unique key of the compound.
	* \param _phase The specified phase type.
	* \return The amount of substance of the compound in the specified phase at the given time point.
	*/
	double GetCompoundMol(double _time, const std::string& _compoundKey, EPhase _phase) const;
	/**
	* \brief Sets the molar fraction of the compound in the specified phase at the given time point.
	* \details \f$f_{i, j} = \f$\p _value\f$\cdot M_j \cdot \frac{n_i}{m_i}\f$, with \f$f_{i, j}\f$ mass fraction of compound \f$j\f$ in phase \f$i\f$, \f$M_j\f$ the molar mass of compound \f$j\f$, \f$n_i\f$ the amount of substance in phase \f$i\f$, and \f$m_i\f$ the mass of phase \f$i\f$.
	* \param _time The specified time point.
	* \param _compoundKey Unique key of the compound.
	* \param _phase The specified phase type.
	* \param _value Value of molar fraction of the compound in the specified phase at the given time point.
	*/
	void SetCompoundMolFraction(double _time, const std::string& _compoundKey, EPhase _phase, double _value);

	////////////////////////////////////////////////////////////////////////////////
	// Phases
	//

	/**
	* \private
	* \brief Adds the specified phase to the stream, if it does not exist yet and returns a pointer to it or already existing phase.
	* \details
	* \param _phase Type of new phase.
	* \param _name Name of new phase.
	* \return Pointer to the added phase or already existing phase.
	*/
	CPhase* AddPhase(EPhase _phase, const std::string& _name);
	/**
	* \private
	* \brief Removes the specified phase from the stream.
	* \details
	* \param _phase Type of the specified phase in the stream.
	*/
	void RemovePhase(EPhase _phase);
	// TODO: maybe remove
	/**
	* \private
	* Returns a pointer to a phase or nullptr if such phase doesn't exist.
	*/
	CPhase* GetPhase(EPhase _phase);
	// TODO: maybe remove
	/**
	* \brief Returns a const pointer to a phase or nullptr if such phase doesn't exist.
	* \details
	* \param _phase Type of the specified phase in the stream.
	* \return Const pointer to a phase or nullptr if such phase doesn't exist.
	*/
	const CPhase* GetPhase(EPhase _phase) const;
	// TODO: remove this.
	/**
	* \private
	* \brief Removes all defined phases.
	* \details
	*/
	void ClearPhases();

	/**
	* \brief Returns the mass fraction of the specified phase at the given time point.
	* \details
	* \param _time The specified time point.
	* \param _phase The specified phase type.
	* \return The mass fraction of the specified phase at the given time point.
	*/
	double GetPhaseFraction(double _time, EPhase _phase) const;
	/**
	* \brief Returns the mass of the specified phase at the given time point.
	* \details \f$m_i = m \cdot w_i\f$, with \f$m_i\f$ the mass of phase \f$i\f$, \f$w_i\f$ the mass fraction of phase \f$i\f$, and \f$m\f$ the total mass of the holdup.
	* \param _time The specified time point.
	* \param _phase The specified phase type.
	* \return The mass of the specified phase at the given time point.
	*/
	double GetPhaseMass(double _time, EPhase _phase) const;
	/**
	* \brief Returns the value of the overall property of the specified phase at the given time point.
	* \details
	* \param _time The specified time point.
	* \param _phase The specified phase type.
	* \param _property Identifier of time-dependent overall property.
	* \return Value of the overall property of the specified phase at the given time point.
	*/
	double GetPhaseProperty(double _time, EPhase _phase, EOverall _property) const;
	/**
	* \brief Returns the value of the constant physical property (MOLAR_MASS) of the specified phase at the given time point.
	* \details For \p MOLAR_MASS: \f$\frac{1}{M_i} = \sum_j {\frac{f_{i,j}}{M_j}}\f$, with \f$M_i\f$ the molar mass of phase \f$i\f$, \f$f_{i,j}\f$ the mass fraction of compound \f$j\f$ in phase \f$i\f$,\f$M_j\f$ the molar mass of compound \f$j\f$.
	* For other const material properties: \f$v_i = \sum_j f_{i,j} \cdot v_j\f$, with \f$v_i\f$ the const physical property of phase \f$i\f$, \f$f_{i,j}\f$ the mass fraction of compound \f$j\f$ in phase \f$i\f$, \f$v_i\f$ the value of the specified const \p _property of compound \f$j\f$.
	* \param _time The specified time point.
	* \param _phase The specified phase type.
	* \param _property Identifier of constant material property.
	* \return Value of the constant physical property of the specified phase at the given time point.
	*/
	double GetPhaseProperty(double _time, EPhase _phase, ECompoundConstProperties _property) const;
	/**
	* \brief Returns the value of the temperature/pressure-dependent physical property (DENSITY, ENTHALPY, etc) of the specified phase at the given time point.
	* \details Available properties are:
	* - \p DENSITY:
	*	- For solid phase: is calculated by \f$\rho = \sum_{i,j} \rho_i (1 - \varepsilon_j) f_{i,j}\f$, with \f$\varepsilon_j\f$ the porosity in interval \f$j\f$, and \f$f_{i,j}\f$ the mass fraction of compound \f$i\f$ with porosity \f$j\f$.
	*	- For liquid and vapor phase: is calculated by \f$\frac{1}{\rho} = \sum_i \frac{w_i}{\rho_i}\f$, with \f$w_i\f$ the mass fraction of compound \f$i\f$ in \p _phase.
	* - \p HEAT_CAPACITY_CP: is calculated by \f$C_p = \sum_i w_i \cdot C_{p,i}\f$, with \f$C_{p,i}\f$ the heat capacity of compound \f$i\f$, and \f$w_i\f$ the mass fraction of compound \f$i\f$ in \p _phase.
	* - \p VAPOR_PRESSURE: is calculated by \f$P_v = \min_{i} (P_v)_i\f$, with \f$(P_v)_i\f$ vapor pressure of compound \f$i\f$.
	* - \p VISCOSITY:
	*	- For solid phase: is calculated by \f$\eta = \sum\limits_i w_i \eta_i\f$, with \f$\eta_i\f$ the viscosity of compound \f$i\f$, and \f$w_i\f$ the mass fraction of compound \f$i\f$.
	*	- For liquid phase: is calculated by \f$\ln \eta = \sum_i w_i \ln \eta_i\f$, with \f$\eta_i\f$ the viscosity of compound \f$i\f$, and \f$w_i\f$ the mass fraction of compound \f$i\f$ in \p _phase.
	*	- For vapor phase: \f$\eta = \frac{\sum_i x_i \sqrt{M_i} \eta_i}{\sum_i x_i \sqrt{M_i}}\f$, with \f$\eta_i\f$ the viscosity of compound \f$i\f$, \f$w_i\f$ the mass fraction of compound \f$i\f$ in \p _phase, and \f$x_i\f$ the mole fraction of compound \f$i\f$ in \p _phase.
	* - \p THERMAL_CONDUCTIVITY:
	*	- For solid phase: is calculated by \f$\lambda = \sum_i w_i \lambda_i\f$, with \f$\lambda_i\f$ the thermal conductivity of compound \f$i\f$.
	*	- For liquid phase: is calculated by \f$\lambda = \frac{1}{\sqrt{\sum_i x_i \lambda_i^{-2}}}\f$, with \f$\lambda_i\f$ the thermal conductivity of compound \f$i\f$.
	*	- For vapor phase: is calculated by \f$\lambda = \sum_i \frac{x_i \lambda_i}{\sum_j x_j F_{i,j}}\f$, \f$F_{i,j} = \frac{(1 + \sqrt{\lambda_i^4 / \lambda_j} \sqrt{M_j / M_i})^2}{\sqrt{8(1 + M_i / M_j)}}\f$. With \f$M_i\f$ the molar mass of compound \f$i\f$.
	* - \p PERMITTIVITY: is calculated by \f$\varepsilon = \sum_i w_i \varepsilon_i\f$, with \f$\varepsilon_i\f$ the permittivity of compound \f$i\f$, and \f$w_i\f$ the mass fraction of compound \f$i\f$ in \p _phase.
	* - \p ENTHALPY: is calculated by \f$H = \sum_i w_i H_i\f$, with \f$H_i\f$ the enthalpy of compound \f$i\f$, and \f$w_i\f$ the mass fraction of compound \f$i\f$ in \p _phase.
	* - \p EQUILIBRIUM_MOISTURE_CONTENT: is calculated by \f$M = \sum_i w_i M_i\f$, with \f$M_i\f$ the equilibrium moisture content of compound \f$i\f$, and \f$w_i\f$ the mass fraction of compound \f$i\f$ in \p _phase.
	* - \p TP_PROP_USER_DEFINED_XX: is calculated by \f$Y = \sum_i w_i Y_i\f$, with \f$Y_i\f$ the property value of compound \f$i\f$, and \f$w_i\f$ the mass fraction of compound \f$i\f$ in \p _phase.
	* \param _time The specified time point.
	* \param _phase The specified phase type.
	* \param _property Identifier of temperature/pressure-dependent property.
	* \return Value of the temperature/pressure-dependent physical property of the specified phase at the given time point.
	*/
	double GetPhaseProperty(double _time, EPhase _phase, ECompoundTPProperties _property) const;

	/**
	* \brief Sets the mass fraction of the specified phase at the given time point.
	* \details
	* \param _time The specified time point.
	* \param _phase The specified phase type.
	* \param _value The mass fraction value of the specified phase.
	*/
	void SetPhaseFraction(double _time, EPhase _phase, double _value);
	/**
	* \brief Sets the mass of the specified phase at the given time point.
	* \details Total mass of the stream is correspondingly adjusted, masses of other phases remain the same. If there is no specified phase in the holdup, the value will not be set.
	* Input parameter \p _value is the mass of one defined phase: \f$m_i =\f$ \p _value and \f$w_i = m_i / m\f$. Meanwhile, the total mass \f$m\f$ changes due to assignment for \f$m_i\f$: \f$m = m_{old} + (\f$ \p _value \f$- m_{i,old})\f$. Here \f$m_i\f$ stands for the mass of phase \f$i\f$, \f$w_i\f$ for the mass fraction of phase \f$i\f$, and \f$m\f$ for the total mass of the holdup.
	* \param _time The specified time point.
	* \param _phase The specified phase type.
	* \param _value The mass value of the specified phase.
	*/
	void SetPhaseMass(double _time, EPhase _phase, double _value);

	/**
	* \brief Returns the molar fraction of the specified phase at the given time point.
	* \details \f$x_i = \frac{n_i}{N}\f$, with \f$x_i\f$ the molar fraction of phase \f$i\f$, \f$n_i\f$ the amount of substance of phase \f$i\f$, and \f$N\f$ the value of the overall amount of substance.
	* \param _time The specified time point.
	* \param _value The mass value of the specified phase.
	* \return The molar fraction of the specified phase at the given time point.
	*/
	double GetPhaseMolFraction(double _time, EPhase _phase) const;
	/**
	* \brief Returns the amount of substance of the specified phase at the given time point.
	* \details \f$n_i = \sum_j n_{i,j}^{mol}\f$, with \f$n_i\f$ the amount of substance of phase \f$i\f$, \f$n_{i,j}^{mol}\f$ the amount of substance of compound \f$j\f$ in phase \f$i\f$.
	* \param _time The specified time point.
	* \param _phase The specified phase type.
	* \return The amount of substance of the specified phase at the given time point.
	*/
	double GetPhaseMol(double _time, EPhase _phase) const;
	/**
	* \brief Sets the molar fraction of the specified phase at the given time point.
	* \details Input parameter \p _value is the molar fraction of one defined phase: \f$f_i =\f$ \p _value \f$\cdot \frac{f_i}{x_i}\f$, with \f$f_i\f$ the mass fraction of phase \f$i\f$, and \f$x_i\f$ the molar fraction of phase \f$i\f$.
	* \param _time The specified time point.
	* \param _phase The specified phase type.
	* \param _value The molar fraction of the specified phase.
	*/
	void SetPhaseMolFraction(double _time, EPhase _phase, double _value);
	/**
	* \brief Sets the amount of substance of the specified phase at the given time point.
	* \details Total mass of the stream is correspondingly adjusted, masses of other phases remain the same.
	* Input parameter \p _value is the amount of substance of one defined phase: \f$m_i = \f$\p _value \f$\cdot \frac{m_i}{n_i}\f$, with \f$m_i\f$ the mass of phase \f$i\f$, and \f$n_i\f$ the amount of substance of phase \f$i\f$.
	* \param _time The specified time point.
	* \param _phase The specified phase type.
	* \param _value The amount of substance of the specified phase.
	*/
	void SetPhaseMol(double _time, EPhase _phase, double _value);

	////////////////////////////////////////////////////////////////////////////////
	// Properties of the total mixture
	//

	/**
	* \brief Returns the value of the property of the total mixture in the stream at the given time point.
	* \details Refer to function CBaseStream::GetOverallProperty(double, EOverall) const.
	* \param _time The specified time point.
	* \param _property Identifier of time-dependent overall property.
	* \return Value of the property of the total mixture in the stream at the given time point.
	*/
	double GetMixtureProperty(double _time, EOverall _property) const;
	/**
	* \brief Returns the value of the constant physical property (MOLAR_MASS) of the total mixture in the stream at the given time point.
	* \details \f$V = \sum_i v_i \cdot w_i\f$, with \f$V\f$ the value of the const physical property of the total mixture, \f$v_i\f$ the value of the const physical property of phase \f$i\f$, and \f$w_i\f$ the mass fraction of phase \f$i\f$.
	* \param _time The specified time point.
	* \param _property Identifier of constant material property.
	* \return Value of the constant physical property of the total mixture in the stream at the given time point.
	*/
	double GetMixtureProperty(double _time, ECompoundConstProperties _property) const;
	/**
	* \brief Returns the value of the temperature/pressure-dependent physical property (DENSITY, ENTHALPY, etc) of the total mixture in the stream at the given time point.
	* \details \f$V = \sum_i v_i \cdot w_i\f$, with \f$V\f$ the value of the temperature/pressure-dependent physical property of the total mixture, \f$v_i\f$ the value of the temperature/pressure-dependent physical property of phase \f$i\f$, and \f$w_i\f$ the mass fraction of phase \f$i\f$.
	* \param _time The specified time point.
	* \param _property Identifier of temperature/pressure-dependent property.
	* \return Value of the temperature/pressure-dependent physical property of the total mixture in the stream at the given time point.
	*/
	double GetMixtureProperty(double _time, ECompoundTPProperties _property) const;

	/**
	* \brief Sets the value of the property of the total mixture in the stream at the given time point.
	* \details Refer to function CBaseStream::SetOverallProperty(double, EOverall, double).
	* \param _time The specified time point.
	* \param _property Identifier of time-dependent overall property.
	* \param _value Value of the property of the total mixture in the stream at the given time point.
	*/
	void SetMixtureProperty(double _time, EOverall _property, double _value);

	////////////////////////////////////////////////////////////////////////////////
	// Material database
	//

	/**
	* \brief Returns the value of the constant physical property (CRITICAL_TEMPERATURE, MOLAR_MASS, etc) of the specified compound.
	* \details These properties are stored in \verbatim embed:rst:inline :ref:`material database <label-materialDataDetailed>` \endverbatim
	* \param _compoundKey Unique key of the compound.
	* \param _property Identifier of constant material property.
	* \return Value of the constant physical property of the specified compound.
	*/
	double GetCompoundProperty(const std::string& _compoundKey, ECompoundConstProperties _property) const;
	/**
	* \brief Returns the value of the temperature/pressure-dependent physical property (DENSITY, ENTHALPY, etc) of the specified compound with the given temperature [K] and pressure [Pa].
	* \details
	* \param _compoundKey Unique key of the compound.
	* \param _property Identifier of temperature/pressure-dependent property.
	* \param _temperature Value of temperature in [K].
	* \param _pressure Value of pressure in [Pa].
	* \return Value of the temperature/pressure-dependent physical property of the specified compound with the given temperature and pressure.
	*/
	double GetCompoundProperty(const std::string& _compoundKey, ECompoundTPProperties _property, double _temperature, double _pressure) const;
	/**
	* \brief Returns the value of the temperature/pressure-dependent physical property (DENSITY, ENTHALPY, etc) of the specified compound at temperature and pressure at the given time point.
	* \details Refer to function CBaseStream::GetCompoundProperty(const std::string&, ECompoundTPProperties, double, double) const.
	* \param _time The specified time point.
	* \param _compoundKey Unique key of the compound.
	* \param _property Identifier of temperature/pressure-dependent property.
	* \return Value of the temperature/pressure-dependent physical property of the specified compound at temperature and pressure at the given time point.
	*/
	double GetCompoundProperty(double _time, const std::string& _compoundKey, ECompoundTPProperties _property) const;
	/**
	* \brief Returns the value of the interaction physical property (INTERFACE_TENSION, etc) between the specified compounds with the given specified temperature [K] and pressure [Pa].
	* \details These properties are stored in the \verbatim embed:rst:inline :ref:`material database <label-materialDataDetailed>` \endverbatim
	* \param _compoundKey1 Unique key of the first compound.
	* \param _compoundKey2 Unique key of the second compound.
	* \param _property Identifier of property, defined for interaction of two compounds.
	* \param _temperature Value of temperature in [K].
	* \param _pressure Value of pressure in [Pa].
	* \return Value of the interaction physical property between the specified compounds with the given specified temperature and pressure.
	*/
	double GetCompoundProperty(const std::string& _compoundKey1, const std::string& _compoundKey2, EInteractionProperties _property, double _temperature, double _pressure) const;
	/**
	* \brief Returns the value of the interaction physical property (INTERFACE_TENSION, etc) between the specified compounds at temperature and pressure at the given time point.
	* \details Refer to function CBaseStream::GetCompoundProperty(const std::string&, const std::string&, EInteractionProperties, double, double) const.
	* \param _time The specified time point.
	* \param _compoundKey1 Unique key of the first compound.
	* \param _compoundKey2 Unique key of the second compound.
	* \param _property Identifier of property, defined for interaction of two compounds.
	* \return Value of the interaction physical property between the specified compounds at temperature and pressure at the given time point.
	*/
	double GetCompoundProperty(double _time, const std::string& _compoundKey1, const std::string& _compoundKey2, EInteractionProperties _property) const;

	////////////////////////////////////////////////////////////////////////////////
	// Distributed properties of the solid phase
	//

	// Returns the mass fraction of a solid material at the specified multidimensional coordinates given for all defined dimensions.
	double GetFraction(double _time, const std::vector<size_t>& _coords) const;
	// Sets the mass fraction of a solid material at the specified multidimensional coordinates given for all defined dimensions.
	void SetFraction(double _time, const std::vector<size_t>& _coords, double _value);

	// Returns the one-dimensional distribution of a solid material over the specified parameter for the given time point.
	std::vector<double> GetDistribution(double _time, EDistrTypes _distribution) const;
	// Returns the two-dimensional distribution of a solid material over the specified parameters for the given time point.
	CMatrix2D GetDistribution(double _time, EDistrTypes _distribution1, EDistrTypes _distribution2) const;
	// Returns the multi-dimensional distribution of a solid material over the specified parameters for the given time point.
	CDenseMDMatrix GetDistribution(double _time, const std::vector<EDistrTypes>& _distributions) const;
	// Returns the one-dimensional distribution of a solid material over the specified parameter for the given compound and time point.
	std::vector<double> GetDistribution(double _time, EDistrTypes _distribution, const std::string& _compoundKey) const;
	// Returns the two-dimensional distribution of a solid material over the specified parameters for the given compound and time point.
	CMatrix2D GetDistribution(double _time, EDistrTypes _distribution1, EDistrTypes _distribution2, const std::string& _compoundKey) const;
	// Returns the multi-dimensional distribution of a solid material over the specified parameters for the given compound and time point.
	CDenseMDMatrix GetDistribution(double _time, const std::vector<EDistrTypes>& _distributions, const std::string& _compoundKey) const;

	// Sets the one-dimensional distribution of a solid material over the specified parameter for the given time point.
	void SetDistribution(double _time, EDistrTypes _distribution, const std::vector<double>& _value);
	// Sets the two-dimensional distribution of a solid material over the specified parameter for the given time point.
	void SetDistribution(double _time, EDistrTypes _distribution1, EDistrTypes _distribution2, const CMatrix2D& _value);
	// Sets the multi-dimensional distribution of a solid material over the specified parameter for the given time point.
	void SetDistribution(double _time, const CDenseMDMatrix& _value);
	// Sets the one-dimensional distribution of a solid material over the specified parameter for the given compound and time point.
	void SetDistribution(double _time, EDistrTypes _distribution, const std::string& _compoundKey, const std::vector<double>& _value);
	// Sets the two-dimensional distribution of a solid material over the specified parameters for the given compound and time point.
	void SetDistribution(double _time, EDistrTypes _distribution1, EDistrTypes _distribution2, const std::string& _compoundKey, const CMatrix2D& _value);
	// Sets the multi-dimensional distribution of a solid material over the specified parameters for the given compound and time point.
	void SetDistribution(double _time, const std::string& _compoundKey, const CDenseMDMatrix& _value);

	// Applies the movement matrix to transform the multidimensional distributed parameters of a solid material for the given time point.
	void ApplyTM(double _time, const CTransformMatrix& _matrix);
	// Applies the movement matrix to transform the multidimensional distributed parameters of a solid material for the given compound and time point.
	void ApplyTM(double _time, const std::string& _compoundKey, const CTransformMatrix& _matrix);

	// Normalizes data in the solids distribution matrix for the given time point.
	void Normalize(double _time);
	// Normalizes data in the solids distribution matrix for the given time interval.
	void Normalize(double _timeBeg, double _timeEnd);
	// Normalizes data in the solids distribution matrix for all time points.
	void Normalize();

	////////////////////////////////////////////////////////////////////////////////
	// Particle size distributions
	//

	// Returns the specified type of the PSD of a total mixture of all solid materials for the given time point.
	std::vector<double> GetPSD(double _time, EPSDTypes _type, EPSDGridType _grid = EPSDGridType::DIAMETER) const;
	// Returns the specified type of the PSD of a selected compound for the given time point.
	std::vector<double> GetPSD(double _time, EPSDTypes _type, const std::string& _compoundKey, EPSDGridType _grid = EPSDGridType::DIAMETER) const;
	// Returns the specified type of the PSD of a mixture of selected compounds for the given time point. If the list of compounds is empty, the whole mixture is considered.
	std::vector<double> GetPSD(double _time, EPSDTypes _type, const std::vector<std::string>& _compoundKeys, EPSDGridType _grid = EPSDGridType::DIAMETER) const;

	/* Sets the specified type of the PSD of a total mixture of all solid materials for the given time point.
	 * For number-related PSD, the distribution is normalized and the total particle mass remains unchanged.*/
	void SetPSD(double _time, EPSDTypes _type, const std::vector<double>& _value, EPSDGridType _grid = EPSDGridType::DIAMETER);
	/* Sets the specified type of the PSD of a selected compound for the given time point.
	 * For number-related PSD, the distribution is normalized and the total particle mass remains unchanged. If the compound key if empty, the whole mixture is considered.*/
	void SetPSD(double _time, EPSDTypes _type, const std::string& _compoundKey, const std::vector<double>& _value, EPSDGridType _grid = EPSDGridType::DIAMETER);

	////////////////////////////////////////////////////////////////////////////////
	// Interactions with other streams
	//

	// Copies all stream data at the given time point. All data after the time point are removed from the stream.
	void Copy(double _time, const CBaseStream& _source);
	// Copies all stream data at the given time interval. All data after the end time point are removed from the stream.
	void Copy(double _timeBeg, double _timeEnd, const CBaseStream& _source);
	// Copies all stream data to the given time point from another time point of source stream. All data after the time point are removed from the stream.
	void Copy(double _timeDst, const CBaseStream& _source, double _timeSrc);

	// Mixes the specified stream with the current stream at the given time point. Can be applied only for streams with the same structure (MD dimensions, phases, materials, etc.).
	void Add(double _time, const CBaseStream& _source);
	/* Mixes the specified stream with the current stream at the given time interval. Can be applied only for streams with the same structure (MD dimensions, phases, materials, etc.).
	 * The stream will contain the union of time points from both streams.*/
	void Add(double _timeBeg, double _timeEnd, const CBaseStream& _source);

	/**
	 * Tests whether all values in the streams at the given time point are equal up to the tolerances.
	 * \retval true Streams are equal.
	 * \retval false Streams are not equal.
	 */
	static bool AreEqual(double _time, const CBaseStream& _stream1, const CBaseStream& _stream2);

	/**
	 * Tests whether all values in the stream at the given time points are equal up to the tolerances.
	 * \retval true Streams are equal.
	 * \retval false Streams are not equal.
	 */
	static bool AreEqual(double _time1, double _time2, const CBaseStream& _stream, double _absTol, double _relTol);
	static bool AreEqual(double _time1, double _time2, const CBaseStream& _stream);

	////////////////////////////////////////////////////////////////////////////////
	// Thermodynamics
	//

	// Returns a pointer to enthalpy calculator.
	[[nodiscard]] CMixtureEnthalpyLookup* GetEnthalpyCalculator() const;

	// Calculates enthalpy of the stream mixture for the temperature at the given time point using a lookup table.
	[[nodiscard]] double CalculateEnthalpyFromTemperature(double _time) const;
	// Calculates temperature of the stream mixture for the enthalpy at the given time point using a lookup table.
	[[nodiscard]] double CalculateTemperatureFromEnthalpy(double _time) const;

	////////////////////////////////////////////////////////////////////////////////
	// Other
	//

	// Returns current grid of distributed parameters.
	const CMultidimensionalGrid& GetGrid() const;

	// TODO: remove, initialize MDB in constructor
	// Sets pointer to the used materials database.
	void SetMaterialsDatabasePtr(const CMaterialsDatabase* _database);

	// Sets grids of distributed parameters.
	void SetGrid(const CMultidimensionalGrid& _grid);

	// Sets new cache settings.
	void SetCacheSettings(const SCacheSettings& _settings);

	// Sets tolerance settings.
	void SetToleranceSettings(const SToleranceSettings& _settings);

	// Sets thermodynamics settings.
	void SetThermodynamicsSettings(const SThermodynamicsSettings& _settings);

	// Performs nearest-neighbor extrapolation of all stream data.
	void Extrapolate(double _timeExtra, double _time);
	// Performs linear extrapolation of all stream data.
	void Extrapolate(double _timeExtra, double _time1, double _time2);
	// Performs cubic spline extrapolation of all stream data.
	void Extrapolate(double _timeExtra, double _time1, double _time2, double _time3);

	// Saves data to file.
	void SaveToFile(CH5Handler& _h5File, const std::string& _path);
	// Loads data from file.
	void LoadFromFile(const CH5Handler& _h5File, const std::string& _path);
	// Loads data from file of an older version.
	void LoadFromFile_v1(const CH5Handler& _h5File, const std::string& _path);

protected:
	using mix_type = std::tuple<std::map<EOverall, double>, std::map<EPhase, double>, std::map<EPhase, CDenseMDMatrix>>;
	// Calculates the mixture of two streams. Does not perform any checks.
	static mix_type CalculateMix(double _time1, const CBaseStream& _stream1, double _mass1, double _time2, const CBaseStream& _stream2, double _mass2);
	// Calculates the pressure of the mixture of two streams. Does not perform any checks.
	static double CalculateMixPressure(double _time1, const CBaseStream& _stream1, double _time2, const CBaseStream& _stream2);
	// Calculates the temperature of the mixture of two streams. Does not perform any checks.
	static double CalculateMixTemperature(double _time1, const CBaseStream& _stream1, double _mass1, double _time2, const CBaseStream& _stream2, double _mass2);
	// Calculates the general overall property of the mixture of two streams. Does not perform any checks.
	static double CalculateMixOverall(double _time1, const CBaseStream& _stream1, double _mass1, double _time2, const CBaseStream& _stream2, double _mass2, EOverall _property);
	// Calculates the phase fractions of the mixture of two streams for the given phase. Does not perform any checks.
	static double CalculateMixPhaseFractions(double _time1, const CBaseStream& _stream1, double _mass1, double _time2, const CBaseStream& _stream2, double _mass2, EPhase _phase);
	// Calculates the multidimensional distributions of the mixture of two streams for the given phase. Does not perform any checks.
	static CDenseMDMatrix CalculateMixDistribution(double _time1, const CBaseStream& _stream1, double _mass1, double _time2, const CBaseStream& _stream2, double _mass2, EPhase _phase);
	// Sets the result of mixing two streams into this stream at the given time point.
	void SetMix(double _time, const mix_type& _data);

private:
	// Inserts the new time into the list of time points, it it does not exist yet.
	void InsertTimePoint(double _time);
	// Checks whether the given time point exists.
	bool HasTime(double _time) const;

	// Checks whether the specified overall property is defined in the stream.
	bool HasOverallProperty(EOverall _property) const;

	// Checks whether the specified compound is defined in the stream.
	bool HasCompound(const std::string& _compoundKey) const;
	// Checks whether all the specified compounds are defined in the stream.
	bool HasCompounds(const std::vector<std::string>& _compoundKeys) const;
	// Returns index of the compound.
	size_t CompoundIndex(const std::string& _compoundKey) const;

	// Checks whether the specified phase is defined in the stream.
	bool HasPhase(EPhase _phase) const;

	/* Calculates the PSD of the stream in mass fractions for the selected compounds. If the list of components is empty, calculates the PSD for the entire mixture.
	 * Takes into account porosity, if specified. All checks of parameters, phases, grids availability, etc. must be executed by the calling code.*/
	std::vector<double> GetPSDMassFraction(double _time, const std::vector<std::string>& _compoundKeys) const;
	/* Calculates the number particle distribution of the stream for the selected compounds. If the list of components is empty, calculates the PSD for the entire mixture.
	 * Takes into account porosity, if specified. All checks of parameters, phases, grids availability, etc. must be executed by the calling code.*/
	std::vector<double> GetPSDNumber(double _time, const std::vector<std::string>& _compoundKeys, EPSDGridType _grid) const;

	// Clears enthalpy calculator.
	void ClearEnthalpyCalculator();

	// TODO: move it somewhere
	////////////////////////////////////////////////////////////////////////////////
	/// Deprecated functions
public:
	[[deprecated("WARNING! AddTimePoint(double, double) is deprecated. Use CopyTimePoint(double, double) instead.")]]
	void AddTimePoint(double _timeDst, double _timeSrc);
	[[deprecated("WARNING! GetStreamName() is deprecated. Use GetName() instead.")]]
	std::string GetStreamName() const;
	[[deprecated("WARNING! SetupStream(const CBaseStream*) is deprecated. Use SetupStructure(const CBaseStream*) instead.")]]
	void SetupStream(const CBaseStream* _stream);
	[[deprecated("WARNING! GetTimePointsForInterval(double, double, bool) is deprecated. Use GetTimePoints(double, double) or GetTimePointsClosed(double, double) instead.")]]
	std::vector<double> GetTimePointsForInterval(double _timeBeg, double _timeEnd, bool _inclusive) const;
	[[deprecated("WARNING! GetOverallProperty(double, unsigned) is deprecated. Use GetOverallProperty(double, EOverall), GetMixtureProperty(double, EOverall), GetMixtureProperty(double, ECompoundConstProperties) or GetMixtureProperty(double, ECompoundTPProperties) instead.")]]
	double GetOverallProperty(double _time, unsigned _property) const;
	[[deprecated("WARNING! GetMass_Base(double) is deprecated. Use GetMass(double) instead.")]]
	double GetMass_Base(double _time) const;
	[[deprecated("WARNING! SetMass_Base(double, double) is deprecated. Use SetMass(double, double) instead.")]]
	void SetMass_Base(double _time, double _value);
	[[deprecated("WARNING! GetCompoundsList() is deprecated. Use a unit-level version CBaseUnit::GetCompoundsList() instead.")]]
	std::vector<std::string> GetCompoundsList() const;
	[[deprecated("WARNING! GetCompoundsNames() is deprecated. Use a unit-level version CBaseUnit::GetAllCompoundsNames(), CBaseUnit::GetCompoundName(const std::string&) or CBaseUnit::GetCompoundName(size_t) instead.")]]
	std::vector<std::string> GetCompoundsNames() const;
	[[deprecated("WARNING! GetCompoundsNumber() is deprecated. Use a unit-level version CBaseUnit::GetCompoundsNumber() instead.")]]
	size_t GetCompoundsNumber() const;
	[[deprecated("WARNING! GetCompoundPhaseFraction(double, const std::string&, EPhaseTypes) is deprecated. Use GetCompoundFraction(double, const std::string&, EPhase) or GetCompoundsFractions(_time, EPhase) instead.")]]
	double GetCompoundPhaseFraction(double _time, const std::string& _compoundKey, unsigned _soa) const;
	[[deprecated("WARNING! GetCompoundPhaseFraction(double, unsigned, EPhaseTypes) is deprecated. Use GetCompoundFraction(double, const std::string&, EPhase) or GetCompoundsFractions(_time, EPhase) instead.")]]
	double GetCompoundPhaseFraction(double _time, unsigned _index, unsigned _soa) const;
	[[deprecated("WARNING! SetCompoundPhaseFraction(double, const std::string&, EPhaseTypes, double, eValueBasises) is deprecated. Use SetCompoundFraction(double, const std::string&, EPhase, double) or SetCompoundMolFraction(double, const std::string&, EPhase, double) instead.")]]
	void SetCompoundPhaseFraction(double _time, const std::string& _compoundKey, unsigned _soa, double _value, unsigned _basis = BASIS_MASS);
	[[deprecated("WARNING! GetPhaseMass_Base(double, EPhaseTypes) is deprecated. Use GetPhaseMass(double, EPhase) instead.")]]
	double GetPhaseMass_Base(double _time, unsigned _soa) const;
	[[deprecated("WARNING! SetPhaseMass_Base(double, EPhaseTypes, double) is deprecated. Use SetPhaseMass(double, EPhase, double) instead.")]]
	void SetPhaseMass_Base(double _time, unsigned _soa, double _value);
	[[deprecated("WARNING! GetPhaseSOA(unsigned) is deprecated. Use a unit-level version CBaseUnit::GetPhaseType(size_t) instead.")]]
	unsigned GetPhaseSOA(unsigned _index) const;
	[[deprecated("WARNING! GetPhaseIndex(EPhaseTypes) is deprecated. Access phases by their type.")]]
	unsigned GetPhaseIndex(unsigned _soa) const;
	[[deprecated("WARNING! GetPhasesNumber() is deprecated. Use a unit-level version CBaseUnit::GetPhasesNumber() instead.")]]
	size_t GetPhasesNumber() const;
	[[deprecated("WARNING! GetCompoundConstant(const std::string&, unsigned) is deprecated. Use GetCompoundProperty(const std::string&, ECompoundConstProperties) instead.")]]
	double GetCompoundConstant(const std::string& _compoundKey, unsigned _property) const;
	[[deprecated("WARNING! GetCompoundTPDProp(double, const std::string&, unsigned) is deprecated. Use GetCompoundProperty(double, const std::string&, ECompoundTPProperties) instead.")]]
	double GetCompoundTPDProp(double _time, const std::string& _compoundKey, unsigned _property) const;
	[[deprecated("WARNING! GetCompoundTPDProp(const std::string&, unsigned, double, double) is deprecated. Use GetCompoundProperty(const std::string&, ECompoundTPProperties, double, double) instead.")]]
	double GetCompoundTPDProp(const std::string& _compoundKey, unsigned _property, double _temperature, double _pressure) const;
	[[deprecated("WARNING! GetCompoundInteractionProp(double, const std::string&, const std::string&, unsigned) is deprecated. Use GetCompoundProperty(double, const std::string&, const std::string&, ECompoundTPProperties) instead.")]]
	double GetCompoundInteractionProp(double _time, const std::string& _compoundKey1, const std::string& _compoundKey2, unsigned _property) const;
	[[deprecated("WARNING! GetCompoundInteractionProp(const std::string&, const std::string&, unsigned, double, double) is deprecated. Use GetCompoundProperty(const std::string&, const std::string&, ECompoundTPProperties, double, double) instead.")]]
	double GetCompoundInteractionProp(const std::string& _compoundKey1, const std::string& _compoundKey2, unsigned _property, double _temperature, double _pressure) const;
	[[deprecated("WARNING! GetDistribution(double, EDistrTypes, std::vector<double>&) is deprecated. Use GetDistribution(double, EDistrTypes) instead.")]]
	bool GetDistribution(double _time, EDistrTypes _distribution, std::vector<double>& _result) const;
	[[deprecated("WARNING! CopyFromStream_Base(const CBaseStream&, double, bool) is deprecated. Use Copy(double, const CBaseStream&) instead.")]]
	void CopyFromStream_Base(const CBaseStream& _source, double _time, bool _deleteDataAfter = true);
	[[deprecated("WARNING! AddStream_Base(const CBaseStream&, double) is deprecated. Use Add(double, const CBaseStream&) instead.")]]
	void AddStream_Base(const CBaseStream& _source, double _time);
	[[deprecated("WARNING! GetSinglePhaseProp(double, unsigned, EPhaseTypes) is deprecated. Use GetPhaseFraction(double, EPhase), GetPhaseMass(double, EPhase), GetPhaseProperty(double, EPhase, EOverall), GetPhaseProperty(double, EPhase, ECompoundConstProperties) or GetPhaseProperty(double, EPhase, ECompoundTPProperties) instead.")]]
	double GetSinglePhaseProp(double _time, unsigned _property, unsigned _soa) const;
	[[deprecated("WARNING! GetPhaseTPDProp(double, unsigned, EPhaseTypes) is deprecated. Use GetPhaseProperty(double, EPhase, ECompoundTPProperties) instead.")]]
	double GetPhaseTPDProp(double _time, unsigned _property, unsigned _soa) const;
	[[deprecated("WARNING! SetSinglePhaseProp(double, unsigned, EPhaseTypes, double) is deprecated. Use SetPhaseFraction(double, EPhase, double) or SetPhaseMass(double, EPhase, double) instead.")]]
	void SetSinglePhaseProp(double _time, unsigned _property, unsigned _soa, double _value);
protected:
	static EPhase SOA2EPhase(unsigned _soa);
	static unsigned EPhase2SOA(EPhase _phase);
};

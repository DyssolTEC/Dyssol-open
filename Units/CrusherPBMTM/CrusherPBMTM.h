/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "UnitDevelopmentDefines.h"

class CCrusherPBMTM : public CDynamicUnit
{
private:
	enum class ESelection : size_t { CONSTANT = 0, LINEAR, QUADRATIC, POWER, EXPONENTIAL, KING, AUSTIN };
	enum class EBreakage : size_t { BINARY = 0, DIEMER, VOGEL, AUSTIN };
	enum class EMethod : size_t { NEWTON = 0, KR2 };

	CMaterialStream* m_inStream;			// Pointer to input stream.
	CMaterialStream* m_outStream;			// Pointer to output stream.
	CHoldup* m_holdup;						// Pointer to holdup.

	unsigned m_classesNum;					// Number of classes for PSD.
	std::vector<double> m_means;			// Classes means of PSD.
	std::vector<double> m_sizes;			// Classes sizes of PSD.
	std::vector<double> m_grid;			    // Classes limits for PSD.
	double m_holdupMass;					// Mass in the holdup.
	std::vector<double> m_S;				// Values of selection function.
	std::vector<std::vector<double>> m_B;	// Values of breakage function.
	std::vector<double> m_nu;				// Parameter for the calculation of weights.
	std::vector<double> m_WB;				// Weighted parameters for birth rate.
	std::vector<double> m_WD;				// Weighted parameters for death rate.
	CMatrix2D m_PT;							// Precalculated matrix to calculate transformation.
	CMatrix2D m_I;							// Identity matrix.
	CTransformMatrix m_TM;					// Transformation matrix.
	double m_dtMin;							// Minimum allowable time step for integration.
	double m_dtMax;							// Maximum allowable time step for integration.
	ESelection m_selectionFun;				// Chosen Selection function.
	EBreakage m_breakageFun;				// Chosen Breakage function.
	double m_sscale;						// Scale factor for Selection function.
	double m_s1;							// Parameter of Selection function.
	double m_s2;							// Parameter of Selection function.
	double m_s3;							// Parameter of Selection function.
	double m_b1;							// Parameter of Breakage function.
	double m_b2;							// Parameter of Breakage function.
	double m_b3;							// Parameter of Breakage function.
	EMethod m_method;						// Chosen solving method.

public:
	CCrusherPBMTM();

	void Initialize(double _dTime) override;
	void Simulate(double _dStartTime, double _dEndTime) override;

private:
	/** \brief Calculates selection function.
	 *  \param _x Volumes of particles.
	 *  \return Values of the selection function for all _x. */
	std::vector<double> CalculateSelectionFunction(const std::vector<double>& _x) const;
	/** \brief Calculates breakage function.
	*  \param _x Volumes of particles.
	*  \return Values of the breakage function for all _x. */
	std::vector<std::vector<double>> CalculateBreakageFunction(const std::vector<double>& _x);
	/** \brief Calculates nu.
	*  \param _x Volumes of particles. */
	std::vector<double> CalculateNu(const std::vector<double>& _x) const;
	/** \brief Calculates weighted parameters for births.
	*  \param _x Volumes of particles. */
	std::vector<double> CalculateBirthWeights(const std::vector<double>& _x) const;
	/** \brief Calculates weighted parameters for death.
	*  \param _x Volumes of particles. */
	std::vector<double> CalculateDeathWeights(const std::vector<double>& _x) const;
	/** \brief Calculates base of the transformation matrix to use for further calculations. */
	CMatrix2D CalculateBaseTransformationMatrix() const;

	/** \brief Calculates transformation matrix.
	*  \param _dt Time interval. */
	void CalculateTransformationMatrixNewton(double _dt);
	void CalculateTransformationMatrixRK2(double _dt);

	/** \brief Calculates selection function for the given particle size.
	*  \param _x Volume of particle.
	*  \return Value of the selection function for the specified _x. */
	double Selection(double _x) const;
	/** \brief Calculates breakage function for the given particles sizes.
	*  \param _x Volume of particle 1.
	*  \param _y Volume of particle 2.
	*  \return Value of the breakage function for the specified _x and _y. */
	double Breakage(double _x, double _y) const;

	/** \brief Calculates maximum allowable time step. */
	double MaxTimeStep(double _dt, const std::vector<double>& _q0) const;

	/** \brief Adaptive Simpson's mathod for numerical evaluation of integral f on interval [a, b].
	 * \param a Left boundary of the evaluation interval.
	 * \param b Right boundary of the evaluation interval.
	 * \param param Parameter.
	 * \param epsilon Error tolerance.
	 * \param recMaxDepth Max recursion depth.
	 * \return Calculated value. */
	double AdaptiveSimpsons(double a, double b, double param, double epsilon, int recMaxDepth) const;
	/** \brief Recursive auxiliary function for AdaptiveSimpsons() function. */
	/**
	* \brief Recursive auxiliary function for AdaptiveSimpsons() function.
	* \param a Left boundary of the evaluation interval.
	* \param b Right boundary of the evaluation interval.
	* \param param Parameter.
	* \param epsilon Error tolerance.
	* \param S
	* \param fa
	* \param fb
	* \param fc
	* \param recDepth Current recursion depth.
	* \return Value at step. */
	double AdaptiveSimpsonsRecursive(double a, double b, double param, double epsilon, double S, double fa, double fb, double fc, int recDepth) const;
};

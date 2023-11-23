/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once
#include "BaseSolver.h"
#include <functional>

/**
 * \brief Agglomeration solver.
 */
class CAgglomerationSolver : public CBaseSolver
{
public:
	/**
	 * \brief Types of agglomeration kernels.
	 */
	enum class EKernels : size_t
	{
		CONSTANT      = 0,  ///< Constant kernel.
		SUM           = 1,	///< Sum kernel.
		PRODUCT       = 2,	///< Product kernel.
		BROWNIAN      = 3,	///< Brownian kernel.
		SHEAR         = 4,	///< Shear kernel.
		PEGLOW        = 5,	///< Peglow kernel.
		COAGULATION   = 6,	///< Coagulation kernel.
		GRAVITATIONAL = 7,	///< Gravitational kernel.
		EKE           = 8,	///< Equipartition kinetic energy kernel.
		THOMPSON      = 9,	///< Thompson kernel.
		CUSTOM        = 10,	///< Custom kernel.
	};

protected:
	using u_matr_t = std::vector<std::vector<size_t>>; ///< Matrix of size_t.
	using d_matr_t = std::vector<std::vector<double>>; ///< Matrix of double.
	using u_vect_t = std::vector<size_t>;			   ///< Vector of size_t.
	using d_vect_t = std::vector<double>;			   ///< Vector of double.

	using kernel_t = double(double, double);	///< Type of the kernel function.

	EKernels m_kernel{ EKernels::BROWNIAN };	///< Selected kernel function.
	double m_beta0{ 1.0 };						///< Size independent agglomeration rate (set zero for no agglomeration).
	std::vector<double> m_parameters;			///< Additional parameters.
	std::vector<double> m_grid;					///< Diameter-related PSD grid.
	std::function<kernel_t> m_CutomKernel{};	///< Custom kernel function.

public:
	/**
	 * \private
	 */
	CAgglomerationSolver();
	/**
	 * \private
	 */
	~CAgglomerationSolver() override                                     = default;
	/**
	 * \private
	 */
	CAgglomerationSolver(const CAgglomerationSolver& _other)             = default;
	/**
	 * \private
	 */
	CAgglomerationSolver(CAgglomerationSolver && _other)                 = default;
	/**
	 * \private
	 */
	CAgglomerationSolver& operator=(const CAgglomerationSolver & _other) = default;
	/**
	 * \private
	 */
	CAgglomerationSolver& operator=(CAgglomerationSolver && _other)      = default;

	/**
	 * \brief Sets all required parameters and calls Initialize()
	 * \param _grid Diameter-related PSD grid
	 * \param _beta0 Size independent agglomeration rate
	 * \param _kernel Type of the agglomeration kernel
	 * \param _parameters Additional parameters
	 */
	void Initialize(const d_vect_t& _grid, double _beta0, EKernels _kernel, const d_vect_t& _parameters = d_vect_t());
	/**
	 * \brief Sets all required parameters and calls Initialize()
	 * \param _grid Diameter-related PSD grid
	 * \param _beta0 Size independent agglomeration rate
	 * \param _kernel Function of the agglomeration kernel
	 * \param _parameters Additional parameters
	 */
	void Initialize(const d_vect_t& _grid, double _beta0, const std::function<kernel_t>& _kernel, const d_vect_t& _parameters = d_vect_t());

	/**
	 * \brief Actual initialization of the solver.
	 */
	void Initialize() override;
	/**
	 * \brief Main calculation function
	 * \param _n Number distribution
	 * \param _rateB Output vector for birth rate
	 * \param _rateD Output vector for death rate
	 */
	virtual void Calculate(const d_vect_t& _n, d_vect_t& _rateB, d_vect_t& _rateD);
	/**
	 * \brief Main calculation function
	 * \param _n Number distribution
	 * \return Birth and death rates
	 */
	std::pair<d_vect_t, d_vect_t> Calculate(const d_vect_t& _n);

protected:
	/**
	 * Calculates the chosen kernel function for particles with volumes _u and _v.
	 */
	[[nodiscard]] double Kernel(double _u, double _v) const;

private:
	/**
	 * Sets all parameters.
	 */
	void SetParameters(const d_vect_t& _grid, double _beta0, EKernels _kernel, const std::function<kernel_t>& _kernelFun, const d_vect_t& _parameters);
};

typedef DECLDIR CAgglomerationSolver* (*CreateAgglomerationSolver)();

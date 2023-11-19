/* Copyright (c) 2020, Dyssol Development Team.
 * Copyright (c) 2023, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <vector>
#include <cstddef>

/**
 * \brief This class describes a two-dimensional matrix in dense format.
 */
class CMatrix2D
{
private:
	typedef std::vector<std::vector<double>> d_matr_t; ///< std::vector<std::vector<double>>
	typedef std::vector<double> d_vect_t;			   ///< std::vector<double>

	d_matr_t m_data;	///< The data itself
	size_t m_rows;		///< Number of rows in the matrix
	size_t m_cols;		///< Number of columns in the matrix

public:
	/**
	 * \brief Basic constructor.
	 * \details Creates empty matrix with zero in all rows and columns.
	 */
	CMatrix2D();
	/**
	 * \brief Creates a matrix with the specified dimensions and sets all cells to zero.
	 * \param _rows Number of rows.
	 * \param _cols Number of columns.
	 */
	CMatrix2D(size_t _rows, size_t _cols);


	//////////////////////////////////////////////////////////////////////////
	// Work with dimensions

	/**
	 * \brief Sets new dimensions of the matrix.
	 * \param _rows Number of rows.
	 * \param _cols Number of columns.
	 */
	void Resize(size_t _rows, size_t _cols);
	/**
	 * Sets new dimensions of the matrix and fills it with the specified value.
	 * \param _rows Number of rows.
	 * \param _cols Number of columns.
	 * \param _val Target value.
	 */
	void Assign(size_t _rows, size_t _cols, double _val);

	/**
	 * \brief Returns the current number of rows.
	 * \return Number of rows.
	 */
	size_t Rows() const;
	/**
	 * \brief Returns the current number of columns.
	 * \return Number of columns.
	 */
	size_t Cols() const;


	//////////////////////////////////////////////////////////////////////////
	// Work with data

	/**
	 * \brief Returns reference to the vector of values of the specified row.
	 * \param _row Index of row.
	 * \return Reference to the vector of values.
	 */
	d_vect_t& operator[](size_t _row);
	/**
	 * Returns const reference to the vector of values of the specified row.
	 * \param _row Index of row.
	 * \return Target reference to the vector of values.
	 */
	const d_vect_t& operator[](size_t _row) const;

	/**
	 * \brief Returns the vector of values for the specified row.
	 * \details If there is no such row, returns an empty vector.
	 * \param _row Index of row.
	 * \return Vector of values.
	 */
	d_vect_t GetRow(size_t _row) const;
	/**
	 * \brief Returns the vector of values for the specified column.
	 * \details If there is no such column, returns an empty vector.
	 * \param _col Index of column.
	 * \return Vector of values.
	 */
	d_vect_t GetCol(size_t _col) const;
	/**
	 * \brief Returns all values in the vector-of-vectors form.
	 * \return All values in the vector-of-vectors form.
	 */
	d_matr_t GetMatrix() const;

	/**
	 * \brief Sets data to the existing row.
	 * \details If dimensions do not match, does nothing.
	 * \param _row Index of row.
	 * \param _vector Vector of values.
	 */
	void SetRow(size_t _row, const d_vect_t& _vector);
	/**
	 * \brief Sets data to the existing column.
	 * \details If dimensions do not match, does nothing.
	 * \param _col Index of column.
	 * \param _vector Vector of values.
	 */
	void SetCol(size_t _col, const d_vect_t& _vector);
	/**
	 * \brief Sets all values to the matrix in the vector-of-vectors form.
	 * \details If dimensions do not match, does nothing.
	 * \param _matrix All values in the vector-of-vectors form.
	 */
	void SetMatrix(const d_matr_t& _matrix);

	/**
	 * \brief Fills the matrix with the specified value.
	 * \param _val Target value.
	 */
	void Fill(double _val);
	/**
	 * \brief Removes all data and information about dimensions from the matrix.
	 */
	void Clear();

	/**
	 * \brief Returns a vector with all values joining all rows one after another.
	 * \return Vector of all data.
	 */
	d_vect_t ToVector() const;


	//////////////////////////////////////////////////////////////////////////
	// Arithmetic

	/**
	 * \brief Normalizes values of the matrix.
	 */
	void Normalize();
	/**
	 * \brief Returns identity matrix with the specified dimensions.
	 * \param _size Size of the square matrix.
	 * \return Identity matrix.
	 */
	static CMatrix2D Identity(size_t _size);

	/**
	 * \brief Adds a value to all elements of the matrix.
	 * \param _val Value.
	 * \return Reference to this matrix.
	 */
	CMatrix2D& operator+=(double _val);
	/**
	 * \brief Adds a value to all elements of the matrix.
	 * \param _val Value.
	 * \return Copy of this matrix.
	 */
	CMatrix2D operator+(double _val) const;

	/**
	 * \brief Subtracts a value from all elements of the matrix.
	 * \param _val Value.
	 * \return Reference to this matrix.
	 */
	CMatrix2D& operator-=(double _val);
	/**
	 * \brief Subtracts a value from all elements of the matrix.
	 * \param _val Value.
	 * \return Copy of this matrix.
	 */
	CMatrix2D operator-(double _val) const;

	/**
	 * \brief Multiplies a value with all elements of the matrix.
	 * \param _val Value.
	 * \return Reference to this matrix.
	 */
	CMatrix2D& operator*=(double _val);
	/**
	 * \brief Multiplies a value with all elements of the matrix.
	 * \param _val Value.
	 * \return Copy of this matrix.
	 */
	CMatrix2D operator*(double _val) const;

	/**
	 * \brief Divides all elements of the matrix by a value .
	 * \param _val Value.
	 * \return Reference to this matrix.
	 */
	CMatrix2D& operator/=(double _val);
	/**
	 * \brief Divides all elements of the matrix by a value .
	 * \param _val Value.
	 * \return Copy of this matrix.
	 */
	CMatrix2D operator/(double _val) const;

	/**
	 * \brief Adds two matrices element-wise.
	 * \details If dimensions of the matrices do not match, std::runtime_error exception is thrown.
	 * \param _matrix Other matrix.
	 * \return Reference to this matrix.
	 */
	CMatrix2D& operator+=(const CMatrix2D& _matrix);
	/**
	 * \brief Adds two matrices element-wise.
	 * \details If dimensions of the matrices do not match, std::runtime_error exception is thrown.
	 * \param _matrix Other matrix.
	 * \return Copy of this matrix.
	 */
	CMatrix2D operator+(const CMatrix2D& _matrix) const;

	/**
	 * \brief Subtracts two matrices element-wise.
	 * \details If dimensions of the matrices do not match, std::runtime_error exception is thrown.
	 * \param _matrix Other matrix.
	 * \return Reference to this matrix.
	 */
	CMatrix2D& operator-=(const CMatrix2D& _matrix);
	/**
	 * \brief Subtracts two matrices element-wise.
	 * \details If dimensions of the matrices do not match, std::runtime_error exception is thrown.
	 * \param _matrix Other matrix.
	 * \return Copy of this matrix.
	 */
	CMatrix2D operator-(const CMatrix2D& _matrix) const;

	/**
	 * \brief Multiplies two matrices element-wise.
	 * \details If dimensions of the matrices do not match, std::runtime_error exception is thrown.
	 * \param _matrix Other matrix.
	 * \return Copy of this matrix.
	 */
	CMatrix2D operator*(const CMatrix2D& _matrix) const;
};

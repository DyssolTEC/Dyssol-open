/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <vector>
#include <cstddef>

/** This class describes a two-dimensional matrix in dense format. */
class CMatrix2D
{
private:
	typedef std::vector<std::vector<double>> d_matr_t;
	typedef std::vector<double> d_vect_t;

	d_matr_t m_data;	///< The data itself
	size_t m_rows;		///< Number of rows in the matrix
	size_t m_cols;		///< Number of columns in the matrix

public:
	/**
	* \brief **Basic constructor**. Creates empty matrix with zero in all rows and columns.
	*/
	CMatrix2D();
	/**
	* \brief Creates a matrix with the specified dimensions and sets all cells to zero.
	* \details
	* \param _rows Number of rows.
	* \param _cols Number of colums.
	*/
	CMatrix2D(size_t _rows, size_t _cols);


	//////////////////////////////////////////////////////////////////////////
	// Work with dimensions

	/**
	* \brief Sets new dimensions of the matrix.
	* \details
	* \param _rows Number of rows.
	* \param _cols Number of columns.
	*/
	void Resize(size_t _rows, size_t _cols);
	/** Sets new dimensions of the matrix and fills it with the specified value. */
	void Assign(size_t _rows, size_t _cols, double _val);

	/**
	* \brief Returns the current number of rows.
	* \details
	* \return Number of rows.
	*/
	size_t Rows() const;
	/**
	* \brief Returns the current number of columns.
	* \details
	* \return Number of columns.
	*/
	size_t Cols() const;


	//////////////////////////////////////////////////////////////////////////
	// Work with data

	/** Returns reference to the vector of values of the specified row. */
	d_vect_t& operator[](size_t _row);
	/** Returns const reference to the vector of values of the specified row. */
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
	* \brief Sets data to the existing row, if dimensions match.
	* \details
	* \param _row Index of row.
	* \param _vector Vector of values.
	*/
	void SetRow(size_t _row, const d_vect_t& _vector);
	/**
	* \brief Sets data to the existing column, if dimensions match.
	* \details
	* \param _col Index of column.
	* \param _vector Vector of values.
	*/
	void SetCol(size_t _col, const d_vect_t& _vector);
	/**
	* \brief Sets all values to the matrix in the vector-of-vectors form, if dimensions match.
	* \details
	* \param _matrix All values in the vector-of-vectors form.
	*/
	void SetMatrix(const d_matr_t& _matrix);

	/** Fills the matrix with the specified value. */
	void Fill(double _val);
	/**
	* \brief Removes all data and information about dimensions from the matrix.
	* \details
	*/
	void Clear();

	/** Returns a vector with all values joining all rows one after another. */
	d_vect_t ToVector() const;


	//////////////////////////////////////////////////////////////////////////
	// Arithmetic

	/**
	* \brief Normalizes values of the matrix.
	* \details
	*/
	void Normalize();
	/** Returns identity matrix with the specified dimensions. */
	static CMatrix2D Identity(size_t _size);

	CMatrix2D& operator+=(double _val);
	CMatrix2D operator+(double _val) const;

	CMatrix2D& operator-=(double _val);
	CMatrix2D operator-(double _val) const;

	CMatrix2D& operator*=(double _val);
	/**
	* \brief Performs **multiplication** of the matrix with a coefficient \p _val.
	* \details
	* \param _val Coefficient.
	*/
	CMatrix2D operator*(double _val) const;

	CMatrix2D& operator/=(double _val);
	CMatrix2D operator/(double _val) const;

	CMatrix2D& operator+=(const CMatrix2D& _matrix);
	/**
	* \brief Performs **addition** of two matrices with the same dimensions.
	* \details
	* \param _matrix Other matrix.
	*/
	CMatrix2D operator+(const CMatrix2D& _matrix) const;

	CMatrix2D& operator-=(const CMatrix2D& _matrix);
	/**
	* \brief Performs **subtraction** of two matrices with the same dimensions.
	* \details
	* \param _matrix Other matrix.
	*/
	CMatrix2D operator-(const CMatrix2D& _matrix) const;

	CMatrix2D operator*(const CMatrix2D& _matrix) const;
};

/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "Matrix2D.h"
#include "ThreadPool.h"
#include <numeric>
#include <algorithm>
#include <iterator>

CMatrix2D::CMatrix2D()
{
	m_rows = 0;
	m_cols = 0;
}

CMatrix2D::CMatrix2D(size_t _rows, size_t _cols)
{
	m_data.resize(_rows, d_vect_t(_cols, 0));
	m_rows = _rows;
	m_cols = _cols;
}

void CMatrix2D::Resize(size_t _rows, size_t _cols)
{
	if (_rows == m_rows && _cols == m_cols) return;
	m_data.assign(_rows, d_vect_t(_cols, 0.0));
	m_rows = _rows;
	m_cols = _cols;
}

void CMatrix2D::Assign(size_t _rows, size_t _cols, double _val)
{
	Resize(_rows, _cols);
	Fill(_val);
}

size_t CMatrix2D::Rows() const
{
	return m_rows;
}

size_t CMatrix2D::Cols() const
{
	return m_cols;
}

CMatrix2D::d_vect_t& CMatrix2D::operator[](size_t _row)
{
	return m_data[_row];
}

const CMatrix2D::d_vect_t& CMatrix2D::operator[](size_t _row) const
{
	return m_data[_row];
}

CMatrix2D::d_vect_t CMatrix2D::GetRow(size_t _row) const
{
	if (_row >= m_rows) return {};
	return m_data[_row];
}

CMatrix2D::d_vect_t CMatrix2D::GetCol(size_t _col) const
{
	if (_col >= m_cols) return {};
	d_vect_t res(m_rows);
	for (size_t i = 0; i < m_rows; ++i)
		res[i] = m_data[i][_col];
	return res;
}

CMatrix2D::d_matr_t CMatrix2D::GetMatrix() const
{
	return m_data;
}

void CMatrix2D::SetRow(size_t _row, const d_vect_t& _vector)
{
	if (_row >= m_rows || _vector.size() != m_cols) return;
	m_data[_row] = _vector;
}

void CMatrix2D::SetCol(size_t _col, const d_vect_t& _vector)
{
	if (_col >= m_cols || _vector.size() != m_rows) return;
	for (size_t i = 0; i < m_rows; ++i)
		m_data[i][_col] = _vector[i];
}

void CMatrix2D::SetMatrix(const d_matr_t& _matrix)
{
	if (_matrix.size() != m_rows) return;
	for (const auto& row : _matrix)
		if (row.size() != m_cols) return;
	m_data = _matrix;
}

void CMatrix2D::Fill(double _val)
{
	for (auto& row : m_data)
		std::fill(row.begin(), row.end(), _val);
}

void CMatrix2D::Clear()
{
	m_data.clear();
	m_rows = m_cols = 0;
}

CMatrix2D::d_vect_t CMatrix2D::ToVector() const
{
	d_vect_t res;
	res.reserve(m_rows * m_cols);
	for (size_t i = 0; i < m_rows; ++i)
		std::copy(m_data[i].begin(), m_data[i].end(), std::back_inserter(res));
	return res;
}

void CMatrix2D::Normalize()
{
	double sum = 0;
	for (auto& row : m_data)
		sum += std::accumulate(row.begin(), row.end(), 0.);
	if (sum != 0 && sum != 1)
		for (auto& row : m_data)
			for (auto& val : row)
				val /= sum;
}

CMatrix2D CMatrix2D::Identity(size_t _size)
{
	CMatrix2D res(_size, _size);
	for (size_t i = 0; i < _size; ++i)
		res.m_data[i][i] = 1.;
	return res;
}

CMatrix2D& CMatrix2D::operator+=(double _val)
{
	for (size_t i = 0; i < m_rows; ++i)
		for (size_t j = 0; j < m_cols; ++j)
			m_data[i][j] += _val;
	return *this;
}

CMatrix2D CMatrix2D::operator+(double _val) const
{
	return CMatrix2D(*this) += _val;
}

CMatrix2D& CMatrix2D::operator-=(double _val)
{
	for (size_t i = 0; i < m_rows; ++i)
		for (size_t j = 0; j < m_cols; ++j)
			m_data[i][j] -= _val;
	return *this;
}

CMatrix2D CMatrix2D::operator-(double _val) const
{
	return CMatrix2D(*this) -= _val;
}

CMatrix2D& CMatrix2D::operator/=(double _val)
{
	for (size_t i = 0; i < m_rows; ++i)
		for (size_t j = 0; j < m_cols; ++j)
			m_data[i][j] /= _val;
	return *this;
}

CMatrix2D CMatrix2D::operator/(double _val) const
{
	return CMatrix2D(*this) /= _val;
}

CMatrix2D& CMatrix2D::operator*=(double _val)
{
	for (size_t i = 0; i < m_rows; ++i)
		for (size_t j = 0; j < m_cols; ++j)
			m_data[i][j] *= _val;
	return *this;
}

CMatrix2D CMatrix2D::operator*(double _val) const
{
	return CMatrix2D(*this) *= _val;
}

CMatrix2D& CMatrix2D::operator+=(const CMatrix2D& _matrix)
{
	if (m_rows != _matrix.m_rows || m_cols != _matrix.m_cols)
		throw std::runtime_error("Matrix dimensions are not the same");
	for (size_t i = 0; i < m_rows; ++i)
		for (size_t j = 0; j < m_cols; ++j)
			m_data[i][j] += _matrix[i][j];
	return *this;
}

CMatrix2D CMatrix2D::operator+(const CMatrix2D& _matrix) const
{
	return CMatrix2D(*this) += _matrix;
}

CMatrix2D& CMatrix2D::operator-=(const CMatrix2D& _matrix)
{
	if (m_rows != _matrix.m_rows || m_cols != _matrix.m_cols)
		throw std::runtime_error("Matrix dimensions are not the same");
	for (size_t i = 0; i < m_rows; ++i)
		for (size_t j = 0; j < m_cols; ++j)
			m_data[i][j] -= _matrix[i][j];
	return *this;
}

CMatrix2D CMatrix2D::operator-(const CMatrix2D& _matrix) const
{
	return CMatrix2D(*this) -= _matrix;
}

CMatrix2D CMatrix2D::operator*(const CMatrix2D& _matrix) const
{
	if (m_cols != _matrix.m_rows)
		throw std::runtime_error("Matrix dimensions are not the same");
	CMatrix2D res(m_rows, _matrix.m_cols);
	ParallelFor(m_rows, [&](size_t i)
	{
		for (size_t j = 0; j < _matrix.m_cols; ++j)
			for (size_t k = 0; k < m_cols; ++k)
				res[i][j] += m_data[i][k] * _matrix.m_data[k][j];
	});
	return res;
}

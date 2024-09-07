/* Copyright (c) 2024, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <iterator>

template <typename T>
struct SReversionWrapper { T& iterable; };

template <typename T>
auto begin(SReversionWrapper<T> _w) { return std::rbegin(_w.iterable); }

template <typename T>
auto end(SReversionWrapper<T> _w) { return std::rend(_w.iterable); }

template <typename T>
SReversionWrapper<T> Reverse(T&& _iterable) { return { _iterable }; }
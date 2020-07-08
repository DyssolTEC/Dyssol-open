/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <future>

namespace ThreadPool
{
	/// A wrapper around the stdandard future that adds the behavior of futures returned from std::async: object will block and wait for execution to finish before going out of scope.
	template <typename T>
	class CTaskFuture
	{
		std::future<T> m_future;

	public:
		CTaskFuture(std::future<T>&& _future) : m_future{ std::move(_future) } { }
		CTaskFuture(const CTaskFuture& _other) = delete;
		CTaskFuture& operator=(const CTaskFuture& _other) = delete;
		CTaskFuture(CTaskFuture&& _other) = default;
		CTaskFuture& operator=(CTaskFuture&& _other) = default;
		~CTaskFuture()
		{
			if (m_future.valid())
				m_future.get();
		}

		auto get() const
		{
			return m_future.get();
		}
	};
}

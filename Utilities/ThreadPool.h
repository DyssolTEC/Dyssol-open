/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include "ThreadSafeQueue.h"
#include "ThreadTask.h"
#include "TaskFuture.h"
#include <functional>

namespace ThreadPool
{
	class CThreadPool
	{
		static size_t m_threadsLimit;								/// Maximum number of threads available for tyhis instance.
		CThreadSafeQueue<std::unique_ptr<IThreadTask>> m_workQueue;	/// Queue of submitted works.
		std::vector<std::thread> m_threads;							/// List of available threads.

	public:
		explicit CThreadPool(size_t _threads = 0);

		~CThreadPool();
		CThreadPool(const CThreadPool& _other) = delete;
		CThreadPool& operator=(const CThreadPool& _other) = delete;
		CThreadPool(CThreadPool&& _other) = delete;
		CThreadPool& operator=(CThreadPool&& _other) = delete;

		/// Specifies the maximum number of threads that can be generated for this instance.
		static void SetMaxThreadsNumber(size_t _threads);
		/// Returns number of threads available on this hardware for this instance.
		static size_t GetAvailableThreadsNumber();
		/// Returns number of defined threads.
		size_t GetThreadsNumber() const;

		/// Submits _count of identical jobs, running _fun(i) _count times with i = [0; count).
		void SubmitParallelJobs(size_t _count, const std::function<void(size_t)>& _fun);

		/// Submits a job _fun with arguments _args.
		template <typename Func, typename... Args>
		auto SubmitJob(Func&& _fun, Args&&... _args)
		{
			auto boundTask = std::bind(std::forward<Func>(_fun), std::forward<Args>(_args)...);
			using ResultType = std::result_of_t<decltype(boundTask)()>;
			using PackagedTask = std::packaged_task<ResultType()>;
			using TaskType = CThreadTask<PackagedTask>;

			PackagedTask task{ std::move(boundTask) };
			CTaskFuture<ResultType> result{ task.get_future() };
			m_workQueue.Push(std::make_unique<TaskType>(std::move(task)));
			return result;
		}

	private:
		/// Constantly running function, which each thread uses to acquire work items from the queue.
		void Worker();
	};
}

/// Default thread pool always ready to execute tasks.
inline ThreadPool::CThreadPool& getThreadPool()
{
	static ThreadPool::CThreadPool pool;
	return pool;
}

/// Initialize thread pool with current parameters.
inline void InitializeThreadPool()
{
	getThreadPool();
}

/// Submits a job _fun of any type with arguments _args.
template <typename Func, typename... Args>
auto ParallelJob(Func&& _fun, Args&&... _args)
{
	return getThreadPool().SubmitJob(std::forward<Func>(_fun), std::forward<Args>(_args)...);
}

/// Submits _count of identical jobs, running function _fun(i) _count times with i = [0; count).
inline void ParallelFor(size_t _count, const std::function<void(size_t)>& _fun)
{
	return getThreadPool().SubmitParallelJobs(_count, _fun);
}

#define PARALLEL_FOR(COUNT, ITERATOR, FUNCTION) ParallelFor(COUNT, [&](size_t ITERATOR) { FUNCTION });

/* Copyright (c) 2020, Dyssol Development Team. All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#include "ThreadPool.h"
#include <limits>
#include <future>

size_t ThreadPool::CThreadPool::m_threadsLimit = std::numeric_limits<size_t>::max();

ThreadPool::CThreadPool::CThreadPool(size_t _threads)
{
	// if number of threads not specified, calculate it
	if (_threads == 0)
		_threads = GetAvailableThreadsNumber();

	// create threads
	for (size_t i = 0; i < _threads; ++i)
		m_threads.emplace_back(&CThreadPool::Worker, this);
}

ThreadPool::CThreadPool::~CThreadPool()
{
	// invalidate the queue
	m_workQueue.Invalidate();
	// join all running threads
	for (auto& thread : m_threads)
		if (thread.joinable())
			thread.join();
}

void ThreadPool::CThreadPool::SetMaxThreadsNumber(size_t _threads)
{
	m_threadsLimit = _threads;
}

size_t ThreadPool::CThreadPool::GetAvailableThreadsNumber()
{
	// use the maximum hardware threads number
	size_t threads = std::thread::hardware_concurrency();
	// if number of threads exceeds the limit, reduce to the limit
	if (threads > m_threadsLimit)
		threads = m_threadsLimit;
	// always create at least one thread
	if (threads == 0)
		threads = 1;
	return threads;
}

size_t ThreadPool::CThreadPool::GetThreadsNumber() const
{
	return m_threads.size();
}

void ThreadPool::CThreadPool::SubmitParallelJobs(size_t _count, const std::function<void(size_t)>& _fun)
{
	using FunType = std::function<void()>;

	// number of available threads
	const size_t threadsNumber = m_threads.size();
	// number of tasks per thread
	const size_t tasksPerThread = _count / threadsNumber;
	// additional tasks, arising if the number of tasks is not evenly distributable by all threads
	const size_t additionalTasks = _count % threadsNumber;

	int result_counter{ 0 };
	std::mutex wait_mutex;
	std::unique_lock<std::mutex> lock(wait_mutex);
	std::condition_variable wait_event;
	CThreadSafeQueue<std::exception_ptr> exceptions; // exceptions catch from threads

	for (size_t iThread = 0; iThread < threadsNumber; ++iThread)
	{
		size_t size = tasksPerThread;
		if (additionalTasks > iThread) ++size;
		if (size == 0) break;

		result_counter++;

		// the batch task
		FunType task = [iThread, threadsNumber, size, &_fun, &result_counter, &wait_event, &wait_mutex, &exceptions]()
		{
			try {
				// call the _fun tasksPerThread times increasing the parameter
				for (size_t j = 0; j < size; ++j)
					_fun(threadsNumber*j + iThread);
			}
			catch(...)
			{
				// remember current exception
				exceptions.Push(std::current_exception());
			}

			std::unique_lock<std::mutex> lock_task(wait_mutex);
			--result_counter;
			if (result_counter == 0)
				wait_event.notify_all();
		};

		// submit the batch task
		m_workQueue.Push(std::make_unique<CThreadTask<const FunType>>(std::move(task)));
	}

	// wait all batch tasks to finish
	while (result_counter != 0)
		wait_event.wait(lock);

	// rethrow an exception, if any, to the calling thread
	if (!exceptions.IsEmpty())
	{
		std::exception_ptr ptr;
		exceptions.WaitPop(ptr);
		std::rethrow_exception(ptr);
	}
}

void ThreadPool::CThreadPool::Worker()
{
	while (true)
	{
		std::unique_ptr<IThreadTask> task{ nullptr };
		if (!m_workQueue.WaitPop(task)) break; // if the queue has been invalidated, finish queuing jobs
		task->Execute();
	}
}

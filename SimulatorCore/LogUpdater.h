#pragma once

#include "BaseUnit.h"
#include "SimulatorLog.h"
#include <thread>
#include <atomic>

class CLogUpdater
{
	CSimulatorLog* m_log{};
	CBaseUnit* m_model{};
	std::mutex m_mutex;
	std::thread m_thread;
	std::atomic<bool> m_running{ false };

public:
	explicit CLogUpdater(CSimulatorLog* _log) : m_log{ _log } {}

	~CLogUpdater()
	{
		ReleaseModel();
		Stop();
	}

	CLogUpdater(const CLogUpdater& _other)            = delete;
	CLogUpdater(CLogUpdater&& _other)                 = delete;
	CLogUpdater& operator=(const CLogUpdater& _other) = delete;
	CLogUpdater& operator=(CLogUpdater&& _other)      = delete;

	void Run()
	{
		if (m_running) return;
		m_running = true;
		m_thread = std::thread{ [&]
		{
			while (m_running)
			{
				{
					std::unique_lock lock{ m_mutex };
					if (m_model) UpdateLog();
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		} };
	}

	void Stop()
	{
		m_running = false;
		if (m_thread.joinable())
			m_thread.join();
	}

	void SetModel(CBaseUnit* _model)
	{
		std::lock_guard lock{ m_mutex };
		m_model = _model;
	}

	void ReleaseModel()
	{
		std::lock_guard lock{ m_mutex };
		if (m_model) UpdateLog();
		m_model = nullptr;
	}

private:
	void UpdateLog() const
	{
		if (m_model->HasWarning())	m_log->WriteWarning(m_model->PopWarningMessage());
		if (m_model->HasInfo())		m_log->WriteInfo(m_model->PopInfoMessage());
	}
};

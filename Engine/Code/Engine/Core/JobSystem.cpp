#include "Engine/Core/JobSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"

JobSystem* g_jobSystem = nullptr;

JobSystem::JobSystem(JobSystemConfig const& config)
	: m_config(config)
{
}


void JobSystem::Startup()
{
	for (int threadIndex = 0; threadIndex < m_config.m_numAdditionalThreads; threadIndex++)
	{
		CreateWorkerThread(Stringf("WorkerThread#%i", threadIndex).c_str());
	}
}


void JobSystem::Shutdown()
{
	for (int threadIndex = 0; threadIndex < (int) m_workerThreads.size(); threadIndex++)
	{
		JobWorkerThread* workerThread = m_workerThreads[threadIndex];
		DestroyWorkerThread(workerThread);
	}

	m_queuedJobsMutex.lock();
	for (int jobIndex = 0; jobIndex < (int) m_queuedJobs.size(); jobIndex++)
	{
		if (m_queuedJobs[jobIndex])
		{
			delete m_queuedJobs[jobIndex];
			m_queuedJobs[jobIndex] = nullptr;
		}
	}
	m_queuedJobsMutex.unlock();

	m_completedJobsMutex.lock();
	for (int jobIndex = 0; jobIndex < (int) m_completedJobs.size(); jobIndex++)
	{
		if (m_completedJobs[jobIndex])
		{
			delete m_completedJobs[jobIndex];
			m_completedJobs[jobIndex] = nullptr;
		}
	}
	m_completedJobsMutex.unlock();
}


void JobSystem::CreateWorkerThread(const char* uniqueWorkerThreadId)
{
	for (int threadIndex = 0; threadIndex < m_workerThreads.size(); threadIndex++)
	{
		if (m_workerThreads[threadIndex] == nullptr)
		{
			m_workerThreads[threadIndex] = new JobWorkerThread(uniqueWorkerThreadId, this);
			//g_console->AddLine(DevConsole::MINOR_INFO, Stringf("Thread created: %s", uniqueWorkerThreadId));
			return;
		}
	}

	JobWorkerThread* workerThread = new JobWorkerThread(uniqueWorkerThreadId, this);
	g_console->AddLine(DevConsole::MINOR_INFO, Stringf("Thread created: %s", uniqueWorkerThreadId));
	m_workerThreads.push_back(workerThread);
}


void JobSystem::DestroyWorkerThread(const char* uniqueWorkerThreadId)
{
	for (int threadIndex = 0; threadIndex < m_workerThreads.size(); threadIndex++)
	{
		JobWorkerThread*& workerThread = m_workerThreads[threadIndex];
		if (workerThread != nullptr && _stricmp(workerThread->m_threadId.c_str(), uniqueWorkerThreadId) == 0)
		{
			DestroyWorkerThread(workerThread);
			return;
		}
	}
}


void JobSystem::DestroyWorkerThread(JobWorkerThread*& workerThread)
{
	if (workerThread == nullptr)
		return;

	workerThread->Stop();
	delete workerThread;
	workerThread = nullptr;
}


Job* JobSystem::FindQueuedJobForWorker()
{
	Job* job = nullptr;
	m_queuedJobsMutex.lock();
	if (m_queuedJobs.size() > 0)
	{
		job = m_queuedJobs.front();
		m_queuedJobs.pop_front();
	}
	m_queuedJobsMutex.unlock();

	if (job != nullptr)
	{
		m_runningJobsMutex.lock();
		for (int jobIndex = 0; jobIndex < (int) m_runningJobs.size(); jobIndex++)
		{
			if (m_runningJobs[jobIndex] == nullptr)
			{
				m_runningJobs[jobIndex] = job;
				m_runningJobsMutex.unlock();
				return job;
			}
		}

		m_runningJobs.push_back(job);
		m_runningJobsMutex.unlock();
		return job;
	}

	return nullptr;
}


void JobSystem::OnJobComplete(Job* completedJob)
{
	g_console->AddLine(Rgba8::RED, Stringf("OnJobComplete Started: %s", completedJob->m_name.c_str()));
	m_runningJobsMutex.lock();
	for (int jobIndex = 0; jobIndex < (int) m_runningJobs.size(); jobIndex++)
	{
		if (m_runningJobs[jobIndex] == completedJob)
		{
			g_console->AddLine(Rgba8::RED, Stringf("OnJobComplete Removed from running: %s", completedJob->m_name.c_str()));
			m_runningJobs[jobIndex] = nullptr;
			break;
		}
	}
	m_runningJobsMutex.unlock();

	m_completedJobsMutex.lock();
	for (int jobIndex = 0; jobIndex < (int) m_completedJobs.size(); jobIndex++)
	{
		if (m_completedJobs[jobIndex] == nullptr)
		{
			g_console->AddLine(Rgba8::RED, Stringf("OnJobComplete Added to completed: %s", completedJob->m_name.c_str()));
			m_completedJobs[jobIndex] = completedJob;
			m_completedJobsMutex.unlock();
			return;
		}
	}

	g_console->AddLine(Rgba8::RED, Stringf("OnJobComplete Added to completed: %s", completedJob->m_name.c_str()));
	m_completedJobs.push_back(completedJob);
	m_completedJobsMutex.unlock();
}


void JobSystem::AddJobToQueue(Job* job)
{
	m_queuedJobsMutex.lock();
	m_queuedJobs.push_back(job);
	m_queuedJobsMutex.unlock();
}


void JobSystem::ClaimAllCompletedJobs()
{
	m_completedJobsMutex.lock();
	g_console->AddLine(Rgba8(186, 107, 11), Stringf("Claim all jobs. Completed jobs count: %i", m_completedJobs.size()));
	for (int jobIndex = 0; jobIndex < (int) m_completedJobs.size(); jobIndex++)
	{
		if (m_completedJobs[jobIndex])
		{
			m_completedJobs[jobIndex]->OnClaimCallback();
			delete m_completedJobs[jobIndex];
			m_completedJobs[jobIndex] = nullptr;
		}
	}
	m_completedJobsMutex.unlock();
}


JobWorkerThread::JobWorkerThread(const char* workerThreadId, JobSystem* jobSystem)
	: m_threadId(workerThreadId)
	, m_isRunning(true)
	, m_jobSystem(jobSystem)
{
	m_workerThread = new std::thread(&JobWorkerThread::WorkerThreadProc, this);
}


void JobWorkerThread::Stop()
{
	m_isRunning = false;
	m_workerThread->join();
	g_console->AddLine(DevConsole::MAJOR_INFO, Stringf("Thread stopped: %s", m_threadId.c_str()));
}


void JobWorkerThread::WorkerThreadProc()
{
	while (m_isRunning)
	{
		Job* job = m_jobSystem->FindQueuedJobForWorker();
		if (job)
		{
			g_console->AddLine(DevConsole::WARNING_MESSAGE, Stringf("Job started: %s | Thread: %s", job->m_name.c_str(), m_threadId.c_str()));
			job->Execute();
			m_jobSystem->OnJobComplete(job);
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
}

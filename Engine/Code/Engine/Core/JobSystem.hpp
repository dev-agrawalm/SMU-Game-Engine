#pragma once
#include <deque>
#include<thread>
#include<mutex>
#include <vector>

class JobSystem;

class Job
{
public:
	virtual ~Job() {}
	virtual void Execute() = 0;
	virtual void OnClaimCallback() {};

public:
	std::string m_name;
};


class JobWorkerThread
{
	friend class JobSystem;

public:
	JobWorkerThread(const char* workerThreadId, JobSystem* jobSystem);
	void Stop();

private:
	void WorkerThreadProc();

private:
	std::thread* m_workerThread = nullptr;
	std::string m_threadId;
	bool m_isRunning = false;
	JobSystem* m_jobSystem = nullptr;
};


struct JobSystemConfig
{
public:
	int m_numAdditionalThreads = 2;
};


class JobSystem
{
	friend class JobWorkerThread;

public:
	JobSystem(JobSystemConfig const& config);

	void Startup();
	void Shutdown();

	void CreateWorkerThread(const char* uniqueWorkerThreadId);
	void DestroyWorkerThread(const char* uniqueWorkerThreadId);
	void AddJobToQueue(Job* job);

	void ClaimAllCompletedJobs();

private:
	void DestroyWorkerThread(JobWorkerThread*& workerThread);
	Job* FindQueuedJobForWorker();
	void OnJobComplete(Job* completedJob);

private:
	JobSystemConfig m_config = {};

	std::vector<JobWorkerThread*> m_workerThreads;
	
	std::mutex m_queuedJobsMutex;
	std::deque<Job*> m_queuedJobs;

	std::mutex m_runningJobsMutex;
	std::vector<Job*> m_runningJobs;

	std::mutex m_completedJobsMutex;
	std::vector<Job*> m_completedJobs;
};

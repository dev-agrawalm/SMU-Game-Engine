#pragma once
#include <optional>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>

template <typename T>
class SynchronizedBlockingQueue
{
public:
	void Push(T const& toPush);
	std::optional<T> Pop();
	void Exit();

private:
	std::condition_variable m_condition;
	std::mutex m_lock;
	std::queue<T> m_queue;
	std::atomic<bool> m_exit;
};


template <typename T>
void SynchronizedBlockingQueue<T>::Exit()
{
	std::unique_lock<std::mutex> guard(m_lock);
// 	while (!m_queue.empty())
// 	{
// 		Pop();
// 	}
	m_exit = true;
	m_condition.notify_all();
}


template <typename T>
std::optional<T> SynchronizedBlockingQueue<T>::Pop()
{
	std::optional<T> value = std::nullopt;
	std::unique_lock<std::mutex> uniqueLock(m_lock);
	while (m_queue.empty() && !m_exit)
	{
		m_condition.wait(uniqueLock);
	}
	if (!m_queue.empty())
	{
		value = m_queue.front();
		m_queue.pop();
	}
	return value;
}


template <typename T>
void SynchronizedBlockingQueue<T>::Push(T const& toPush)
{
	std::unique_lock<std::mutex> guard(m_lock);
	m_queue.push(toPush);
	m_condition.notify_all();
}

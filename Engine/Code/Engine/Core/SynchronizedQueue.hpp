#pragma once
#include <optional>
#include <atomic>
#include <queue>

template <typename T>
class SynchronizedQueue
{
public:
	void Push(T const& toPush);
	std::optional<T> Pop();

protected:
	void Lock();
	void Unlock();

private:
	std::atomic<bool> m_lock = false;
	std::queue<T> m_queue;
};


template <typename T>
void SynchronizedQueue<T>::Unlock()
{
	m_lock.store(false, std::memory_order_release);
}


template <typename T>
void SynchronizedQueue<T>::Lock()
{
	while (m_lock.exchange(true, std::memory_order_acquire))
	{
		//do nothing
		//lock acquired
	}
}


template <typename T>
std::optional<T> SynchronizedQueue<T>::Pop()
{
	std::optional<T> value = std::nullopt;
	Lock();
	if (!m_queue.empty())
	{
		value = m_queue.front();
		m_queue.pop();
	}
	Unlock();
	return value;
}


template <typename T>
void SynchronizedQueue<T>::Push(T const& toPush)
{
	Lock();
	m_queue.push(toPush);
	Unlock();
}

#pragma once
#include <vector>
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <stdexcept>

template <class T>
class CircularBuffer
{
public:
	CircularBuffer();
	CircularBuffer(int bufferSize);
	explicit CircularBuffer(CircularBuffer<T> const& copyBuffer);
	~CircularBuffer();

	T Pop();
	void Push(T const& toPush);
	void Clear();
	void Resize(int newBufferSize);

	int GetFrontIndex() const;
	int GetBackIndex() const;
	T GetFrontElement() const;
	T GetBackElement() const;
	T GetElementAtIndex(int index) const;
	T GetElementFromBack(int offsetFromBack) const;
	T GetElementFromFront(int offsetFromFront) const;
	T GetNthElement(int n);
	std::vector<T> GetBufferAsVector() const;
	bool IsEmpty() const;
	int GetSize() const;
private:
	std::vector<T> m_data;
	int m_front = -1;
	int m_back = -1;
	int m_size = -1;
};


template <class T>
T CircularBuffer<T>::GetNthElement(int n)
{
	if (IsEmpty())
	{
		ERROR_AND_DIE(Stringf("Trying to access %i-th element of an empty circular buffer", n));
	}

	int index = m_front + n;
	index %= m_size;
	return m_data[index];
}


template <class T>
std::vector<T> CircularBuffer<T>::GetBufferAsVector() const
{
	std::vector<float> bufferAsArray;
	if (IsEmpty())
		return bufferAsArray;

	int front = m_front;

	if (front > m_back)
	{
		while (front < m_size)
		{
			float element = GetElementAtIndex(front);
			bufferAsArray.push_back(element);
			front++;
		}
		front = 0;
	}

	while (front <= m_back)
	{
		float element = GetElementAtIndex(front);
		bufferAsArray.push_back(element);
		front++;
	}

	return bufferAsArray;
}


template <class T>
T CircularBuffer<T>::GetElementFromFront(int offsetFromFront) const
{
	if (IsEmpty())
	{
		ERROR_AND_DIE(Stringf("Trying to access element of an empty circular buffer"));
	}

	return T();
}


template <class T>
T CircularBuffer<T>::GetElementFromBack(int offsetFromBack) const
{
	if (IsEmpty())
	{
		ERROR_AND_DIE(Stringf("Trying to access element of an empty circular buffer"));
	}

	int back = m_back;
	int offset = offsetFromBack;

	if (back < m_front)
	{
		while (back > 0 && offset > 0)
		{
			back--;
			offset--;
		}

		if (offset <= 0)
			return m_data[back];

		back = m_size - 1;
	}

	while (back > m_front && offset > 0)
	{
		offset--;
		back--;
	}

	return m_data[back];
}


template <class T>
CircularBuffer<T>::CircularBuffer()
{

}


template <class T>
CircularBuffer<T>::CircularBuffer(int bufferSize)
	: m_size(bufferSize)
{
	m_data.resize(m_size);
}


template <class T>
CircularBuffer<T>::CircularBuffer(CircularBuffer<T> const& copyBuffer)
	: m_size(copyBuffer.m_size)
	, m_front(copyBuffer.m_front)
	, m_back(copyBuffer.m_back)
{
	m_data.resize(m_size);
	memcpy(m_data.data(), copyBuffer.m_data.data(), m_size);
}


template <class T>
CircularBuffer<T>::~CircularBuffer()
{
	m_data.clear();
}


template <class T>
T CircularBuffer<T>::Pop()
{
	if (IsEmpty())
	{
		throw std::runtime_error::runtime_error("Trying to pop out of an empty circular buffer");
		ERROR_AND_DIE("Trying to pop out of an empty circular buffer");
		return T();
	}

	T poppedElement = m_data[m_front];
	if (m_front == m_back)
	{
		m_front = -1;
		m_back = -1;
	}
	else
	{
		m_front++;
		m_front %= m_size;
	}

	return poppedElement;
}


template <class T>
void CircularBuffer<T>::Push(T const& toPush)
{
	if (m_front < 0 && m_back < 0) //very first element being pushed into buffer
	{
		m_front = 0;
		m_back = 0;
		m_data[m_back] = (toPush);
		return;
	}

	m_back++;
	m_back %= m_size;
	if (m_back == m_front)
	{
		m_front++;
		m_front %= m_size;
	}

	m_data[m_back] = toPush;
}


template <class T>
void CircularBuffer<T>::Clear()
{
	m_front = -1;
	m_back = -1;
}


template <class T>
void CircularBuffer<T>::Resize(int newBufferSize)
{
	m_data.resize(newBufferSize);
	m_size = newBufferSize;
	Clear();
}


template <class T>
T CircularBuffer<T>::GetBackElement() const
{
	if (IsEmpty())
	{
		//throw std::runtime_error::runtime_error("Trying to access singleton instance that has not been created");
		ERROR_AND_DIE(Stringf("Trying to access back element of an empty circular buffer"));
	}

	return m_data[m_back];
}


template <class T>
T CircularBuffer<T>::GetFrontElement() const
{
	if (IsEmpty())
	{
		ERROR_AND_DIE(Stringf("Trying to access front element of an empty circular buffer"));
	}

	return m_data[m_front];
}


template <class T>
int CircularBuffer<T>::GetBackIndex() const
{
	return m_back;
}


template <class T>
int CircularBuffer<T>::GetFrontIndex() const
{
	return m_front;
}


template <class T>
T CircularBuffer<T>::GetElementAtIndex(int index) const
{
	if (IsEmpty())
	{
		ERROR_AND_DIE(Stringf("Trying to access element %i of an empty circular buffer", index));
	}

	index %= m_size;
	return m_data[index];
}


template <class T>
bool CircularBuffer<T>::IsEmpty() const
{
	return m_back == -1 && m_front == -1;
}


template <class T>
int CircularBuffer<T>::GetSize() const
{
	if (m_back >= m_front)
	{
		return m_back - m_front;
	}

	return m_size - (m_front - m_back - 1);
}

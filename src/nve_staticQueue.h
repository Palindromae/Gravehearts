#pragma once
#include "trace.h"
template <typename T>

class nve_staticQueue
{
private:
	T* data;
	long long head = 0;
	long long tail = 0;

	void circulate(long long& i) {
		if (i >= end)
			i = 0;
	}
	long long end = 0;

public:
	long long size = 0;
	nve_staticQueue(long long length);

	bool Push(T item);
	bool Peek(T& item);

	bool Pull(T& item);

	bool LookAt(long long index, T& item);
	
	void RemoveIndex(long long index);

	int GetHead();
	int currentLength();
};

template<typename T>
nve_staticQueue<T>::nve_staticQueue(long long length) {
	//+1 to account for the fact that if the head is just before the tail it cannot append
	end = length + 1;
	size = length;
	data = new T[length];
}

template<typename T>
bool nve_staticQueue<T>::Push(T item) {
	if (head + 1 == tail)
	{
		trace_error("failed to add item onto queue");
		return false;
	}
	data[head] = item;
	head++;
	circulate(head);
	return true;
}

template<typename T>
bool nve_staticQueue<T>::Pull(T& item) {
	if (tail == head)
		return false;

	item = data[tail];
	tail++;
	circulate(tail);
	return true;
}

/// <summary>
/// This removes the index and swaps in the head. Do no use if order matters
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="index"></param>
template<typename T>
void nve_staticQueue<T>::RemoveIndex(long long index)
{
	(data[head], data[index]) = (data[index], data[head]);
	data[head] = nullptr; 
	head--;
}


template<typename T>
bool nve_staticQueue<T>::Peek(T& item) {
	if (tail == head)
		return false;

	item = data[tail];

	return true;
}

template<typename T>
bool nve_staticQueue<T>::LookAt(long long index, T& item) {
	if (data[index] == nullptr)
		return false;

	item = data[index];

	return true;
}

template<typename T>
int nve_staticQueue<T>::GetHead() {
	return head;
}

template<typename T>
int nve_staticQueue<T>::currentLength() {
	return head-tail;
}
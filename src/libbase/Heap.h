#ifndef LIBBASE_HEAP_H
#define LIBBASE_HEAP_H

#include <libbase/Noncopyable.h>
#include <libbase/Debug.h>

#include <vector>
#include <functional>
#include <algorithm>
#include <iterator>

namespace libbase
{

template <class T>
bool defaultComparator(const T& lhs, const T& rhs);

template<class T>
class Heap: private Noncopyable
{
public:
	typedef std::function<bool(const T&, const T&)> Comparator;	
	explicit Heap(bool flag = true, int arySize = 2, const Comparator& cmpIn = defaultComparator<T>) :
	isMinHeap(flag), aryNum(arySize), sz(0), cmp(cmpIn)
	{
		CHECK(arySize >= 2);
		elemArray.resize(INITCAP, T());
	} 

	~Heap() {}
	void insert(T elem);
	T topAndPop();
	T top();

	bool isEmpty() { return (sz == 0); }
	size_t currentSize() { return sz; }

private:
	bool compare(const T&, const T&);
	void swim(int );
	void sink(int );

	bool isMinHeap;
	int aryNum;
	int sz;
	std::vector<T> elemArray;
	Comparator cmp;
	const int INITCAP = 10;
};

template <class T>
inline bool defaultComparator(const T& lhs, const T& rhs)
{
	return (lhs < rhs);
}

template <class T>
inline bool Heap<T>::compare(const T& lhs, const T& rhs)
{
	bool ret = cmp(lhs, rhs);
	return (isMinHeap ? ret : !ret);	
}

template <class T>
void Heap<T>::swim(int loc)
{
	int parentNode = (loc - 1)/aryNum;
	int childNode = loc;
	T tmp = elemArray[childNode];
	while(parentNode >= 0 && childNode > 0 && compare(tmp, elemArray[parentNode]))
	{
		elemArray[childNode] = elemArray[parentNode];
		childNode = parentNode;
		parentNode = (childNode - 1)/aryNum;
	}
	elemArray[childNode] = tmp;
}

template <class T>
void Heap<T>::sink(int loc)
{
	int parentNode = loc;
	int childNode = loc*aryNum + 1;
	T tmp = elemArray[parentNode];
	while(childNode < sz)
	{
		int rightMost = (childNode + aryNum) < sz ? (childNode + aryNum) : sz;
		for(int num = childNode + 1; num < rightMost; ++num)
			if (!compare(elemArray[childNode], elemArray[num]))
				childNode = num;
		if (compare(tmp, elemArray[childNode])) 
			break;
		elemArray[parentNode] = elemArray[childNode];
		parentNode = childNode;
		childNode = parentNode*aryNum + 1;
	}
	elemArray[parentNode] = tmp;
}

template <class T>
void Heap<T>::insert(T elem)
{
	if (static_cast<unsigned int>(sz) == elemArray.size()) 
		elemArray.resize(2*sz, T());
	elemArray[sz++] = elem;
	swim(sz - 1);
}

template <class T>
T Heap<T>::topAndPop()
{
	CHECK(sz > 0);
	T ret = elemArray[0];
	elemArray[0] = elemArray[--sz];
	sink(0);
	return ret;
}

template <class T>
T Heap<T>::top()
{
	CHECK(sz > 0);
	return elemArray[0];
}
}

#endif

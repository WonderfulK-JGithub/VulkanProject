#pragma once
#include <algorithm>
#include <assert.h>

template<typename T, typename IndexType = int>
class PrimitiveVector
{
public:
	PrimitiveVector(const PrimitiveVector& aOther);
	PrimitiveVector(PrimitiveVector&& aOther) noexcept;
	PrimitiveVector& operator=(const PrimitiveVector& aOther);
	PrimitiveVector& operator=(PrimitiveVector&& aOther) noexcept;

	PrimitiveVector()  = default;
	~PrimitiveVector();

	void push_back(const T& aItem);
	void insert(IndexType aIndex, const T& aItem);

	template<typename... Args>
	void emplace_back(Args&&... aArgs);

	template<typename... Args>
	void emplace_at(IndexType aIndex, Args&&... aArgs);

	void pop_back();
	void erase(IndexType aIndex);
	void erase_n(IndexType aIndexStart, IndexType aCount);
	void swap_and_pop_back(IndexType aIndex);

	void clear();

	void resize(IndexType aNewSize);
	void resize(IndexType aNewSize, const T& aItem);

	void reserve(IndexType aIncrease);

	T& operator[](IndexType aIndex);
	const T& operator[](IndexType aIndex) const;

	T& front();
	const T& front() const;

	T& back();
	const T& back() const;

	T* data();
	const T* data() const;

	IndexType size() const;
	IndexType capacity() const;
private:
	static constexpr IndexType typeSize = sizeof(T);

	T* myData            = nullptr;
	IndexType mySize     = 0;
	IndexType myCapacity = 0;
};

template<typename T, typename IndexType>
PrimitiveVector<T, IndexType>::PrimitiveVector(const PrimitiveVector& aOther)
{
	mySize     = aOther.mySize;
	myCapacity = aOther.myCapacity;

	if (aOther.myCapacity > 0)
	{
		myData = reinterpret_cast<T*>(new char[myCapacity * sizeof(T)]);
		std::copy_n(aOther.myData, mySize, myData);
	}
}

template<typename T, typename IndexType>
PrimitiveVector<T, IndexType>& PrimitiveVector<T, IndexType>::operator=(const PrimitiveVector& aOther)
{
	delete[] myData;

	mySize     = aOther.mySize;
	myCapacity = aOther.myCapacity;

	myData = reinterpret_cast<T*>(new char[myCapacity * sizeof(T)]);
	std::copy_n(aOther.myData, mySize, myData);

	return *this;
}

template<typename T, typename IndexType>
PrimitiveVector<T, IndexType>::PrimitiveVector(PrimitiveVector&& aOther) noexcept
{
	myData     = aOther.myData;
	mySize     = aOther.mySize;
	myCapacity = aOther.myCapacity;

	aOther.myData     = nullptr;
	aOther.mySize     = 0;
	aOther.myCapacity = 0;
}

template<typename T, typename IndexType>
PrimitiveVector<T, IndexType>& PrimitiveVector<T, IndexType>::operator=(PrimitiveVector&& aOther) noexcept
{
	delete[] myData;

	myData     = aOther.myData;
	mySize     = aOther.mySize;
	myCapacity = aOther.myCapacity;

	aOther.myData     = nullptr;
	aOther.mySize     = 0;
	aOther.myCapacity = 0;

	return *this;
}

template<typename T, typename IndexType>
PrimitiveVector<T, IndexType>::~PrimitiveVector()
{
	//::operator delete[](myData, myCapacity);
	delete[] myData;
}

template<typename T, typename IndexType>
void PrimitiveVector<T, IndexType>::push_back(const T& aItem)
{
	if (mySize >= myCapacity)
	{
		reserve(myCapacity == 0 ? 1 : myCapacity * 2);
	}

	new(myData + mySize) T(aItem);
	++mySize;
}

template<typename T, typename IndexType>
void PrimitiveVector<T, IndexType>::insert(const IndexType aIndex, const T& aItem)
{
	assert(aIndex <= mySize && aIndex >= 0 && "Can't insert because index is out of range");
	if (mySize > myCapacity)
	{
		reserve(myCapacity == 0 ? 1 : myCapacity * 2);
	}
	++mySize;

	IndexType itemsAbove = mySize - aIndex;

	for (int i = itemsAbove - 1; i >= 0; --i)
	{
		myData[aIndex + i + 1] = myData[aIndex + i];
	}

	new(myData + aIndex) T(aItem);
	
}

template<typename T, typename IndexType>
template<typename... Args>
void PrimitiveVector<T, IndexType>::emplace_back(Args&&... aArgs)
{
	
	if (mySize > myCapacity)
	{
		reserve(myCapacity == 0 ? 1 : myCapacity * 2);
	}

	new(myData + mySize) T(std::forward<Args>(aArgs)...);
	++mySize;
}

template<typename T, typename IndexType>
template<typename... Args>
void PrimitiveVector<T, IndexType>::emplace_at(IndexType aIndex, Args&&... aArgs)
{
	assert(aIndex <= mySize && aIndex >= 0 && "Can't insert because index is out of range");
	if (mySize > myCapacity)
	{
		reserve(myCapacity == 0 ? 1 : myCapacity * 2);
	}
	++mySize;

	IndexType itemsAbove = mySize - aIndex;

	for (int i = itemsAbove - 1; i >= 0; --i)
	{
		myData[aIndex + i + 1] = myData[aIndex + i];
	}

	new(myData + aIndex) T(std::forward<Args>(aArgs)...);
}

template<typename T, typename IndexType>
void PrimitiveVector<T, IndexType>::pop_back()
{
	--mySize;
	myData[mySize].~T();
}

template<typename T, typename IndexType>
void PrimitiveVector<T, IndexType>::erase(IndexType aIndex)
{
	assert(aIndex < mySize && aIndex >= 0 && "Erase not possible because index is out of range");
	--mySize;
	myData[aIndex].~T();

	IndexType itemsAbove = mySize - aIndex;
	if (itemsAbove > 0)
	{
		for (int i = 0; i < itemsAbove; ++i)
		{
			myData[aIndex + i] = myData[aIndex + i + 1];
		}

		myData[mySize].~T();
	}
}

template<typename T, typename IndexType>
void PrimitiveVector<T, IndexType>::erase_n(IndexType aIndexStart, IndexType aCount)
{
	if (aCount <= 0)
	{
		return;
	}

	assert(aIndexStart < mySize && aIndexStart >= 0 && "Erase not possible because index is out of range");
	IndexType endIndex = aIndexStart + aCount - 1;
	assert(endIndex < mySize && "Erase not possible because index is out of range");

	for (IndexType i = aIndexStart; i <= endIndex; ++i)
	{
		myData[i].~T();
	}

	IndexType itemsAbove = mySize - endIndex - 1;
	if (itemsAbove > 0)
	{
		for (int i = 0; i < itemsAbove; ++i)
		{
			myData[aIndexStart + i] = myData[endIndex + i + 1];
		}

		myData[mySize].~T();
	}

	mySize -= aCount;
}

template<typename T, typename IndexType>
void PrimitiveVector<T, IndexType>::swap_and_pop_back(const IndexType aIndex)
{
	assert(mySize > IndexType(0) && "Can't swap and pop when empty");
	--mySize;

	std::swap(myData[aIndex], myData[mySize]);
	myData[mySize].~T();
}

template<typename T, typename IndexType>
void PrimitiveVector<T, IndexType>::clear()
{
	for (int i = 0; i < mySize; ++i)
	{
		myData[i].~T();
	}

	mySize = 0;
}

template<typename T, typename IndexType>
void PrimitiveVector<T, IndexType>::resize(IndexType aNewSize)
{
	if (aNewSize > myCapacity)
	{
		reserve(aNewSize - myCapacity);
	}

	if (aNewSize > mySize)
	{
		const IndexType increase = aNewSize - mySize;
		for (int i = 0; i < increase; ++i)
		{
			new(myData + mySize + i) T();
		}
	}
	else
	{
		const IndexType loss = mySize - aNewSize;
		for (int i = 0; i < loss; ++i)
		{
			(myData + mySize - i - 1)->~T();
		}
	}

	mySize = aNewSize;
}

template<typename T, typename IndexType>
void PrimitiveVector<T, IndexType>::resize(IndexType aNewSize, const T& aItem)
{
	if (aNewSize > myCapacity)
	{
		reserve(aNewSize - myCapacity);
	}

	if (aNewSize > mySize)
	{
		const IndexType increase = aNewSize - mySize;
		for (int i = 0; i < increase; ++i)
		{
			new(myData + mySize + i) T(aItem);
		}
	}
	else
	{
		const IndexType loss = mySize - aNewSize;
		for (int i = 0; i < loss; ++i)
		{
			(myData + mySize - i - 1)->~T();
		}
	}

	mySize = aNewSize;
}

template<typename T, typename IndexType>
void PrimitiveVector<T, IndexType>::reserve(IndexType aIncrease)
{
	const IndexType newCapacity = myCapacity + aIncrease;

	T* newData = new T[newCapacity];
	//T* newData = static_cast<T*>(::operator new[](newCapacity * sizeof(T), static_cast<std::align_val_t>(alignof(T))));

	if (myCapacity > 0)
	{
		std::copy_n(myData, mySize, newData);

		delete[] myData;
	}
	
	myData     = newData;
	myCapacity = newCapacity;
}

template<typename T, typename IndexType>
T& PrimitiveVector<T, IndexType>::operator[](const IndexType aIndex)
{
	return myData[aIndex];
}

template<typename T, typename IndexType>
const T& PrimitiveVector<T, IndexType>::operator[](const IndexType aIndex) const
{
	return myData[aIndex];
}

template<typename T, typename IndexType>
T& PrimitiveVector<T, IndexType>::front()
{
	assert(mySize > 0 && "Vector is empty and has no front");
	return myData[0];
}

template<typename T, typename IndexType>
const T& PrimitiveVector<T, IndexType>::front() const
{
	assert(mySize > 0 && "Vector is empty and has no front");
	return myData[0];
}

template<typename T, typename IndexType>
T& PrimitiveVector<T, IndexType>::back()
{
	assert(mySize > 0 && "Vector is empty and has no back");
	return myData[mySize - IndexType(1)];
}

template<typename T, typename IndexType>
const T& PrimitiveVector<T, IndexType>::back() const
{
	assert(mySize > 0 && "Vector is empty and has no back");
	return myData[mySize - IndexType(1)];
}

template<typename T, typename IndexType>
T* PrimitiveVector<T, IndexType>::data()
{
	return myData;
}

template<typename T, typename IndexType>
const T* PrimitiveVector<T, IndexType>::data() const
{
	return myData;
}

template<typename T, typename IndexType>
IndexType PrimitiveVector<T, IndexType>::size() const
{
	return mySize;
}

template<typename T, typename IndexType>
IndexType PrimitiveVector<T, IndexType>::capacity() const
{
	return myCapacity;
}

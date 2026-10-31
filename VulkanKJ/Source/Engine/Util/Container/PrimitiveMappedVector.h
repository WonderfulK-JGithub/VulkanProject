#pragma once
#include <assert.h>
#include <unordered_map>

namespace Wonderful
{
	// Same as MappedVector, but does not store key values in a vector. Use if you don't want to access keys and don't use erase often
	template<typename KeyType, typename ValueType, typename IndexType = size_t>
	class PrimitiveMappedVector
	{
	public:
		ValueType& at(const KeyType& aKey);
		const ValueType& at(const KeyType& aKey) const;

		ValueType& operator[](const KeyType& aKey);
		const ValueType& operator[](const KeyType& aKey) const;

		ValueType* data();
		const ValueType* data() const;

		bool insert(const KeyType& aKey, const ValueType& aValue);
		void insert_or_assign(const KeyType& aKey, const ValueType& aValue);
		template<typename... Args>
		void emplace(const KeyType& aKey, Args&&... aArgs);
		template<typename... Args>
		bool try_emplace(const KeyType& aKey, Args&&... aArgs);

		// um, this function is slow :skull:, if you use this often use MappedVector instead
		void erase(const KeyType& aKey);
		void clear();
		void reserve(size_t aCapacity);

		[[nodiscard]] size_t size() const;
		[[nodiscard]] bool empty() const;
		[[nodiscard]] bool contains(const KeyType& aKey) const;

	private:
		std::unordered_map<KeyType, IndexType> myKeyToIndex;
		std::vector<ValueType> myValues;
	};

	template<typename KeyType, typename ValueType, typename IndexType>
	ValueType& PrimitiveMappedVector<KeyType, ValueType, IndexType>::at(const KeyType& aKey)
	{
		assert(myKeyToIndex.contains(aKey) && "Map read with non existing key");
		return myValues[myKeyToIndex.at(aKey)];
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	const ValueType& PrimitiveMappedVector<KeyType, ValueType, IndexType>::at(const KeyType& aKey) const
	{
		assert(myKeyToIndex.contains(aKey) && "Map read with non existing key");
		return myValues[myKeyToIndex.at(aKey)];
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	ValueType& PrimitiveMappedVector<KeyType, ValueType, IndexType>::operator[](const KeyType& aKey)
	{
		assert(myKeyToIndex.contains(aKey) && "Map read with non existing key");
		return myValues[myKeyToIndex.at(aKey)];
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	const ValueType& PrimitiveMappedVector<KeyType, ValueType, IndexType>::operator[](const KeyType& aKey) const
	{
		assert(myKeyToIndex.contains(aKey) && "Map read with non existing key");
		return myValues[myKeyToIndex.at(aKey)];
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	ValueType* PrimitiveMappedVector<KeyType, ValueType, IndexType>::data()
	{
		return myValues.data();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	const ValueType* PrimitiveMappedVector<KeyType, ValueType, IndexType>::data() const
	{
		return myValues.data();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	bool PrimitiveMappedVector<KeyType, ValueType, IndexType>::insert(const KeyType& aKey, const ValueType& aValue)
	{
		auto result = myKeyToIndex.insert({aKey, 0});

		// success
		if (result.second)
		{
			IndexType index       = myValues.size();
			myKeyToIndex.at(aKey) = index;
			myValues.push_back(aValue);
		}

		return result.second;
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	void PrimitiveMappedVector<KeyType, ValueType, IndexType>::insert_or_assign(const KeyType& aKey, const ValueType& aValue)
	{
		auto result = myKeyToIndex.insert({aKey, 0});

		if (result.second)
		{
			IndexType index       = myValues.size();
			myKeyToIndex.at(aKey) = index;
			myValues.push_back(aValue);
		}
		else
		{
			myValues[result.first->second] = aValue;
		}
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	template<typename... Args>
	void PrimitiveMappedVector<KeyType, ValueType, IndexType>::emplace(const KeyType& aKey, Args&&... aArgs)
	{
		auto result = myKeyToIndex.insert({aKey, 0});

		// success
		if (result.second)
		{
			IndexType index       = myValues.size();
			myKeyToIndex.at(aKey) = index;
			myValues.emplace_back(std::forward<Args>(aArgs));
		}
		else
		{
			myValues[result.first->second] = {std::forward<Args>(aArgs)...};
		}
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	template<typename... Args>
	bool PrimitiveMappedVector<KeyType, ValueType, IndexType>::try_emplace(const KeyType& aKey, Args&&... aArgs)
	{
		auto result = myKeyToIndex.insert({aKey, 0});

		// success
		if (result.second)
		{
			IndexType index       = myValues.size();
			myKeyToIndex.at(aKey) = index;
			myValues.emplace_back(std::forward<Args>(aArgs));
		}

		return result.second;
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	void PrimitiveMappedVector<KeyType, ValueType, IndexType>::erase(const KeyType& aKey)
	{
		assert(myKeyToIndex.contains(aKey) && "Map erased with non existing key");

		IndexType indexOfRemoved = myKeyToIndex.at(aKey);
		myKeyToIndex.erase(aKey);

		// this is why it's slow :skull:
		for (auto& element : myKeyToIndex)
		{
			if (element.second == myValues.size() - 1)
			{
				element.second = indexOfRemoved;
				break;
			}
		}

		std::swap(myValues.back(), myValues.at(indexOfRemoved));

		myValues.pop_back();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	void PrimitiveMappedVector<KeyType, ValueType, IndexType>::clear()
	{
		myKeyToIndex.clear();
		myValues.clear();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	void PrimitiveMappedVector<KeyType, ValueType, IndexType>::reserve(size_t aCapacity)
	{
		myKeyToIndex.reserve(aCapacity);
		myValues.reserve(aCapacity);
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	size_t PrimitiveMappedVector<KeyType, ValueType, IndexType>::size() const
	{
		return myValues.size();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	bool PrimitiveMappedVector<KeyType, ValueType, IndexType>::empty() const
	{
		return myValues.empty();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	bool PrimitiveMappedVector<KeyType, ValueType, IndexType>::contains(const KeyType& aKey) const
	{
		return myKeyToIndex.contains(aKey);
	}
}

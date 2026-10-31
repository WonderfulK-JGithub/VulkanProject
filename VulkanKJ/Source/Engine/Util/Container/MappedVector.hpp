#pragma once
#include <assert.h>
#include <unordered_map>
#include <vector>

// ReSharper disable CppInconsistentNaming

namespace Goose
{
	// Map that stores keys and values in a vector. Uses an unordered map to access values O(1)
	template<typename KeyType, typename ValueType, typename IndexType = size_t>
	class MappedVector
	{
	public:
		ValueType& at(const KeyType& aKey);
		const ValueType& at(const KeyType& aKey) const;

		// Note: this does not create a new element if it doesn't exist, unlike std:s maps
		ValueType& operator[](const KeyType& aKey);
		const ValueType& operator[](const KeyType& aKey) const;

		ValueType* data();
		const ValueType* data() const;

		KeyType* dataKeys();
		const KeyType* dataKeys() const;

		const std::unordered_map<KeyType, IndexType>& indexMap() const;

		bool insert(const KeyType& aKey, const ValueType& aValue);
		void insert_or_assign(const KeyType& aKey, const ValueType& aValue);
		template<typename... Args>
		void emplace(const KeyType& aKey, Args&&... aArgs);
		template<typename... Args>
		bool try_emplace(const KeyType& aKey, Args&&... aArgs);

		void erase(const KeyType& aKey);
		void erase_at_index(IndexType aIndex);
		void clear();
		void reserve(size_t aCapacity);

		void internal_swap(IndexType aA, IndexType aB);

		[[nodiscard]] size_t size() const;
		[[nodiscard]] bool empty() const;
		[[nodiscard]] bool contains(const KeyType& aKey) const;

		ValueType* find(const KeyType& aKey);
		const ValueType* find(const KeyType& aKey) const;
		
	private:
		std::unordered_map<KeyType, IndexType> myKeyToIndex;
		std::vector<KeyType> myKeys;
		std::vector<ValueType> myValues;
	};

	template<typename KeyType, typename ValueType, typename IndexType>
	ValueType& MappedVector<KeyType, ValueType, IndexType>::at(const KeyType& aKey)
	{
		assert(myKeyToIndex.contains(aKey) && "Map read with non existing key");
		return myValues[myKeyToIndex.at(aKey)];
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	const ValueType& MappedVector<KeyType, ValueType, IndexType>::at(const KeyType& aKey) const
	{
		assert(myKeyToIndex.contains(aKey) && "Map read with non existing key");
		return myValues[myKeyToIndex.at(aKey)];
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	ValueType& MappedVector<KeyType, ValueType, IndexType>::operator[](const KeyType& aKey)
	{
		assert(myKeyToIndex.contains(aKey) && "Map read with non existing key");
		return myValues[myKeyToIndex.at(aKey)];
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	const ValueType& MappedVector<KeyType, ValueType, IndexType>::operator[](const KeyType& aKey) const
	{
		assert(myKeyToIndex.contains(aKey) && "Map read with non existing key");
		return myValues[myKeyToIndex.at(aKey)];
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	ValueType* MappedVector<KeyType, ValueType, IndexType>::data()
	{
		return myValues.data();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	const ValueType* MappedVector<KeyType, ValueType, IndexType>::data() const
	{
		return myValues.data();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	KeyType* MappedVector<KeyType, ValueType, IndexType>::dataKeys()
	{
		return myKeys.data();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	const KeyType* MappedVector<KeyType, ValueType, IndexType>::dataKeys() const
	{
		return myKeys.data();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	const std::unordered_map<KeyType, IndexType>& MappedVector<KeyType, ValueType, IndexType>::indexMap() const
	{
		return myKeyToIndex;
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	bool MappedVector<KeyType, ValueType, IndexType>::insert(const KeyType& aKey, const ValueType& aValue)
	{
		auto result = myKeyToIndex.insert({aKey, 0});

		// success
		if (result.second)
		{
			IndexType index       = static_cast<IndexType>(myValues.size());
			myKeyToIndex.at(aKey) = index;
			myKeys.push_back(aKey);
			myValues.push_back(aValue);
		}

		return result.second;
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	void MappedVector<KeyType, ValueType, IndexType>::insert_or_assign(const KeyType& aKey, const ValueType& aValue)
	{
		auto result = myKeyToIndex.insert({aKey, 0});

		if (result.second)
		{
			IndexType index       = static_cast<IndexType>(myValues.size());
			myKeyToIndex.at(aKey) = index;
			myKeys.push_back(aKey);
			myValues.push_back(aValue);
		}
		else
		{
			myValues[result.first->second] = aValue;
		}
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	template<typename... Args>
	bool MappedVector<KeyType, ValueType, IndexType>::try_emplace(const KeyType& aKey, Args&&... aArgs)
	{
		auto result = myKeyToIndex.insert({aKey, 0});

		// success
		if (result.second)
		{
			IndexType index       = static_cast<IndexType>(myValues.size());
			myKeyToIndex.at(aKey) = index;
			myKeys.push_back(aKey);
			myValues.emplace_back(std::forward<Args>(aArgs)...);
		}

		return result.second;
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	void MappedVector<KeyType, ValueType, IndexType>::erase(const KeyType& aKey)
	{
		assert(myKeyToIndex.contains(aKey) && "Map erased with non existing key");

		IndexType indexOfRemoved = myKeyToIndex.at(aKey);
		erase_at_index(indexOfRemoved);
		/*myKeyToIndex.at(myKeys.back()) = indexOfRemoved;
		myKeyToIndex.erase(aKey);

		std::swap(myValues.back(), myValues.at(indexOfRemoved));
		std::swap(myKeys.back(), myKeys.at(indexOfRemoved));

		myValues.pop_back();
		myKeys.pop_back();*/
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	void MappedVector<KeyType, ValueType, IndexType>::erase_at_index(IndexType aIndex)
	{
		myKeyToIndex.at(myKeys.back()) = aIndex;
		myKeyToIndex.erase(myKeys[aIndex]);

		std::swap(myValues.back(), myValues.at(aIndex));
		std::swap(myKeys.back(), myKeys.at(aIndex));

		myValues.pop_back();
		myKeys.pop_back();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	template<typename... Args>
	void MappedVector<KeyType, ValueType, IndexType>::emplace(const KeyType& aKey, Args&&... aArgs)
	{
		auto result = myKeyToIndex.insert({aKey,IndexType(0)});

		// success
		if (result.second)
		{
			IndexType index       = static_cast<IndexType>(myValues.size());
			myKeyToIndex.at(aKey) = index;
			myKeys.push_back(aKey);
			myValues.emplace_back(std::forward<Args>(aArgs)...);
		}
		else
		{
			myValues[result.first->second] = {std::forward<Args>(aArgs)...};
		}
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	void MappedVector<KeyType, ValueType, IndexType>::clear()
	{
		myKeyToIndex.clear();
		myKeys.clear();
		myValues.clear();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	void MappedVector<KeyType, ValueType, IndexType>::reserve(size_t aCapacity)
	{
		myKeyToIndex.reserve(aCapacity);
		myKeys.reserve(aCapacity);
		myValues.reserve(aCapacity);
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	void MappedVector<KeyType, ValueType, IndexType>::internal_swap(IndexType aA, IndexType aB)
	{
		std::swap(myKeyToIndex[myKeys[aA]], myKeyToIndex[myKeys[aB]]);
		std::swap(myKeys[aA], myKeys[aB]);
		std::swap(myValues[aA], myValues[aB]);
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	size_t MappedVector<KeyType, ValueType, IndexType>::size() const
	{
		return myValues.size();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	bool MappedVector<KeyType, ValueType, IndexType>::empty() const
	{
		return myValues.empty();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	bool MappedVector<KeyType, ValueType, IndexType>::contains(const KeyType& aKey) const
	{
		return myKeyToIndex.contains(aKey);
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	ValueType* MappedVector<KeyType, ValueType, IndexType>::find(const KeyType& aKey)
	{
		return myKeyToIndex.contains(aKey) ? &at(aKey) : nullptr;
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	const ValueType* MappedVector<KeyType, ValueType, IndexType>::find(const KeyType& aKey) const
	{
		return myKeyToIndex.contains(aKey) ? &at(aKey) : nullptr;
	}
}


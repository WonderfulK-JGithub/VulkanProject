#pragma once
#include <unordered_map>

#include "Util/Logging/Logging.h"

namespace Goose
{
	// Unordered set & vector
	template<typename KeyType, typename IndexType = size_t>
	class VectorSet
	{
	public:
		KeyType& at(const IndexType aIndex);
		const KeyType& at(const IndexType aIndex) const;

		// Note: this does not create a new element if it doesn't exist, unlike std:s maps
		KeyType& operator[](const IndexType aIndex);
		const KeyType& operator[](const IndexType aIndex) const;

		KeyType* data();
		const KeyType* data() const;

		bool insert(const KeyType& aValue);

		void erase(const KeyType& aValue);
		void erase_at_index(IndexType aIndex);
		bool try_erase(const KeyType& aValue);
		void clear();
		void reserve(size_t aCapacity);
		
		void push_back(const KeyType& aValue);

		[[nodiscard]] size_t size() const;
		[[nodiscard]] bool empty() const;
		[[nodiscard]] bool contains(const KeyType& aKey) const;

	private:
		std::unordered_map<KeyType, IndexType> myKeysToIndex;
		std::vector<KeyType> myKeys;
	};

	template<typename KeyType, typename IndexType>
	KeyType& VectorSet<KeyType, IndexType>::at(const IndexType aIndex)
	{
		return myKeys.at(aIndex);
	}

	template<typename KeyType, typename IndexType>
	const KeyType& VectorSet<KeyType, IndexType>::at(const IndexType aIndex) const
	{
		return myKeys.at(aIndex);
	}

	template<typename KeyType, typename IndexType>
	KeyType& VectorSet<KeyType, IndexType>::operator[](const IndexType aIndex)
	{
		return myKeys[aIndex];
	}

	template<typename KeyType, typename IndexType>
	const KeyType& VectorSet<KeyType, IndexType>::operator[](const IndexType aIndex) const
	{
		return myKeys[aIndex];
	}

	template<typename KeyType, typename IndexType>
	KeyType* VectorSet<KeyType, IndexType>::data()
	{
		return myKeys.data();
	}

	template<typename KeyType, typename IndexType>
	const KeyType* VectorSet<KeyType, IndexType>::data() const
	{
		return myKeys.data();
	}

	template<typename KeyType, typename IndexType>
	bool VectorSet<KeyType, IndexType>::insert(const KeyType& aValue)
	{
		auto result = myKeysToIndex.insert({aValue, 0});
		if (result.second)
		{
			result.first->second = myKeys.size();
			myKeys.push_back(aValue);
			return true;
		}

		return false;
	}

	template<typename KeyType, typename IndexType>
	void VectorSet<KeyType, IndexType>::erase(const KeyType& aValue)
	{
		IndexType vectorIndex = myKeysToIndex.at(aValue);
		erase_at_index(vectorIndex);
	}

	template<typename KeyType, typename IndexType>
	void VectorSet<KeyType, IndexType>::erase_at_index(IndexType aIndex)
	{
		IndexType backIndex = myKeys.size() - 1u;
		if (aIndex != backIndex)
		{
			myKeysToIndex.at(myKeys.back()) = backIndex;
			std::swap(myKeys[aIndex], myKeys[backIndex]);
		}
		myKeysToIndex.erase(myKeys[aIndex]);
		myKeys.pop_back();
	}

	template<typename KeyType, typename IndexType>
	bool VectorSet<KeyType, IndexType>::try_erase(const KeyType& aValue)
	{
		auto it = myKeysToIndex.find(aValue);
		
		if (it != myKeysToIndex.end())
		{
			erase_at_index(it->second);
			
			return true;
		}
		
		return false;
	}

	template<typename KeyType, typename IndexType>
	void VectorSet<KeyType, IndexType>::clear()
	{
		myKeys.clear();
		myKeysToIndex.clear();
	}

	template<typename KeyType, typename IndexType>
	void VectorSet<KeyType, IndexType>::reserve(size_t aCapacity)
	{
		myKeys.reserve(aCapacity);
		myKeysToIndex.reserve(aCapacity);
	}

	template<typename KeyType, typename IndexType>
	void VectorSet<KeyType, IndexType>::push_back(const KeyType& aValue)
	{
		if (myKeysToIndex.insert({aValue, myKeys.size()}).second == false)
		{
			LOG_ERROR("%s","Key was pushed back when it already exists");
			return;
		}
		
		myKeys.push_back(aValue);
	}

	template<typename KeyType, typename IndexType>
	size_t VectorSet<KeyType, IndexType>::size() const
	{
		return myKeys.size();
	}

	template<typename KeyType, typename IndexType>
	bool VectorSet<KeyType, IndexType>::empty() const
	{
		return myKeys.empty();
	}

	template<typename KeyType, typename IndexType>
	bool VectorSet<KeyType, IndexType>::contains(const KeyType& aKey) const
	{
		return myKeysToIndex.contains(aKey);
	}
}

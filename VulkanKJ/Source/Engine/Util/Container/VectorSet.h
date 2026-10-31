#pragma once
#include <unordered_map>

namespace Goose
{
	// Unordered set & vector
	template<typename ValueType, typename IndexType = size_t>
	class VectorSet
	{
	public:
		ValueType& at(const IndexType aIndex);
		const ValueType& at(const IndexType aIndex) const;

		// Note: this does not create a new element if it doesn't exist, unlike std:s maps
		ValueType& operator[](const IndexType aIndex);
		const ValueType& operator[](const IndexType aIndex) const;

		ValueType* data();
		const ValueType* data() const;

		bool insert(const ValueType& aValue);

		void erase(const ValueType& aValue);
		void erase_at_index(IndexType aIndex);
		void clear();
		void reserve(size_t aCapacity);

		[[nodiscard]] size_t size() const;
		[[nodiscard]] bool empty() const;
		[[nodiscard]] bool contains(const ValueType& aKey) const;

		ValueType* find(const ValueType& aKey);
		const ValueType* find(const ValueType& aKey) const;

	private:
		std::unordered_map<ValueType, IndexType> myValueToIndex;
		std::vector<ValueType> myValues;
	};

	template<typename ValueType, typename IndexType>
	ValueType& VectorSet<ValueType, IndexType>::at(const IndexType aIndex)
	{
		return myValues.at(aIndex);
	}

	template<typename ValueType, typename IndexType>
	const ValueType& VectorSet<ValueType, IndexType>::at(const IndexType aIndex) const
	{
		return myValues.at(aIndex);
	}

	template<typename ValueType, typename IndexType>
	ValueType& VectorSet<ValueType, IndexType>::operator[](const IndexType aIndex)
	{
		return myValues[aIndex];
	}

	template<typename ValueType, typename IndexType>
	const ValueType& VectorSet<ValueType, IndexType>::operator[](const IndexType aIndex) const
	{
		return myValues[aIndex];
	}

	template<typename ValueType, typename IndexType>
	ValueType* VectorSet<ValueType, IndexType>::data()
	{
		return myValues.data();
	}

	template<typename ValueType, typename IndexType>
	const ValueType* VectorSet<ValueType, IndexType>::data() const
	{
		return myValues.data();
	}

	template<typename ValueType, typename IndexType>
	bool VectorSet<ValueType, IndexType>::insert(const ValueType& aValue)
	{
		auto result = myValueToIndex.insert({aValue, 0});
		if (result.second)
		{
			result.first->second = myValues.size();
			myValues.push_back(aValue);
			return true;
		}

		return false;
	}

	template<typename ValueType, typename IndexType>
	void VectorSet<ValueType, IndexType>::erase(const ValueType& aValue)
	{
		IndexType vectorIndex = myValueToIndex.at(aValue);
		erase_at_index(vectorIndex);
		/*myValueToIndex.erase(aValue);

		IndexType backIndex = myValues.size() - 1u;
		if (vectorIndex != backIndex)
		{
			myValueToIndex.at(myValues.back()) = backIndex;
			std::swap(myValues[vectorIndex], myValues[backIndex]);
		}
		myValues.pop_back();*/
	}

	template<typename ValueType, typename IndexType>
	void VectorSet<ValueType, IndexType>::erase_at_index(IndexType aIndex)
	{
		IndexType backIndex = myValues.size() - 1u;
		if (aIndex != backIndex)
		{
			myValueToIndex.at(myValues.back()) = backIndex;
			std::swap(myValues[aIndex], myValues[backIndex]);
		}
		myValueToIndex.erase(myValues[aIndex]);
		myValues.pop_back();
	}

	template<typename ValueType, typename IndexType>
	void VectorSet<ValueType, IndexType>::clear()
	{
		myValues.clear();
		myValueToIndex.clear();
	}

	template<typename ValueType, typename IndexType>
	void VectorSet<ValueType, IndexType>::reserve(size_t aCapacity)
	{
		myValues.reserve(aCapacity);
		myValueToIndex.reserve(aCapacity);
	}

	template<typename ValueType, typename IndexType>
	size_t VectorSet<ValueType, IndexType>::size() const
	{
		return myValues.size();
	}

	template<typename ValueType, typename IndexType>
	bool VectorSet<ValueType, IndexType>::empty() const
	{
		return myValues.empty();
	}

	template<typename ValueType, typename IndexType>
	bool VectorSet<ValueType, IndexType>::contains(const ValueType& aKey) const
	{
		return myValueToIndex.contains(aKey);
	}

}

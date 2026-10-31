#pragma once
#include <assert.h>
#include <vector>

namespace Goose
{
	template<typename KeyType, typename ValueType, typename IndexType = size_t>
	class SimpleMap
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

		ValueType& back();
		const ValueType& back() const;

		bool insert(const KeyType& aKey, const ValueType& aValue);
		void insert_or_assign(const KeyType& aKey, const ValueType& aValue);
		void push_back(const KeyType& aKey, const ValueType& aValue);


		template<typename... Args>
		void emplace(const KeyType& aKey, Args&&... aArgs);
		template<typename... Args>
		bool try_emplace(const KeyType& aKey, Args&&... aArgs);
		template<typename... Args>
		void emplace_back(const KeyType& aKey, Args&&... aArgs);

		void erase(const KeyType& aKey);
		void erase_keep_order(const KeyType& aKey);
		void clear();
		void reserve(size_t aCapacity);
		// Don't use unless you know what you're doing!!!!!
		void resize(size_t aSize);

		[[nodiscard]] size_t size() const;
		[[nodiscard]] bool empty() const;
		[[nodiscard]] bool contains(const KeyType& aKey) const;

	private:
		std::vector<KeyType> myKeys;
		std::vector<ValueType> myValues;
	};

	template<typename KeyType, typename ValueType, typename IndexType>
	ValueType& SimpleMap<KeyType, ValueType, IndexType>::at(const KeyType& aKey)
	{
		for (IndexType i = 0; i < myKeys.size(); ++i)
		{
			if (aKey == myKeys[i])
			{
				return myValues[i];
			}
		}
		assert(false && "Key not found in Simple map");
		return myValues.front();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	const ValueType& SimpleMap<KeyType, ValueType, IndexType>::at(const KeyType& aKey) const
	{
		for (IndexType i = 0; i < myKeys.size(); ++i)
		{
			if (aKey == myKeys[i])
			{
				return myValues[i];
			}
		}
		assert(false && "Key not found in Simple map");
		return myValues.front();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	ValueType& SimpleMap<KeyType, ValueType, IndexType>::operator[](const KeyType& aKey)
	{
		for (IndexType i = 0; i < myKeys.size(); ++i)
		{
			if (aKey == myKeys[i])
			{
				return myValues[i];
			}
		}
		assert(false && "Key not found in Simple map");
		return myValues.front();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	const ValueType& SimpleMap<KeyType, ValueType, IndexType>::operator[](const KeyType& aKey) const
	{
		for (IndexType i = 0; i < myKeys.size(); ++i)
		{
			if (aKey == myKeys[i])
			{
				return myValues[i];
			}
		}
		assert(false && "Key not found in Simple map");
		return myValues.front();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	ValueType* SimpleMap<KeyType, ValueType, IndexType>::data()
	{
		return myValues.data();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	const ValueType* SimpleMap<KeyType, ValueType, IndexType>::data() const
	{
		return myValues.data();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	KeyType* SimpleMap<KeyType, ValueType, IndexType>::dataKeys()
	{
		return myKeys.data();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	const KeyType* SimpleMap<KeyType, ValueType, IndexType>::dataKeys() const
	{
		return myKeys.data();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	ValueType& SimpleMap<KeyType, ValueType, IndexType>::back()
	{
		return myValues.back();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	const ValueType& SimpleMap<KeyType, ValueType, IndexType>::back() const
	{
		return myValues.back();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	bool SimpleMap<KeyType, ValueType, IndexType>::insert(const KeyType& aKey, const ValueType& aValue)
	{
		for (IndexType i = 0; i < myKeys.size(); ++i)
		{
			if (aKey == myKeys[i])
			{
				return false;
			}
		}

		myKeys.push_back(aKey);
		myValues.push_back(aValue);

		return true;
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	void SimpleMap<KeyType, ValueType, IndexType>::insert_or_assign(const KeyType& aKey, const ValueType& aValue)
	{
		for (IndexType i = 0; i < myKeys.size(); ++i)
		{
			if (aKey == myKeys[i])
			{
				myValues[i] = aValue;
				return;
			}
		}

		myKeys.push_back(aKey);
		myValues.push_back(aValue);
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	void SimpleMap<KeyType, ValueType, IndexType>::push_back(const KeyType& aKey, const ValueType& aValue)
	{
		myKeys.push_back(aKey);
		myValues.push_back(aValue);
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	template<typename... Args>
	void SimpleMap<KeyType, ValueType, IndexType>::emplace_back(const KeyType& aKey, Args&&... aArgs)
	{
		myKeys.push_back(aKey);
		myValues.emplace_back(std::forward<Args>(aArgs)...);
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	template<typename... Args>
	void SimpleMap<KeyType, ValueType, IndexType>::emplace(const KeyType& aKey, Args&&... aArgs)
	{
		for (IndexType i = 0; i < myKeys.size(); ++i)
		{
			if (aKey == myKeys[i])
			{
				myValues[i] = {std::forward<Args>(aArgs)...};
				return;
			}
		}

		myKeys.push_back(aKey);
		myValues.emplace_back(std::forward<Args>(aArgs)...);
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	template<typename... Args>
	bool SimpleMap<KeyType, ValueType, IndexType>::try_emplace(const KeyType& aKey, Args&&... aArgs)
	{
		for (IndexType i = 0; i < myKeys.size(); ++i)
		{
			if (aKey == myKeys[i])
			{
				return false;
			}
		}

		myKeys.push_back(aKey);
		myValues.emplace_back(std::forward<Args>(aArgs)...);

		return true;
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	void SimpleMap<KeyType, ValueType, IndexType>::erase(const KeyType& aKey)
	{
		for (IndexType i = 0; i < myKeys.size(); ++i)
		{
			if (aKey == myKeys[i])
			{
				IndexType lastIndex = static_cast<IndexType>(myKeys.size()) - IndexType(1);
				if (i != lastIndex)
				{
					std::swap(myKeys[i], myKeys[lastIndex]);
					std::swap(myValues[i], myValues[lastIndex]);
				}

				myKeys.pop_back();
				myValues.pop_back();
				break;
			}
		}
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	void SimpleMap<KeyType, ValueType, IndexType>::erase_keep_order(const KeyType& aKey)
	{
		for (IndexType i = 0; i < myKeys.size(); ++i)
		{
			if (aKey == myKeys[i])
			{
				myKeys.erase(myKeys.begin() + i);
				myValues.erase(myValues.begin() + i);
			}
		}
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	void SimpleMap<KeyType, ValueType, IndexType>::clear()
	{
		myKeys.clear();
		myValues.clear();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	void SimpleMap<KeyType, ValueType, IndexType>::reserve(size_t aCapacity)
	{
		myKeys.reserve(aCapacity);
		myValues.reserve(aCapacity);
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	void SimpleMap<KeyType, ValueType, IndexType>::resize(size_t aSize)
	{
		myKeys.resize(aSize);
		myValues.resize(aSize);
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	size_t SimpleMap<KeyType, ValueType, IndexType>::size() const
	{
		return myKeys.size();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	bool SimpleMap<KeyType, ValueType, IndexType>::empty() const
	{
		return myKeys.empty();
	}

	template<typename KeyType, typename ValueType, typename IndexType>
	bool SimpleMap<KeyType, ValueType, IndexType>::contains(const KeyType& aKey) const
	{
		for (auto& key : myKeys)
		{
			if (key == aKey)
			{
				return true;
			}
		}

		return false;
	}
}

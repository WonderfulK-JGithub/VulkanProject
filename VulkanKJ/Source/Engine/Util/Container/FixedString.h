#pragma once
#include <assert.h>
#include <cstdint>
#include <string>

#include "Util/Logging/Logging.h"

namespace Goose
{
	template<uint8_t CAPACITY>
	class FixedString
	{
	public:
		FixedString()  = default;
		~FixedString() = default;

		FixedString(const char* aText);
		FixedString(const std::string& aText);

		FixedString& operator=(const char* aText);
		FixedString& operator=(const std::string& aText);

		void safe_assign(const char* aText);
		void safe_assign(const std::string& aText);

		char* data();
		const char* c_str() const;
		char& operator[](uint8_t aIndex);
		const char& operator[](uint8_t aIndex) const;

		uint8_t size() const;
		uint8_t capacity() const;
		bool empty() const;
		bool contains(char aCharacter) const;
		bool contains(const std::string_view& aText) const;
		bool has_only_letters_or_numbers_or_chosen(const std::string_view& aChosenCharacters) const;

		bool operator==(const FixedString& aOther) const;
		bool operator==(const char* aText) const;
		bool operator==(const std::string_view& aText) const;

		void resize(uint8_t aNewSize);
		void clear();

		FixedString& operator+=(const FixedString& aOther);
		FixedString& operator+=(const char* aText);
		FixedString& operator+=(const std::string& aText);
		FixedString& operator+=(const std::string_view& aText);

		void insert(const std::string_view& aText, uint8_t aIndex);
		void erase(uint8_t aIndex);
		void erase(uint8_t aBeginIndex, uint8_t aCount);
		void replace(char aToBeReplaced, char aReplacement);
		void replace(const std::string_view& aToBeReplaced, const std::string_view& aReplacement);
		
		char* begin() { return data(); }
		char* end() { return data() + size(); }
		
	private:
		char myData[CAPACITY + 1u]{};
		uint8_t mySize{};
	};

	using String32 = FixedString<30>;
	using String64 = FixedString<62>;

	template<uint8_t CAPACITY>
	FixedString<CAPACITY>::FixedString(const char* aText)
	{
		for (uint8_t i = 0; i < CAPACITY; ++i)
		{
			if (aText[i] == '\0')
			{
				mySize = i;
				return;
			}
			myData[i] = aText[i];
		}

		GOOSE_ASSERT(aText[CAPACITY] == '\0', "const char* used to construct is to large!");

		mySize = CAPACITY;
	}

	template<uint8_t CAPACITY>
	FixedString<CAPACITY>::FixedString(const std::string& aText)
	{
		const char* cString = aText.c_str();
		for (uint8_t i = 0; i < CAPACITY; ++i)
		{
			if (cString[i] == '\0')
			{
				mySize = i;
				return;
			}
			myData[i] = cString[i];
		}

		if (aText[CAPACITY] != '\0')
		{
			LOG_WARNING("String %s is too large for a fixed string with capacity %d", aText.c_str(), CAPACITY);
		}

		//GOOSE_ASSERT(aText[CAPACITY] == '\0', "string used to construct is to large!");

		mySize = CAPACITY;
	}

	template<uint8_t CAPACITY>
	FixedString<CAPACITY>& FixedString<CAPACITY>::operator=(const char* aText)
	{
		mySize = 0;
		for (uint8_t i = 0; i < CAPACITY; ++i)
		{
			if (aText[i] == '\0')
			{
				break;
			}
			myData[i] = aText[i];
			mySize++;
		}

		GOOSE_ASSERT(mySize != CAPACITY || aText[CAPACITY] == '\0', "const char* used to assign is to large!");

		if (mySize < CAPACITY)
		{
			std::fill_n(myData + mySize, CAPACITY - mySize, '\0');
		}

		return *this;
	}

	template<uint8_t CAPACITY>
	FixedString<CAPACITY>& FixedString<CAPACITY>::operator=(const std::string& aText)
	{
		GOOSE_ASSERT(aText.size() <= CAPACITY, "string used to assign is to large!");

		safe_assign(aText);
		return *this;
	}

	template<uint8_t CAPACITY>
	void FixedString<CAPACITY>::safe_assign(const char* aText)
	{
		mySize = 0;
		for (uint8_t i = 0; i < CAPACITY; ++i)
		{
			if (aText[i] == '\0')
			{
				break;
			}
			myData[i] = aText[i];
			mySize++;
		}

		if (mySize < CAPACITY)
		{
			std::fill_n(myData + mySize, CAPACITY - mySize, '\0');
		}
	}

	template<uint8_t CAPACITY>
	void FixedString<CAPACITY>::safe_assign(const std::string& aText)
	{
		mySize              = 0;
		const char* cString = aText.c_str();
		for (uint8_t i = 0; i < CAPACITY; ++i)
		{
			if (cString[i] == '\0')
			{
				break;
			}
			myData[i] = cString[i];
			mySize++;
		}

		if (mySize < CAPACITY)
		{
			std::fill_n(myData + mySize, CAPACITY - mySize, '\0');
		}
	}

	template<uint8_t CAPACITY>
	char* FixedString<CAPACITY>::data()
	{
		return myData;
	}

	template<uint8_t CAPACITY>
	const char* FixedString<CAPACITY>::c_str() const
	{
		return myData;
	}

	template<uint8_t CAPACITY>
	char& FixedString<CAPACITY>::operator[](uint8_t aIndex)
	{
		GOOSE_ASSERT(aIndex < mySize, "Index out of bounds");
		return myData[aIndex];
	}

	template<uint8_t CAPACITY>
	const char& FixedString<CAPACITY>::operator[](uint8_t aIndex) const
	{
		GOOSE_ASSERT(aIndex < mySize, "Index out of bounds");
		return myData[aIndex];
	}

	template<uint8_t CAPACITY>
	uint8_t FixedString<CAPACITY>::size() const
	{
		return mySize;
	}

	template<uint8_t CAPACITY>
	uint8_t FixedString<CAPACITY>::capacity() const
	{
		return CAPACITY;
	}

	template<uint8_t CAPACITY>
	bool FixedString<CAPACITY>::empty() const
	{
		return mySize == 0u;
	}

	template<uint8_t CAPACITY>
	bool FixedString<CAPACITY>::contains(char aCharacter) const
	{
		for (uint8_t i = 0; i < mySize; ++i)
		{
			if (myData[i] == aCharacter)
			{
				return true;
			}
		}

		return false;
	}

	template<uint8_t CAPACITY>
	bool FixedString<CAPACITY>::contains(const std::string_view& aText) const
	{
		const uint8_t otherSize = static_cast<uint8_t>(aText.size());
		for (uint8_t selfIndex = 0; (selfIndex + otherSize) <= mySize; ++selfIndex)
		{
			bool success = true;
			for (uint32_t otherIndex = 0; otherIndex < otherSize; ++otherIndex)
			{
				if (aText[otherIndex] != myData[selfIndex + otherIndex])
				{
					success = false;
					break;
				}
			}

			if (success)
			{
				return true;
			}
		}

		return false;
	}

	template<uint8_t CAPACITY>
	bool FixedString<CAPACITY>::has_only_letters_or_numbers_or_chosen(const std::string_view& aChosenCharacters) const
	{
		for (uint8_t i = 0; i < mySize; ++i)
		{
			if (myData[i] >= 'a' && myData[i] <= 'z')
			{
				continue;
			}
			if (myData[i] >= 'A' && myData[i] <= 'Z')
			{
				continue;
			}
			
			if (myData[i] >= '0' && myData[i] <= '9')
			{
				continue;
			}
			
			bool foundMatch = false;
			for (uint64_t j = 0; j < aChosenCharacters.size(); ++j)
			{
				if (myData[i] == aChosenCharacters[j])
				{
					foundMatch = true;
					break;
				}
			}
			
			if (foundMatch == false)
			{
				return false;
			}
		}
		
		return true;
	}

	template<uint8_t CAPACITY>
	bool FixedString<CAPACITY>::operator==(const FixedString& aOther) const
	{
		if (mySize != aOther.mySize)
		{
			return false;
		}

		for (uint8_t i = 0; i < mySize; ++i)
		{
			if (myData[i] != aOther.myData[i])
			{
				return false;
			}
		}

		return true;
	}

	template<uint8_t CAPACITY>
	bool FixedString<CAPACITY>::operator==(const char* aText) const
	{
		for (uint8_t i = 0; i < mySize; ++i)
		{
			if (myData[i] != aText[i])
			{
				return false;
			}
		}

		return aText[mySize] == '\0';
	}

	template<uint8_t CAPACITY>
	bool FixedString<CAPACITY>::operator==(const std::string_view& aText) const
	{
		if (mySize != aText.size())
		{
			return false;
		}

		for (uint8_t i = 0; i < mySize; ++i)
		{
			if (myData[i] != aText[i])
			{
				return false;
			}
		}

		return true;
	}

	template<uint8_t CAPACITY>
	void FixedString<CAPACITY>::resize(const uint8_t aNewSize)
	{
		GOOSE_ASSERT(mySize <= CAPACITY, "Cannot resize over the capacity limit!");
		mySize = aNewSize;
		if (mySize < CAPACITY)
		{
			std::fill_n(myData + mySize, CAPACITY - mySize, '\0');
		}
	}

	template<uint8_t CAPACITY>
	void FixedString<CAPACITY>::clear()
	{
		mySize = 0u;
		std::fill_n(myData, CAPACITY, '\0');
	}

	template<uint8_t CAPACITY>
	FixedString<CAPACITY>& FixedString<CAPACITY>::operator+=(const FixedString& aOther)
	{
		GOOSE_ASSERT(aOther.mySize + mySize <= CAPACITY, "Capacity reached when adding with FixedString");

		for (uint8_t i = 0; i < aOther.mySize; ++i)
		{
			myData[mySize] = aOther.myData[i];
			mySize++;
		}

		return *this;
	}

	template<uint8_t CAPACITY>
	FixedString<CAPACITY>& FixedString<CAPACITY>::operator+=(const char* aText)
	{
		GOOSE_ASSERT(mySize <= CAPACITY, "Capacity reached when adding with const char*");

		for (uint8_t i = 0; i < CAPACITY; ++i)
		{
			if (aText[i] == '\0')
			{
				break;
			}
			myData[mySize] = aText[i];
			mySize++;
		}

		return *this;
	}

	template<uint8_t CAPACITY>
	FixedString<CAPACITY>& FixedString<CAPACITY>::operator+=(const std::string& aText)
	{
		GOOSE_ASSERT(aText.size() + mySize <= CAPACITY, "Capacity reached when adding with string");

		for (uint8_t i = 0; i < CAPACITY; ++i)
		{
			if (i == aText.size() || aText[i] == '\0')
			{
				break;
			}
			myData[mySize] = aText[i];
			mySize++;
		}

		return *this;
	}

	template<uint8_t CAPACITY>
	FixedString<CAPACITY>& FixedString<CAPACITY>::operator+=(const std::string_view& aText)
	{
		GOOSE_ASSERT(aText.size() + mySize <= CAPACITY, "Capacity reached when adding with string_view");

		for (uint8_t i = 0; i < CAPACITY; ++i)
		{
			if (i == aText.size() || aText[i] == '\0')
			{
				break;
			}
			myData[mySize] = aText[i];
			mySize++;
		}

		return *this;
	}

	template<uint8_t CAPACITY>
	void FixedString<CAPACITY>::insert(const std::string_view& aText, const uint8_t aIndex)
	{
		const uint8_t insertionSize = aText.size();
		GOOSE_ASSERT(insertionSize + mySize <= CAPACITY, "Capacity reached when inserting");

		char dataCopy[CAPACITY]{};

		uint8_t movedSize = mySize - aIndex;
		std::copy_n(myData + aIndex, movedSize, dataCopy);
		std::copy_n(aText.data(), insertionSize, myData);
		std::copy_n(dataCopy, movedSize, myData + aIndex + insertionSize);

		mySize += insertionSize;
	}

	template<uint8_t CAPACITY>
	void FixedString<CAPACITY>::erase(const uint8_t aIndex)
	{
		erase(aIndex, 1);
	}

	template<uint8_t CAPACITY>
	void FixedString<CAPACITY>::erase(uint8_t aBeginIndex, uint8_t aCount)
	{
		GOOSE_ASSERT(aBeginIndex + aCount <= mySize, "Tries to erase outside of bounds");
		std::copy_n(myData + aBeginIndex + aCount, mySize - aCount, myData + aBeginIndex);
		std::fill_n(myData + aBeginIndex + aCount, aCount, '\0');
		mySize -= aCount;
	}

	template<uint8_t CAPACITY>
	void FixedString<CAPACITY>::replace(char aToBeReplaced, char aReplacement)
	{
		for (uint8_t i = 0; i < mySize; ++i)
		{
			if (myData[i] == aToBeReplaced)
			{
				myData[i] = aReplacement;
			}
		}
	}

	template<uint8_t CAPACITY>
	void FixedString<CAPACITY>::replace(const std::string_view& aToBeReplaced, const std::string_view& aReplacement)
	{
		const uint8_t otherSize   = aToBeReplaced.size();
		const uint8_t replaceSize = aReplacement.size();
		for (uint8_t selfIndex = 0; selfIndex + otherSize < mySize; ++selfIndex)
		{
			bool success = true;
			for (uint8_t otherIndex = 0; otherIndex < otherSize; ++otherIndex)
			{
				if (myData[selfIndex + otherIndex] != aToBeReplaced[otherIndex])
				{
					success = false;
					break;
				}
			}

			if (success)
			{
				if (replaceSize == otherSize)
				{
					std::copy_n(aReplacement.data(), otherSize, myData + selfIndex);
					selfIndex += otherSize - 1u;
				}
				else if (replaceSize > otherSize)
				{
					const uint8_t sizeIncrease = replaceSize - otherSize;
					GOOSE_ASSERT(mySize + (sizeIncrease), "Capacity reached when replacing");
					const uint8_t movedSize = mySize - (selfIndex + otherSize);
					char dataCopy[CAPACITY]{};
					std::copy_n(myData + selfIndex + otherSize, movedSize, dataCopy);
					std::copy_n(aReplacement.data(), replaceSize, myData + selfIndex);
					std::copy_n(dataCopy, movedSize, myData + selfIndex + replaceSize);

					selfIndex += replaceSize - 1u;
					mySize += sizeIncrease;
				}
				else
				{
					const uint8_t sizeDecrease = otherSize - replaceSize;
					std::copy_n(aReplacement.data(), replaceSize, myData + selfIndex);
					for (uint8_t i = selfIndex + replaceSize; i < mySize - sizeDecrease; ++i)
					{
						myData[i] = myData[i + sizeDecrease];
					}

					std::fill_n(myData + mySize - sizeDecrease, sizeDecrease, '\0');

					selfIndex += replaceSize - 1u;
					mySize -= sizeDecrease;
				}
			}
		}
	}

	template<uint8_t CAPACITY>
	FixedString<CAPACITY> operator+(const FixedString<CAPACITY>& aLeft, const FixedString<CAPACITY>& aRight)
	{
		FixedString<CAPACITY> newString(aLeft);
		newString += aRight;
		return newString;
	}
	template<uint8_t CAPACITY>
	FixedString<CAPACITY> operator+(const std::string_view& aLeft, const FixedString<CAPACITY>& aRight)
	{
		FixedString<CAPACITY> newString(aRight);
		newString += aLeft;
		return newString;
	}
	template<uint8_t CAPACITY>
	FixedString<CAPACITY> operator+(const FixedString<CAPACITY>& aLeft, const std::string_view& aRight)
	{
		FixedString<CAPACITY> newString(aLeft);
		newString += aRight;
		return newString;
	}
}

template<>
struct std::hash<Goose::String32>
{
	std::size_t operator()(const Goose::String32& aString) const noexcept
	{
		const char* c_string = aString.c_str();

		constexpr uint64_t A     = 54059; /* a prime */
		constexpr uint64_t B     = 76963; /* another prime */
		constexpr uint64_t FIRST = 37;    /* also prime */

		uint64_t h = FIRST;
		while (*c_string)
		{
			h = (h * A) ^ (static_cast<uint64_t>(c_string[0]) * B);
			c_string++;
		}
		return h;
	}
};

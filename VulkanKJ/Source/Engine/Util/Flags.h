#pragma once
#include <type_traits>

#define BIT(x) (1 << (x))

template<typename T> requires std::is_enum_v<T>
class Flags
{
public:
    using MaskType = std::underlying_type_t<T>;

public:
    constexpr Flags() = default;
    constexpr Flags(T aFlags);
    constexpr Flags(const Flags& aOther);
    
    constexpr Flags(Flags&& aOther) noexcept = default;
    constexpr Flags& operator=(const Flags& aOther) = default;
    constexpr Flags& operator=(Flags&& aOther) noexcept = default;

public:
    constexpr Flags(MaskType aMask)
        : myMask(aMask) {}

public:
    constexpr Flags& operator|=(T aFlags);
    constexpr Flags& operator|=(const Flags& aFlags);
    constexpr Flags& operator&=(T aFlags);
    constexpr Flags& operator&=(const Flags& aFlags);
    constexpr Flags& operator^=(T aFlags);
    constexpr Flags& operator^=(const Flags& aFlags);

    constexpr Flags operator|(T aFlags);
    constexpr Flags operator|(const Flags& aFlags);
    constexpr Flags operator&(T aFlags) const;
    constexpr Flags operator&(const Flags& aFlags) const;
    constexpr Flags operator^(T aFlags);
    constexpr Flags operator^(const Flags& aFlags);
    constexpr Flags operator~();

    constexpr operator MaskType() const { return myMask; }
    constexpr operator bool() const { return myMask; }

    explicit constexpr operator T() const { return static_cast<T>(myMask); }

private:
    MaskType myMask{};
};

template<typename T>
struct FlagTraits
{
    static constexpr bool isBitMask = false;
};

#define DEFINE_FLAG_TRAITS(T)                   \
    template<>                                  \
    struct FlagTraits<T>                        \
    {                                           \
        static constexpr bool isBitMask = true; \
        using Type = std::underlying_type_t<T>; \
    }

#pragma region Member Operators

template <typename T> requires std::is_enum_v<T>
constexpr Flags<T>::Flags(T aFlags)
    : myMask(static_cast<MaskType>(aFlags))
{}

template <typename T> requires std::is_enum_v<T>
constexpr Flags<T>::Flags(const Flags& aOther)
{
    myMask = aOther.myMask;       
}

template <typename T> requires std::is_enum_v<T>
constexpr Flags<T>& Flags<T>::operator|=(T aFlags)
{
    myMask |= static_cast<MaskType>(aFlags);
    return *this;
}

template <typename T> requires std::is_enum_v<T>
constexpr Flags<T>& Flags<T>::operator|=(const Flags& aFlags)
{
    myMask |= aFlags.myMask;
    return *this;
}

template <typename T> requires std::is_enum_v<T>
constexpr Flags<T>& Flags<T>::operator&=(T aFlags)
{
    myMask &= static_cast<MaskType>(aFlags);
    return *this;
}

template <typename T> requires std::is_enum_v<T>
constexpr Flags<T>& Flags<T>::operator&=(const Flags& aFlags)
{
    myMask &= aFlags.myMask;
    return *this;
}

template <typename T> requires std::is_enum_v<T>
constexpr Flags<T>& Flags<T>::operator^=(T aFlags)
{
    myMask ^= static_cast<MaskType>(aFlags);
    return *this;
}

template <typename T> requires std::is_enum_v<T>
constexpr Flags<T>& Flags<T>::operator^=(const Flags& aFlags)
{
    myMask ^= aFlags.myMask;
    return *this;   
}

template <typename T> requires std::is_enum_v<T>
constexpr Flags<T> Flags<T>::operator|(T aFlags)
{
    return Flags<T>(myMask | static_cast<MaskType>(aFlags));   
}

template <typename T> requires std::is_enum_v<T>
constexpr Flags<T> Flags<T>::operator|(const Flags& aFlags)
{
    return Flags(myMask | aFlags.myMask);  
}

template <typename T> requires std::is_enum_v<T>
constexpr Flags<T> Flags<T>::operator&(T aFlags) const
{
    return Flags(myMask & static_cast<MaskType>(aFlags));  
}

template <typename T> requires std::is_enum_v<T>
constexpr Flags<T> Flags<T>::operator&(const Flags& aFlags) const
{
    return Flags<T>(myMask & aFlags.myMask); 
}

template <typename T> requires std::is_enum_v<T>
constexpr Flags<T> Flags<T>::operator^(T aFlags)
{
    return Flags<T>(myMask ^ static_cast<MaskType>(aFlags));
}

template <typename T> requires std::is_enum_v<T>
constexpr Flags<T> Flags<T>::operator^(const Flags& aFlags)
{
    return Flags<T>(myMask ^ aFlags.myMask);
}

template <typename T> requires std::is_enum_v<T>
constexpr Flags<T> Flags<T>::operator~()
{
    return Flags<T>(~myMask);
}
#pragma endregion

template <typename T, typename std::enable_if<FlagTraits<T>::isBitMask, bool>::type = true>
constexpr Flags<T> operator|(T aLhs, T aRhs)
{
    return Flags<T>(aLhs) | aRhs;
}

template <typename T, typename std::enable_if<FlagTraits<T>::isBitMask, bool>::type = true>
constexpr Flags<T> operator&(T aLhs, T aRhs)
{
    return Flags<T>(aLhs) & aRhs;
}

template <typename T, typename std::enable_if<FlagTraits<T>::isBitMask, bool>::type = true>
constexpr Flags<T> operator^(T aLhs, T aRhs)
{
    return Flags<T>(aLhs) ^ aRhs;
}

template <typename T, typename std::enable_if<FlagTraits<T>::isBitMask, bool>::type = true>
constexpr Flags<T> operator~(T aFlag)
{
    typename FlagTraits<T>::Type mask = static_cast<typename FlagTraits<T>::Type>(aFlag);
    return Flags<T>(~mask);
}





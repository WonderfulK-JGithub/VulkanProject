#pragma once
#include "Clipboard.h"

namespace Wonder::Clipboard
{
    template<typename T>
    class VariableCopyItem : public CopyItemBase
    {
    public:
        explicit VariableCopyItem(const T& aValue) : myValue(aValue) {}
        
        std::string GetName() const override;
        uint64_t GetTypeId() const override;
        
        T& AccessValue();
        const char* GetData() override;

    private:
        T myValue;
    };

    template<typename T>
    std::string VariableCopyItem<T>::GetName() const
    {
        return std::string(typeid(VariableCopyItem<T>).name());
    }

    template<typename T>
    uint64_t VariableCopyItem<T>::GetTypeId() const
    {
        return typeid(VariableCopyItem<T>).hash_code();
    }

    template<typename T>
    T& VariableCopyItem<T>::AccessValue()
    {
        return myValue;
    }

    template<typename T>
    const char* VariableCopyItem<T>::GetData()
    {
        return reinterpret_cast<char*>(&myValue);
    }
}
